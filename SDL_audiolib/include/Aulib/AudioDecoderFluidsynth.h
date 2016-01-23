// This is copyrighted software. More information is at the end of this file.
#pragma once

#include <Aulib/AudioDecoder.h>

namespace Aulib {

/*!
 * \brief FluidSynth decoder.
 */
class AULIB_EXPORT AudioDecoderFluidSynth final: public AudioDecoder
{
public:
    AudioDecoderFluidSynth();
    ~AudioDecoderFluidSynth() override;

    int loadSoundfont(const std::string& filename);
    /*!
     * \brief Load a soundfont from an rwops.
     *
     * Ownership of the rwops is transfered to the decoder. The rwops is closed immediately if an
     * error occurs.
     *
     * \return 0 on success, non-zero if an error occurred.
     */
    int loadSoundfont(SDL_RWops* rwops);

    /*!
     * \brief Get the current master gain.
     */
    float gain() const;

    /*!
     * \brief Set the synthesizer gain.
     *
     * Lowering the gain helps reduce clipping in overly loud soundfonts. Some soundfonts however
     * might be too quiet and thus need a higher gain.
     *
     * FluidSynth's default is 0.2. Can be between 0 and 10.
     *
     * \param gain
     * Gain level.
     */
    void setGain(float gain);

    bool open(SDL_RWops* rwops) override;
    int getChannels() const override;
    int getRate() const override;
    bool rewind() override;
    std::chrono::microseconds duration() const override;
    bool seekToTime(std::chrono::microseconds pos) override;

protected:
    int doDecoding(float buf[], int len, bool& callAgain) override;

private:
    const std::unique_ptr<struct AudioDecoderFluidSynth_priv> d;
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
