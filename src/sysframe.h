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
#ifndef SYSFRAME_H
#define SYSFRAME_H

#include <QApplication>

#include "htmlsys.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysFrameQt: public QApplication, public CHtmlSysFrame {
	Q_OBJECT

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
	class CHtmlSysWinGroupQt* fMainWin;

	// Main HTML window.
	class CHtmlSysWinInputQt* fGameWin;

	// Tads HTML parser.
	class CHtmlParser* fParser;

	// Tads HTML formatter for the main game window.
	class CHtmlFormatterInput* fFormatter;

	// List of banners.
	QList<class CHtmlSysWinQt*> fBannerList;

	// List of orphaned banners.
	QList<class CHtmlFormatterBannerExt*> fOrhpanBannerList;

	// Fonts we created. We keep a list of every font we created since we're
	// responsible for deleting them when they're no longer needed.
	QList<class CHtmlSysFontQt*> fFontList;

	// Are we currently executing a game?
	bool fGameRunning;

	// Is the game we're currently running (if we're running one) a Tads 3
	// game?
	bool fTads3;

	// The game we should try to run after the current one ends.
	QString fNextGame;

	// Is there a reformat pending?
	bool fReformatPending;

	// Current input font color.
	HTML_color_t fInputColor;

	// Run the game file contained in fNextGame.
	void
	fRunGame();

	void
	fRunT2Game( const QString& fname );

	void
	fRunT3Game( const QString& fname );

  signals:
	void gameStarting();
	void gameQuitting();
	void gameHasQuit();

  public slots:
	// Replacement for main().  We need this so that we can start the Tads VM
	// after the QApplication main event loop has started.
	void
	main( QString gameFileName );

  public:
	CHtmlSysFrameQt( int& argc, char* argv[], const char* appName, const char* appVersion, const char* orgName,
					 const char* orgDomain );

	virtual
	~CHtmlSysFrameQt();

	class QTadsSettings*
	settings()
	{ return this->fSettings; }

	class CHtmlSysWinInputQt*
	gameWindow()
	{ return this->fGameWin; }

	HTML_color_t
	inputColor()
	{ return this->fInputColor; }

	void
	inputColor( HTML_color_t color )
	{ this->fInputColor = color; }

	CHtmlSysFontQt*
	createFont( const CHtmlFontDesc* font_desc );

	bool
	gameRunning()
	{ return this->fGameRunning; }

	void
	setGameRunning( bool f )
	{
		this->fGameRunning = f;
		if (f == false) {
			emit gameQuitting();
		}
	}

	void
	setNextGame( const QString& fname )
	{
		this->fNextGame = fname;
		// If no game is currently executing, run it now. Otherwise, end the
		// current game.
		if (not this->fGameRunning) {
			this->fRunGame();
		} else {
			this->setGameRunning(false);
		}
	}

	bool
	tads3()
	{ return this->fTads3; }

	// Return a list of all banners that are childs of 'parent'.
	const QList<CHtmlSysWinQt*>&
	childBannersOf( const CHtmlSysWinQt* parent );

	// Recalculate and adjust the sizes of all HTML banners.
	void
	adjustBannerSizes();

	// Reformat all HTML banners.
	void
	reformatBanners();

	// Schedule a reformat.
	void
	scheduleReformat()
	{ this->fReformatPending = true; }

	// Prune the main window's parse tree, if we're using too much memory.
	// This should be called before getting user input; we'll check to see how
	// much memory the parse tree is taking up, and cut it down a bit if it's
	// too big.
	// TODO: Implement configurable max memory size.
	void
	pruneParseTree();

	// Notify the application that preferences have changed.
	void
	notifyPreferencesChange( const class QTadsSettings* sett );

	// Advance the event loop.
	void
	advanceEventLoop( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents )
	{
		// DeferredDelete events need to be dispatched manually, since we don't
		// return to the main event loop while a game is running.
#ifndef Q_WS_MAC
		// On OS X, this causes CPU utilization to go through the roof.  Not
		// sure why.  Disable this for now on OS X until further information
		// is available on this.
		this->sendPostedEvents(0, QEvent::DeferredDelete);
#endif
		this->sendPostedEvents();
		this->processEvents(flags);
		this->sendPostedEvents();
	}

	// Advance the event loop with a timeout.
	void
	advanceEventLoop( QEventLoop::ProcessEventsFlags flags, int maxtime )
	{
#ifndef Q_WS_MAC
		this->sendPostedEvents(0, QEvent::DeferredDelete);
#endif
		this->sendPostedEvents();
		this->processEvents(flags, maxtime);
		this->sendPostedEvents();
	}

	//
	// CHtmlSysFrame interface implementation.
	//
	virtual void
	flush_txtbuf( int fmt, int immediate_redraw );

	virtual class CHtmlParser*
	get_parser()
	{ return this->fParser; }

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


#endif
