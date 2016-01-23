// This is copyrighted software. More information is at the end of this file.
#include "Aulib/AudioStream.h"

#include "Aulib/AudioDecoder.h"
#include "Aulib/AudioResampler.h"
#include "SdlAudioLocker.h"
#include "audiostream_p.h"
#include "aulib.h"
#include "aulib_debug.h"
#include "aulib_global.h"
#include "sampleconv.h"
#include <SDL_audio.h>
#include <SDL_timer.h>

/* This is implemented here in order to avoid having the dtor call stop(),
 * which is a virtual.
 */
static void stop_impl(Aulib::AudioStream_priv* d, std::chrono::microseconds fadeTime)
{
    if (not d->fIsOpen or not d->fIsPlaying) {
        return;
    }
    SdlAudioLocker locker;
    if (fadeTime.count() > 0) {
        d->fFadingIn = false;
        d->fFadingOut = true;
        d->fFadeOutDuration = std::chrono::duration_cast<std::chrono::milliseconds>(fadeTime);
        d->fFadeOutStartTick = SDL_GetTicks();
        d->fStopAfterFade = true;
    } else {
        d->fStop();
    }
}

Aulib::AudioStream::AudioStream(const std::string& filename, std::unique_ptr<AudioDecoder> decoder,
                                std::unique_ptr<AudioResampler> resampler)
    : AudioStream(SDL_RWFromFile(filename.c_str(), "rb"), std::move(decoder), std::move(resampler),
                  true)
{}

Aulib::AudioStream::AudioStream(SDL_RWops* rwops, std::unique_ptr<AudioDecoder> decoder,
                                std::unique_ptr<AudioResampler> resampler, bool closeRw)
    : d(std::make_unique<AudioStream_priv>(this, std::move(decoder), std::move(resampler), rwops,
                                           closeRw))
{}

Aulib::AudioStream::~AudioStream()
{
    stop_impl(d.get(), std::chrono::microseconds::zero());
}

bool Aulib::AudioStream::open()
{
    if (d->fIsOpen) {
        return true;
    }
    if (d->fRWops == nullptr) {
        return false;
    }
    if (not d->fDecoder->open(d->fRWops)) {
        return false;
    }
    if (d->fResampler) {
        d->fResampler->setSpec(Aulib::spec().freq, Aulib::spec().channels, Aulib::spec().samples);
    }
    d->fIsOpen = true;
    return true;
}

bool Aulib::AudioStream::play(int iterations, std::chrono::microseconds fadeTime)
{
    if (not open()) {
        return false;
    }
    if (d->fIsPlaying) {
        return true;
    }
    d->fCurrentIteration = 0;
    d->fWantedIterations = iterations;
    d->fPlaybackStartTick = SDL_GetTicks();
    if (fadeTime.count() > 0) {
        d->fInternalVolume = 0.f;
        d->fFadingIn = true;
        d->fFadingOut = false;
        d->fFadeInDuration = std::chrono::duration_cast<std::chrono::milliseconds>(fadeTime);
        d->fFadeInStartTick = d->fPlaybackStartTick;
    } else {
        d->fInternalVolume = 1.f;
        d->fFadingIn = false;
    }
    SdlAudioLocker locker;
    d->fStreamList.push_back(this);
    d->fIsPlaying = true;
    return true;
}

void Aulib::AudioStream::stop(std::chrono::microseconds fadeTime)
{
    stop_impl(d.get(), fadeTime);
}

void Aulib::AudioStream::pause(std::chrono::microseconds fadeTime)
{
    if (not open() or d->fIsPaused) {
        return;
    }
    SdlAudioLocker locker;
    if (fadeTime.count() > 0) {
        d->fFadingIn = false;
        d->fFadingOut = true;
        d->fFadeOutDuration = std::chrono::duration_cast<std::chrono::milliseconds>(fadeTime);
        d->fFadeOutStartTick = SDL_GetTicks();
        d->fStopAfterFade = false;
    } else {
        d->fIsPaused = true;
    }
}

void Aulib::AudioStream::resume(std::chrono::microseconds fadeTime)
{
    if (not d->fIsPaused) {
        return;
    }
    SdlAudioLocker locker;
    if (fadeTime.count() > 0) {
        d->fInternalVolume = 0.f;
        d->fFadingIn = true;
        d->fFadingOut = false;
        d->fFadeInDuration = std::chrono::duration_cast<std::chrono::milliseconds>(fadeTime);
        d->fFadeInStartTick = SDL_GetTicks();
    } else {
        d->fInternalVolume = 1.f;
    }
    d->fIsPaused = false;
}

bool Aulib::AudioStream::rewind()
{
    if (not open()) {
        return false;
    }
    SdlAudioLocker locker;
    return d->fDecoder->rewind();
}

void Aulib::AudioStream::setVolume(float volume)
{
    if (volume < 0.f) {
        volume = 0.f;
    }
    SdlAudioLocker locker;
    d->fVolume = volume;
}

float Aulib::AudioStream::volume() const
{
    return d->fVolume;
}

void Aulib::AudioStream::mute()
{
    SdlAudioLocker locker;
    d->fIsMuted = true;
}

void Aulib::AudioStream::unmute()
{
    SdlAudioLocker locker;
    d->fIsMuted = false;
}

bool Aulib::AudioStream::isMuted() const
{
    return d->fIsMuted;
}

bool Aulib::AudioStream::isPlaying() const
{
    return d->fIsPlaying;
}

bool Aulib::AudioStream::isPaused() const
{
    return d->fIsPaused;
}

std::chrono::microseconds Aulib::AudioStream::duration() const
{
    return d->fDecoder->duration();
}

bool Aulib::AudioStream::seekToTime(std::chrono::microseconds pos)
{
    return d->fDecoder->seekToTime(pos);
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
