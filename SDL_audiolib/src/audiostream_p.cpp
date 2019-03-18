// This is copyrighted software. More information is at the end of this file.
#include "audiostream_p.h"

#include "Aulib/AudioDecoder.h"
#include "Aulib/AudioResampler.h"
#include "Aulib/AudioStream.h"
#include "aulib_debug.h"
#include <SDL_timer.h>
#include <algorithm>
#include <cmath>
#include <type_traits>

void (*Aulib::AudioStream_priv::fSampleConverter)(Uint8[], const Buffer<float>& src) = nullptr;
SDL_AudioSpec Aulib::AudioStream_priv::fAudioSpec;
std::vector<Aulib::AudioStream*> Aulib::AudioStream_priv::fStreamList;
Buffer<float> Aulib::AudioStream_priv::fFinalMixBuf{0};
Buffer<float> Aulib::AudioStream_priv::fStrmBuf{0};
Buffer<float> Aulib::AudioStream_priv::fProcessorBuf{0};

Aulib::AudioStream_priv::AudioStream_priv(AudioStream* pub, std::unique_ptr<AudioDecoder> decoder,
                                          std::unique_ptr<AudioResampler> resampler,
                                          SDL_RWops* rwops, bool closeRw)
    : q(pub)
    , fRWops(rwops)
    , fCloseRw(closeRw)
    , fDecoder(std::move(decoder))
    , fResampler(std::move(resampler))
{
    if (fResampler) {
        fResampler->setDecoder(fDecoder);
    }
}

Aulib::AudioStream_priv::~AudioStream_priv()
{
    if (fCloseRw and fRWops != nullptr) {
        SDL_RWclose(fRWops);
    }
}

void Aulib::AudioStream_priv::fProcessFade()
{
    static_assert(std::is_same<decltype(fFadeInDuration), std::chrono::milliseconds>::value, "");
    static_assert(std::is_same<decltype(fFadeOutDuration), std::chrono::milliseconds>::value, "");

    if (fFadingIn) {
        Sint64 now = SDL_GetTicks();
        Sint64 curPos = now - fFadeInStartTick;
        if (curPos >= fFadeInDuration.count()) {
            fInternalVolume = 1.f;
            fFadingIn = false;
            return;
        }
        fInternalVolume =
            std::pow(static_cast<float>(now - fFadeInStartTick) / fFadeInDuration.count(), 3.f);
    } else if (fFadingOut) {
        Sint64 now = SDL_GetTicks();
        Sint64 curPos = now - fFadeOutStartTick;
        if (curPos >= fFadeOutDuration.count()) {
            fInternalVolume = 0.f;
            fFadingIn = false;
            if (fStopAfterFade) {
                fStopAfterFade = false;
                fStop();
            } else {
                fIsPaused = true;
            }
            return;
        }
        fInternalVolume = std::pow(
            -static_cast<float>(now - fFadeOutStartTick) / fFadeOutDuration.count() + 1.f, 3.f);
    }
}

void Aulib::AudioStream_priv::fStop()
{
    fStreamList.erase(std::remove(fStreamList.begin(), fStreamList.end(), this->q),
                      fStreamList.end());
    fDecoder->rewind();
    fIsPlaying = false;
}

void Aulib::AudioStream_priv::fSdlCallbackImpl(void* /*unused*/, Uint8 out[], int outLen)
{
    AM_debugAssert(AudioStream_priv::fSampleConverter != nullptr);

    int wantedSamples = outLen / ((fAudioSpec.format & 0xFF) / 8);

    if (fStrmBuf.size() != wantedSamples) {
        fFinalMixBuf.reset(wantedSamples);
        fStrmBuf.reset(wantedSamples);
        fProcessorBuf.reset(wantedSamples);
    }

    // Fill with silence.
    std::fill(fFinalMixBuf.begin(), fFinalMixBuf.end(), 0.f);

    // Iterate over a copy of the original stream list, since we might want to
    // modify the original as we go, removing streams that have stopped.
    std::vector<AudioStream*> streamList(fStreamList);

    for (const auto stream : streamList) {
        if (stream->d->fWantedIterations != 0
            and stream->d->fCurrentIteration >= stream->d->fWantedIterations) {
            continue;
        }
        if (stream->d->fIsPaused) {
            continue;
        }

        bool has_finished = false;
        bool has_looped = false;
        int len = 0;

        while (len < wantedSamples) {
            if (stream->d->fResampler) {
                len += stream->d->fResampler->resample(fStrmBuf.get() + len, wantedSamples - len);
            } else {
                bool callAgain = true;
                while (len < wantedSamples and callAgain) {
                    len += stream->d->fDecoder->decode(fStrmBuf.get() + len, wantedSamples - len,
                                                       callAgain);
                }
            }
            for (const auto& proc : stream->d->processors) {
                proc->process(fProcessorBuf.get(), fStrmBuf.get(), len);
                std::memcpy(fStrmBuf.get(), fProcessorBuf.get(), len * sizeof(*fStrmBuf.get()));
            }
            if (len < wantedSamples) {
                stream->d->fDecoder->rewind();
                if (stream->d->fWantedIterations != 0) {
                    ++stream->d->fCurrentIteration;
                    if (stream->d->fCurrentIteration >= stream->d->fWantedIterations) {
                        stream->d->fIsPlaying = false;
                        fStreamList.erase(
                            std::remove(fStreamList.begin(), fStreamList.end(), stream),
                            fStreamList.end());
                        has_finished = true;
                        break;
                    }
                    has_looped = true;
                }
            }
        }

        stream->d->fProcessFade();
        float volume = stream->d->fVolume * stream->d->fInternalVolume;

        // Avoid mixing on zero volume.
        if (not stream->d->fIsMuted and volume > 0.f) {
            // Avoid scaling operation when volume is 1.
            if (volume != 1.f) {
                for (int i = 0; i < len; ++i) {
                    fFinalMixBuf[i] += fStrmBuf[i] * volume;
                }
            } else {
                for (int i = 0; i < len; ++i) {
                    fFinalMixBuf[i] += fStrmBuf[i];
                }
            }
        }

        if (has_finished) {
            stream->invokeFinishCallback();
        } else if (has_looped) {
            stream->invokeLoopCallback();
        }
    }
    AudioStream_priv::fSampleConverter(out, fFinalMixBuf);
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
