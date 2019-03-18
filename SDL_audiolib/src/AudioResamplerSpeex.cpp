// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioResamplerSpeex.h"

#include "Aulib/AudioDecoder.h"
#include "SdlAudioLocker.h"
#include "aulib_global.h"
#include "speex_resampler.h"

#include <algorithm>

namespace Aulib {

struct AudioResamplerSpeex_priv final
{
    explicit AudioResamplerSpeex_priv(int quality)
        : fQuality(quality)
    {}

    std::unique_ptr<SpeexResamplerState, decltype(&speex_resampler_destroy)> fResampler{
        nullptr, &speex_resampler_destroy};
    int fQuality;
};

} // namespace Aulib

Aulib::AudioResamplerSpeex::AudioResamplerSpeex(int quality)
    : d(std::make_unique<AudioResamplerSpeex_priv>(std::min(std::max(0, quality), 10)))
{}

Aulib::AudioResamplerSpeex::~AudioResamplerSpeex() = default;

int Aulib::AudioResamplerSpeex::quality() const noexcept
{
    return d->fQuality;
}

void Aulib::AudioResamplerSpeex::setQuality(int quality)
{
    auto newQ = std::min(std::max(0, quality), 10);
    d->fQuality = newQ;
    if (d->fResampler == nullptr) {
        return;
    }
    SdlAudioLocker lock;
    speex_resampler_set_quality(d->fResampler.get(), newQ);
}

void Aulib::AudioResamplerSpeex::doResampling(float dst[], const float src[], int& dstLen,
                                              int& srcLen)
{
    if (not d->fResampler) {
        dstLen = srcLen = 0;
        return;
    }

    int channels = currentChannels();
    auto spxInLen = static_cast<spx_uint32_t>(srcLen / channels);
    auto spxOutLen = static_cast<spx_uint32_t>(dstLen / channels);
    if (spxInLen == 0 or spxOutLen == 0) {
        dstLen = srcLen = 0;
        return;
    }
    speex_resampler_process_interleaved_float(d->fResampler.get(), src, &spxInLen, dst, &spxOutLen);
    dstLen = static_cast<int>(spxOutLen) * channels;
    srcLen = static_cast<int>(spxInLen) * channels;
}

int Aulib::AudioResamplerSpeex::adjustForOutputSpec(int dstRate, int srcRate, int channels)
{
    int err;
    d->fResampler.reset(speex_resampler_init(
        static_cast<spx_uint32_t>(channels), static_cast<spx_uint32_t>(srcRate),
        static_cast<spx_uint32_t>(dstRate), d->fQuality, &err));
    if (err != 0) {
        d->fResampler = nullptr;
        return -1;
    }
    return 0;
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
