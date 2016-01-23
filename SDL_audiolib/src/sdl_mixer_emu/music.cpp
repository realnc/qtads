// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderFluidsynth.h"
#include "Aulib/AudioDecoderModplug.h"
#include "Aulib/AudioDecoderMpg123.h"
#include "Aulib/AudioDecoderSndfile.h"
#include "Aulib/AudioDecoderVorbis.h"
#include "Aulib/AudioResamplerSpeex.h"
#include "Aulib/AudioStream.h"
#include "aulib_config.h"
#include "aulib_debug.h"
#include "aulib_global.h"
#include "sdl_mixer_emu.h"

// Currently active global music stream (SDL_mixer only supports one.)
static Aulib::AudioStream* gMusic = nullptr;
static float gMusicVolume = 1.0f;

extern "C" {
static void (*gMusicFinishHook)() = nullptr;
}

static void musicFinishHookWrapper(Aulib::Stream& /*unused*/)
{
    if (gMusicFinishHook != nullptr) {
        gMusicFinishHook();
    }
}

Mix_Music* Mix_LoadMUS(const char* file)
{
    AM_debugPrintLn(__func__);

    auto strm = new Aulib::AudioStream(file, Aulib::AudioDecoder::decoderFor(file),
                                       std::make_unique<Aulib::AudioResamplerSpeex>());
    strm->open();
    return (Mix_Music*)strm;
}

Mix_Music* Mix_LoadMUS_RW(SDL_RWops* rw)
{
    AM_debugPrintLn(__func__);

    auto strm = new Aulib::AudioStream(rw, Aulib::AudioDecoder::decoderFor(rw),
                                       std::make_unique<Aulib::AudioResamplerSpeex>(), false);
    strm->open();
    return (Mix_Music*)strm;
}

Mix_Music* Mix_LoadMUSType_RW(SDL_RWops* rw, Mix_MusicType type, int freesrc)
{
    AM_debugPrintLn(__func__ << " type: " << type);

    auto decoder = std::unique_ptr<Aulib::AudioDecoder>(nullptr);
    switch (type) {
    case MUS_WAV:
    case MUS_FLAC:
        decoder = std::make_unique<Aulib::AudioDecoderSndfile>();
        break;

    case MUS_MOD:
    case MUS_MODPLUG:
        decoder = std::make_unique<Aulib::AudioDecoderModPlug>();
        break;

    case MUS_MID:
        decoder = std::make_unique<Aulib::AudioDecoderFluidSynth>();
        break;

    case MUS_OGG:
        decoder = std::make_unique<Aulib::AudioDecoderVorbis>();
        break;

    case MUS_MP3:
    case MUS_MP3_MAD:
        decoder = std::make_unique<Aulib::AudioDecoderMpg123>();
        break;

    default:
        AM_debugPrintLn("NO DECODER FOUND");
        return nullptr;
    }

    auto strm = new Aulib::AudioStream(
        rw, std::move(decoder), std::make_unique<Aulib::AudioResamplerSpeex>(), freesrc != 0);
    strm->open();
    return (Mix_Music*)strm;
}

void Mix_FreeMusic(Mix_Music* music)
{
    AM_debugPrintLn(__func__);

    auto* strm = (Aulib::AudioStream*)music;
    if (gMusic == strm) {
        gMusic = nullptr;
    }
    delete strm;
}

int Mix_GetNumMusicDecoders()
{
    AM_debugPrintLn(__func__);

    return 0;
}

const char* Mix_GetMusicDecoder(int /*index*/)
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

Mix_MusicType Mix_GetMusicType(const Mix_Music* /*music*/)
{
    AM_debugPrintLn(__func__);

    return MUS_NONE;
}

void Mix_HookMusic(void (*/*mix_func*/)(void*, Uint8*, int), void* /*arg*/)
{
    AM_debugPrintLn(__func__);
}

void Mix_HookMusicFinished(void (*music_finished)())
{
    AM_debugPrintLn(__func__);

    if (music_finished == nullptr) {
        if (gMusic != nullptr) {
            gMusic->unsetFinishCallback();
        }
        gMusicFinishHook = nullptr;
    } else {
        gMusicFinishHook = music_finished;
        if (gMusic != nullptr) {
            gMusic->setFinishCallback(musicFinishHookWrapper);
        }
    }
}

void* Mix_GetMusicHookData()
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

int Mix_PlayMusic(Mix_Music* music, int loops)
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        gMusic->stop();
        gMusic = nullptr;
    }
    if (loops == 0) {
        return 0;
    }
    gMusic = (Aulib::AudioStream*)music;
    gMusic->setVolume(gMusicVolume);
    return static_cast<int>(gMusic->play(loops == -1 ? 0 : loops));
}

int Mix_FadeInMusic(Mix_Music* music, int loops, int /*ms*/)
{
    AM_debugPrintLn(__func__);

    return Mix_PlayMusic(music, loops);
}

int Mix_FadeInMusicPos(Mix_Music* /*music*/, int /*loops*/, int /*ms*/, double /*position*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

int Mix_VolumeMusic(int volume)
{
    AM_debugPrintLn(__func__);

    int prevVol = gMusicVolume * MIX_MAX_VOLUME;
    volume = std::min(std::max(-1, volume), MIX_MAX_VOLUME);

    if (volume >= 0) {
        gMusicVolume = (float)volume / (float)MIX_MAX_VOLUME;
        if (gMusic != nullptr) {
            gMusic->setVolume(gMusicVolume);
        }
    }
    return prevVol;
}

int Mix_HaltMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        gMusic->stop();
        gMusic = nullptr;
    }
    return 0;
}

int Mix_FadeOutMusic(int /*ms*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

Mix_Fading Mix_FadingMusic()
{
    AM_debugPrintLn(__func__);

    return MIX_NO_FADING;
}

void Mix_PauseMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        gMusic->pause();
    }
}

void Mix_ResumeMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr and gMusic->isPaused()) {
        gMusic->resume();
    }
}

void Mix_RewindMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        gMusic->rewind();
    }
}

int Mix_PausedMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        return static_cast<int>(gMusic->isPaused());
    }
    return 0;
}

int Mix_SetMusicPosition(double /*position*/)
{
    AM_debugPrintLn(__func__);

    return -1;
}

int Mix_PlayingMusic()
{
    AM_debugPrintLn(__func__);

    if (gMusic != nullptr) {
        return static_cast<int>(gMusic->isPlaying());
    }
    return 0;
}

int Mix_SetMusicCMD(const char* /*command*/)
{
    AM_debugPrintLn(__func__);

    return -1;
}

int Mix_SetSynchroValue(int /*value*/)
{
    AM_debugPrintLn(__func__);

    return -1;
}

int Mix_GetSynchroValue()
{
    AM_debugPrintLn(__func__);

    return -1;
}

int Mix_SetSoundFonts(const char* /*paths*/)
{
    AM_debugPrintLn(__func__);

    return 0;
}

const char* Mix_GetSoundFonts()
{
    AM_debugPrintLn(__func__);

    return nullptr;
}

int Mix_EachSoundFont(int (*/*function*/)(const char*, void*), void* /*data*/)
{
    AM_debugPrintLn(__func__);

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
