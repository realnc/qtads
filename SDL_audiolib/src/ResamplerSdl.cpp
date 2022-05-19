// This is copyrighted software. More information is at the end of this file.
#include "Aulib/ResamplerSdl.h"
#include "aulib_log.h"
#include <SDL_audio.h>

#if SDL_VERSION_ATLEAST(2, 0, 7)

namespace Aulib {

struct ResamplerSdl_priv final
{
    std::unique_ptr<SDL_AudioStream, decltype(&SDL_FreeAudioStream)> fResampler{
        nullptr, &SDL_FreeAudioStream};
};

} // namespace Aulib

Aulib::ResamplerSdl::ResamplerSdl()
    : d(std::make_unique<ResamplerSdl_priv>())
{}

Aulib::ResamplerSdl::~ResamplerSdl() = default;

void Aulib::ResamplerSdl::doResampling(float dst[], const float src[], int& dstLen, int& srcLen)
{
    if (not d->fResampler) {
        dstLen = srcLen = 0;
        return;
    }

    if (SDL_AudioStreamPut(d->fResampler.get(), src, srcLen * sizeof(*src)) < 0) {
        dstLen = srcLen = 0;
        return;
    }

    const auto bytes_resampled =
        SDL_AudioStreamGet(d->fResampler.get(), dst, dstLen * sizeof(*dst));
    if (bytes_resampled < 0) {
        dstLen = srcLen = 0;
        return;
    }
    dstLen = bytes_resampled / sizeof(*dst);
}

auto Aulib::ResamplerSdl::adjustForOutputSpec(const int dstRate, const int srcRate,
                                              const int channels) -> int
{
    d->fResampler.reset(
        SDL_NewAudioStream(AUDIO_F32, channels, srcRate, AUDIO_F32, channels, dstRate));
    if (not d->fResampler) {
        return -1;
    }
    return 0;
}

void Aulib::ResamplerSdl::doDiscardPendingSamples()
{
    if (d->fResampler) {
        SDL_AudioStreamClear(d->fResampler.get());
    }
}

#endif // SDL_VERSION_ATLEAST

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
