// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_export.h"
#include <SDL_stdinc.h>
#include <chrono>
#include <memory>

struct SDL_RWops;

namespace Aulib {

/*!
 * \brief Abstract base class for audio decoders.
 */
class AULIB_EXPORT Decoder
{
public:
    Decoder();
    virtual ~Decoder();

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;

    static std::unique_ptr<Decoder> decoderFor(const std::string& filename);
    static std::unique_ptr<Decoder> decoderFor(SDL_RWops* rwops);

    bool isOpen() const;
    int decode(float buf[], int len, bool& callAgain);

    virtual bool open(SDL_RWops* rwops) = 0;
    virtual int getChannels() const = 0;
    virtual int getRate() const = 0;
    virtual bool rewind() = 0;
    virtual std::chrono::microseconds duration() const = 0;
    virtual bool seekToTime(std::chrono::microseconds pos) = 0;

protected:
    void setIsOpen(bool f);
    virtual int doDecoding(float buf[], int len, bool& callAgain) = 0;

private:
    const std::unique_ptr<struct Decoder_priv> d;
};

} // namespace Aulib

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
