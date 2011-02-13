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

#include "syswin.h"
#include "qtadsimage.h"
#include "settings.h"


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
		return;
	}

	if (mode == HTMLIMG_DRAW_STRETCH) {
		// If the image doesn't fit exactly, scale it. Use the "smooth"
		// transformation mode (which uses a bilinear filter) if enabled in
		// the settings.
		Qt::TransformationMode mode = qFrame->settings()->useSmoothScaling ?
									  Qt::SmoothTransformation : Qt::FastTransformation;
		if (this->width() != pos->right - pos->left or this->height() != pos->bottom - pos->top) {
			painter.drawImage(QPoint(pos->left, pos->top),
							  this->scaled(pos->right - pos->left, pos->bottom - pos->top,
										   Qt::IgnoreAspectRatio, mode));
		} else {
			painter.drawImage(QPoint(pos->left, pos->top), *this);
		}
		return;
	}

	// If we get here, 'mode' must have been HTMLIMG_DRAW_TILE.
	Q_ASSERT(mode == HTMLIMG_DRAW_TILE);
	QPixmap pix(QPixmap::fromImage(*this));
	painter.drawTiledPixmap(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top, pix);
}
