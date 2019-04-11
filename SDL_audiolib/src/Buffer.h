// This is copyrighted software. More information is at the end of this file.
#pragma once

#include "aulib_debug.h"
#include <algorithm>
#include <cstring>
#include <memory>

/*
 * Simple RAII wrapper for buffers/arrays. More restrictive than std::vector.
 */
template <typename T>
class Buffer final
{
public:
    explicit Buffer(const int size)
        : fData(std::make_unique<T[]>(size))
        , fSize(size)
    {
        AM_debugAssert(size >= 0);
    }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    int size() const noexcept
    {
        return fSize;
    }

    size_t usize() const noexcept
    {
        return size();
    }

    void reset(const int newSize)
    {
        AM_debugAssert(newSize >= 0);
        fData = std::make_unique<T[]>(newSize);
        fSize = newSize;
    }

    void resize(const int newSize)
    {
        AM_debugAssert(newSize >= 0);
        auto newData = std::make_unique<T[]>(newSize);
        std::memcpy(newData.get(), fData.get(), sizeof(T) * std::min(newSize, fSize));
        fData.swap(newData);
        fSize = newSize;
    }

    void swap(Buffer& other) noexcept
    {
        fData.swap(other.fData);
        std::swap(fSize, other.fSize);
    }

    // unique_ptr::operator[] is not noexcept, but in reality, it can't throw.
    const T& operator[](const int pos) const noexcept
    {
        AM_debugAssert(pos >= 0 and pos < fSize);
        return fData[pos];
    }

    T& operator[](const int pos) noexcept
    {
        AM_debugAssert(pos >= 0 and pos < fSize);
        return fData[pos];
    }

    T* get() noexcept
    {
        return fData.get();
    }

    const T* get() const noexcept
    {
        return fData.get();
    }

    T* begin() noexcept
    {
        return get();
    }

    T* end() noexcept
    {
        return begin() + size();
    }

    const T* begin() const noexcept
    {
        return get();
    }

    const T* end() const noexcept
    {
        return begin() + size();
    }

private:
    std::unique_ptr<T[]> fData;
    int fSize;
};

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
