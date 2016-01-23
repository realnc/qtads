// This is copyrighted software. More information is at the end of this file.
#include "aulib.h"

#include "Aulib/AudioStream.h"
#include "audiostream_p.h"
#include "aulib_debug.h"
#include "sampleconv.h"
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_version.h>

static bool gInitialized = false;

extern "C" {
static void sdlCallback(void* /*unused*/, Uint8 out[], int outLen)
{
    Aulib::AudioStream_priv::fSdlCallbackImpl(nullptr, out, outLen);
}
}

int Aulib::init(int freq, SDL_AudioFormat format, int channels, int bufferSize)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        return -1;
    }

    // We only support mono and stereo at this point.
    channels = std::min(std::max(1, channels), 2);

    SDL_AudioSpec requestedSpec{};
    requestedSpec.freq = freq;
    requestedSpec.format = format;
    requestedSpec.channels = channels;
    requestedSpec.samples = bufferSize;
    requestedSpec.callback = ::sdlCallback;
    AudioStream_priv::fAudioSpec = requestedSpec;
    if (SDL_OpenAudio(&requestedSpec, &AudioStream_priv::fAudioSpec) != 0) {
        Aulib::quit();
        return -1;
    }

    AM_debugPrint("SDL initialized with sample format: ");
    switch (AudioStream_priv::fAudioSpec.format) {
    case AUDIO_S8:
        AM_debugPrintLn("S8");
        AudioStream_priv::fSampleConverter = Aulib::floatToS8;
        break;
    case AUDIO_U8:
        AM_debugPrintLn("U8");
        AudioStream_priv::fSampleConverter = Aulib::floatToU8;
        break;
    case AUDIO_S16LSB:
        AM_debugPrintLn("S16LSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToS16LSB;
        break;
    case AUDIO_U16LSB:
        AM_debugPrintLn("U16LSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToU16LSB;
        break;
    case AUDIO_S16MSB:
        AM_debugPrintLn("S16MSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToS16MSB;
        break;
    case AUDIO_U16MSB:
        AM_debugPrintLn("U16MSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToU16MSB;
        break;
    case AUDIO_S32LSB:
        AM_debugPrintLn("S32LSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToS32LSB;
        break;
    case AUDIO_S32MSB:
        AM_debugPrintLn("S32MSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToS32MSB;
        break;
    case AUDIO_F32LSB:
        AM_debugPrintLn("F32LSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToFloatLSB;
        break;
    case AUDIO_F32MSB:
        AM_debugPrintLn("F32MSB");
        AudioStream_priv::fSampleConverter = Aulib::floatToFloatMSB;
        break;
    default:
        AM_warnLn("Unknown audio format spec: " << AudioStream_priv::fAudioSpec.format);
        Aulib::quit();
        return -1;
    }

    SDL_PauseAudio(0);
    gInitialized = true;
    std::atexit(Aulib::quit);
    return 0;
}

void Aulib::quit()
{
    if (not gInitialized) {
        return;
    }
    for (const auto stream : AudioStream_priv::fStreamList) {
        if (stream->isPlaying()) {
            stream->stop();
        }
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    AudioStream_priv::fSampleConverter = nullptr;
    gInitialized = false;
}

const SDL_AudioSpec& Aulib::spec()
{
    return AudioStream_priv::fAudioSpec;
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
