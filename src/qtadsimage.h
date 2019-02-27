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
#ifndef QTADSIMAGE_H
#define QTADSIMAGE_H

#include <QImage>

#include "htmlsys.h"

/* We handle all types of images the same way, so we implement that handling
 * in this class and derive the various CHtmlSysImage* classes from this one.
 */
class QTadsImage: public QImage
{
public:
    QTadsImage()
    {}

    QTadsImage(const QImage& qImg)
        : QImage(qImg)
    {}

    // A call to this method is only allowed to happen from inside
    // QTadsDisplayWidget::paintEvent().  This always happens indirectly
    // through CHtmlFormatter::draw(), which QTadsDisplayWidget::painEvent() is
    // using to repaint the window.
    void drawFromPaintEvent(CHtmlSysWin* win, class CHtmlRect* pos, htmlimg_draw_mode_t mode);
};

#endif
