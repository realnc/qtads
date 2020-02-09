// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "qtadsimage.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysImageJpegQt: public QTadsImage, public CHtmlSysImageJpeg
{
public:
    //
    // CHtmlSysImageJpeg interface implementation.
    //
    void draw_image(CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode) override
    {
        QTadsImage::drawFromPaintEvent(win, pos, mode);
    }

    auto get_width() const -> unsigned long override
    {
        return QTadsImage::width();
    }

    auto get_height() const -> unsigned long override
    {
        return QTadsImage::height();
    }

    auto map_palette(CHtmlSysWin*, int) -> int override
    {
        return false;
    }
};

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
