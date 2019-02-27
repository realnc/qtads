/* Copyright (C) 2013 Nikos Chantziaras.
 *
 * This file is part of the QTads program.  This program is free software; you
 * can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING.  If not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */
#ifndef SYSIMAGEPNG_H
#define SYSIMAGEPNG_H

#include "config.h"
#include "qtadsimage.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysImagePngQt: public QTadsImage, public CHtmlSysImagePng
{
public:
    //
    // CHtmlSysImagePng interface implementation.
    //
    void draw_image(CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode) override
    {
        QTadsImage::drawFromPaintEvent(win, pos, mode);
    }

    unsigned long get_width() const override
    {
        return QTadsImage::width();
    }

    unsigned long get_height() const override
    {
        return QTadsImage::height();
    }

    int map_palette(CHtmlSysWin*, int) override
    {
        return false;
    }
};

#endif
