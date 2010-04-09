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

#include "htmlqt.h"
#include "qtadsdispwidget.h"
#include "qtadstimer.h"
#include "qtadssettings.h"

#include "htmlfmt.h"
#include "htmlrc.h"


CHtmlSysWinQt::CHtmlSysWinQt( CHtmlFormatter* formatter, QWidget* parent )
: QScrollArea(parent), CHtmlSysWin(formatter), fDontReformat(false), fVFrame(0), fHFrame(0), fVLine(0), fHLine(0),
  fVLayout(0), fHLayout(0), fMargins(5, 2, 5, 2), fBgImage(0), fDispWidget(new QTadsDisplayWidget(this))
{
	this->formatter_->set_win(this, &fMargins);
	this->setForegroundRole(QPalette::Text);
	this->setBackgroundRole(QPalette::Base);
	this->setWidget(this->fDispWidget);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	this->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	this->setLineWidth(0);
	this->setContentsMargins(0, 0, 0, 0);

	// TEMP: Just to make the area of this widget visibly more apparent.
	//this->setBackgroundRole(QPalette::ToolTipBase);
}


CHtmlSysWinQt::~CHtmlSysWinQt()
{
	//qDebug() << Q_FUNC_INFO;

	// Don't allow the formatter to reference us anymore, since
	// we're about to be deleted.
	this->formatter_->unset_win();

	if (this->fVFrame != 0) delete this->fVFrame;
	if (this->fVLayout != 0) delete this->fVLayout;
	if (this->fVLine != 0) delete this->fVLine;
	if (this->fHFrame != 0) delete this->fHFrame;
	if (this->fHLayout != 0) delete this->fHLayout;
	if (this->fHLine != 0) delete this->fHLine;
}


void
CHtmlSysWinQt::resizeEvent( QResizeEvent* event )
{
	if (not this->fDontReformat) {
		this->formatter_->start_at_top(false);
		this->do_formatting(true, false, true);
	}
	QScrollArea::resizeEvent(event);
}


void
CHtmlSysWinQt::addBanner( CHtmlSysWinQt* banner, int where, CHtmlSysWinQt* other, HTML_BannerWin_Pos_t pos,
						  unsigned long style )
{
	// Don't try to reformat while we're in here or we'll crash and burn.
	this->fDontReformat = true;

	QWidget* frame;
	QBoxLayout* layout;
	QWidget* oldCentralWidget = qWinGroup->centralWidget();

	banner->fBannerPos = pos;
	banner->fStyle = style;

	if (qWinGroup->centralWidget() == this) {
		// No banners exist yet.  Set up a new frame and layout and use the
		// new frame as the main window's central widget.
		frame = new QWidget;
		if (pos == HTML_BANNERWIN_POS_TOP or pos == HTML_BANNERWIN_POS_BOTTOM) {
			layout = new QVBoxLayout(frame);
		} else {
			layout = new QHBoxLayout(frame);
		}
		switch (pos) {
		  case HTML_BANNERWIN_POS_TOP:
		  case HTML_BANNERWIN_POS_BOTTOM:
			this->fVLayout = static_cast<QVBoxLayout*>(layout);
			this->fVFrame = frame;
			break;

		  case HTML_BANNERWIN_POS_LEFT:
		  case HTML_BANNERWIN_POS_RIGHT:
			this->fHLayout = static_cast<QHBoxLayout*>(layout);
			this->fHFrame = frame;
			break;

		  default:
			// TODO: Be graceful.
			qFatal("Unknonwn HTML_BannerWin_Pos_t.");
		}
		layout->addWidget(this);
	} else {
		switch (pos) {
		  case HTML_BANNERWIN_POS_TOP:
		  case HTML_BANNERWIN_POS_BOTTOM:
			if (this->fVFrame == 0) {
				this->fVFrame = new QWidget;
				this->fVLayout = new QVBoxLayout(this->fVFrame);
				this->fVLayout->addWidget(oldCentralWidget);
			}
			frame = this->fVFrame;
			layout = this->fVLayout;
			break;

		  case HTML_BANNERWIN_POS_LEFT:
		  case HTML_BANNERWIN_POS_RIGHT:
			if (this->fHFrame == 0) {
				this->fHFrame = new QWidget;
				this->fHLayout = new QHBoxLayout(this->fHFrame);
				this->fHLayout->addWidget(oldCentralWidget);
			}
			layout = this->fHLayout;
			frame = this->fHFrame;
			break;

		  default:
			// TODO: Be graceful.
			qFatal("Unknonwn HTML_BannerWin_Pos_t.");
		}
	}

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	//qDebug() << "INDEX INSIDE LAYOUT:" << layout->indexOf(this);

	switch (where) {
	  case OS_BANNER_FIRST:
		//qDebug() << "OS_BANNER_FIRST";
		if (pos == HTML_BANNERWIN_POS_TOP or pos == HTML_BANNERWIN_POS_LEFT) {
			layout->insertWidget(layout->indexOf(this), banner);
		} else {
			layout->insertWidget(layout->indexOf(this) + 1, banner);
		}
		break;

	  case OS_BANNER_LAST:
		//qDebug() << "OS_BANNER_LAST";
		if (pos == HTML_BANNERWIN_POS_TOP or pos == HTML_BANNERWIN_POS_LEFT) {
			//qDebug() << "TOP or LEFT";
			layout->insertWidget(layout->indexOf(this), banner);
		} else {
			//qDebug() << "BOTTOM or RIGHT";
			layout->insertWidget(layout->indexOf(this) + 2, banner);
		}
		break;

	  case OS_BANNER_BEFORE:
		Q_ASSERT(other != 0);
		abort();
		layout->insertWidget(layout->indexOf(other), banner);
		break;

	  case OS_BANNER_AFTER:
		Q_ASSERT(other != 0);
		abort();
		layout->insertWidget(layout->indexOf(other) - 1, banner);
		break;

	  default:
		qFatal("Unknown value for 'where' in addBanner()");
	}

	qWinGroup->setCentralWidget(frame);
	qWinGroup->centralWidget()->show();
	this->fDontReformat = false;
}


void
CHtmlSysWinQt::notify_clear_contents()
{
	//qDebug() << Q_FUNC_INFO;
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
	QPainter painter(this->fDispWidget);
	const CHtmlSysFontQt& fontCast = *static_cast<CHtmlSysFontQt*>(font);
	painter.setFont(fontCast);
	if (fontCast.color().isValid()) {
		painter.setPen(fontCast.color());
	}
	if (hilite) {
		painter.setBackgroundMode(Qt::OpaqueMode);
		painter.setBackground(QApplication::palette().highlight());
		painter.setPen(QApplication::palette().color(QPalette::HighlightedText));
	}
	painter.drawText(x, y + QFontMetrics(fontCast).ascent(), QString::fromUtf8(str, len));
}


void
CHtmlSysWinQt::draw_text_space( int hilite, long x, long y, CHtmlSysFont* font, long wid )
{
	//qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::draw_bullet( int hilite, long x, long y, CHtmlSysFont* font, HTML_SysWin_Bullet_t style )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::draw_hrule( const CHtmlRect* pos, int shade )
{
	//qDebug() << Q_FUNC_INFO;

	QPainter(this->fDispWidget).drawRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top);
}


void
CHtmlSysWinQt::draw_table_border( const CHtmlRect* pos, int width, int cell )
{
	//qDebug() << Q_FUNC_INFO;

	QPainter(this->fDispWidget).drawRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top);
}


void
CHtmlSysWinQt::draw_table_bkg( const CHtmlRect* pos, HTML_color_t bgcolor )
{
	//qDebug() << Q_FUNC_INFO;

	QPainter painter(this->fDispWidget);
	int red = HTML_color_red(bgcolor);
	int green = HTML_color_green(bgcolor);
	int blue = HTML_color_blue(bgcolor);
	painter.fillRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top, QColor(red, green, blue));
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

	// Freeze the display, if requested.
	if (freeze_display) {
		this->formatter_->freeze_display(true);
	}

	// Format all remaining lines.
	while (this->formatter_->more_to_do()) {
		this->formatter_->do_formatting();
	}

	// We make the view a bit higher (5 pixels) then required by the real
	// document height so that we get a bit of extra space under the input
	// prompt because it looks nicer.  This is only done if we're the main game
	// window.
	unsigned long height = this->formatter_->get_max_y_pos();
	if (this == qFrame->gameWindow()) {
		height += 5;
	}
	this->fDispWidget->resize(this->formatter_->get_outer_max_line_width(), height);

	// Unfreeze the display if we froze it before.
	if (freeze_display) {
		this->formatter_->freeze_display(false);
	}
	return false;
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
	timer->start(1000);
	this->fTimerList.append(timer);
}


void
CHtmlSysWinQt::unregister_timer_func( void (*timer_func)( void * ), void* func_ctx )
{
	for (int i = 0; i < this->fTimerList.size(); ++i) {
		if (this->fTimerList.at(i)->func_ == timer_func) {
			delete this->fTimerList.takeAt(i);
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

	if (this->formatter_->get_outer_max_line_width() > this->fDispWidget->width()) {
		this->fDispWidget->resize(this->formatter_->get_outer_max_line_width(), this->fDispWidget->height());
	}
}


void
CHtmlSysWinQt::fmt_adjust_vscroll()
{
	//qDebug() << Q_FUNC_INFO;

	if (this->formatter_->get_max_y_pos() > static_cast<unsigned long>(this->fDispWidget->height())) {
		this->fDispWidget->resize(this->fDispWidget->width(), this->formatter_->get_max_y_pos());
	}
}


void
CHtmlSysWinQt::inval_doc_coords( const CHtmlRect* area )
{
	//qDebug() << Q_FUNC_INFO;

	//qDebug() << "Invalidating area" << area->left << area->top << area->right << area->bottom;

	long width = area->right == HTMLSYSWIN_MAX_RIGHT ?
				 this->fDispWidget->width() - area->left
				 : area->right - area->left;
	long height = area->bottom == HTMLSYSWIN_MAX_BOTTOM ?
				  this->fDispWidget->height() - area->top
				  : area->bottom - area->top;

	this->fDispWidget->update(QRect(area->left, area->top, width, height));
}


void
CHtmlSysWinQt::advise_clearing_disp_list()
{
	//qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::scroll_to_doc_coords( const CHtmlRect* pos )
{
	qDebug() << Q_FUNC_INFO;

	qApp->processEvents();
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

	qWinGroup->setWindowTitle(qApp->applicationName() + " - " + QString::fromUtf8(title));
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
		p.setColor(QPalette::Base, QScrollArea::palette().color(QPalette::Base));
		this->setPalette(p);
		return;
	}

	int red = HTML_color_red(color);
	int green = HTML_color_green(color);
	int blue = HTML_color_blue(color);
	QPalette p(this->palette());
	p.setColor(QPalette::Base, QColor(red, green, blue));
	this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_text_color( HTML_color_t color, int use_default )
{
	//qDebug() << Q_FUNC_INFO;

	if (use_default) {
		QPalette p(this->palette());
		p.setColor(QPalette::Text, QScrollArea::palette().color(QPalette::Text));
		return;
	}

	int red = HTML_color_red(color);
	int green = HTML_color_green(color);
	int blue = HTML_color_blue(color);
	QPalette p(this->palette());
	p.setColor(QPalette::Text, QColor(red, green, blue));
	this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_input_color(HTML_color_t clr, int use_default)
{
	//qDebug() << Q_FUNC_INFO;
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

	switch (color) {
	  case HTML_COLOR_STATUSBG:   return HTML_make_color(210, 200, 180);
	  case HTML_COLOR_STATUSTEXT: return HTML_make_color(0, 0, 0);
	  case HTML_COLOR_LINK:       return HTML_make_color(0, 0, 255);
	  case HTML_COLOR_ALINK:      return HTML_make_color(0, 0, 255);
	  case HTML_COLOR_TEXT:       return HTML_make_color(0, 0, 0);
	  case HTML_COLOR_BGCOLOR:    return HTML_make_color(255, 255, 255);
	  case HTML_COLOR_INPUT:      return HTML_make_color(0, 0, 0);
	  case HTML_COLOR_HLINK:      return HTML_make_color(0, 0, 255);
	  default:
		// Return black for anything else.
		return 0;
	}
}


HTML_color_t
CHtmlSysWinQt::get_html_link_color() const
{
	//qDebug() << Q_FUNC_INFO;

	return HTML_make_color(0, 0, 255);
}


HTML_color_t
CHtmlSysWinQt::get_html_alink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	return HTML_make_color(0, 0, 255);
}


HTML_color_t
CHtmlSysWinQt::get_html_vlink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	return HTML_make_color(0, 0, 255);
}


HTML_color_t
CHtmlSysWinQt::get_html_hlink_color() const
{
	//qDebug() << Q_FUNC_INFO;

	return HTML_make_color(0, 0, 255);
}


int
CHtmlSysWinQt::get_html_link_underline() const
{
	//qDebug() << Q_FUNC_INFO;

	return true;
}


int
CHtmlSysWinQt::get_html_show_links() const
{
	//qDebug() << Q_FUNC_INFO;

	return true;
}


int
CHtmlSysWinQt::get_html_show_graphics() const
{
	//qDebug() << Q_FUNC_INFO;

	return true;
}


void
CHtmlSysWinQt::set_html_bg_image( CHtmlResCacheObject* image )
{
	//qDebug() << Q_FUNC_INFO;

	if (image == 0 or image->get_image() == 0) {
		if (this->fBgImage != 0) {
			this->fBgImage->remove_ref();
			this->fBgImage = 0;
			QPalette p(this->palette());
			p.setBrush(QPalette::Base, QBrush());
			this->setPalette(p);
		}
		return;
	}

	QTadsPixmap* castImg;
	switch (image->get_image()->get_res_type()) {
	  case HTML_res_type_JPEG:
		castImg = static_cast<CHtmlSysImageJpegQt*>(image->get_image());
		break;
	  case HTML_res_type_PNG:
		castImg = static_cast<CHtmlSysImagePngQt*>(image->get_image());
		break;
	  case HTML_res_type_MNG:
		castImg = static_cast<CHtmlSysImageMngQt*>(image->get_image());
		break;
	  default:
		qWarning() << Q_FUNC_INFO << "Unknown resource type";
		castImg = dynamic_cast<QTadsPixmap*>(image->get_image());
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

	QBoxLayout* castLayout = static_cast<QBoxLayout*>(this->parentWidget()->layout());
	if (castLayout->direction() == QBoxLayout::TopToBottom or castLayout->direction() == QBoxLayout::BottomToTop) {
		// We're a horizontal banner.
		if (not use_height) {
			return;
		}
		if (height_units == HTML_BANNERWIN_UNITS_PCT) {
			//qDebug() << "Setting banner height (percent) of" << this << "to" << height;
			castLayout->setStretchFactor(this, height);
		} else if (height_units == HTML_BANNERWIN_UNITS_CHARS) {
			//qDebug() << "Setting banner height (height of '0') of" << this << "to" << height;
			this->setMaximumHeight(this->fontMetrics().boundingRect('0').height() * height);
			this->setMinimumHeight(this->fontMetrics().boundingRect('0').height() * height);
		} else if (height_units == HTML_BANNERWIN_UNITS_PIX) {
			//qDebug() << "Setting banner height (pixels) of" << this << "to" << height;
			this->setMaximumHeight(height);
			this->setMinimumHeight(height);
		}
	} else {
		// We're a vertical banner.
		if (not use_width) {
			return;
		}
		if (width_units == HTML_BANNERWIN_UNITS_PCT) {
			//qDebug() << "Setting banner width (percent) of" << this << "to" << width;
			castLayout->setStretchFactor(this, width);
		} else if (width_units == HTML_BANNERWIN_UNITS_CHARS) {
			//qDebug() << "Setting banner width (width of '0') of" << this << "to" << width;
			this->setMaximumWidth(this->fontMetrics().boundingRect('0').width() * width);
			this->setMinimumWidth(this->fontMetrics().boundingRect('0').width() * width);
		} else if (width_units == HTML_BANNERWIN_UNITS_PIX) {
			//qDebug() << "Setting banner width (pixels) of" << this << "to" << width;
			this->setMaximumWidth(width);
			this->setMinimumWidth(width);
		}
	}
}


void
CHtmlSysWinQt::set_banner_info( HTML_BannerWin_Pos_t pos, unsigned long style )
{
	//qDebug() << Q_FUNC_INFO;

	this->fBannerPos = pos;
	this->fStyle = style;
}


void
CHtmlSysWinQt::get_banner_info( HTML_BannerWin_Pos_t* pos, unsigned long* style )
{
	//qDebug() << Q_FUNC_INFO;
	*pos = this->fBannerPos;
	*style = this->fStyle;
}
