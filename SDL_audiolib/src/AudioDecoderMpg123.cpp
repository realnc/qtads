// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderMpg123.h"

#include "aulib_debug.h"
#include <SDL_audio.h>
#include <SDL_rwops.h>
#include <mpg123.h>

namespace chrono = std::chrono;

static bool initialized = false;

static int initLibMpg()
{
    if (mpg123_init() != MPG123_OK) {
        return -1;
    }
    initialized = true;
    return 0;
}

static int initMpgFormats(mpg123_handle* handle)
{
    const long* list;
    size_t len;
    mpg123_rates(&list, &len);
    mpg123_format_none(handle);
    for (size_t i = 0; i < len; ++i) {
        if (mpg123_format(handle, list[i], MPG123_STEREO | MPG123_MONO, MPG123_ENC_FLOAT_32)
            != MPG123_OK) {
            return -1;
        }
    }
    return 0;
}

extern "C" {

static ssize_t mpgReadCallback(void* rwops, void* buf, size_t len)
{
    return static_cast<ssize_t>(SDL_RWread(static_cast<SDL_RWops*>(rwops), buf, 1, len));
}

static off_t mpgSeekCallback(void* rwops, off_t pos, int whence)
{
    switch (whence) {
    case SEEK_SET:
        whence = RW_SEEK_SET;
        break;
    case SEEK_CUR:
        whence = RW_SEEK_CUR;
        break;
    default:
        whence = RW_SEEK_END;
    }
    return SDL_RWseek(static_cast<SDL_RWops*>(rwops), pos, whence);
}

} // extern "C"

namespace Aulib {

struct AudioDecoderMpg123_priv final
{
    AudioDecoderMpg123_priv();

    std::unique_ptr<mpg123_handle, decltype(&mpg123_delete)> fMpgHandle{nullptr, &mpg123_delete};
    int fChannels = 0;
    int fRate = 0;
    bool fEOF = false;
    chrono::microseconds fDuration{};
};

} // namespace Aulib

Aulib::AudioDecoderMpg123_priv::AudioDecoderMpg123_priv()
{
    if (not initialized) {
        initLibMpg();
    }
}

Aulib::AudioDecoderMpg123::AudioDecoderMpg123()
    : d(std::make_unique<AudioDecoderMpg123_priv>())
{}

Aulib::AudioDecoderMpg123::~AudioDecoderMpg123() = default;

bool Aulib::AudioDecoderMpg123::open(SDL_RWops* rwops)
{
    if (isOpen()) {
        return true;
    }
    if (not initialized) {
        return false;
    }
    d->fMpgHandle.reset(mpg123_new(nullptr, nullptr));
    if (not d->fMpgHandle) {
        return false;
    }
    mpg123_param(d->fMpgHandle.get(), MPG123_FLAGS, MPG123_QUIET, 0);
    if (initMpgFormats(d->fMpgHandle.get()) < 0) {
        return false;
    }
    mpg123_replace_reader_handle(d->fMpgHandle.get(), mpgReadCallback, mpgSeekCallback, nullptr);
    mpg123_open_handle(d->fMpgHandle.get(), rwops);
    long rate;
    int channels, encoding;
    if (mpg123_getformat(d->fMpgHandle.get(), &rate, &channels, &encoding) != 0) {
        return false;
    }
    d->fChannels = channels;
    d->fRate = rate;
    off_t len = mpg123_length(d->fMpgHandle.get());
    if (len == MPG123_ERR) {
        d->fDuration = chrono::microseconds::zero();
    } else {
        using namespace std::chrono;
        using std::chrono::duration;
        d->fDuration =
            duration_cast<microseconds>(duration<double>(static_cast<double>(len) / rate));
    }
    setIsOpen(true);
    return true;
}

int Aulib::AudioDecoderMpg123::getChannels() const
{
    return d->fChannels;
}

int Aulib::AudioDecoderMpg123::getRate() const
{
    return d->fRate;
}

int Aulib::AudioDecoderMpg123::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    if (d->fEOF) {
        return 0;
    }

    int bytesWanted = len * static_cast<int>(sizeof(*buf));
    size_t decBytes = 0;
    int totalBytes = 0;

    while (totalBytes < bytesWanted and not callAgain) {
        int ret = mpg123_read(d->fMpgHandle.get(), reinterpret_cast<unsigned char*>(buf),
                              static_cast<size_t>(bytesWanted), &decBytes);
        totalBytes += decBytes;
        if (ret == MPG123_NEW_FORMAT) {
            long rate;
            int channels, encoding;
            mpg123_getformat(d->fMpgHandle.get(), &rate, &channels, &encoding);
            d->fChannels = channels;
            d->fRate = rate;
            callAgain = true;
        } else if (ret == MPG123_DONE) {
            d->fEOF = true;
            break;
        }
    }
    return totalBytes / static_cast<int>(sizeof(*buf));
}

bool Aulib::AudioDecoderMpg123::rewind()
{
    if (mpg123_seek(d->fMpgHandle.get(), 0, SEEK_SET) < 0) {
        return false;
    }
    d->fEOF = false;
    return true;
}

chrono::microseconds Aulib::AudioDecoderMpg123::duration() const
{
    return d->fDuration;
}

bool Aulib::AudioDecoderMpg123::seekToTime(chrono::microseconds pos)
{
    using std::chrono::duration;
    off_t targetFrame = mpg123_timeframe(d->fMpgHandle.get(), duration<double>(pos).count());
    if (targetFrame < 0 or mpg123_seek_frame(d->fMpgHandle.get(), targetFrame, SEEK_SET) < 0) {
        return false;
    }
    d->fEOF = false;
    return true;
}

/*

Copyright (C) 2014, 2015, 2016, 2017, 2018 Nikos Chantziaras.

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
