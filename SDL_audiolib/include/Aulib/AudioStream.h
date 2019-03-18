// This is copyrighted software. More information is at the end of this file.
#pragma once

#include <Aulib/Stream.h>
#include <aulib.h>
#include <memory>

struct SDL_RWops;
struct SDL_AudioSpec;

namespace Aulib {

class AudioDecoder;
class AudioResampler;
class Processor;

/*!
 * \brief Sample-based audio playback stream.
 *
 * This class handles playback for sample-based, digital audio streams produced by an AudioDecoder.
 */
class AULIB_EXPORT AudioStream: public Stream
{
public:
    /*!
     * \brief Constructs an audio stream from the given file name, decoder and resampler.
     *
     * \param filename
     *  File name from which to feed data to the decoder. Must not be null.
     *
     * \param decoder
     *  Decoder to use for decoding the contents of the file. Must not be null.
     *
     * \param resampler
     *  Resampler to use for converting the sample rate of the audio we get from the decoder. If
     *  this is null, then no resampling will be performed.
     */
    explicit AudioStream(const std::string& filename, std::unique_ptr<AudioDecoder> decoder,
                         std::unique_ptr<AudioResampler> resampler);

    /*!
     * \brief Constructs an audio stream from the given SDL_RWops, decoder and resampler.
     *
     * \param rwops
     *  SDL_RWops from which to feed data to the decoder. Must not be null.
     *
     * \param decoder
     *  Decoder to use for decoding the contents of the SDL_RWops. Must not be null.
     *
     * \param resampler
     *  Resampler to use for converting the sample rate of the audio we get from the decoder. If
     *  this is null, then no resampling will be performed.
     *
     * \param closeRw
     *  Specifies whether 'rwops' should be automatically closed when the stream is destroyed.
     */
    explicit AudioStream(SDL_RWops* rwops, std::unique_ptr<AudioDecoder> decoder,
                         std::unique_ptr<AudioResampler> resampler, bool closeRw);

    ~AudioStream() override;

    AudioStream(const AudioStream&) = delete;
    AudioStream& operator=(const AudioStream&) = delete;

    /*!
     * \brief Add an audio processor to the bottom of the processor list.
     *
     * You can add multiple processors. They will be run in the order they were added, each one
     * using the previous processor's output as input. If the processor instance already exists in
     * the processor list, or is a nullptr, the function does nothing.
     *
     * \param processor The processor to add.
     */
    void addProcessor(std::shared_ptr<Processor> processor);

    /*!
     * \brief Remove a processor from the stream.
     *
     * If the processor instance is not found, the function does nothing.
     *
     * \param processor Processor to remove.
     */
    void removeProcessor(Processor* processor);

    /*!
     * \brief Remove all processors from the stream.
     */
    void clearProcessors();

    bool open() override;
    bool play(int iterations = 1, std::chrono::microseconds fadeTime = {}) override;
    void stop(std::chrono::microseconds fadeTime = {}) override;
    void pause(std::chrono::microseconds fadeTime = {}) override;
    void resume(std::chrono::microseconds fadeTime = {}) override;
    bool rewind() override;
    void setVolume(float volume) override;
    float volume() const override;
    void mute() override;
    void unmute() override;
    bool isMuted() const override;
    bool isPlaying() const override;
    bool isPaused() const override;
    std::chrono::microseconds duration() const override;
    bool seekToTime(std::chrono::microseconds pos) override;

private:
    friend struct AudioStream_priv;
    friend int Aulib::init(int, SDL_AudioFormat, int, int);

    const std::unique_ptr<struct AudioStream_priv> d;
};

} // namespace Aulib

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
