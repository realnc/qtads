// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoder.h"

#include "Aulib/AudioDecoderAdlmidi.h"
#include "Aulib/AudioDecoderBassmidi.h"
#include "Aulib/AudioDecoderFluidsynth.h"
#include "Aulib/AudioDecoderModplug.h"
#include "Aulib/AudioDecoderMpg123.h"
#include "Aulib/AudioDecoderMusepack.h"
#include "Aulib/AudioDecoderOpenmpt.h"
#include "Aulib/AudioDecoderOpus.h"
#include "Aulib/AudioDecoderSndfile.h"
#include "Aulib/AudioDecoderVorbis.h"
#include "Aulib/AudioDecoderWildmidi.h"
#include "Aulib/AudioDecoderXmp.h"
#include "Buffer.h"
#include "aulib.h"
#include "aulib_config.h"
#include <SDL_audio.h>
#include <SDL_rwops.h>
#include <array>

namespace Aulib {

struct AudioDecoder_priv final
{
    Buffer<float> stereoBuf{0};
    bool isOpen = false;
};

} // namespace Aulib

Aulib::AudioDecoder::AudioDecoder()
    : d(std::make_unique<Aulib::AudioDecoder_priv>())
{}

Aulib::AudioDecoder::~AudioDecoder() = default;

std::unique_ptr<Aulib::AudioDecoder> Aulib::AudioDecoder::decoderFor(const std::string& filename)
{
    auto rwopsClose = [](SDL_RWops* rwops) { SDL_RWclose(rwops); };
    std::unique_ptr<SDL_RWops, decltype(rwopsClose)> rwops(SDL_RWFromFile(filename.c_str(), "rb"),
                                                           rwopsClose);
    return AudioDecoder::decoderFor(rwops.get());
}

std::unique_ptr<Aulib::AudioDecoder> Aulib::AudioDecoder::decoderFor(SDL_RWops* rwops)
{
    const auto rwPos = SDL_RWtell(rwops);

    auto rewindRwops = [rwops, rwPos] { SDL_RWseek(rwops, rwPos, RW_SEEK_SET); };

    auto tryDecoder = [rwops, &rewindRwops](auto dec) {
        rewindRwops();
        bool ret = dec->open(rwops);
        rewindRwops();
        return ret;
    };

#if USE_DEC_LIBVORBIS
    if (tryDecoder(std::make_unique<AudioDecoderVorbis>())) {
        return std::make_unique<Aulib::AudioDecoderVorbis>();
    }
#endif
#if USE_DEC_LIBOPUSFILE
    if (tryDecoder(std::make_unique<AudioDecoderOpus>())) {
        return std::make_unique<Aulib::AudioDecoderOpus>();
    }
#endif
#if USE_DEC_MUSEPACK
    if (tryDecoder(std::make_unique<AudioDecoderMusepack>())) {
        return std::make_unique<Aulib::AudioDecoderMusepack>();
    }
#endif
#if USE_DEC_FLUIDSYNTH or USE_DEC_BASSMIDI or USE_DEC_WILDMIDI or USE_DEC_ADLMIDI
    {
        std::array<char, 5> head{};
        if (SDL_RWread(rwops, head.data(), 1, 4) == 4 and head == decltype(head){"MThd"}) {
            using midi_dec_type =
#    if USE_DEC_FLUIDSYNTH
                AudioDecoderFluidSynth;
#    elif USE_DEC_BASSMIDI
                AudioDecoderBassmidi;
#    elif USE_DEC_WILDMIDI
                AudioDecoderWildmidi;
#    elif USE_DEC_ADLMIDI
                AudioDecoderAdlmidi;
#    endif
            if (tryDecoder(std::make_unique<midi_dec_type>())) {
                return std::make_unique<midi_dec_type>();
            }
        }
    }
#endif
#if USE_DEC_SNDFILE
    if (tryDecoder(std::make_unique<AudioDecoderSndfile>())) {
        return std::make_unique<Aulib::AudioDecoderSndfile>();
    }
#endif
#if USE_DEC_OPENMPT
    if (tryDecoder(std::make_unique<AudioDecoderOpenmpt>())) {
        return std::make_unique<Aulib::AudioDecoderOpenmpt>();
    }
#endif

#if USE_DEC_XMP
    if (tryDecoder(std::make_unique<AudioDecoderXmp>())) {
        return std::make_unique<Aulib::AudioDecoderXmp>();
    }
#endif
#if USE_DEC_MODPLUG
    // We don't try ModPlug, since it thinks just about anything is a module
    // file, which would result in virtually everything we feed it giving a
    // false positive.
#endif
#if USE_DEC_MPG123
    if (tryDecoder(std::make_unique<AudioDecoderMpg123>())) {
        return std::make_unique<Aulib::AudioDecoderMpg123>();
    }
#endif
    return nullptr;
}

bool Aulib::AudioDecoder::isOpen() const
{
    return d->isOpen;
}

// Conversion happens in-place.
static constexpr void monoToStereo(float buf[], int len)
{
    if (len < 1 or buf == nullptr) {
        return;
    }
    for (int i = len / 2 - 1, j = len - 1; i > 0; --i) {
        buf[j--] = buf[i];
        buf[j--] = buf[i];
    }
}

static constexpr void stereoToMono(float dst[], const float src[], int srcLen)
{
    if (srcLen < 1 or dst == nullptr or src == nullptr) {
        return;
    }
    for (int i = 0, j = 0; i < srcLen; i += 2, ++j) {
        dst[j] = src[i] * 0.5f;
        dst[j] += src[i + 1] * 0.5f;
    }
}

int Aulib::AudioDecoder::decode(float buf[], int len, bool& callAgain)
{
    const SDL_AudioSpec& spec = Aulib::spec();

    if (this->getChannels() == 1 and spec.channels == 2) {
        int srcLen = this->doDecoding(buf, len / 2, callAgain);
        monoToStereo(buf, srcLen * 2);
        return srcLen * 2;
    }

    if (this->getChannels() == 2 and spec.channels == 1) {
        if (d->stereoBuf.size() != len * 2) {
            d->stereoBuf.reset(len * 2);
        }
        int srcLen = this->doDecoding(d->stereoBuf.get(), d->stereoBuf.size(), callAgain);
        stereoToMono(buf, d->stereoBuf.get(), srcLen);
        return srcLen / 2;
    }
    return this->doDecoding(buf, len, callAgain);
}

void Aulib::AudioDecoder::setIsOpen(bool f)
{
    d->isOpen = f;
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
