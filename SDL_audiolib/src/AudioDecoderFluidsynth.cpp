// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioDecoderFluidsynth.h"

#include "Buffer.h"
#include "aulib.h"
#include "aulib_debug.h"

#include <SDL_audio.h>
#include <SDL_rwops.h>
#include <array>
#include <cstdio>
#include <fluidsynth.h>

namespace chrono = std::chrono;

static fluid_settings_t* settings = nullptr;

/* Huge kludge. Fluidsynth doesn't have a nice API for custom soundfont loading, so we hijack the
 * filename string to store a pointer to the rwops. We prefix such filenames with a "&".
 */
static void* sfontOpenCb(const char* filename)
{
    if (filename == nullptr) {
        return nullptr;
    }
    if (filename[0] != '&') {
        return SDL_RWFromFile(filename, "rb");
    }
    void* rwops;
    if (sscanf(filename, "&%p", &rwops) != 1) {
        AM_warnLn(
            "failed to extract rwops pointer from string - rwops might have leaked (please "
            "file a bug)");
        return nullptr;
    }
    return rwops;
}

static int sfontReadCb(void* dst, int count, void* rwops)
{
    Buffer<char> buf(count);
    if (SDL_RWread(static_cast<SDL_RWops*>(rwops), buf.get(), 1, count) <= 0) {
        return FLUID_FAILED;
    }
    memcpy(dst, buf.get(), count);
    return FLUID_OK;
}

static int sfontSeekCb(void* rwops, long offset, int whence)
{
    switch (whence) {
    case SEEK_SET:
        whence = RW_SEEK_SET;
        break;
    case SEEK_CUR:
        whence = RW_SEEK_CUR;
        break;
    default:
        whence = RW_SEEK_END;
    }
    if (SDL_RWseek(static_cast<SDL_RWops*>(rwops), offset, whence) == -1) {
        return FLUID_FAILED;
    }
    return FLUID_OK;
}

static int sfontCloseCb(void* rwops)
{
    if (SDL_RWclose(static_cast<SDL_RWops*>(rwops)) != 0) {
        return FLUID_FAILED;
    }
    return FLUID_OK;
}

static long sfontTellCb(void* rwops)
{
    auto pos = SDL_RWtell(static_cast<SDL_RWops*>(rwops));
    if (pos == -1) {
        return FLUID_FAILED;
    }
    return pos;
}

static int initFluidSynth()
{
    if (settings != nullptr) {
        return 0;
    }
    if ((settings = new_fluid_settings()) == nullptr) {
        return -1;
    }
    fluid_settings_setnum(settings, "synth.sample-rate", Aulib::spec().freq);
    for (const auto i : {FLUID_PANIC, FLUID_ERR, FLUID_WARN, FLUID_INFO, FLUID_DBG}) {
        fluid_set_log_function(i, nullptr, nullptr);
    }
    return 0;
}

namespace Aulib {

/// \private
struct AudioDecoderFluidSynth_priv final
{
    AudioDecoderFluidSynth_priv();

    std::unique_ptr<fluid_synth_t, decltype(&delete_fluid_synth)> fSynth{nullptr,
                                                                         &delete_fluid_synth};
    std::unique_ptr<fluid_player_t, decltype(&delete_fluid_player)> fPlayer{nullptr,
                                                                            &delete_fluid_player};
    fluid_sfloader_t* sfloader = nullptr;
    Buffer<Uint8> fMidiData{0};
    bool fEOF = false;
};

} // namespace Aulib

Aulib::AudioDecoderFluidSynth_priv::AudioDecoderFluidSynth_priv()
{
    if (settings == nullptr) {
        initFluidSynth();
    }
    fSynth.reset(new_fluid_synth(settings));
    if (not fSynth) {
        return;
    }
    fluid_synth_set_interp_method(fSynth.get(), -1, FLUID_INTERP_7THORDER);
    fluid_synth_set_reverb(fSynth.get(),
                           0.6, // Room size
                           0.5, // Damping
                           0.5, // Width
                           0.3); // Level
    sfloader = new_fluid_defsfloader(settings);
    fluid_sfloader_set_callbacks(sfloader, sfontOpenCb, sfontReadCb, sfontSeekCb, sfontTellCb,
                                 sfontCloseCb);
    fluid_synth_add_sfloader(fSynth.get(), sfloader);
}

Aulib::AudioDecoderFluidSynth::AudioDecoderFluidSynth()
    : d(std::make_unique<AudioDecoderFluidSynth_priv>())
{}

Aulib::AudioDecoderFluidSynth::~AudioDecoderFluidSynth() = default;

int Aulib::AudioDecoderFluidSynth::loadSoundfont(const std::string& filename)
{
    fluid_synth_sfload(d->fSynth.get(), filename.c_str(), 1);
    return 0;
}

int Aulib::AudioDecoderFluidSynth::loadSoundfont(SDL_RWops* rwops)
{
    auto closeRwops = [rwops] {
        if (SDL_RWclose(rwops) != 0) {
            AM_warnLn("failed to close rwops: " << SDL_GetError());
        }
    };

    std::array<char, 64> bogus_fname;
    auto ret = snprintf(bogus_fname.data(), bogus_fname.size(), "&%p", static_cast<void*>(rwops));
    if (ret < 0 or ret >= static_cast<int>(bogus_fname.size())) {
        AM_warnLn("internal string representation of pointer is too long (please file a bug)");
        closeRwops();
        return -1;
    }
    if (fluid_synth_sfload(d->fSynth.get(), bogus_fname.data(), 1) == FLUID_FAILED) {
        AM_warnLn("failed to load soundfont from rwops");
        closeRwops();
        return -1;
    }
    return 0;
}

float Aulib::AudioDecoderFluidSynth::gain() const
{
    return fluid_synth_get_gain(d->fSynth.get());
}

void Aulib::AudioDecoderFluidSynth::setGain(float gain)
{
    fluid_synth_set_gain(d->fSynth.get(), gain);
}

bool Aulib::AudioDecoderFluidSynth::open(SDL_RWops* rwops)
{
    if (isOpen()) {
        return true;
    }
    if (not d->fSynth) {
        return false;
    }

    // FIXME: error reporting
    Sint64 midiDataLen = SDL_RWsize(rwops);
    if (midiDataLen <= 0) {
        return false;
    }
    Buffer<Uint8> newMidiData(midiDataLen);
    if (SDL_RWread(rwops, newMidiData.get(), newMidiData.size(), 1) != 1) {
        return false;
    }
    d->fPlayer.reset(new_fluid_player(d->fSynth.get()));
    if (not d->fPlayer
        or fluid_player_add_mem(d->fPlayer.get(), newMidiData.get(), newMidiData.usize())
               != FLUID_OK
        or fluid_player_play(d->fPlayer.get()) != FLUID_OK) {
        return false;
    }
    d->fMidiData.swap(newMidiData);
    setIsOpen(true);
    return true;
}

int Aulib::AudioDecoderFluidSynth::getChannels() const
{
    int channels;
    fluid_settings_getint(settings, "synth.audio-channels", &channels);
    // This is the amount of stereo channel *pairs*, so each pair has *two* audio channels.
    return channels * 2;
}

int Aulib::AudioDecoderFluidSynth::getRate() const
{
    double rate;
    fluid_settings_getnum(settings, "synth.sample-rate", &rate);
    return rate;
}

int Aulib::AudioDecoderFluidSynth::doDecoding(float buf[], int len, bool& callAgain)
{
    callAgain = false;
    if (not d->fPlayer or d->fEOF) {
        return 0;
    }

    len /= Aulib::spec().channels;
    int res = fluid_synth_write_float(d->fSynth.get(), len, buf, 0, 2, buf, 1, 2);
    if (fluid_player_get_status(d->fPlayer.get()) == FLUID_PLAYER_DONE) {
        d->fEOF = true;
    }
    if (res == FLUID_OK) {
        return len * Aulib::spec().channels;
    }
    return 0;
}

bool Aulib::AudioDecoderFluidSynth::rewind()
{
    fluid_player_stop(d->fPlayer.get());
    d->fPlayer.reset(new_fluid_player(d->fSynth.get()));
    if (not d->fPlayer) {
        return false;
    }
    fluid_player_add_mem(d->fPlayer.get(), d->fMidiData.get(), d->fMidiData.usize());
    fluid_player_play(d->fPlayer.get());
    d->fEOF = false;
    return true;
}

chrono::microseconds Aulib::AudioDecoderFluidSynth::duration() const
{
    // We can't tell how long a MIDI file is.
    return {};
}

bool Aulib::AudioDecoderFluidSynth::seekToTime(chrono::microseconds /*pos*/)
{
    // We don't support seeking.
    return false;
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
