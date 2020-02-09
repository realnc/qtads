// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QString>
#include <QTextCodec>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "globals.h"
#include "missing.h"
#include "settings.h"
#include "sysframe.h"

#ifndef _WIN32
auto memicmp(const char* s1, const char* s2, size_t len) -> int
{
    char* x1 = new char[len];
    char* x2 = new char[len];

    for (size_t i = 0; i < len; ++i) {
        x1[i] = std::tolower(s1[i]);
        x2[i] = std::tolower(s2[i]);
    }
    int ret = std::memcmp(x1, x2, len);
    delete[] x1;
    delete[] x2;
    return ret;
}
#endif

auto stricmp(const char* s1, const char* s2) -> int
{
    if (qFrame->tads3()) {
        return QString::localeAwareCompare(
            QString::fromUtf8(s1).toLower(), QString::fromUtf8(s2).toLower());
    }
    // TADS 2 does not use UTF-8; use the encoding from our settings.
    QTextCodec* codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);
    return QString::localeAwareCompare(
        codec->toUnicode(s1).toLower(), codec->toUnicode(s2).toLower());
}

auto strnicmp(const char* s1, const char* s2, size_t n) -> int
{
    QString qs1;
    QString qs2;

    if (qFrame->tads3()) {
        qs1 = QString::fromUtf8(s1);
        qs2 = QString::fromUtf8(s2);
    } else {
        // TADS 2 does not use UTF-8; use the encoding from our settings.
        QTextCodec* codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);
        qs1 = codec->toUnicode(s1);
        qs2 = codec->toUnicode(s2);
    }

    qs1.truncate(n);
    qs2.truncate(n);
    return QString::compare(qs1.toLower(), qs2.toLower());
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
