// This is copyrighted software. More information is at the end of this file.
#include "Aulib/Stream.h"

#include "Aulib/Decoder.h"
#include "Aulib/Processor.h"
#include "Aulib/Resampler.h"
#include "SdlAudioLocker.h"
#include "aulib.h"
#include "aulib_debug.h"
#include "aulib_global.h"
#include "sampleconv.h"
#include "stream_p.h"
#include <SDL_audio.h>
#include <SDL_timer.h>
#include <algorithm>

/* This is implemented here in order to avoid having the dtor call stop(),
 * which is a virtual.
 */
static void stop_impl(Aulib::Stream_priv* d, std::chrono::microseconds fadeTime)
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

Aulib::Stream::Stream(const std::string& filename, std::unique_ptr<Decoder> decoder,
                      std::unique_ptr<Resampler> resampler)
    : Stream(SDL_RWFromFile(filename.c_str(), "rb"), std::move(decoder), std::move(resampler), true)
{}

Aulib::Stream::Stream(const std::string& filename, std::unique_ptr<Decoder> decoder)
    : Stream(SDL_RWFromFile(filename.c_str(), "rb"), std::move(decoder), true)
{}

Aulib::Stream::Stream(SDL_RWops* rwops, std::unique_ptr<Decoder> decoder,
                      std::unique_ptr<Resampler> resampler, bool closeRw)
    : d(std::make_unique<Stream_priv>(this, std::move(decoder), std::move(resampler), rwops,
                                      closeRw))
{}

Aulib::Stream::Stream(SDL_RWops* rwops, std::unique_ptr<Decoder> decoder, bool closeRw)
    : d(std::make_unique<Stream_priv>(this, std::move(decoder), nullptr, rwops, closeRw))
{}

Aulib::Stream::~Stream()
{
    stop_impl(d.get(), std::chrono::microseconds::zero());
}

bool Aulib::Stream::open()
{
    if (d->fIsOpen) {
        return true;
    }
    if (not d->fDecoder->open(d->fRWops)) {
        return false;
    }
    if (d->fResampler) {
        d->fResampler->setSpec(Aulib::sampleRate(), Aulib::channelCount(), Aulib::frameSize());
    }
    d->fIsOpen = true;
    return true;
}

bool Aulib::Stream::play(int iterations, std::chrono::microseconds fadeTime)
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

void Aulib::Stream::stop(std::chrono::microseconds fadeTime)
{
    stop_impl(d.get(), fadeTime);
}

void Aulib::Stream::pause(std::chrono::microseconds fadeTime)
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

void Aulib::Stream::resume(std::chrono::microseconds fadeTime)
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

bool Aulib::Stream::rewind()
{
    if (not open()) {
        return false;
    }
    SdlAudioLocker locker;
    return d->fDecoder->rewind();
}

void Aulib::Stream::setVolume(float volume)
{
    if (volume < 0.f) {
        volume = 0.f;
    }
    SdlAudioLocker locker;
    d->fVolume = volume;
}

float Aulib::Stream::volume() const
{
    return d->fVolume;
}

void Aulib::Stream::mute()
{
    SdlAudioLocker locker;
    d->fIsMuted = true;
}

void Aulib::Stream::unmute()
{
    SdlAudioLocker locker;
    d->fIsMuted = false;
}

bool Aulib::Stream::isMuted() const
{
    return d->fIsMuted;
}

bool Aulib::Stream::isPlaying() const
{
    return d->fIsPlaying;
}

bool Aulib::Stream::isPaused() const
{
    return d->fIsPaused;
}

std::chrono::microseconds Aulib::Stream::duration() const
{
    return d->fDecoder->duration();
}

bool Aulib::Stream::seekToTime(std::chrono::microseconds pos)
{
    return d->fDecoder->seekToTime(pos);
}

void Aulib::Stream::setFinishCallback(Callback func)
{
    d->fFinishCallback = std::move(func);
}

void Aulib::Stream::unsetFinishCallback()
{
    d->fFinishCallback = nullptr;
}

void Aulib::Stream::setLoopCallback(Callback func)
{
    d->fLoopCallback = std::move(func);
}

void Aulib::Stream::unsetLoopCallback()
{
    d->fLoopCallback = nullptr;
}

void Aulib::Stream::addProcessor(std::shared_ptr<Processor> processor)
{
    if (processor == nullptr
        or std::find_if(
               d->processors.begin(), d->processors.end(),
               [&processor](std::shared_ptr<Processor>& p) { return p.get() == processor.get(); })
               != d->processors.end()) {
        return;
    }
    SdlAudioLocker locker;
    d->processors.push_back(std::move(processor));
}

void Aulib::Stream::removeProcessor(Processor* processor)
{
    auto it =
        std::find_if(d->processors.begin(), d->processors.end(),
                     [&processor](std::shared_ptr<Processor>& p) { return p.get() == processor; });
    if (it == d->processors.end()) {
        return;
    }
    SdlAudioLocker locker;
    d->processors.erase(it);
}

void Aulib::Stream::clearProcessors()
{
    SdlAudioLocker locker;
    d->processors.clear();
}

void Aulib::Stream::invokeFinishCallback()
{
    if (d->fFinishCallback) {
        d->fFinishCallback(*this);
    }
}

void Aulib::Stream::invokeLoopCallback()
{
    if (d->fLoopCallback) {
        d->fLoopCallback(*this);
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
