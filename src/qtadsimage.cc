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

#include <QPainter>

#include "htmlqt.h"
#include "qtadsimage.h"

void
QTadsImage::drawFromPaintEvent( class CHtmlSysWin* win, class CHtmlRect* pos, htmlimg_draw_mode_t mode )
{
	QPainter painter(static_cast<CHtmlSysWinQt*>(win)->widget());
	if (mode == HTMLIMG_DRAW_CLIP) {
		// Clip mode.  Only draw the part of the image that would fit.  If the
		// image is smaller than pos, adjust the drawing area to avoid scaling.
		int targetWidth;
		int targetHeight;
		if (this->width() > pos->right - pos->left) {
			targetWidth = pos->right - pos->left;
		} else {
			targetWidth = this->width();
		}
		if (this->height() > pos->bottom - pos->top) {
			targetHeight = pos->bottom - pos->top;
		} else {
			targetHeight = this->height();
		}
		painter.drawImage(pos->left, pos->top, *this, 0, 0, targetWidth, targetHeight);
	} else {
		// QPainter will scale it by default.
		painter.drawImage(QRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top), *this);
	}
}
