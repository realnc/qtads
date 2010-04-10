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

/* Tads HTML layer classes whose interfaces need to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information about
 * them.
 */

#ifndef HTMLQT_H
#define HTMLQT_H

#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QMainWindow>
#include <QScrollArea>
#include <QTemporaryFile>
#include <phonon/mediaobject.h>

#include "hos_qt.h"
#include "htmlsys.h"

#include "qtadspixmap.h"


/* Works like qApp, but contains the global CHtmlSysFrameQt object instead.  If
 * this variable is 0, it means that no such object has been created yet.
 *
 * qApp and qFrame actually both point to the same object (the global
 * QApplication instance), but qFrame is provided simply to avoid casting the
 * global qApp object into a CHtmlSysFrameQt when we need to use it as such.
 */
extern class CHtmlSysFrameQt* qFrame;

/* The global CHtmlSysWinGroupQt object.  0 if none exists.  Like qApp/qFrame,
 * this is a singleton object and it's handy to have a global pointer to it.
 */
extern class CHtmlSysWinGroupQt* qWinGroup;


class CHtmlSysFontQt: public QFont, public CHtmlSysFont {
  private:
	QColor fColor;
	QColor fBgColor;

  public:
	// When color() is a valid color (QColor::isValid()) it should be used as
	// the foreground color when drawing text in this font.
	const QColor&
	color() const
	{ return this->fColor; }

	void
	color( HTML_color_t color )
	{ this->fColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)); }

	// When bgColor() is a valid color (QColor::isValid()) it should be used as
	// the background color when drawing text in this font.
	const QColor&
	bgColor() const
	{ return this->fBgColor; }

	void
	bgColor( HTML_color_t color )
	{ this->fBgColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)); }

	bool
	operator ==( const CHtmlSysFontQt& f ) const
	{ return QFont::operator ==(f) and this->fColor == f.fColor and this->fBgColor == f.fBgColor; }

	CHtmlSysFontQt&
	operator =( const QFont& f )
	{
		QFont::operator =(f);
		return *this;
	}

	//
	// CHtmlSysFont interface implementation.
	//
	virtual void
	get_font_metrics( CHtmlFontMetrics* m )
	{
		//qDebug() << Q_FUNC_INFO << "called";

		QFontMetrics tmp(*this);

		m->ascender_height = tmp.ascent();
		m->descender_height = tmp.descent();
		m->total_height = tmp.height();
	}

	virtual int
	is_fixed_pitch()
	{ return QFontInfo(*this).fixedPitch(); }

	virtual int
	get_em_size()
	{ return QFontInfo(*this).pixelSize(); }
};


class CHtmlSysFrameQt: public QApplication, public CHtmlSysFrame {
  private:
	// Preferences (fonts, colors, etc.)
	class QTadsSettings* fSettings;

	// Tads2 application container context.
	appctxdef fAppctx;

	// Tads3 host and client services interfaces.
	class CVmHostIfc* fHostifc;
	class CVmMainClientConsole* fClientifc;

	class CHtmlTextBuffer fBuffer;
	textchar_t* fInputBuffer;
	class CHtmlInputBuf* fTadsBuffer;

	// Main window.
	CHtmlSysWinGroupQt* fMainWin;

	// Main HTML window.
	class CHtmlSysWinInputQt* fGameWin;

	// Tads HTML parser.
	class CHtmlParser* fParser;

	// Tads HTML formatter.
	class CHtmlFormatterInput* fFormatter;

	// List of banners.
	QList<class CHtmlSysWinQt*> fBannerList;

	// Fonts we created. We keep a list of every font we created since we're
	// responsible for deleting them when they're no longer needed.
	QList<CHtmlSysFontQt*> fFontList;

  public:
	CHtmlSysFrameQt( int& argc, char* argv[], class CHtmlParser* parser, const char* appName, const char* appVersion,
					 const char* orgName, const char* orgDomain );

	virtual
	~CHtmlSysFrameQt();

	class QTadsSettings*
	settings()
	{ return this->fSettings; }

	class CHtmlSysWinInputQt*
	gameWindow()
	{ return this->fGameWin; }

	CHtmlSysFontQt*
	createFont( const CHtmlFontDesc* font_desc );

	int
	runT2Game( const QString& fname );

	int
	runT3Game( const QString& fname );

	//
	// CHtmlSysFrame interface implementation.
	//
	virtual void
	flush_txtbuf( int fmt, int immediate_redraw );

	virtual class CHtmlParser*
	get_parser();

	virtual void
	start_new_page();

	virtual void
	set_nonstop_mode( int flag );

	virtual void
	display_output( const textchar_t* buf, size_t len );

	virtual int
	check_break_key();

	virtual int
	get_input( textchar_t* buf, size_t bufsiz );

	virtual int
	get_input_timeout( textchar_t* buf, size_t buflen, unsigned long timeout, int use_timeout );

	virtual void
	get_input_cancel( int reset );

	virtual int
	get_input_event( unsigned long ms, int use_timeout, os_event_info_t* info );

	virtual textchar_t
	wait_for_keystroke( int pause_only );

	virtual void
	pause_for_exit();

	virtual void
	pause_for_more();

	virtual void
	dbg_print( const char* msg );

	virtual class CHtmlSysWin*
	create_banner_window( class CHtmlSysWin* parent, HTML_BannerWin_Type_t window_type,
						  class CHtmlFormatter* formatter, int where, class CHtmlSysWin* other,
						  HTML_BannerWin_Pos_t pos, unsigned long style );

	virtual void
	orphan_banner_window( class CHtmlFormatterBannerExt* banner );

	virtual CHtmlSysWin*
	create_aboutbox_window( class CHtmlFormatter* formatter );

	virtual void
	remove_banner_window( CHtmlSysWin* win );

	virtual int
	get_exe_resource( const textchar_t* resname, size_t resnamelen, textchar_t* fname_buf, size_t fname_buf_len,
					  unsigned long* seek_pos, unsigned long* siz );
};


class CHtmlSysWinGroupQt: public QMainWindow, public CHtmlSysWinGroup {
	Q_OBJECT

  private:
	class QTadsConfDialog* fConfDialog;

  private slots:
	void
	showConfDialog();

	void
	hideConfDialog();

  public:
	CHtmlSysWinGroupQt();
	virtual ~CHtmlSysWinGroupQt();

	//
	// CHtmlSysWinGroup interface implementation.
	//
	virtual oshtml_charset_id_t
	get_default_win_charset() const;

	virtual size_t
	xlat_html4_entity( textchar_t* result, size_t result_size, unsigned int charval, oshtml_charset_id_t* charset,
					   int* changed_charset );
};


class CHtmlSysWinQt: public QScrollArea, public CHtmlSysWin {
	Q_OBJECT

  private:
	// If we're a banner, what is our HTML_BannerWin_Pos_t and style?
	//
	// Note: To determine whether we're a banner, we don't use a flag but
	// simply check whether "this == qFrame->gameWindow()"; if we're the game
	// window, we can't be a banner.
	HTML_BannerWin_Pos_t fBannerPos;
	unsigned long fStyle;

	// Do not attempt to reformat during a resize event.  This is set when in
	// the process of creating a new banner.  If we reformat during that
	// process, the formatter will call the banner-creating routine again and
	// we will crash due to the re-entrancy.
	bool fDontReformat;

	// Frames and layouts for our adjacent banners, if there are any.  We can
	// have rows and columns of banners attached to us in vertical and
	// horizontal directions.  We keep child banners inside a frame and assign
	// it its own layout.  This allows for an infinite number of banners being
	// added recursively.  We also need a frame that does nothing except draw a
	// line; this is needed for banners that have a border.
	QWidget* fVFrame;
	QWidget* fHFrame;
	QFrame* fVLine;
	QFrame* fHLine;
	class QVBoxLayout* fVLayout;
	class QHBoxLayout* fHLayout;

	// Margins.
	CHtmlRect fMargins;

	// List of timers with registered callbacks.
	QList<class QTadsTimer*> fTimerList;

	// Our banner background image.
	class CHtmlResCacheObject* fBgImage;

  protected:
	// Our display widget.
	class QTadsDisplayWidget* fDispWidget;

	// The formatter we're associated with.
	class CHtmlFormatter* fFormatter;

	//virtual void
	//keyPressEvent( QKeyEvent* event );

	void
	singleKeyPressEvent( QKeyEvent* event );

	virtual void
	resizeEvent( QResizeEvent* event );

  public:
	CHtmlSysWinQt( class CHtmlFormatter* formatter, QWidget* parent );

	virtual
	~CHtmlSysWinQt();

	// Returns our display widget.
	class QTadsDisplayWidget* displayWidget()
	{ return this->fDispWidget; }

	void
	addBanner( CHtmlSysWinQt* banner, int where, CHtmlSysWinQt* other, HTML_BannerWin_Pos_t pos,
			   unsigned long style );

	//
	// CHtmlSysWin interface implementation.
	//
  public:
	virtual class CHtmlSysWinGroup*
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
	set_html_input_color( HTML_color_t clr, int use_default );

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
	inval_html_bg_image( unsigned int x, unsigned int y, unsigned int wid,
			     unsigned int ht );

	virtual void
	set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width,
			 long height, HTML_BannerWin_Units_t height_units, int use_height );

	virtual void
	set_banner_info( HTML_BannerWin_Pos_t pos, unsigned long style );

	virtual void
	get_banner_info( HTML_BannerWin_Pos_t* pos, unsigned long* style );
};


/* An input-capable CHtmlSysWinQt.
 */
class CHtmlSysWinInputQt: public CHtmlSysWinQt {
	Q_OBJECT

  private:
	// We have a finished user input.
	bool fInputReady;

	// We are accepting input.
	bool fAcceptInput;

	// If we're accepting input, should we get a whole line, or a single
	// keypress.
	bool fSingleKeyInput;

	// In single keypress input mode, these store the last pressed key.  Only
	// one of fLastKeyEvent and fLastKeyText can be valid.
	//
	// fLastKeyEvent is used in cases where the user pressed a non-text key,
	// like backspace, space, enter, the up-arrow button, etc.  In that case,
	// fLastKeyEvent contains that key key press in form of a Qt::Key and
	// fLastKeyText will be a null QChar.
	//
	// If the user pressed a text key (for example "C", "8" or "!"), then
	// fLastKeyEvent will be zero and fLastKeyText will contain the character
	// that corresponds to the pressed key.
	Qt::Key fLastKeyEvent;
	QChar fLastKeyText;

	// The input tag we use to communicate with the base code.
	class CHtmlTagTextInput* fTag;

	// The externally managed input buffer.
	class CHtmlInputBuf* fTadsBuffer;

  protected:
	virtual void
	keyPressEvent( QKeyEvent* event );

	void
	singleKeyPressEvent( QKeyEvent* event );

  public:
	CHtmlSysWinInputQt( class CHtmlFormatter* formatter, QWidget* parent );

	virtual
	~CHtmlSysWinInputQt()
	{ }

	// Read a line of input.
	bool
	getInput( class CHtmlInputBuf* tadsBuffer );

	/* Uses os_getc_raw() semantics, but with a timeout.
	 *
	 * If 'timeout' is 0 or negative, then the routine behaves exactly like
	 * os_getc_raw().  If 'timeout' is positive, then we only wait for a key
	 * for 'timeout' milliseconds.  If the operation times out before a key
	 * has been pressed, we return 0 and set 'timedOut' to true.  If a key
	 * is pressed before the timeout is reached, we return the same as
	 * os_getc_raw() and set 'timedOut' to false.
	 *
	 * TODO: Timeouts are not handled yet.
	 */
	int
	getKeypress( int timeout = -1, bool* timedOut = 0 );

	bool
	inputReady()
	{ return this->fInputReady; }

	void
	startLineInput( class CHtmlInputBuf* tadsBuffer, class CHtmlTagTextInput* tag, class CHtmlFormatter* formatter );

	void
	startKeypressInput();
};


class CHtmlSysImageJpegQt: public QTadsPixmap, public CHtmlSysImageJpeg {
  public:
	//
	// CHtmlSysImageJpeg interface implementation.
	//
	virtual void
	draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
	{ QTadsPixmap::drawFromPaintEvent(win, pos, mode); }

	virtual unsigned long
	get_width() const
	{ return QTadsPixmap::width(); }

	virtual unsigned long
	get_height() const
	{ return QTadsPixmap::height(); }

	virtual int
	map_palette( CHtmlSysWin* win, int foreground )
	{ return false; }
};


class CHtmlSysImagePngQt: public QTadsPixmap, public CHtmlSysImagePng {
  public:
	//
	// CHtmlSysImagePng interface implementation.
	//
	virtual void
	draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
	{ QTadsPixmap::drawFromPaintEvent(win, pos, mode); }

	virtual unsigned long
	get_width() const
	{ return QTadsPixmap::width(); }

	virtual unsigned long
	get_height() const
	{ return QTadsPixmap::height(); }

	virtual int
	map_palette( CHtmlSysWin* win, int foreground )
	{ return false; }
};


class CHtmlSysImageMngQt: public QTadsPixmap, public CHtmlSysImageMng {
  public:
	//
	// CHtmlSysImageMng interface implementation.
	//
	virtual void
	draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
	{ QTadsPixmap::drawFromPaintEvent(win, pos, mode); }

	virtual unsigned long
	get_width() const
	{ return QTadsPixmap::width(); }

	virtual unsigned long
	get_height() const
	{ return QTadsPixmap::height(); }

	virtual int
	map_palette( CHtmlSysWin* win, int foreground )
	{ return false; }

	virtual void
	notify_timer()
	{ qDebug() << Q_FUNC_INFO << "called"; }

	virtual void
	notify_image_change( int x, int y, int wid, int ht )
	{ qDebug() << Q_FUNC_INFO << "called"; }
};


namespace Phonon { class AudioOutput; };
class QTadsMediaObject: public Phonon::MediaObject {
	Q_OBJECT

  private:
	Phonon::AudioOutput* fOutput;
	QTemporaryFile* fFile;

	// Callback to invoke on stop.
	void (*fDone_func)(void*, int repeat_count);

	// CTX to pass to the callback.
	void* fDone_func_ctx;

	// How many times we repeated the sound.
	int fRepeats;

	// How many times should we repeat the sound.
	// 0 means repeat forever.
	int fRepeatsWanted;

  private slots:
	void
	fLoop();

	void
	fFinish();

  public:
	QTadsMediaObject( QObject* parent );

	void
	startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat );

	virtual
	~QTadsMediaObject()
	{ delete this->fFile; }

	enum SoundType { WAV, OGG, MPEG };
	static CHtmlSysSound*
	createSound( const class CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
				 unsigned long filesize, CHtmlSysWin* win, SoundType type );
};


class CHtmlSysSoundWavQt: public QTadsMediaObject, public CHtmlSysSoundWav {
  public:
	CHtmlSysSoundWavQt( QObject* parent )
	: QTadsMediaObject(parent)
	{ }

	//
	// CHtmlSysSoundWav interface implementation.
	//
	virtual int
	play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
				const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade );

	virtual void
	add_crossfade( CHtmlSysWin* win, long ms );

	virtual void
	cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg );

	virtual int
	maybe_suspend( CHtmlSysSound* )
	// We always return false since we have no limitation regarding the amount
	// of sounds we can play simultaneously.
	{ return false; }

	virtual void
	resume();
};


class CHtmlSysSoundOggQt: public QTadsMediaObject, public CHtmlSysSoundOgg {
  public:
	CHtmlSysSoundOggQt( QObject* parent )
	: QTadsMediaObject(parent)
	{ }

	//
	// CHtmlSysSoundOgg interface implementation.
	//
	virtual int
	play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
				const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade );

	virtual void
	add_crossfade( CHtmlSysWin* win, long ms );

	virtual void
	cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg );

	virtual int
	maybe_suspend( CHtmlSysSound* )
	// We always return false since we have no limitation regarding the amount
	// of sounds we can play simultaneously.
	{ return false; }

	virtual void
	resume();
};


class CHtmlSysSoundMpegQt: public QTadsMediaObject, public CHtmlSysSoundMpeg {
  public:
	CHtmlSysSoundMpegQt( QObject* parent )
	: QTadsMediaObject(parent)
	{ }

	//
	// CHtmlSysSoundMpeg interface implementation.
	//
	virtual int
	play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
				const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade );

	virtual void
	add_crossfade( CHtmlSysWin* win, long ms );

	virtual void
	cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg );

	virtual int
	maybe_suspend( CHtmlSysSound* )
	// We always return false since we have no limitation regarding the amount
	// of sounds we can play simultaneously.
	{ return false; }

	virtual void
	resume();
};


#endif // HTMLQT_H
