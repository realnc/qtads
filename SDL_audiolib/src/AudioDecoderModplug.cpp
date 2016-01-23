// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderModplug.h"

#include "Buffer.h"
#include "aulib.h"
#include <SDL_audio.h>
#include <libmodplug/modplug.h>
#include <limits>

namespace chrono = std::chrono;

static ModPlug_Settings modplugSettings;
static bool initialized = false;

static void initModPlug(const SDL_AudioSpec& spec)
{
    ModPlug_GetSettings(&modplugSettings);
    modplugSettings.mFlags = MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION;
    // TODO: can modplug handle more than 2 channels?
    modplugSettings.mChannels = spec.channels == 1 ? 1 : 2;
    // It seems MogPlug does resample to any samplerate. 32, 44.1, up to
    // 192K all seem to work correctly.
    modplugSettings.mFrequency = spec.freq;
    modplugSettings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
    modplugSettings.mBits = 32;
    ModPlug_SetSettings(&modplugSettings);
    initialized = true;
}

namespace Aulib {

/// \private
struct AudioDecoderModPlug_priv final
{
    AudioDecoderModPlug_priv();

    std::unique_ptr<ModPlugFile, decltype(&ModPlug_Unload)> mpHandle{nullptr, &ModPlug_Unload};
    bool atEOF = false;
    chrono::microseconds fDuration{};
};

} // namespace Aulib

Aulib::AudioDecoderModPlug_priv::AudioDecoderModPlug_priv()
{
    if (not initialized) {
        initModPlug(Aulib::spec());
    }
}

Aulib::AudioDecoderModPlug::AudioDecoderModPlug()
    : d(std::make_unique<AudioDecoderModPlug_priv>())
{}

Aulib::AudioDecoderModPlug::~AudioDecoderModPlug() = default;

bool Aulib::AudioDecoderModPlug::open(SDL_RWops* rwops)
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
    d->mpHandle.reset(ModPlug_Load(data.get(), data.size()));
    if (not d->mpHandle) {
        return false;
    }
    ModPlug_SetMasterVolume(d->mpHandle.get(), 192);
    d->fDuration = chrono::milliseconds(ModPlug_GetLength(d->mpHandle.get()));
    setIsOpen(true);
    return true;
}

int Aulib::AudioDecoderModPlug::getChannels() const
{
    return modplugSettings.mChannels;
}

int Aulib::AudioDecoderModPlug::getRate() const
{
    return modplugSettings.mFrequency;
}

int Aulib::AudioDecoderModPlug::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    if (d->atEOF) {
        return 0;
    }
    Buffer<Sint32> tmpBuf(len);
    int ret = ModPlug_Read(d->mpHandle.get(), tmpBuf.get(), len * 4);
    // Convert from 32-bit to float.
    for (int i = 0; i < len; ++i) {
        buf[i] = tmpBuf[i] / 2147483648.f;
    }
    if (ret == 0) {
        d->atEOF = true;
    }
    return ret / static_cast<int>(sizeof(*buf));
}

bool Aulib::AudioDecoderModPlug::rewind()
{
    return seekToTime(chrono::microseconds::zero());
}

chrono::microseconds Aulib::AudioDecoderModPlug::duration() const
{
    return d->fDuration;
}

bool Aulib::AudioDecoderModPlug::seekToTime(chrono::microseconds pos)
{
    ModPlug_Seek(d->mpHandle.get(), chrono::duration_cast<chrono::milliseconds>(pos).count());
    d->atEOF = false;
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
