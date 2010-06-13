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

#include <QBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>

#include "dispwidget.h"
#include "qtadstimer.h"
#include "settings.h"
#include "syswininput.h"
#include "sysimagejpeg.h"
#include "sysimagepng.h"

#include "htmlfmt.h"
#include "htmlrc.h"


CHtmlSysWinQt::CHtmlSysWinQt( CHtmlFormatter* formatter, DisplayWidget* dispWidget, QWidget* parent )
  : QScrollArea(parent), CHtmlSysWin(formatter), fBannerStyleAutoVScroll(true), fDontReformat(0),
	fParentBanner(0), fBgImage(0), margins(8, 2, 8, 2), bannerSize(0), bannerSizeUnits(HTML_BANNERWIN_UNITS_PIX)
{
	if (dispWidget == 0) {
		this->dispWidget = new DisplayWidget(this, formatter);
		this->setWidget(this->dispWidget);
	}
	this->formatter_->set_win(this, &margins);
	this->viewport()->setForegroundRole(QPalette::Text);
	this->viewport()->setBackgroundRole(QPalette::Base);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	this->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	this->setLineWidth(0);
	this->setContentsMargins(0, 0, 0, 0);

	QPalette p(this->palette());
	p.setColor(QPalette::Base, qFrame->settings()->bannerBgColor);
	p.setColor(QPalette::Text, qFrame->settings()->bannerTextColor);
	this->setPalette(p);

	// TEMP: Just to make the area of this widget visibly more apparent.
	//this->viewport()->setBackgroundRole(QPalette::ToolTipBase);
}


CHtmlSysWinQt::~CHtmlSysWinQt()
{
	//qDebug() << Q_FUNC_INFO;

	// Remove my reference on the background image if I have one.
	if (this->fBgImage != 0) {
		this->fBgImage->remove_ref();
	}

	// Don't allow the formatter to reference us anymore, since
	// we're about to be deleted.
	this->formatter_->unset_win();

	// Remove ourselves from our parent banner's child list.
	if (this->fParentBanner != 0) {
		this->fParentBanner->fChildBanners.removeAll(this);
	}

	// Remove all references to us from our child banners.
	for (int i = 0; i < this->fChildBanners.size(); ++i) {
		Q_ASSERT(this->fChildBanners.at(i)->fParentBanner == this);
		this->fChildBanners.at(i)->fParentBanner = 0;
	}
}


void
CHtmlSysWinQt::keyPressEvent( QKeyEvent* event )
{
	qFrame->gameWindow()->keyPressEvent(event);
}


void
CHtmlSysWinQt::calcChildBannerSizes( QRect& parentSize )
{
	//qDebug() << Q_FUNC_INFO;
	++this->fDontReformat;

	QRect newSize;
	QRect oldSize;

	// Get our current size.
	oldSize = this->geometry();

	// Start off assuming we'll take the entire parent area.  If we're a
	// top-level window, we'll leave it exactly like that.  If we're a banner,
	// we'll be aligned on three sides with the parent area, since we always
	// carve out a chunk by splitting the parent area; we'll adjust the fourth
	// side according to our banner size and alignment specifications.
	newSize = parentSize;

	// If we're a banner window, carve out our own area from the parent window;
	// otherwise, we must be a top-level window, so take the entire available
	// space for ourselves.
	if (this != qFrame->gameWindow()) {
		// Calculate our current on-screen size.
		int wid = oldSize.width();
		int ht = oldSize.height();

		// Convert the width units to pixels.
		switch (this->bannerSizeUnits) {
		  case HTML_BANNERWIN_UNITS_PIX:
			// Pixels - use the stored width directly.
			wid = this->bannerSize;
			break;

		  case HTML_BANNERWIN_UNITS_CHARS:
			// Character cells - calculate the size in terms of the width of a
			// "0" character in the window's default font.
			wid = this->bannerSize * measure_text(get_default_font(), "0", 1, 0).x;
			break;

		  case HTML_BANNERWIN_UNITS_PCT:
			// Percentage - calculate the width as a percentage of the parent
			// size.
			wid = (this->bannerSize * parentSize.width()) / 100;
			break;
		}

		// Convert the height units to pixels.
		switch (this->bannerSizeUnits) {
		  case HTML_BANNERWIN_UNITS_PIX:
			// Pixels - use the stored height directly.
			ht = this->bannerSize;
			break;

		  case HTML_BANNERWIN_UNITS_CHARS:
			// Character cells - calculate the size in terms of the height of a
			// "0" character in the window's default font.
			ht = this->bannerSize * measure_text(get_default_font(), "0", 1, 0).y;
			break;

		  case HTML_BANNERWIN_UNITS_PCT:
			// Percentage - calculate the size as a percentage of the parent
			// size.
			ht = (this->bannerSize * parentSize.height()) / 100;
			break;
		}

		// Make sure that the banner doesn't exceed the available area.
		if (wid > parentSize.width()) {
			wid = parentSize.width();
		}
		if (ht > parentSize.height()) {
			ht = parentSize.height();
		}

		// Position the banner according to our alignment type.
		switch(this->fBannerPos) {
		case HTML_BANNERWIN_POS_TOP:
			// Align the banner at the top of the window.
			newSize.setBottom(newSize.top() + ht - 1);

			// Take the space out of the top of the parent window.
			parentSize.setTop(parentSize.top() + ht);
			break;

		case HTML_BANNERWIN_POS_BOTTOM:
			// Align the banner at the bottom of the window.
			newSize.setTop((newSize.top() + newSize.height()) - ht + 1);

			// Take the space out of the bottom of the parent area.
			parentSize.setBottom((parentSize.top() + parentSize.height()) - ht);
			break;

		case HTML_BANNERWIN_POS_LEFT:
			// Align the banner at the left of the window.
			newSize.setRight(newSize.left() + wid - 1);

			// Take the space from the left of the parent window.
			parentSize.setLeft(parentSize.left() + wid);
			break;

		case HTML_BANNERWIN_POS_RIGHT:
			// Align the banner at the right of the window.
			newSize.setLeft(newSize.left() + newSize.width() - wid + 1);

			// Take the space from the right of the parent window.
			parentSize.setRight(parentSize.left() + parentSize.width() - wid);
			break;
		}
	}

	// Now that we know our own full area, lay out our banner children.  This
	// will update newSize to reflect our actual size after deducting space for
	// our children.
	for (int i = 0; i < this->fChildBanners.size(); ++i) {
		this->fChildBanners.at(i)->calcChildBannerSizes(newSize);
	}

	// Set our new window position if it differs from the old one.
	if (newSize != oldSize) {
		this->setGeometry(newSize);
		// Since we changed size, we will need a reformat.
		qFrame->scheduleReformat();
	}

	qFrame->gameWindow()->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
	--this->fDontReformat;
}


void
CHtmlSysWinQt::addBanner( CHtmlSysWinQt* banner, int where, CHtmlSysWinQt* other, HTML_BannerWin_Pos_t pos,
						  unsigned long style )
{
	Q_ASSERT(banner->fParentBanner == 0);

	// Enable/disable the scrollbars, according to what was requested.
	if (style & OS_BANNER_STYLE_VSCROLL) {
		banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		banner->fBannerStyleVScroll = true;
	} else {
		banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		banner->fBannerStyleVScroll = false;
	}
	if (style & OS_BANNER_STYLE_HSCROLL) {
		banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		banner->fBannerStyleHScroll = true;
	} else {
		banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		banner->fBannerStyleHScroll = true;
	}

	banner->fParentBanner = this;
	banner->fBannerPos = pos;
	banner->fBannerWhere = where;
	banner->fBannerStyleAutoVScroll = style & OS_BANNER_STYLE_AUTO_VSCROLL;

	banner->setGeometry(this->geometry());
	// Qt will not update the viewport() geometry unless the widget is shown
	// first.
	banner->show();
	banner->hide();

	// Add the banner to our child list.
	Q_ASSERT(not this->fChildBanners.contains(banner));
	switch (where) {
	  case OS_BANNER_FIRST:
		this->fChildBanners.prepend(banner);
		break;
	  case OS_BANNER_LAST:
		this->fChildBanners.append(banner);
		break;
	  case OS_BANNER_BEFORE:
		Q_ASSERT(this->fChildBanners.contains(other));
		this->fChildBanners.insert(this->fChildBanners.indexOf(other), banner);
		break;
	  default:
		Q_ASSERT(where == OS_BANNER_AFTER);
		Q_ASSERT(this->fChildBanners.contains(other));
		this->fChildBanners.insert(this->fChildBanners.indexOf(other) + 1, banner);
	}
}


void
CHtmlSysWinQt::notify_clear_contents()
{
	//qDebug() << Q_FUNC_INFO;

	// Tell our display widget about it.
	this->dispWidget->notifyClearContents();
}


int
CHtmlSysWinQt::close_window( int force )
{
	qDebug() << Q_FUNC_INFO;

	return force ? true : false;
}


CHtmlPoint
CHtmlSysWinQt::measure_text( CHtmlSysFont* font, const textchar_t* str, size_t len, int* ascent )
{
	//qDebug() << Q_FUNC_INFO;

	const QFontMetrics& tmpMetr = QFontMetrics(*static_cast<CHtmlSysFontQt*>(font));
	if (ascent != 0) {
		*ascent = tmpMetr.ascent();
	}
	// We don't return the actual width of the text, but the distance to where
	// subsequent text should be drawn.  This is really what our caller needs
	// to know, otherwise letters will start jumping left and right when
	// selecting text or moving the text cursor.
	return CHtmlPoint(tmpMetr.width(QString::fromUtf8(str, len)), tmpMetr.height());
}


size_t
CHtmlSysWinQt::get_max_chars_in_width( CHtmlSysFont* font, const textchar_t* str, size_t len, long wid )
{
	// We do a binary search on the results of measure_text() until we find an
	// amount of characters that fit.
	// FIXME: This won't really work with UTF-8 characters.
	int first = 1;
	int last = len;
	long mid;
	while (first <= last) {
		// New mid point.
		mid = (first + last) / 2;
		long bestFit = this->measure_text(font, str, mid, 0).x;
		if (bestFit < wid) {
			first = mid + 1;
		} else if (bestFit > wid) {
			last = mid - 1;
		} else {
			// Exact match.
			return mid;
		}
	}
	// We didn't find an exact match, which means one less than the result we
	// got will fit.
	return mid - 1;
}


void
CHtmlSysWinQt::draw_text( int hilite, long x, long y, CHtmlSysFont* font, const textchar_t* str, size_t len )
{
	QPainter painter(this->dispWidget);
	const CHtmlSysFontQt& fontCast = *static_cast<CHtmlSysFontQt*>(font);
	painter.setFont(fontCast);

	if (fontCast.use_font_color()) {
		// The font has its own color; use it.
		HTML_color_t color = fontCast.get_font_color();
		painter.setPen(QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
	} else if (font->get_font_color() == HTML_COLOR_INPUT) {
		painter.setPen(qFrame->inputColor());
	}

	if (hilite) {
		painter.setBackgroundMode(Qt::OpaqueMode);
		painter.setBackground(QApplication::palette().highlight());
		painter.setPen(QApplication::palette().color(QPalette::HighlightedText));
	}

	if (fontCast.use_font_bgcolor()) {
		painter.setBackgroundMode(Qt::OpaqueMode);
		HTML_color_t color = fontCast.get_font_bgcolor();
		painter.setBackground(QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
	}

	painter.drawText(x, y + QFontMetrics(fontCast).ascent(), QString::fromUtf8(str, len));
}


void
CHtmlSysWinQt::draw_text_space( int hilite, long x, long y, CHtmlSysFont* font, long wid )
{
	//qDebug() << Q_FUNC_INFO;
	/*
	QByteArray str;
	str.append('_');
	const QFontMetrics& tmpMetr = QFontMetrics(*static_cast<CHtmlSysFontQt*>(font));
	long res = this->measure_text(font, str.constData(), str.size(), 0).x;
	qDebug() << "--- til now:" << str;
	while (res < wid*wid) {
		str.append('_');
		res = tmpMetr.boundingRect(str).width();
		qDebug() << "--- til now:" << str;
	}
	//this->draw_text(hilite, x, y, font, str.constData(), str.size());
	this->draw_text(hilite, x, y, font, "XXXXXXXXX", 9);
	*/
}


void
CHtmlSysWinQt::draw_bullet( int hilite, long x, long y, CHtmlSysFont* font, HTML_SysWin_Bullet_t style )
{
	int unicode;
	switch (style) {
	  case HTML_SYSWIN_BULLET_SQUARE:
		unicode = 0x25AA;
		break;
	  case HTML_SYSWIN_BULLET_CIRCLE:
		unicode = 0x25CB;
		break;
	  case HTML_SYSWIN_BULLET_DISC:
		unicode = 0x2022;
		break;
	  default:
		Q_ASSERT(style == HTML_SYSWIN_BULLET_PLAIN);
		// Nothing to draw.
		return;
	}
	const QByteArray& utf8 = QString(QChar(unicode)).toUtf8();
	this->draw_text(hilite, x, y, font, utf8.constData(), utf8.size());
}


void
CHtmlSysWinQt::draw_hrule( const CHtmlRect* pos, int shade )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(pos != 0);

	QPainter painter(this->dispWidget);
	if (shade) {
		if (pos->bottom - pos->top > 2) {
			qDrawShadePanel(&painter, pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
							this->palette(), true, 1, 0);
		} else {
			qDrawShadeLine(&painter, pos->left, pos->top, pos->right, pos->top, this->palette(), true, 1, 0);
		}
	} else {
		painter.fillRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
						 QApplication::palette().color(QPalette::Dark));
	}
}


void
CHtmlSysWinQt::draw_table_border( const CHtmlRect* pos, int width, int cell )
{
	//qDebug() << Q_FUNC_INFO;

	QPainter pnt(this->dispWidget);
	QPalette pal;
	// Use Midlight for Light to closer match Windows HTML TADS appearance.
	pal.setColor(QPalette::Light, QApplication::palette().color(QPalette::Midlight));
	qDrawShadePanel(&pnt, pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top, pal, cell, width);
}


void
CHtmlSysWinQt::draw_table_bkg( const CHtmlRect* pos, HTML_color_t bgcolor )
{
	//qDebug() << Q_FUNC_INFO;

	QPainter painter(this->dispWidget);
	int red = HTML_color_red(bgcolor);
	int green = HTML_color_green(bgcolor);
	int blue = HTML_color_blue(bgcolor);
	painter.fillRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
					 QColor(red, green, blue));
}


void
CHtmlSysWinQt::draw_dbgsrc_icon( const CHtmlRect* pos, unsigned int stat )
{
	qDebug() << Q_FUNC_INFO;
}


int
CHtmlSysWinQt::do_formatting( int /*show_status*/, int update_win, int freeze_display )
{
	//qDebug() << Q_FUNC_INFO;

	if (this->fDontReformat != 0) {
		return false;
	}

	// Freeze the display, if requested.
	if (freeze_display) {
		this->formatter_->freeze_display(true);
	}

	// Format all remaining lines.
	while (this->formatter_->more_to_do()) {
		this->formatter_->do_formatting();
		this->dispWidget->resize(this->dispWidget->width(), this->formatter_->get_max_y_pos());
		if (update_win) {
			if (this->fBannerStyleAutoVScroll) {
				this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
			}
			qFrame->advanceEventLoop();
		}
	}

	// We make the view a bit higher (5 pixels) than required by the real
	// document height so that we get a bit of extra space under the input
	// prompt because it looks nicer.  This is only done if we're the main game
	// window.
	//
	// FIXME: Disable it for now until we fix the "jumping text" problem.
	/*
	unsigned long height = this->formatter_->get_max_y_pos();
	if (this == qFrame->gameWindow()) {
		height += 5;
	}
	this->fDispWidget->resize(this->formatter_->get_outer_max_line_width(), height);
	*/

	if (this->fBannerStyleAutoVScroll) {
		this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
	}

	if (update_win) {
		this->dispWidget->update();
	}

	// Unfreeze the display if we froze it before.
	if (freeze_display) {
		this->formatter_->freeze_display(false);
	}
	return true;
}


CHtmlSysFont*
CHtmlSysWinQt::get_default_font()
{
	//qDebug() << Q_FUNC_INFO;

	CHtmlFontDesc desc;
	qFrame->settings()->mainFont.get_font_desc(&desc);
	desc.htmlsize = 3;
	return get_font(&desc);
}


void
CHtmlSysWinQt::register_timer_func( void (*timer_func)(void*), void* func_ctx )
{
	QTadsTimer* timer = new QTadsTimer(timer_func, func_ctx, this);
	timer->setSingleShot(false);
	timer->set_repeating(true);
	timer->set_active(true);
	timer->start(1000);
	this->fTimerList.append(timer);
}


void
CHtmlSysWinQt::unregister_timer_func( void (*timer_func)( void * ), void* func_ctx )
{
	for (int i = 0; i < this->fTimerList.size(); ++i) {
		if (this->fTimerList.at(i)->func_ == timer_func) {
			delete this->fTimerList.takeAt(i);
			return;
		}
	}
}


CHtmlSysTimer*
CHtmlSysWinQt::create_timer( void (*timer_func)( void* ), void *func_ctx )
{
	qDebug() << Q_FUNC_INFO;

	return new QTadsTimer(timer_func, func_ctx, this);
}


void
CHtmlSysWinQt::set_timer( CHtmlSysTimer* timer, long interval_ms, int repeat )
{
	qDebug() << Q_FUNC_INFO;

	static_cast<QTadsTimer*>(timer)->setSingleShot(not repeat);
	static_cast<QTadsTimer*>(timer)->set_repeating(repeat);
	static_cast<QTadsTimer*>(timer)->start(interval_ms);
}


void
CHtmlSysWinQt::cancel_timer( CHtmlSysTimer* timer )
{
	qDebug() << Q_FUNC_INFO;

	static_cast<QTadsTimer*>(timer)->stop();
}


void
CHtmlSysWinQt::delete_timer( CHtmlSysTimer* timer )
{
	qDebug() << Q_FUNC_INFO;

	delete timer;
}


void
CHtmlSysWinQt::fmt_adjust_hscroll()
{
	//qDebug() << Q_FUNC_INFO;

	if (this->formatter_->get_outer_max_line_width() > this->dispWidget->width()) {
		this->dispWidget->resize(this->formatter_->get_outer_max_line_width(), this->dispWidget->height());
	}
}


void
CHtmlSysWinQt::fmt_adjust_vscroll()
{
	//qDebug() << Q_FUNC_INFO;

	if (this->formatter_->get_max_y_pos() > static_cast<unsigned long>(this->dispWidget->height())) {
		this->dispWidget->resize(this->dispWidget->width(), this->formatter_->get_max_y_pos());
	}
}


void
CHtmlSysWinQt::inval_doc_coords( const CHtmlRect* area )
{
	//qDebug() << Q_FUNC_INFO;

	//qDebug() << "Invalidating area" << area->left << area->top << area->right << area->bottom;

	long width = area->right == HTMLSYSWIN_MAX_RIGHT ?
				 this->dispWidget->width() - area->left
				 : area->right - area->left;
	long height = area->bottom == HTMLSYSWIN_MAX_BOTTOM ?
				  this->dispWidget->height() - area->top
				  : area->bottom - area->top;

	this->dispWidget->update(QRect(area->left, area->top, width, height));
}


void
CHtmlSysWinQt::advise_clearing_disp_list()
{
	//qDebug() << Q_FUNC_INFO;
	this->dispWidget->notifyClearContents();
}


void
CHtmlSysWinQt::scroll_to_doc_coords( const CHtmlRect* pos )
{
	qDebug() << Q_FUNC_INFO;

	//qFrame->advanceEventLoop();
}


void
CHtmlSysWinQt::get_scroll_doc_coords( CHtmlRect* pos )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::set_window_title( const textchar_t* title )
{
	//qDebug() << Q_FUNC_INFO;

	qWinGroup->setWindowTitle(QString::fromUtf8(title));
}


void
CHtmlSysWinQt::set_html_bg_color( HTML_color_t color, int use_default )
{
	//qDebug() << Q_FUNC_INFO;

	// If we have an active background image, ignore the call.
	if (this->fBgImage != 0) {
		return;
	}

	if (use_default) {
		QPalette p(this->palette());
		p.setColor(QPalette::Base, qFrame->settings()->mainBgColor);
		this->setPalette(p);
		return;
	}

	color = this->map_system_color(color);
	QPalette p(this->palette());
	p.setColor(QPalette::Base, QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
	this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_text_color( HTML_color_t color, int use_default )
{
	//qDebug() << Q_FUNC_INFO;

	if (use_default) {
		QPalette p(this->palette());
		if (this == qFrame->gameWindow()) {
			p.setColor(QPalette::Text, qFrame->settings()->mainTextColor);
		} else {
			p.setColor(QPalette::Text, qFrame->settings()->bannerTextColor);
		}
		this->setPalette(p);
		return;
	}

	color = this->map_system_color(color);
	QPalette p(this->palette());
	p.setColor(QPalette::Text, QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
	this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_link_colors( HTML_color_t link_color, int link_use_default,
					 HTML_color_t vlink_color, int vlink_use_default,
					 HTML_color_t alink_color, int alink_use_default,
					 HTML_color_t hlink_color, int hlink_use_default )
{
	//qDebug() << Q_FUNC_INFO;
}


HTML_color_t
CHtmlSysWinQt::map_system_color( HTML_color_t color )
{
	//qDebug() << Q_FUNC_INFO;

	// If it's just an RGB value, return it unchanged.
	if (not html_is_color_special(color)) {
		return color;
	}

	QColor col;

	switch (color) {
	  case HTML_COLOR_STATUSBG:
		col = qFrame->settings()->bannerBgColor;
		break;

	  case HTML_COLOR_STATUSTEXT:
		col = qFrame->settings()->bannerTextColor;
		break;

	  case HTML_COLOR_LINK:
		col = qFrame->settings()->unvisitedLinkColor;
		break;

	  case HTML_COLOR_ALINK:
		col = qFrame->settings()->clickedLinkColor;
		break;

	  case HTML_COLOR_TEXT:
		col = qFrame->settings()->mainTextColor;
		break;

	  case HTML_COLOR_BGCOLOR:
		col = qFrame->settings()->mainBgColor;
		break;

	  case HTML_COLOR_INPUT:
		col = qFrame->settings()->inputColor;
		break;

	  case HTML_COLOR_HLINK:
		col = qFrame->settings()->hoveringLinkColor;
		break;

	  default:
		// Return black for everything else.
		return 0;
	}

	return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_link_color() const
{
	//qDebug() << Q_FUNC_INFO;

	const QColor& col = qFrame->settings()->unvisitedLinkColor;
	return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_alink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	const QColor& col = qFrame->settings()->clickedLinkColor;
	return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_vlink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	// Visited links aren't really implemented by the HTML base code.
	return this->get_html_link_color();
}


HTML_color_t
CHtmlSysWinQt::get_html_hlink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	const QColor& col = qFrame->settings()->hoveringLinkColor;
	return HTML_make_color(col.red(), col.green(), col.blue());
}


int
CHtmlSysWinQt::get_html_link_underline() const
{
	//qDebug() << Q_FUNC_INFO;

	return qFrame->settings()->underlineLinks;
}


int
CHtmlSysWinQt::get_html_show_links() const
{
	//qDebug() << Q_FUNC_INFO;

	return qFrame->settings()->enableLinks;
}


int
CHtmlSysWinQt::get_html_show_graphics() const
{
	//qDebug() << Q_FUNC_INFO;

	return qFrame->settings()->enableGraphics;
}


void
CHtmlSysWinQt::set_html_bg_image( CHtmlResCacheObject* image )
{
	//qDebug() << Q_FUNC_INFO;

	if (image == 0 or image->get_image() == 0) {
		// No image specified; forget the current image if we have one and
		// restore the default background color.
		if (this->fBgImage != 0) {
			this->fBgImage->remove_ref();
			this->fBgImage = 0;
		}
		QPalette p(this->palette());
		p.setColor(QPalette::Base, qFrame->settings()->mainBgColor);
		this->setPalette(p);
		return;
	}

	QTadsImage* castImg;
	switch (image->get_image()->get_res_type()) {
	  case HTML_res_type_JPEG:
		castImg = static_cast<CHtmlSysImageJpegQt*>(image->get_image());
		break;
	  case HTML_res_type_PNG:
		castImg = static_cast<CHtmlSysImagePngQt*>(image->get_image());
		break;
	  case HTML_res_type_MNG:
		// FIXME: Handle MNG backgrounds.
		//castImg = static_cast<CHtmlSysImageMngQt*>(image->get_image());
		return;
	  default:
		qWarning() << Q_FUNC_INFO << "Unknown resource type";
		castImg = dynamic_cast<QTadsImage*>(image->get_image());
	}

	QPalette p(this->palette());
	p.setBrush(QPalette::Base, *castImg);
	this->setPalette(p);
	image->add_ref();
	this->fBgImage = image;
}


void
CHtmlSysWinQt::inval_html_bg_image( unsigned int x, unsigned int y, unsigned int wid, unsigned int ht )
{
	//qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width,
								long height, HTML_BannerWin_Units_t height_units, int use_height )
{
	//qDebug() << Q_FUNC_INFO;

	if (this == qFrame->gameWindow()) {
		// We're the main game window.  Ignore the call.
		return;
	}

	if (this->fBannerPos == HTML_BANNERWIN_POS_TOP or this->fBannerPos == HTML_BANNERWIN_POS_BOTTOM) {
		if (not use_height) {
			return;
		}
		this->bannerSize = height;
		this->bannerSizeUnits = height_units;
	} else {
		Q_ASSERT(this->fBannerPos == HTML_BANNERWIN_POS_LEFT or this->fBannerPos == HTML_BANNERWIN_POS_RIGHT);
		if (not use_width) {
			return;
		}
		this->bannerSize = width;
		this->bannerSizeUnits = width_units;
	}
	qFrame->adjustBannerSizes();
	if (not this->isVisible()) {
		this->show();
	}
	return;
}


void
CHtmlSysWinQt::set_banner_info( HTML_BannerWin_Pos_t pos, unsigned long style )
{
	//qDebug() << Q_FUNC_INFO;

	// Set the new auto-vscroll flag.
	this->fBannerStyleAutoVScroll = style & OS_BANNER_STYLE_AUTO_VSCROLL;

	// Set the new alignemnt.
	this->fBannerPos = pos;

	// FIXME: ignore the scrollbar changes (for now, anyway)
}


void
CHtmlSysWinQt::get_banner_info( HTML_BannerWin_Pos_t* pos, unsigned long* style )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(pos != 0);
	Q_ASSERT(style != 0);

	*pos = this->fBannerPos;

	// Set the style flags.
	*style = 0;
	if (this->fBannerStyleAutoVScroll) {
		*style |= OS_BANNER_STYLE_AUTO_VSCROLL;
	}

	// We provide full HTML interpretation, so we support <TAB>.
	*style |= OS_BANNER_STYLE_TAB_ALIGN;
}
