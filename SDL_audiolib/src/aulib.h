// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_global.h"
#include <SDL_audio.h>

struct SDL_AudioSpec;

namespace Aulib {

/*!
 * \brief Initializes the audio system.
 *
 * \param freq
 *  Sample rate that the audio device should be opened in.
 *
 * \param format
 *  Audio format. The most common format is AUDIO_S16LSB. The formats are defined by SDL
 *  (SDL_audio.h).
 *
 * \param channels
 *  Amount of output channels to use. Can either be 1 (mono) or 2 (stereo.) Lower or higher values
 *  will be adjusted.
 *
 * \param bufferSize
 *  Size in bytes of the internal buffer that is used to feed audio samples to SDL. Lower values
 *  provide lower latency on audio operations, at the cost of increased CPU usage and risk of audio
 *  drop-outs. A good value for 44.1kHz output for music players is 8192 bytes (8kB), while a game
 *  that needs to play sound effects without much latency would use something like 2048 instead.
 *
 * \return
 *  \retval 0 The audio system was initialized successfully.
 *  \retval !=0 The audio system could not be initialized.
 */
AULIB_EXPORT int init(int freq, SDL_AudioFormat format, int channels, int bufferSize);

/*!
 *  \brief Shuts down the SDL_audiolib library.
 *
 *  It is not required to call this function manually, as this happens automatically at program
 *  exit, but it is useful in cases where you want to shut down the audio system for some reason.
 */
AULIB_EXPORT void quit();

//! \private
// Don't use this. It will be removed soon.
AULIB_EXPORT const SDL_AudioSpec& spec();

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
