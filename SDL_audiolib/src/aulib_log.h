// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "aulib_config.h"
#include <fmt/core.h>

namespace aulib {
namespace log {

template <typename... Args>
void debug([[maybe_unused]] fmt::format_string<Args...>&& fmt_str, [[maybe_unused]] Args&&... args)
{
#if AULIB_DEBUG
    fmt::print(stderr, "SDL_audiolib debug: {}",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
#endif
}

template <typename... Args>
void debugLn([[maybe_unused]] fmt::format_string<Args...>&& fmt_str,
             [[maybe_unused]] Args&&... args)
{
#if AULIB_DEBUG
    fmt::print(stderr, "SDL_audiolib debug: {}\n",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
#endif
}

template <typename... Args>
void warn(fmt::format_string<Args...>&& fmt_str, Args&&... args)
{
    fmt::print(stderr, "SDL_audiolib warning: {}",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
}

template <typename... Args>
void warnLn(fmt::format_string<Args...>&& fmt_str, Args&&... args)
{
    fmt::print(stderr, "SDL_audiolib warning: {}\n",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
}

template <typename... Args>
void info(fmt::format_string<Args...>&& fmt_str, Args&&... args)
{
    fmt::print("SDL_audiolib info: {}",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
}

template <typename... Args>
void infoLn(fmt::format_string<Args...>&& fmt_str, Args&&... args)
{
    fmt::print("SDL_audiolib info: {}\n",
               fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str),
                           std::forward<Args>(args)...));
}

} // namespace log
} // namespace aulib

/*

Copyright (C) 2022 Nikos Chantziaras.

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
