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
#ifndef DISPWIDGETINPUT_H
#define DISPWIDGETINPUT_H

#include "dispwidget.h"
#include "config.h"


class DisplayWidgetInput: public DisplayWidget {
    Q_OBJECT

  private:
    // Position of the text cursor.
    QPoint fCursorPos;

    // Height of the text cursor in pixels.
    unsigned fHeight;

    // Last position of the text cursor.
    QPoint fLastCursorPos;

    // Is the text cursor visible?
    bool fCursorVisible;

    // Text cursor blink visibility.  Changed by a timer to show/hide the
    // cursor at specific intervals if fCursorVisible is true.
    bool fBlinkVisible;

    // Text cursor blink timer.
    class QTimer* fBlinkTimer;

  private slots:
    // Called by the timer to blink the text cursor.
    void
    fBlinkCursor();

    // We need to know when the application loses focus entirely so that we
    // can disable keyboard cursor blinking when we lose focus.
    void
    fHandleFocusChange( QWidget* old, QWidget* now );

  protected:
    void
    paintEvent( QPaintEvent* e ) override;

  public:
    DisplayWidgetInput( class CHtmlSysWinQt* parent, class CHtmlFormatter* formatter );

    // Change the text cursor position.
    void
    moveCursorPos( const QPoint& pos )
    { this->fCursorPos = pos; }

    // Set the height of the text cursor in pixels.
    void
    setCursorHeight( unsigned height )
    { this->fHeight = height; }

    // Show/hide the text cursor.
    void
    setCursorVisible( bool visible )
    { this->fCursorVisible = visible; }

    bool
    isCursorVisible()
    { return this->fCursorVisible; }

    void
    updateCursorPos( class CHtmlFormatter* formatter, class CHtmlInputBuf* tadsBuffer,
                     class CHtmlTagTextInput* tag );

    // Reset cursor blink timer.  This will read the blinking rate from the
    // desktop environment and ajust the blink timer as needed.
    void
    resetCursorBlinking();
};


#endif
