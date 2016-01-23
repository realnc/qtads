// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_global.h"
#include <SDL_stdinc.h>
#include <chrono>
#include <functional>
#include <memory>

namespace Aulib {

/*!
 * \brief Abstract base class for playback streams.
 */
class AULIB_EXPORT Stream
{
public:
    using Callback = std::function<void(Stream&)>;

    Stream();
    virtual ~Stream();

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
    virtual bool open() = 0;

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
    virtual bool play(int iterations = 1, std::chrono::microseconds fadeTime = {}) = 0;

    /*!
     * \brief Stop playback.
     *
     * When calling this, the stream is reset to the beginning again.
     *
     * \param fadeTime
     *  Fade-out over the specified amount of time.
     */
    virtual void stop(std::chrono::microseconds fadeTime = {}) = 0;

    /*!
     * \brief Pause playback.
     *
     * \param fadeTime
     *  Fade-out over the specified amount of time.
     */
    virtual void pause(std::chrono::microseconds fadeTime = {}) = 0;

    /*!
     * \brief Resume playback.
     *
     * \param fadeTime
     *  Fade-in over the specified amount of time.
     */
    virtual void resume(std::chrono::microseconds fadeTime = {}) = 0;

    /*!
     * \brief Rewind stream to the beginning.
     *
     * \return
     *  \retval true Stream was rewound successfully.
     *  \retval false Stream could not be rewound.
     */
    virtual bool rewind() = 0;

    /*!
     * \brief Change playback volume.
     *
     * \param volume
     *  0.0 means total silence, while 1.0 means non-attenuated, 100% (0db) volume. Values above 1.0
     *  are possible and will result in gain being applied (which might result in distortion.) For
     *  example, 3.5 would result in 350% percent volume. There is no upper limit.
     */
    virtual void setVolume(float volume) = 0;

    /*!
     * \brief Get current playback volume.
     *
     * \return
     *  Current playback volume.
     */
    virtual float volume() const = 0;

    /*!
     * \brief Mute the stream.
     *
     * A muted stream still accepts volume changes, but it will stay inaudible until it is unmuted.
     */
    virtual void mute() = 0;

    /*!
     * \brief Unmute the stream.
     */
    virtual void unmute() = 0;

    /*!
     * \brief Returns true if the stream is muted, false otherwise.
     */
    virtual bool isMuted() const = 0;

    /*!
     * \brief Get current playback state.
     *
     * Note that a paused stream is still considered as being in the playback state.
     *
     * \return
     *  \retval true Playback has been started.
     *  \retval false Playback has not been started yet, or was stopped.
     */
    virtual bool isPlaying() const = 0;

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
    virtual bool isPaused() const = 0;

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
    virtual std::chrono::microseconds duration() const = 0;

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
    virtual bool seekToTime(std::chrono::microseconds pos) = 0;

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
    const std::unique_ptr<struct Stream_priv> d;
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
