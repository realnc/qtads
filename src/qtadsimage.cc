// This is copyrighted software. More information is at the end of this file.
#include <QPainter>

#include "qtadsimage.h"
#include "settings.h"
#include "syswin.h"

void QTadsImage::drawFromPaintEvent(class CHtmlSysWin* win, class CHtmlRect* pos,
                                    htmlimg_draw_mode_t mode)
{
    QPainter painter(static_cast<CHtmlSysWinQt*>(win)->widget());
    if (mode == HTMLIMG_DRAW_CLIP) {
        // Clip mode.  Only draw the part of the image that would fit.  If the
        // image is smaller than pos, adjust the drawing area to avoid scaling.
        int targetWidth;
        int targetHeight;
        if (width() > pos->right - pos->left) {
            targetWidth = pos->right - pos->left;
        } else {
            targetWidth = width();
        }
        if (height() > pos->bottom - pos->top) {
            targetHeight = pos->bottom - pos->top;
        } else {
            targetHeight = height();
        }
        painter.drawImage(pos->left, pos->top, *this, 0, 0, targetWidth, targetHeight);
        return;
    }

    if (mode == HTMLIMG_DRAW_STRETCH) {
        // If the image doesn't fit exactly, scale it. Use the "smooth"
        // transformation mode (which uses a bilinear filter) if enabled in
        // the settings.
        Qt::TransformationMode mode = qFrame->settings()->useSmoothScaling
                                          ? Qt::SmoothTransformation
                                          : Qt::FastTransformation;
        if (width() != pos->right - pos->left or height() != pos->bottom - pos->top) {
            painter.drawImage(QPoint(pos->left, pos->top),
                              scaled(pos->right - pos->left, pos->bottom - pos->top,
                                     Qt::IgnoreAspectRatio, mode));
        } else {
            painter.drawImage(QPoint(pos->left, pos->top), *this);
        }
        return;
    }

    // If we get here, 'mode' must have been HTMLIMG_DRAW_TILE.
    Q_ASSERT(mode == HTMLIMG_DRAW_TILE);
    QPixmap pix(QPixmap::fromImage(*this));
    painter.drawTiledPixmap(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
                            pix);
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
