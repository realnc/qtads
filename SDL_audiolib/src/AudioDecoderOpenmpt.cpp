// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderOpenmpt.h"

#include "Buffer.h"
#include "aulib.h"
#include <SDL_rwops.h>
#include <libopenmpt/libopenmpt.hpp>
#include <limits>
#include <memory>

namespace chrono = std::chrono;

namespace Aulib {

struct AudioDecoderOpenmpt_priv final
{
    std::unique_ptr<openmpt::module> fModule = nullptr;
    bool atEOF = false;
    chrono::microseconds fDuration{};
};

} // namespace Aulib

Aulib::AudioDecoderOpenmpt::AudioDecoderOpenmpt()
    : d(std::make_unique<AudioDecoderOpenmpt_priv>())
{}

Aulib::AudioDecoderOpenmpt::~AudioDecoderOpenmpt() = default;

bool Aulib::AudioDecoderOpenmpt::open(SDL_RWops* rwops)
{
    if (isOpen()) {
        return true;
    }
    // FIXME: error reporting
    Sint64 dataSize = SDL_RWsize(rwops);
    if (dataSize <= 0 or dataSize > std::numeric_limits<int>::max()) {
        return false;
    }
    Buffer<Uint8> data(dataSize);
    if (SDL_RWread(rwops, data.get(), data.size(), 1) != 1) {
        return false;
    }

    std::unique_ptr<openmpt::module> module(nullptr);
    try {
        module = std::make_unique<openmpt::module>(data.get(), data.size());
    }
    catch (const openmpt::exception& e) {
        AM_warnLn("libopenmpt failed to load mod: " << e.what());
        return false;
    }

    d->fDuration = chrono::duration_cast<chrono::microseconds>(
        chrono::duration<double>(module->get_duration_seconds()));
    d->fModule.swap(module);
    setIsOpen(true);
    return true;
}

int Aulib::AudioDecoderOpenmpt::getChannels() const
{
    return Aulib::spec().channels;
}

int Aulib::AudioDecoderOpenmpt::getRate() const
{
    return Aulib::spec().freq;
}

bool Aulib::AudioDecoderOpenmpt::rewind()
{
    return seekToTime(chrono::microseconds::zero());
}

chrono::microseconds Aulib::AudioDecoderOpenmpt::duration() const
{
    return d->fDuration;
}

bool Aulib::AudioDecoderOpenmpt::seekToTime(chrono::microseconds pos)
{
    d->fModule->set_position_seconds(chrono::duration<double>(pos).count());
    d->atEOF = false;
    return true;
}

int Aulib::AudioDecoderOpenmpt::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    if (d->atEOF) {
        return 0;
    }
    int ret;
    if (Aulib::spec().channels == 2) {
        ret = d->fModule->read_interleaved_stereo(Aulib::spec().freq, static_cast<size_t>(len / 2),
                                                  buf)
              * 2;
    } else {
        AM_debugAssert(Aulib::spec().channels == 1);
        ret = d->fModule->read(Aulib::spec().freq, static_cast<size_t>(len), buf);
    }
    if (ret == 0) {
        d->atEOF = true;
    }
    return ret;
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
