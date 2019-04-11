// This is copyrighted software. More information is at the end of this file.
#include "Aulib/DecoderMusepack.h"

#include "Buffer.h"
#include "aulib_config.h"
#include "aulib_debug.h"
#include <SDL_rwops.h>
#include <cstring>
#include <mpc/mpcdec.h>
#include <mpc/reader.h>

#ifdef MPC_FIXED_POINT
#    error Fixed point decoder versions of libmpcdec are not supported!
#endif

namespace chrono = std::chrono;

extern "C" {

static mpc_int32_t mpcReadCb(mpc_reader* reader, void* ptr, mpc_int32_t size)
{
    return SDL_RWread(static_cast<SDL_RWops*>(reader->data), ptr, 1, size);
}

static mpc_bool_t mpcSeekCb(mpc_reader* reader, mpc_int32_t offset)
{
    return SDL_RWseek(static_cast<SDL_RWops*>(reader->data), offset, RW_SEEK_SET) >= 0;
}

static mpc_int32_t mpcTellCb(mpc_reader* reader)
{
    return SDL_RWtell(static_cast<SDL_RWops*>(reader->data));
}

static mpc_int32_t mpcGetSizeCb(mpc_reader* reader)
{
    return SDL_RWsize(static_cast<SDL_RWops*>(reader->data));
}

static mpc_bool_t mpcCanseekCb(mpc_reader* reader)
{
    return SDL_RWseek(static_cast<SDL_RWops*>(reader->data), 0, RW_SEEK_CUR) > -1;
}

} // extern "C"

namespace Aulib {

struct DecoderMusepack_priv final
{
    mpc_reader reader{mpcReadCb, mpcSeekCb, mpcTellCb, mpcGetSizeCb, mpcCanseekCb, nullptr};
    std::unique_ptr<mpc_demux, decltype(&mpc_demux_exit)> demuxer{nullptr, &mpc_demux_exit};
    Buffer<float> curFrameBuffer{MPC_DECODER_BUFFER_LENGTH};
    mpc_frame_info curFrame{0, 0, curFrameBuffer.get(), 0};
    mpc_streaminfo strmInfo{};
    int frameBufPos = 0;
    bool eof = false;
    chrono::microseconds duration{};
};

} // namespace Aulib

Aulib::DecoderMusepack::DecoderMusepack()
    : d(std::make_unique<DecoderMusepack_priv>())
{}

Aulib::DecoderMusepack::~DecoderMusepack() = default;

bool Aulib::DecoderMusepack::open(SDL_RWops* rwops)
{
    if (isOpen()) {
        return true;
    }
    d->reader.data = rwops;
    d->demuxer.reset(mpc_demux_init(&d->reader));
    if (not d->demuxer) {
        d->reader.data = nullptr;
        return false;
    }
    mpc_demux_get_info(d->demuxer.get(), &d->strmInfo);
    setIsOpen(true);
    return true;
}

int Aulib::DecoderMusepack::getChannels() const
{
    return d->demuxer ? d->strmInfo.channels : 0;
}

int Aulib::DecoderMusepack::getRate() const
{
    return d->demuxer ? d->strmInfo.sample_freq : 0;
}

int Aulib::DecoderMusepack::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    int totalSamples = 0;
    int wantedSamples = len;

    if (d->eof) {
        return 0;
    }

    // If we have any left-over samples from the previous frame, copy them out.
    if (d->curFrame.samples > 0) {
        int copyLen = std::min(static_cast<int>(d->curFrame.samples * d->strmInfo.channels), len);
        std::memcpy(buf, d->curFrame.buffer + d->frameBufPos,
                    static_cast<size_t>(copyLen) * sizeof(*buf));
        d->curFrame.samples -= copyLen / d->strmInfo.channels;
        if (d->curFrame.samples != 0) {
            // There's still more samples left.
            d->frameBufPos += copyLen;
            return copyLen;
        }
        buf += copyLen;
        len -= copyLen;
        totalSamples += copyLen;
        d->frameBufPos = 0;
    }

    // Decode one frame at a time, until we have enough samples.
    while (totalSamples < wantedSamples) {
        if (mpc_demux_decode(d->demuxer.get(), &d->curFrame) != MPC_STATUS_OK) {
            AM_warnLn("DecoderMusepack decoding error.");
            return 0;
        }
        int copyLen = std::min(static_cast<int>(d->curFrame.samples * d->strmInfo.channels), len);
        std::memcpy(buf, d->curFrame.buffer, static_cast<size_t>(copyLen) * sizeof(*buf));
        d->frameBufPos = copyLen;
        d->curFrame.samples -= copyLen / d->strmInfo.channels;
        totalSamples += copyLen;
        len -= copyLen;
        buf += copyLen;
        if (d->curFrame.bits == -1) {
            d->eof = true;
            return totalSamples;
        }
    }
    return totalSamples;
}

bool Aulib::DecoderMusepack::rewind()
{
    return seekToTime(chrono::microseconds::zero());
}

chrono::microseconds Aulib::DecoderMusepack::duration() const
{
    if (d->demuxer == nullptr) {
        return chrono::microseconds::zero();
    }
    using namespace std::chrono;
    using std::chrono::duration;
    return duration_cast<microseconds>(duration<double>(mpc_streaminfo_get_length(&d->strmInfo)));
}

bool Aulib::DecoderMusepack::seekToTime(chrono::microseconds pos)
{
    using namespace std::chrono;
    using std::chrono::duration;
    if (d->demuxer == nullptr
        or mpc_demux_seek_second(d->demuxer.get(), duration<double>(pos).count())
               != MPC_STATUS_OK) {
        return false;
    }
    d->eof = false;
    return true;
}

/*

Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019 Nikos Chantziaras.

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
