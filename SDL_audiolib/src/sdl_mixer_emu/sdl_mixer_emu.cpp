// This is copyrighted software. More information is at the end of this file.
#include "sdl_mixer_emu.h"
#include "aulib.h"
#include "aulib_config.h"
#include "aulib_global.h"
#include "aulib_log.h"
//#include "stream.h"

auto Mix_Linked_Version() -> const SDL_version*
{
    aulib::log::debugLn("{}", __func__);

    static SDL_version v = {SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL};
    return &v;
}

auto Mix_Init(int flags) -> int
{
    aulib::log::debugLn("{}", __func__);

    return flags;
}

void Mix_Quit()
{
    aulib::log::debugLn("{}", __func__);

    Aulib::quit();
}

auto Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize) -> int
{
    aulib::log::debugLn("{}", __func__);

    return Aulib::init(frequency, format, channels, chunksize);
}

auto Mix_AllocateChannels(int numchans) -> int
{
    aulib::log::debugLn("{}", __func__);

    return numchans;
}

auto Mix_QuerySpec(int* frequency, Uint16* format, int* channels) -> int
{
    aulib::log::debugLn("{}", __func__);

    if (frequency) {
        *frequency = Aulib::sampleRate();
    }
    if (format) {
        *format = Aulib::sampleFormat();
    }
    if (channels) {
        *channels = Aulib::channelCount();
    }
    return 1; // TODO
}

auto Mix_LoadWAV_RW(SDL_RWops* /*src*/, int /*freesrc*/) -> Mix_Chunk*
{
    aulib::log::debugLn("{}", __func__);

    return nullptr;
}

auto Mix_QuickLoad_WAV(Uint8* /*mem*/) -> Mix_Chunk*
{
    aulib::log::debugLn("{}", __func__);

    return nullptr;
}

auto Mix_QuickLoad_RAW(Uint8* /*mem*/, Uint32 /*len*/) -> Mix_Chunk*
{
    aulib::log::debugLn("{}", __func__);

    return nullptr;
}

void Mix_FreeChunk(Mix_Chunk* /*chunk*/)
{
    aulib::log::debugLn("{}", __func__);
}

auto Mix_GetNumChunkDecoders() -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GetChunkDecoder(int /*index*/) -> const char*
{
    aulib::log::debugLn("{}", __func__);

    return nullptr;
}

void Mix_SetPostMix(void (*/*mix_func*/)(void*, Uint8*, int), void* /*arg*/)
{
    aulib::log::debugLn("{}", __func__);
}

void Mix_ChannelFinished(void (*/*channel_finished*/)(int))
{
    aulib::log::debugLn("{}", __func__);
}

auto Mix_RegisterEffect(int /*chan*/, Mix_EffectFunc_t /*f*/, Mix_EffectDone_t /*d*/, void* /*arg*/)
    -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_UnregisterEffect(int /*channel*/, Mix_EffectFunc_t /*f*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_UnregisterAllEffects(int /*channel*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_SetPanning(int /*channel*/, Uint8 /*left*/, Uint8 /*right*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_SetPosition(int /*channel*/, Sint16 /*angle*/, Uint8 /*distance*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_SetDistance(int /*channel*/, Uint8 /*distance*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_SetReverseStereo(int /*channel*/, int /*flip*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_ReserveChannels(int /*num*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GroupChannel(int /*which*/, int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GroupChannels(int /*from*/, int /*to*/, int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GroupAvailable(int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return -1;
}

auto Mix_GroupCount(int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GroupOldest(int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GroupNewer(int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_PlayChannelTimed(int /*channel*/, Mix_Chunk* /*chunk*/, int /*loops*/, int /*ticks*/)
    -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_FadeInChannelTimed(int /*channel*/, Mix_Chunk* /*chunk*/, int /*loops*/, int /*ms*/,
                            int /*ticks*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_Volume(int /*channel*/, int /*volume*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_VolumeChunk(Mix_Chunk* /*chunk*/, int /*volume*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_HaltChannel(int /*channel*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_HaltGroup(int /*tag*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_ExpireChannel(int /*channel*/, int /*ticks*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_FadeOutChannel(int /*which*/, int /*ms*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_FadeOutGroup(int /*tag*/, int /*ms*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_FadingChannel(int /*which*/) -> Mix_Fading
{
    aulib::log::debugLn("{}", __func__);

    return MIX_NO_FADING;
}

void Mix_Pause(int /*channel*/)
{
    aulib::log::debugLn("{}", __func__);
}

void Mix_Resume(int /*channel*/)
{
    aulib::log::debugLn("{}", __func__);
}

auto Mix_Paused(int /*channel*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_Playing(int /*channel*/) -> int
{
    aulib::log::debugLn("{}", __func__);

    return 0;
}

auto Mix_GetChunk(int /*channel*/) -> Mix_Chunk*
{
    aulib::log::debugLn("{}", __func__);

    return nullptr;
}

void Mix_CloseAudio()
{
    aulib::log::debugLn("{}", __func__);
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
