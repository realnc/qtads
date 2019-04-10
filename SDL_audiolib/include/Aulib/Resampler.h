// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_global.h"
#include <cstddef>
#include <memory>

namespace Aulib {

class Decoder;
struct Resampler_priv;

/*!
 * \brief Abstract base class for audio resamplers.
 *
 * This class receives audio from an Decoder and resamples it to the requested sample rate.
 */
class AULIB_EXPORT Resampler
{
public:
    /*!
     * \brief Constructs an audio resampler.
     */
    Resampler();

    virtual ~Resampler();

    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;

    /*! \brief Sets the decoder that is to be used as source.
     *
     * \param decoder
     *  The decoder to use as source. Must not be null.
     */
    void setDecoder(std::shared_ptr<Decoder> decoder);

    /*! \brief Sets the target sample rate, channels and chuck size.
     *
     * \param dstRate Wanted sample rate.
     *
     * \param channels Wanted amount of channels.
     *
     * \param chunkSize
     *  Specifies how many samples per channel to resample at most in each call to the resample()
     *  function. It is recommended to set this to the same value that was used as buffer size in
     *  the call to Aulib::init().
     */
    int setSpec(int dstRate, int channels, int chunkSize);

    int currentRate() const;
    int currentChannels() const;
    int currentChunkSize() const;

    /*! \brief Fills an output buffer with resampled audio samples.
     *
     * \param dst Output buffer.
     *
     * \param dstLen Size of output buffer (amount of elements, not size in bytes.)
     *
     * \return The amount of samples that were stored in the buffer. This can be smaller than
     *         'dstLen' if the decoder has no more samples left.
     */
    int resample(float dst[], int dstLen);

protected:
    /*! \brief Change sample rate and amount of channels.
     *
     * This function must be implemented when subclassing. It is used to notify subclasses about
     * changes in source and target sample rates, as well as the amount of channels in the audio.
     *
     * \param dstRate Target sample rate (rate being resampled to.)
     *
     * \param srcRate Source sample rate (rate being resampled from.)
     *
     * \param channels Amount of channels in both the source as well as the target audio buffers.
     */
    virtual int adjustForOutputSpec(int dstRate, int srcRate, int channels) = 0;

    /*! This function must be implemented when subclassing. It must resample
     * the audio contained in 'src' containing 'srcLen' samples, and store the
     * resulting samples in 'dst', which has a capacity of at most 'dstLen'
     * samples.
     *
     * The 'src' buffer contains audio in either mono or stereo. Stereo is
     * stored in interleaved format.
     *
     * The source and target sample rates, as well as the amount of channels
     * that are to be used must be those that were specified in the last call
     * to the adjustForOutputSpec() function.
     *
     * 'dstLen' and 'srcLen' are both input as well as output parameters. The
     * function must set 'dstLen' to the amount of samples that were actually
     * stored in 'dst', and 'srcLen' to the amount of samples that were
     * actually used from 'src'. For example, if in the following call:
     *
     *      dstLen = 200;
     *      srcLen = 100;
     *      doResampling(dst, src, dstLen, srcLen);
     *
     * the function resamples 98 samples from 'src', resulting in 196 samples
     * which are stored in 'dst', the function must set 'srcLen' to 98 and
     * 'dstLen' to 196.
     *
     * So when implementing this function, you do not need to worry about using
     * up all the available samples in 'src'. Simply resample as much audio
     * from 'src' as you can in order to fill 'dst' as much as possible, and if
     * there's anything left at the end that cannot be resampled, simply ignore
     * it.
     */
    virtual void doResampling(float dst[], const float src[], int& dstLen, int& srcLen) = 0;

private:
    friend Resampler_priv;
    const std::unique_ptr<Resampler_priv> d;
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
