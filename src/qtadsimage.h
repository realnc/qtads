// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QImage>

#include "htmlsys.h"

/* We handle all types of images the same way, so we implement that handling
 * in this class and derive the various CHtmlSysImage* classes from this one.
 */
class QTadsImage: public QImage
{
public:
    QTadsImage() = default;

    QTadsImage(const QImage& qImg)
        : QImage(qImg)
    { }

    // A call to this method is only allowed to happen from inside
    // QTadsDisplayWidget::paintEvent().  This always happens indirectly
    // through CHtmlFormatter::draw(), which QTadsDisplayWidget::painEvent() is
    // using to repaint the window.
    void drawFromPaintEvent(CHtmlSysWin* win, class CHtmlRect* pos, htmlimg_draw_mode_t mode);
};

/* Helper routine.  Loads any type of image from the specified offset inside
 * the given file and returns it.  Has the same semantics as the various
 * CHtmlSysImage*::create_*() routines.  The image type is specified in
 * 'imageType'.  It has the same format as the list returned by
 * QImageReader::supportedImageFormats() (like "JPG", "PNG", etc.)
 */
auto createImageFromFile(
    const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos, unsigned long filesize,
    CHtmlSysWin* win, const QString& imageType) -> CHtmlSysResource*;

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
