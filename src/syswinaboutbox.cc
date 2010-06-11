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
#include "htmlfmt.h"

#include "syswinaboutbox.h"
#include "qtadsdispwidget.h"


CHtmlSysWinAboutBoxQt::CHtmlSysWinAboutBoxQt( class CHtmlFormatter* formatter, QWidget* parent )
  : CHtmlSysWinQt(formatter, 0, parent)
{
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setMinimumSize(200,140);
}


void
CHtmlSysWinAboutBoxQt::resizeEvent( QResizeEvent* e )
{
	this->formatter_->start_at_top(false);
	this->do_formatting(true, false, true);
	QScrollArea::resizeEvent(e);
}


QSize
CHtmlSysWinAboutBoxQt::sizeHint() const
{
	// Ensure that we're always large enough to show the whole contents of the
	// "about" content.
	return this->displayWidget()->size();
}


void
CHtmlSysWinAboutBoxQt::set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width,
										long height, HTML_BannerWin_Units_t height_units, int use_height )
{
	this->bannerSize = height;
	this->bannerSizeUnits = height_units;
	this->dispWidget->resize(width + this->margins.left + this->margins.right,
							 height + this->margins.top + this->margins.bottom);
	QRect rec(this->geometry());
	this->calcChildBannerSizes(rec);
	this->adjustSize();
}
