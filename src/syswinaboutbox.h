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
#ifndef SYSWINABOUTBOX_H
#define SYSWINABOUTBOX_H

#include "syswin.h"


/* We need special handling for the "About this game" box.
 */
class CHtmlSysWinAboutBoxQt: public CHtmlSysWinQt {
	Q_OBJECT

protected:
	virtual void
	keyPressEvent( QKeyEvent* e )
	// It shouldn't be possible to do game input from the about box, so we
	// bypass the inherited input handling and revert to the default.
	{ QScrollArea::keyPressEvent(e); }

	virtual void
	resizeEvent( QResizeEvent* e );

	virtual QSize
	sizeHint() const;

  public:
	CHtmlSysWinAboutBoxQt( class CHtmlFormatter* formatter, QWidget* parent );

	// We have scrollbars always disabled, so we can report our own
	// width/height rather than our viewport's.  We need to do that because
	// the formatter needs to know our size before we become visible, and our
	// viewport only reports a valid size after show() is called.
	virtual long
	get_disp_width()
	{ return this->width(); }

	virtual long
	get_disp_height()
	{ return this->height(); }

	virtual void
	set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width,
					 long height, HTML_BannerWin_Units_t height_units, int use_height );
};


#endif
