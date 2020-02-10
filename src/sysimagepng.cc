// This is copyrighted software. More information is at the end of this file.
#include "sysimagepng.h"

auto CHtmlSysImagePng::create_png(
    const CHtmlUrl* const url, const textchar_t* const filename, const unsigned long seekpos,
    const unsigned long filesize, CHtmlSysWin* const win) -> CHtmlSysResource*
{
    return createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("PNG"));
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
