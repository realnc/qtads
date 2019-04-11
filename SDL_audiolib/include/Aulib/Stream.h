// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_export.h"
#include <SDL_stdinc.h>
#include <aulib.h>
#include <chrono>
#include <functional>
#include <memory>

struct SDL_RWops;
struct SDL_AudioSpec;

namespace Aulib {

class Decoder;
class Resampler;
class Processor;

/*!
 * \brief A \ref Stream handles playback for audio produced by a Decoder.
 */
class AULIB_EXPORT Stream
{
public:
    using Callback = std::function<void(Stream&)>;

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
    explicit Stream(const std::string& filename, std::unique_ptr<Decoder> decoder,
                    std::unique_ptr<Resampler> resampler);

    //! \overload
    explicit Stream(const std::string& filename, std::unique_ptr<Decoder> decoder);

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
    explicit Stream(SDL_RWops* rwops, std::unique_ptr<Decoder> decoder,
                    std::unique_ptr<Resampler> resampler, bool closeRw);

    //! \overload
    explicit Stream(SDL_RWops* rwops, std::unique_ptr<Decoder> decoder, bool closeRw);

    virtual ~Stream();

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    /*!
     * \brief Open the stream and prepare it for playback.
     *
     * Although calling this function is not required, you can use it in order to determine whether
     * the stream can be loaded successfully prior to starting playback.
     *
     * \return
     *  \retval true Stream was opened successfully.
     *  \retval false The stream could not be opened.
     */
    virtual bool open();

    /*!
     * \brief Start playback.
     *
     * \param iterations
     *  The amount of times the stream should be played. If zero, the stream will loop forever.
     *
     * \param fadeTime
     *  Fade-in over the specified amount of time.
     *
     * \return
     *  \retval true Playback was started successfully, or it was already started.
     *  \retval false Playback could not be started.
     */
    virtual bool play(int iterations = 1, std::chrono::microseconds fadeTime = {});

    /*!
     * \brief Stop playback.
     *
     * When calling this, the stream is reset to the beginning again.
     *
     * \param fadeTime
     *  Fade-out over the specified amount of time.
     */
    virtual void stop(std::chrono::microseconds fadeTime = {});

    /*!
     * \brief Pause playback.
     *
     * \param fadeTime
     *  Fade-out over the specified amount of time.
     */
    virtual void pause(std::chrono::microseconds fadeTime = {});

    /*!
     * \brief Resume playback.
     *
     * \param fadeTime
     *  Fade-in over the specified amount of time.
     */
    virtual void resume(std::chrono::microseconds fadeTime = {});

    /*!
     * \brief Rewind stream to the beginning.
     *
     * \return
     *  \retval true Stream was rewound successfully.
     *  \retval false Stream could not be rewound.
     */
    virtual bool rewind();

    /*!
     * \brief Change playback volume.
     *
     * \param volume
     *  0.0 means total silence, while 1.0 means non-attenuated, 100% (0db) volume. Values above 1.0
     *  are possible and will result in gain being applied (which might result in distortion.) For
     *  example, 3.5 would result in 350% percent volume. There is no upper limit.
     */
    virtual void setVolume(float volume);

    /*!
     * \brief Get current playback volume.
     *
     * \return
     *  Current playback volume.
     */
    virtual float volume() const;

    /*!
     * \brief Mute the stream.
     *
     * A muted stream still accepts volume changes, but it will stay inaudible until it is unmuted.
     */
    virtual void mute();

    /*!
     * \brief Unmute the stream.
     */
    virtual void unmute();

    /*!
     * \brief Returns true if the stream is muted, false otherwise.
     */
    virtual bool isMuted() const;

    /*!
     * \brief Get current playback state.
     *
     * Note that a paused stream is still considered as being in the playback state.
     *
     * \return
     *  \retval true Playback has been started.
     *  \retval false Playback has not been started yet, or was stopped.
     */
    virtual bool isPlaying() const;

    /*!
     * \brief Get current pause state.
     *
     * Note that a stream that is not in playback state is not considered paused. This will return
     * false even for streams where playback is stopped.
     *
     * \return
     *  \retval true The stream is currently paused.
     *  \retval false The stream is currently not paused.
     */
    virtual bool isPaused() const;

    /*!
     * \brief Get stream duration.
     *
     * It is possible that for some streams (for example MOD files and some MP3 files), the reported
     * duration can be wrong. For some streams, it might not even be possible to get a duration at
     * all (MIDI files, for example.)
     *
     * \return
     * Stream duration. If the stream does not provide duration information, a zero duration is
     * returned.
     */
    virtual std::chrono::microseconds duration() const;

    /*!
     * \brief Seek to a time position in the stream.
     *
     * This will change the current playback position in the stream to the specified time.
     *
     * Note that for some streams (for example MOD files and some MP3 files), it might not be
     * possible to do an accurate seek, in which case the position that is set might be off by
     * some margin. In some streams, seeking is not possible at all (MIDI files, for example.)
     *
     * \param pos
     *  Position to seek to.
     *
     * \return
     *  \retval true The playback position was changed successfully.
     *  \retval false This stream does not support seeking.
     */
    virtual bool seekToTime(std::chrono::microseconds pos);

    /*!
     * \brief Set a callback for when the stream finishes playback.
     *
     * The callback will be called when the stream finishes playing. This can happen when you
     * manually stop it, or when it finishes playing on its own.
     *
     * \param func
     *  Anything that can be wrapped by an std::function (like a function pointer, functor or
     *  lambda.)
     */
    void setFinishCallback(Callback func);

    /*!
     * \brief Removes any finish-playback callback that was previously set.
     */
    void unsetFinishCallback();

    /*!
     * \brief Set a callback for when the stream loops.
     *
     * The callback will be called when the stream loops. It will be called once per loop.
     *
     * \param func
     *  Anything that can be wrapped by an std::function (like a function pointer, functor or
     *  lambda.)
     */
    void setLoopCallback(Callback func);

    /*!
     * \brief Removes any loop callback that was previously set.
     */
    void unsetLoopCallback();

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

protected:
    /*!
     * \brief Invokes the finish-playback callback, if there is one.
     *
     * In your subclass, you must call this function whenever your stream stops playing.
     */
    void invokeFinishCallback();

    /*!
     * \brief Invokes the loop callback, if there is one.
     *
     * In your subclass, you must call this function whenever your stream loops.
     */
    void invokeLoopCallback();

private:
    friend struct Stream_priv;
    friend bool Aulib::init(int, SDL_AudioFormat, int, int);

    const std::unique_ptr<struct Stream_priv> d;
};

} // namespace Aulib

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
