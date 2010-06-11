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
#ifndef SYSWINGROUP_H
#define SYSWINGROUP_H

#include <QMainWindow>
#include <QScrollArea>

#include "htmlsys.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysWinGroupQt: public QMainWindow, public CHtmlSysWinGroup {
	Q_OBJECT

  private:
	class QTadsFrame: public QFrame {
	  protected:
		virtual void
		resizeEvent( QResizeEvent* e );

	  public:
		QTadsFrame( QWidget* parent )
		: QFrame(parent)
		{ }
	};

	class QTadsConfDialog* fConfDialog;
	QScrollArea* fScrollArea;
	QTadsFrame* fFrame;
	class QDialog* fAboutBoxDialog;
	class CHtmlSysWinAboutBoxQt* fAboutBox;
	class QMenu* fRecentGamesMenu;
	class QAction* fAboutGameAction;
	class QAction* fEndCurrentGameAction;

	bool
	fAskQuitGameDialog();

  private slots:
	void
	fShowConfDialog();

	void
	fHideConfDialog();

	void
	fShowAboutGame();

	void
	fOpenNewGame();

	void
	fRecentGameTriggered( QAction* action );

	bool
	fEndCurrentGame();

	void
	fNotifyGameQuitting();

	void
	fNotifyGameStarting();

  protected:
	virtual void
	closeEvent( QCloseEvent* e );

  public:
	CHtmlSysWinGroupQt();

	virtual
	~CHtmlSysWinGroupQt();

	CHtmlSysWinAboutBoxQt*
	createAboutBox( class CHtmlFormatter* formatter );

	void
	deleteAboutBox();

	CHtmlSysWinAboutBoxQt*
	aboutBox()
	{ return this->fAboutBox; }

	void
	updateRecentGames();

	//
	// CHtmlSysWinGroup interface implementation.
	//
	virtual oshtml_charset_id_t
	get_default_win_charset() const;

	virtual size_t
	xlat_html4_entity( textchar_t* result, size_t result_size, unsigned int charval, oshtml_charset_id_t* charset,
					   int* changed_charset );
};


#endif
