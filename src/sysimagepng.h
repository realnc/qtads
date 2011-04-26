/* Copyright (C) 2010 Nikos Chantziaras.
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
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef SYSIMAGEPNG_H
#define SYSIMAGEPNG_H

#include "qtadsimage.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysImagePngQt: public QTadsImage, public CHtmlSysImagePng {
  public:
    //
    // CHtmlSysImagePng interface implementation.
    //
    virtual void
    draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
    { QTadsImage::drawFromPaintEvent(win, pos, mode); }

    virtual unsigned long
    get_width() const
    { return QTadsImage::width(); }

    virtual unsigned long
    get_height() const
    { return QTadsImage::height(); }

    virtual int
    map_palette( CHtmlSysWin* win, int foreground )
    { return false; }
};


#endif
