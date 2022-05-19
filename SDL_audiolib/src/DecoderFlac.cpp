// This is copyrighted software. More information is at the end of this file.
#include "Aulib/DecoderFlac.h"
#include "aulib_log.h"
#include "missing.h"
#include <FLAC/stream_decoder.h>
#include <SDL_rwops.h>
#include <array>

namespace chrono = std::chrono;

namespace Aulib {

struct DecoderFlac_priv final
{
    using FlacHandle = std::unique_ptr<FLAC__StreamDecoder, decltype(&FLAC__stream_decoder_delete)>;

    explicit DecoderFlac_priv(const DecoderFlac::FileFormat file_type)
        : fFileFormat(file_type)
    { }

    FlacHandle fFlacHandle{nullptr, FLAC__stream_decoder_delete};
    SDL_RWops* fRwops = nullptr;
    std::array<const FLAC__int32*, 2> fBuffers{};
    const FLAC__Frame* fFlacFrame = nullptr;
    float fSampleConvFactor = 0;
    int fRemainingFrames = 0;
    chrono::microseconds fDuration{};
    int fSampleRate = 0;
    int fChannels = 0;
    bool fEOF = false;
    DecoderFlac::FileFormat fFileFormat;
    const char* fLastError = nullptr;
    bool fHasLostSync = false;
};

} // namespace Aulib

extern "C" {

auto flacReadCb(
    const FLAC__StreamDecoder*, FLAC__byte* const buffer, size_t* const bytes, void* const d_ptr)
    -> FLAC__StreamDecoderReadStatus
{
    if (*bytes == 0) {
        aulib::log::warnLn("DecoderFlac: libFLAC requested read of zero bytes.");
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    const auto want_bytes = *bytes;
    auto* const d = static_cast<Aulib::DecoderFlac_priv*>(d_ptr);

    *bytes = SDL_RWread(d->fRwops, buffer, sizeof(FLAC__byte), want_bytes);
    if (*bytes == 0) {
        d->fEOF = true;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

auto flacSeekCb(const FLAC__StreamDecoder*, const FLAC__uint64 pos, void* const d_ptr)
    -> FLAC__StreamDecoderSeekStatus
{
    if (SDL_RWseek(static_cast<Aulib::DecoderFlac_priv*>(d_ptr)->fRwops, pos, RW_SEEK_SET) < 0) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
    }
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

auto flacTellCb(const FLAC__StreamDecoder*, FLAC__uint64* const out_pos, void* const d_ptr)
    -> FLAC__StreamDecoderTellStatus
{
    const auto pos = SDL_RWtell(static_cast<Aulib::DecoderFlac_priv*>(d_ptr)->fRwops);
    if (pos < 0) {
        aulib::log::debugLn("DecoderFlac: rwops does not support seeking, or seeking failed.");
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *out_pos = pos;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

auto flacLengthCb(const FLAC__StreamDecoder*, FLAC__uint64* const out_size, void* const d_ptr)
    -> FLAC__StreamDecoderLengthStatus
{
    const auto size = SDL_RWsize(static_cast<Aulib::DecoderFlac_priv*>(d_ptr)->fRwops);
    if (size == -1) {
        aulib::log::debugLn("DecoderFlac: rwops length is unknown.");
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }
    if (size < -1) {
        aulib::log::debugLn(
            "DecoderFlac: Got error trying to get rwops length: {}", SDL_GetError());
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }
    *out_size = size;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

auto flacEofCb(const FLAC__StreamDecoder*, void* const d_ptr) -> FLAC__bool
{
    return static_cast<Aulib::DecoderFlac_priv*>(d_ptr)->fEOF;
}

void flacMetadataCb(
    const FLAC__StreamDecoder*, const FLAC__StreamMetadata* const metadata, void* const d_ptr)
{
    if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO) {
        return;
    }

    auto* const d = static_cast<Aulib::DecoderFlac_priv*>(d_ptr);
    const auto& info = metadata->data.stream_info;
    d->fSampleConvFactor = 1u << (info.bits_per_sample - 1);
    d->fSampleRate = info.sample_rate;
    d->fChannels = info.channels;
    d->fDuration = chrono::duration_cast<chrono::microseconds>(
        chrono::duration<double>(static_cast<float>(info.total_samples) / info.sample_rate));

    aulib::log::debugLn(
        "DecoderFlac: Audio attributes: {}-bit {}Hz {} channels {} samples ({}us).",
        info.bits_per_sample, d->fSampleRate, d->fChannels, info.total_samples,
        d->fDuration.count());
}

auto flacWriteCb(
    const FLAC__StreamDecoder*, const FLAC__Frame* const frame,
    const FLAC__int32* const* const buffer, void* const d_ptr) -> FLAC__StreamDecoderWriteStatus
{
    auto* const d = static_cast<Aulib::DecoderFlac_priv*>(d_ptr);

    d->fFlacFrame = frame;
    d->fRemainingFrames = frame->header.blocksize;
    d->fBuffers[0] = buffer[0];
    if (d->fChannels > 1) {
        d->fBuffers[1] = buffer[1];
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flacErrorCb(
    const FLAC__StreamDecoder*, const FLAC__StreamDecoderErrorStatus status, void* const d_ptr)
{
    auto* const d = static_cast<Aulib::DecoderFlac_priv*>(d_ptr);

    d->fLastError = FLAC__StreamDecoderErrorStatusString[status];
    aulib::log::warnLn("DecoderFlac: libFLAC error: {}", d->fLastError);

    if (status == FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC) {
        d->fRemainingFrames = 0;
        d->fEOF = true;
        d->fHasLostSync = true;
    }
}

} // extern "C"

Aulib::DecoderFlac::DecoderFlac(const FileFormat file_type)
    : d(std::make_unique<DecoderFlac_priv>(file_type))
{ }

Aulib::DecoderFlac::~DecoderFlac() = default;

static auto rwopsHeaderMatches(SDL_RWops* const rwops, const std::string_view magic) -> bool
{
    std::array<char, 4> rwops_magic{};
    const auto initial_pos = SDL_RWtell(rwops);
    const auto read_res = SDL_RWread(rwops, rwops_magic.data(), 4, 1);

    SDL_RWseek(rwops, initial_pos, RW_SEEK_SET);
    return read_res == 1 and std::string_view(rwops_magic.data(), rwops_magic.size()) == magic;
}

static auto getFlacInitFunction(SDL_RWops* const rwops, const Aulib::DecoderFlac::FileFormat format)
    -> decltype(&FLAC__stream_decoder_init_stream)
{
    using FileFormat = Aulib::DecoderFlac::FileFormat;

    switch (format) {
    case FileFormat::Detect:
        if (rwopsHeaderMatches(rwops, "OggS")) {
            aulib::log::debugLn("DecoderFlac: detected Ogg container.");
            return FLAC__stream_decoder_init_ogg_stream;
        }
        if (rwopsHeaderMatches(rwops, "fLaC")) {
            aulib::log::debugLn("DecoderFlac: detected raw FLAC.");
            return FLAC__stream_decoder_init_stream;
        }
        break;
    case FileFormat::Flac:
        aulib::log::debugLn("DecoderFlac: assuming raw FLAC.");
        return FLAC__stream_decoder_init_stream;
    case FileFormat::Ogg:
        aulib::log::debugLn("DecoderFlac: assuming Ogg container.");
        return FLAC__stream_decoder_init_ogg_stream;
    }
    return nullptr;
}

auto Aulib::DecoderFlac::open(SDL_RWops* const rwops) -> bool
{
    if (isOpen()) {
        return true;
    }

    d->fLastError = nullptr;
    d->fFlacHandle.reset(FLAC__stream_decoder_new());
    if (not d->fFlacHandle) {
        if (d->fLastError) {
            SDL_SetError("DecoderFlac: Failed to allocate FLAC decoder: %s", d->fLastError);
        } else {
            SDL_SetError("DecoderFlac: Failed to allocate FLAC decoder.");
        }
        return false;
    }

    d->fRwops = rwops;

    const auto init_func = getFlacInitFunction(rwops, d->fFileFormat);
    if (not init_func) {
        SDL_SetError("DecoderFlac: Failed to detect FLAC file type.");
        return false;
    }
    if (init_func == FLAC__stream_decoder_init_ogg_stream and not FLAC_API_SUPPORTS_OGG_FLAC) {
        SDL_SetError(
            "DecoderFlac: Input is an Ogg container but libFLAC was built without Ogg support.");
        return false;
    }

    const auto init_res = init_func(
        d->fFlacHandle.get(), flacReadCb, flacSeekCb, flacTellCb, flacLengthCb, flacEofCb,
        flacWriteCb, flacMetadataCb, flacErrorCb, d.get());
    if (init_res != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        SDL_SetError(
            "DecoderFlac: Failed to initialize libFLAC decoder: %s",
            FLAC__StreamDecoderInitStatusString[init_res]);
        return false;
    }

    if (not FLAC__stream_decoder_process_until_end_of_metadata(d->fFlacHandle.get())) {
        const auto decoder_state = FLAC__stream_decoder_get_state(d->fFlacHandle.get());
        SDL_SetError(
            "DecoderFlac: libFLAC failed to read metadata: %s",
            FLAC__StreamDecoderStateString[decoder_state]);
        return false;
    }
    if (d->fHasLostSync) {
        SDL_SetError("DecoderFlac: libFLAC has lost sync.");
        return false;
    }

    setIsOpen(true);
    return true;
}

auto Aulib::DecoderFlac::getChannels() const -> int
{
    return d->fChannels;
}

auto Aulib::DecoderFlac::getRate() const -> int
{
    return d->fSampleRate;
}

auto Aulib::DecoderFlac::doDecoding(float buf[], const int len, bool& /*callAgain*/) -> int
{
    if ((d->fEOF and d->fRemainingFrames == 0) or not isOpen()) {
        return 0;
    }
    if (d->fHasLostSync) {
        aulib::log::warnLn("DecoderFlac: Refusing to decode since libFLAC has lost sync.");
        return 0;
    }

    const int channels = std::min(d->fChannels, 2);
    int total_samples = 0;

    while (total_samples < len) {
        if (d->fRemainingFrames == 0) {
            d->fLastError = nullptr;
            if (not FLAC__stream_decoder_process_single(d->fFlacHandle.get())) {
                const auto state = FLAC__stream_decoder_get_state(d->fFlacHandle.get());
                aulib::log::warnLn(
                    "DecoderFlac: libFLAC error while decoding: {}.",
                    FLAC__StreamDecoderStateString[state]);
                d->fRemainingFrames = 0;
                return 0;
            }
            if (d->fLastError) {
                aulib::log::warnLn("DecoderFlac: possible error while decoding: {}", d->fLastError);
            }
        }

        if (d->fHasLostSync) {
            aulib::log::warnLn("DecoderFlac: libFLAC has lost sync during decoding.");
            return 0;
        }
        if (d->fRemainingFrames == 0) {
            d->fEOF = true;
            return total_samples;
        }

        for (auto frame = d->fFlacFrame->header.blocksize - d->fRemainingFrames;
             total_samples < len and d->fRemainingFrames > 0; ++frame, --d->fRemainingFrames)
        {
            for (int chan = 0; chan < channels; ++chan, ++buf, ++total_samples) {
                *buf = d->fBuffers[chan][frame] / d->fSampleConvFactor;
            }
        }
    }
    return total_samples;
}

auto Aulib::DecoderFlac::rewind() -> bool
{
    return seekToTime({});
}

auto Aulib::DecoderFlac::duration() const -> chrono::microseconds
{
    return d->fDuration;
}

auto Aulib::DecoderFlac::seekToTime(const chrono::microseconds pos) -> bool
{
    if (not isOpen()) {
        SDL_SetError("DecoderFlac: Decoder has not been opened.");
        return false;
    }
    if (d->fHasLostSync) {
        SDL_SetError("DecoderFlac: libFLAC has lost sync.");
        return false;
    }

    const FLAC__uint64 sample_pos = chrono::duration<double>(pos).count() * d->fSampleRate;
    const auto seek_ok = FLAC__stream_decoder_seek_absolute(d->fFlacHandle.get(), sample_pos);
    auto state = FLAC__stream_decoder_get_state(d->fFlacHandle.get());

    // A seek that failed with SEEK_ERROR does not mean that the seek has failed nor that there was
    // an error. (Insert "wat" GIF here.) Instead, it means that flushing is required... What the
    // fuck, libFLAC.
    if (not seek_ok) {
        if (state != FLAC__STREAM_DECODER_SEEK_ERROR) {
            SDL_SetError(
                "DecoderFlac: Error while seeking FLAC stream: %s",
                FLAC__StreamDecoderStateString[state]);
            return false;
        }
        if (not FLAC__stream_decoder_flush(d->fFlacHandle.get())) {
            state = FLAC__stream_decoder_get_state(d->fFlacHandle.get());
            SDL_SetError(
                "DecoderFlac: Failed to flush decoder after seek: %s",
                FLAC__StreamDecoderStateString[state]);
            return false;
        }
    }

    d->fEOF = false;
    return true;
}

/*

Copyright (C) 2022 Nikos Chantziaras.

This file is part of SDL_audiolib.

SDL_audiolib is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

SDL_audiolib is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License
along with SDL_audiolib. If not, see <http://www.gnu.org/licenses/>.

*/
