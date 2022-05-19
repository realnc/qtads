// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <Aulib/Resampler.h>
#include <SDL_version.h>

namespace Aulib {

struct ResamplerSdl_priv;

/*!
 * \brief SDL_AudioStream resampler.
 *
 * This uses the built-in resampling functionality of SDL (through the SDL_AudioStream API) and has
 * no external dependencies. Requires at least SDL 2.0.7. Note that SDL can be built to use
 * libsamplerate instead of its own resampler. There's no way to detect whether this is the case or
 * not.
 *
 * It usually makes no sense to use this resampler, unless you have a specific need to use SDL's
 * resampler and you know that the SDL version you're running on was not built with libsamplerate
 * support. If you do want libsamplerate, then you can just use \ref Aulib::ResamplerSrc instead.
 */
class AULIB_EXPORT ResamplerSdl: public Resampler
{
public:
    ResamplerSdl();
    ~ResamplerSdl() override;

protected:
    void doResampling(float dst[], const float src[], int& dstLen, int& srcLen) override;
    auto adjustForOutputSpec(int dstRate, int srcRate, int channels) -> int override;
    void doDiscardPendingSamples() override;

private:
    const std::unique_ptr<ResamplerSdl_priv> d;
};

} // namespace Aulib

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
