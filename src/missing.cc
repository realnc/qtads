// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QString>
#include <QStringConverter>
#include <QStringDecoder>
#include <QStringEncoder>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "globals.h"
#include "missing.h"
#include "settings.h"
#include "sysframe.h"

/*
 * These functions are only used by the base code to compare ASCII strings. No need to convert
 * anything to QString for unicode-aware comparisons.
 */

#ifndef _WIN32
auto memicmp(const char* const s1, const char* const s2, const size_t len) -> int
{
    for (size_t i = 0; i < len; ++i) {
        if (std::tolower(s1[i]) < std::tolower(s2[i])) {
            return -1;
        }
        if (std::tolower(s1[i]) > std::tolower(s2[i])) {
            return 1;
        }
    }
    return 0;
}
#endif

auto stricmp(const char* const s1, const char* const s2) -> int
{
    return qstricmp(s1, s2);
}

auto strnicmp(const char* const s1, const char* const s2, const size_t n) -> int
{
    return qstrnicmp(s1, s2, n);
}

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
