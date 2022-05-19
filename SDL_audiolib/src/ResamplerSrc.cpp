// This is copyrighted software. More information is at the end of this file.
#include "Aulib/ResamplerSrc.h"

#include "aulib_debug.h"
#include "aulib_log.h"
#include <cstring>
#include <samplerate.h>

namespace Aulib {

struct ResamplerSrc_priv final
{
    explicit ResamplerSrc_priv(ResamplerSrc::Quality quality)
        : fQuality(quality)
    {}

    std::unique_ptr<SRC_STATE, decltype(&src_delete)> fResampler{nullptr, &src_delete};
    SRC_DATA fData{};
    ResamplerSrc::Quality fQuality;
};

} // namespace Aulib

Aulib::ResamplerSrc::ResamplerSrc(Quality quality)
    : d(std::make_unique<ResamplerSrc_priv>(quality))
{}

Aulib::ResamplerSrc::~ResamplerSrc() = default;

auto Aulib::ResamplerSrc::quality() const noexcept -> Aulib::ResamplerSrc::Quality
{
    return d->fQuality;
}

void Aulib::ResamplerSrc::doResampling(float dst[], const float src[], int& dstLen, int& srcLen)
{
    if (not d->fResampler) {
        dstLen = srcLen = 0;
        return;
    }

    d->fData.data_in = src;
    d->fData.data_out = dst;
    int channels = currentChannels();
    d->fData.input_frames = srcLen / channels;
    d->fData.output_frames = dstLen / channels;
    d->fData.end_of_input = 0;

    src_process(d->fResampler.get(), &d->fData);

    dstLen = d->fData.output_frames_gen * channels;
    srcLen = d->fData.input_frames_used * channels;
}

auto Aulib::ResamplerSrc::adjustForOutputSpec(int dstRate, int srcRate, int channels) -> int
{
    int err;
    d->fData.src_ratio = static_cast<double>(dstRate) / srcRate;

    const int src_quality = [&] {
        switch (d->fQuality) {
        case Quality::Linear:
            return SRC_LINEAR;
        case Quality::ZeroOrderHold:
            return SRC_ZERO_ORDER_HOLD;
        case Quality::SincFastest:
            return SRC_SINC_FASTEST;
        case Quality::SincMedium:
            return SRC_SINC_MEDIUM_QUALITY;
        case Quality::SincBest:
            return SRC_SINC_BEST_QUALITY;
        }
        aulib::log::warnLn(
            "ResamplerSrc: Unrecognized ResamplerSrc::Quality value {}. Will use "
            "Quality::SincMedium.",
            static_cast<int>(d->fQuality));
        return SRC_SINC_MEDIUM_QUALITY;
    }();

    d->fResampler.reset(src_new(src_quality, channels, &err));
    if (not d->fResampler) {
        return -1;
    }
    return 0;
}

void Aulib::ResamplerSrc::doDiscardPendingSamples()
{
    if (d->fResampler) {
        src_reset(d->fResampler.get());
    }
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
