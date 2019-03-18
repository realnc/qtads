// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderWildmidi.h"

#include "Buffer.h"
#include <SDL_rwops.h>
#include <algorithm>
#include <wildmidi_lib.h>

namespace chrono = std::chrono;

namespace Aulib {

struct AudioDecoderWildmidi_priv final
{
    std::unique_ptr<midi, decltype(&WildMidi_Close)> midiHandle{nullptr, &WildMidi_Close};
    Buffer<unsigned char> midiData{0};
    Buffer<Sint16> sampBuf{0};
    bool eof = false;

    static bool initialized;
    static int rate;
};

bool AudioDecoderWildmidi_priv::initialized = false;
int AudioDecoderWildmidi_priv::rate = 0;

} // namespace Aulib

Aulib::AudioDecoderWildmidi::AudioDecoderWildmidi()
    : d(std::make_unique<Aulib::AudioDecoderWildmidi_priv>())
{}

Aulib::AudioDecoderWildmidi::~AudioDecoderWildmidi() = default;

bool Aulib::AudioDecoderWildmidi::init(const std::string& configFile, int rate, bool hqResampling,
                                       bool reverb)
{
    if (AudioDecoderWildmidi_priv::initialized) {
        return true;
    }
    rate = std::min(std::max(11025, rate), 65000);
    AudioDecoderWildmidi_priv::rate = rate;
    unsigned short flags = 0;
    if (hqResampling) {
        flags |= WM_MO_ENHANCED_RESAMPLING;
    }
    if (reverb) {
        flags |= WM_MO_REVERB;
    }
    if (WildMidi_Init(configFile.c_str(), rate, flags) != 0) {
        return false;
    }
    AudioDecoderWildmidi_priv::initialized = true;
    return true;
}

void Aulib::AudioDecoderWildmidi::quit()
{
    WildMidi_Shutdown();
}

bool Aulib::AudioDecoderWildmidi::open(SDL_RWops* rwops)
{
    if (isOpen()) {
        return true;
    }
    if (not AudioDecoderWildmidi_priv::initialized) {
        return false;
    }

    // FIXME: error reporting
    Sint64 newMidiDataLen = SDL_RWsize(rwops);
    if (newMidiDataLen <= 0) {
        return false;
    }

    Buffer<unsigned char> newMidiData(newMidiDataLen);
    if (SDL_RWread(rwops, newMidiData.get(), newMidiData.size(), 1) != 1) {
        return false;
    }
    d->midiHandle.reset(WildMidi_OpenBuffer(newMidiData.get(), newMidiData.usize()));
    if (not d->midiHandle) {
        return false;
    }
    d->midiData.swap(newMidiData);
    setIsOpen(true);
    return true;
}

int Aulib::AudioDecoderWildmidi::getChannels() const
{
    return 2;
}

int Aulib::AudioDecoderWildmidi::getRate() const
{
    return AudioDecoderWildmidi_priv::rate;
}

int Aulib::AudioDecoderWildmidi::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    if (not d->midiHandle or d->eof) {
        return 0;
    }

    if (d->sampBuf.size() != len) {
        d->sampBuf.reset(len);
    }
#ifdef LIBWILDMIDI_VERSION
    using sample_type = int8_t*;
#else
    using sample_type = char*;
#endif
    int res =
        WildMidi_GetOutput(d->midiHandle.get(), reinterpret_cast<sample_type>(d->sampBuf.get()),
                           static_cast<unsigned long>(len) * 2);
    if (res < 0) {
        return 0;
    }
    // Convert from 16-bit to float.
    for (int i = 0; i < res / 2; ++i) {
        buf[i] = d->sampBuf[i] / 32768.f;
    }
    if (res < len) {
        d->eof = true;
    }
    return res / 2;
}

bool Aulib::AudioDecoderWildmidi::rewind()
{
    return seekToTime(chrono::microseconds::zero());
}

chrono::microseconds Aulib::AudioDecoderWildmidi::duration() const
{
    _WM_Info* info;
    if (not d->midiHandle or (info = WildMidi_GetInfo(d->midiHandle.get())) == nullptr) {
        return {};
    }
    auto sec = static_cast<double>(info->approx_total_samples) / getRate();
    return chrono::duration_cast<chrono::microseconds>(chrono::duration<double>(sec));
}

bool Aulib::AudioDecoderWildmidi::seekToTime(chrono::microseconds pos)
{
    if (not d->midiHandle) {
        return false;
    }

    unsigned long samplePos = chrono::duration<double>(pos).count() * getRate();
    if (WildMidi_FastSeek(d->midiHandle.get(), &samplePos) != 0) {
        return false;
    }
    d->eof = false;
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
