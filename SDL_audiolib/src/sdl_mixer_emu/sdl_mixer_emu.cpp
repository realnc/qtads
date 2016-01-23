// This is copyrighted software. More information is at the end of this file.
#include "sdl_mixer_emu.h"
#include "aulib.h"
#include "aulib_config.h"
#include "aulib_debug.h"
#include "aulib_global.h"
//#include "audiostream.h"

const SDL_version* Mix_Linked_Version()
{
    AM_debugPrintLn(__func__);

    static SDL_version v = {SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL};
    return &v;
}

int Mix_Init(int flags)
{
    AM_debugPrintLn(__func__);

    return flags;
}

void Mix_Quit()
{
    AM_debugPrintLn(__func__);

    Aulib::quit();
}

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
    AM_debugPrintLn(__func__);

    return Aulib::init(frequency, format, channels, chunksize);
}

int Mix_AllocateChannels(int numchans)
{
    AM_debugPrintLn(__func__);

    return numchans;
}

int Mix_QuerySpec(int* frequency, Uint16* format, int* channels)
{
    AM_debugPrintLn(__func__);

    const SDL_AudioSpec& spec = Aulib::spec();
    if (frequency != nullptr) {
        *frequency = spec.freq;
    }
    if (format != nullptr) {
        *format = spec.format;
    }
    if (channels != nullptr) {
        *channels = spec.channels;
    }
    return 1; // TODO
}

Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops* /*src*/, int /*freesrc*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

Mix_Chunk* Mix_QuickLoad_WAV(Uint8* /*mem*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

Mix_Chunk* Mix_QuickLoad_RAW(Uint8* /*mem*/, Uint32 /*len*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

void Mix_FreeChunk(Mix_Chunk* /*chunk*/)
{
    AM_debugPrintLn(__func__);
}

int Mix_GetNumChunkDecoders()
{
    AM_debugPrintLn(__func__);

    return 0;
}

const char* Mix_GetChunkDecoder(int /*index*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

void Mix_SetPostMix(void (*/*mix_func*/)(void*, Uint8*, int), void* /*arg*/)
{
    AM_debugPrintLn(__func__);
}

void Mix_ChannelFinished(void (*/*channel_finished*/)(int))
{
    AM_debugPrintLn(__func__);
}

int Mix_RegisterEffect(int /*chan*/, Mix_EffectFunc_t /*f*/, Mix_EffectDone_t /*d*/, void* /*arg*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_UnregisterEffect(int /*channel*/, Mix_EffectFunc_t /*f*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_UnregisterAllEffects(int /*channel*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_SetPanning(int /*channel*/, Uint8 /*left*/, Uint8 /*right*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_SetPosition(int /*channel*/, Sint16 /*angle*/, Uint8 /*distance*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_SetDistance(int /*channel*/, Uint8 /*distance*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_SetReverseStereo(int /*channel*/, int /*flip*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_ReserveChannels(int /*num*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_GroupChannel(int /*which*/, int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_GroupChannels(int /*from*/, int /*to*/, int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_GroupAvailable(int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return -1;
}

int Mix_GroupCount(int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_GroupOldest(int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_GroupNewer(int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_PlayChannelTimed(int /*channel*/, Mix_Chunk* /*chunk*/, int /*loops*/, int /*ticks*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_FadeInChannelTimed(int /*channel*/, Mix_Chunk* /*chunk*/, int /*loops*/, int /*ms*/,
                           int /*ticks*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_Volume(int /*channel*/, int /*volume*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_VolumeChunk(Mix_Chunk* /*chunk*/, int /*volume*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_HaltChannel(int /*channel*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_HaltGroup(int /*tag*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_ExpireChannel(int /*channel*/, int /*ticks*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_FadeOutChannel(int /*which*/, int /*ms*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_FadeOutGroup(int /*tag*/, int /*ms*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

Mix_Fading Mix_FadingChannel(int /*which*/)
{
    AM_debugPrintLn(__func__);

    return MIX_NO_FADING;
}

void Mix_Pause(int /*channel*/)
{
    AM_debugPrintLn(__func__);
}

void Mix_Resume(int /*channel*/)
{
    AM_debugPrintLn(__func__);
}

int Mix_Paused(int /*channel*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_Playing(int /*channel*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

Mix_Chunk* Mix_GetChunk(int /*channel*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

void Mix_CloseAudio()
{
    AM_debugPrintLn(__func__);
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
