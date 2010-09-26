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
#ifndef SYSWIN_H
#define SYSWIN_H

#include <QApplication>
#include <QDesktopWidget>
#include <QScrollArea>

#include "globals.h"
#include "sysfont.h"
#include "sysframe.h"
#include "syswingroup.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysWinQt: public QScrollArea, public CHtmlSysWin {
  private:
	// If we're a banner, what is our HTML_BannerWin_Pos_t, placement and
	// style flags.
	//
	// Note: To determine whether we're a banner, we don't use a flag but
	// simply check whether "this == qFrame->gameWindow()"; if we're the game
	// window, we can't be a banner.
	HTML_BannerWin_Pos_t fBannerPos;
	int fBannerWhere;
	bool fBannerStyleVScroll;
	bool fBannerStyleAutoVScroll;
	bool fBannerStyleHScroll;
	bool fBannerStyleGrid;

	// Do not attempt to reformat during a resize event.  This is set when in
	// the process of creating a new banner.  If we reformat during that
	// process, the formatter will call the banner-creating routine again and
	// we will crash due to the re-entrancy.
	int fDontReformat;

	// Our parent banner, if any.  This is 0 if we don't have one.
	CHtmlSysWinQt* fParentBanner;

	// Our child banners, if any.
	QList<CHtmlSysWinQt*> fChildBanners;

	// List of timers with registered callbacks.
	QList<class QTadsTimer*> fTimerList;

	// Our banner background image.
	class CHtmlResCacheObject* fBgImage;

  protected:	
	// The content height at the time of the last user input.  When the
	// formatter is producing a long run of output, we pause between screens to
	// make sure the user has had a chance to see all of the text before we
	// scroll it away.  This member records the position of the last input, so
	// we can be sure not to add more than another screenfull without getting
	// more input.
	int lastInputHeight;

	// Margins.
	CHtmlRect margins;

	// Banner size information, if we are a banner.  This is either our width
	// or height, according to our alignment type (vertical or horizontal).
	long bannerSize;
	HTML_BannerWin_Units_t bannerSizeUnits;

	// Our display widget.
	class DisplayWidget* dispWidget;

	virtual void
	keyPressEvent( QKeyEvent* event );

	virtual void
	scrollContentsBy(int dx, int dy);

  public:
	CHtmlSysWinQt( class CHtmlFormatter* formatter, class DisplayWidget* dispWidget, QWidget* parent );

	virtual
	~CHtmlSysWinQt();

	// Returns our display widget.
	class DisplayWidget*
	displayWidget() const
	{ return this->dispWidget; }

	// Calculate and adjust the sizes of our child banners.  On entry,
	// 'parentSize' contains the size of the full parent window area; on
	// return, it is updated to indicate the final size of the parent banner's
	// area after deducting the space carved out for children.
	void
	calcChildBannerSizes( QRect& parentSize );

	// Do a complete reformat.
	void
	doReformat( int showStatus, int freezeDisplay, int resetSounds);

	void
	addBanner( CHtmlSysWinQt* banner, HTML_BannerWin_Type_t type, int where, CHtmlSysWinQt* other,
			   HTML_BannerWin_Pos_t pos, unsigned long style );

	// Our parent banner, if there is one.
	CHtmlSysWinQt*
	parentBanner() const
	{ return this->fParentBanner; }

	//
	// CHtmlSysWin interface implementation.
	//
	virtual CHtmlSysWinGroup*
	get_win_group()
	{ return qWinGroup; }

	virtual void
	notify_clear_contents();

	virtual int
	close_window( int force );

	virtual long
	get_disp_width()
	{ return this->viewport()->width() - 3; }

	virtual long
	get_disp_height()
	{ return this->viewport()->height(); }

	virtual long
	get_pix_per_inch()
	{ return QApplication::desktop()->logicalDpiX(); }

	virtual CHtmlPoint
	measure_text( class CHtmlSysFont* font, const textchar_t* str, size_t len, int *ascent );

	virtual CHtmlPoint
	measure_dbgsrc_icon()
	{ return CHtmlPoint(); }

	virtual size_t
	get_max_chars_in_width( CHtmlSysFont* font, const textchar_t* str, size_t len, long wid );

	virtual void
	draw_text( int hilite, long x, long y, CHtmlSysFont* font, const textchar_t* str, size_t len );

	virtual void
	draw_text_space( int hilite, long x, long y, CHtmlSysFont* font, long wid );

	virtual void
	draw_bullet( int hilite, long x, long y, CHtmlSysFont* font, HTML_SysWin_Bullet_t style );

	virtual void
	draw_hrule( const CHtmlRect* pos, int shade );

	virtual void
	draw_table_border( const CHtmlRect* pos, int width, int cell );

	virtual void
	draw_table_bkg( const CHtmlRect* pos, HTML_color_t bgcolor );

	virtual void
	draw_dbgsrc_icon( const CHtmlRect* pos, unsigned int stat );

	virtual int
	do_formatting( int show_status, int update_win, int freeze_display );

	// We don't deal with palettes, so this is a no-op.
	virtual void
	recalc_palette()
	{ }

	// We don't deal with palettes, so always return false.
	virtual int
	get_use_palette()
	{ return false; }

	virtual CHtmlSysFont*
	get_default_font();

	virtual CHtmlSysFont*
	get_font( const CHtmlFontDesc* font_desc )
	{ return qFrame->createFont(font_desc); }

	virtual CHtmlSysFont*
	get_bullet_font( CHtmlSysFont* current_font )
	{ return current_font; }

	virtual void
	register_timer_func( void (*timer_func)( void* ), void* func_ctx );

	virtual void
	unregister_timer_func( void (*timer_func)( void * ), void* func_ctx );

	virtual CHtmlSysTimer*
	create_timer( void (*timer_func)( void* ), void *func_ctx );

	virtual void
	set_timer( CHtmlSysTimer* timer, long interval_ms, int repeat );

	virtual void
	cancel_timer( CHtmlSysTimer* timer );

	virtual void
	delete_timer( CHtmlSysTimer* timer );

	virtual void
	fmt_adjust_hscroll();

	virtual void
	fmt_adjust_vscroll();

	virtual void
	inval_doc_coords( const CHtmlRect* area );

	virtual void
	advise_clearing_disp_list();

	virtual void
	scroll_to_doc_coords( const CHtmlRect* pos );

	virtual void
	get_scroll_doc_coords( CHtmlRect* pos );

	virtual void
	set_window_title( const textchar_t* title );

	virtual void
	set_html_bg_color( HTML_color_t color, int use_default );

	virtual void
	set_html_text_color( HTML_color_t color, int use_default );

	virtual void
	set_html_input_color( HTML_color_t, int )
	// We don't provide input, so we don't deal with this.
	{ }

	virtual void
	set_html_link_colors( HTML_color_t link_color, int link_use_default, HTML_color_t vlink_color,
						  int vlink_use_default, HTML_color_t alink_color, int alink_use_default,
						  HTML_color_t hlink_color, int hlink_use_default );

	virtual HTML_color_t
	map_system_color( HTML_color_t color );

	virtual HTML_color_t
	get_html_link_color() const;

	virtual HTML_color_t
	get_html_alink_color() const;

	virtual HTML_color_t
	get_html_vlink_color() const;

	virtual HTML_color_t
	get_html_hlink_color() const;

	virtual int
	get_html_link_underline() const;

	virtual int
	get_html_show_links() const;

	virtual int
	get_html_show_graphics() const;

	virtual void
	set_html_bg_image( class CHtmlResCacheObject* image );

	virtual void
	inval_html_bg_image( unsigned int x, unsigned int y, unsigned int wid, unsigned int ht );

	virtual void
	set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width, long height,
					 HTML_BannerWin_Units_t height_units, int use_height );

	virtual void
	set_banner_info( HTML_BannerWin_Pos_t pos, unsigned long style );

	virtual void
	get_banner_info( HTML_BannerWin_Pos_t* pos, unsigned long* style );
};


#endif
