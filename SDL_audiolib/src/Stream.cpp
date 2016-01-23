// This is copyrighted software. More information is at the end of this file.
#include "Aulib/Stream.h"

#include "aulib_debug.h"
#include <utility>

namespace Aulib {

/// \private
struct Stream_priv final
{
    Stream::Callback fFinishCallback;
    Stream::Callback fLoopCallback;
};

} // namespace Aulib

Aulib::Stream::Stream()
    : d(std::make_unique<Stream_priv>())
{}

Aulib::Stream::~Stream() = default;

void Aulib::Stream::setFinishCallback(Callback func)
{
    d->fFinishCallback = std::move(func);
}

void Aulib::Stream::unsetFinishCallback()
{
    d->fFinishCallback = nullptr;
}

void Aulib::Stream::setLoopCallback(Callback func)
{
    d->fLoopCallback = std::move(func);
}

void Aulib::Stream::unsetLoopCallback()
{
    d->fLoopCallback = nullptr;
}

void Aulib::Stream::invokeFinishCallback()
{
    if (d->fFinishCallback) {
        d->fFinishCallback(*this);
    }
}

void Aulib::Stream::invokeLoopCallback()
{
    if (d->fLoopCallback) {
        d->fLoopCallback(*this);
    }
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
