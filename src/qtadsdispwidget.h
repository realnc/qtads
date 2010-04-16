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

#ifndef QTADSDISPWIDGET_H
#define QTADSDISPWIDGET_H

#include "config.h"

#include <QDebug>
#include <QWidget>


/* A QScrollArea with custom event handlers for player input.
 */
class QTadsDisplayWidget: public QWidget {
	Q_OBJECT

  private:
	// The window we're embeded in.
	class CHtmlSysWinQt* fParentSysWin;

	// Our parent's formatter, for easy access.
	class CHtmlFormatter* fFormatter;

	// Position of the text cursor.
	QPoint fCursorPos;

	// Last position of the text cursor.
	QPoint fLastCursorPos;

	// Is the text cursor visible?
	bool fCursorVisible;

	// Text cursor blink visibility.  Changed by a timer to show/hide the
	// cursor at specific intervals if fCursorVisible is true.
	bool fBlinkVisible;

	// Text cursor blink timer.
	class QTimer* fBlinkTimer;

  protected:
	virtual void
	paintEvent( QPaintEvent* e );

	//virtual void
	//mouseMoveEvent( QMouseEvent* e );

	virtual void
	mousePressEvent( QMouseEvent* e );

private slots:
	// Called by the timer to blink the text cursor.
	void
	fBlinkCursor();

  public:
	QTadsDisplayWidget( class CHtmlSysWinQt* parent, class CHtmlFormatter* formatter );

	// Change the text cursor position.
	void
	moveCursorPos( const QPoint& pos )
	{ this->fCursorPos = pos; }

	// Show/hide the text cursor.
	void
	setCursorVisible( bool visible )
	{ this->fCursorVisible = visible; }

	void
	updateCursorPos( class CHtmlFormatter* formatter, class CHtmlInputBuf* tadsBuffer, class CHtmlTagTextInput* tag );
};


#endif
