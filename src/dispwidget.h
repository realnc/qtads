/* Copyright (C) 2013 Nikos Chantziaras.
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
#ifndef DISPWIDGET_H
#define DISPWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QTime>

#include "config.h"


/* The widget where CHtmlSysWin* performs actual paint operations.  It also
 * handles mouse events.
 */
class DisplayWidget: public QWidget {
    Q_OBJECT

  private:
    // We track the current link the mouse is currently hovering over and the
    // link over which the mouse button has been pressed but not released yet.
    class CHtmlDispLink* fHoverLink;
    class CHtmlDispLink* fClickedLink;

    // Origin of current selection range.
    QPoint fSelectOrigin;

    // Do we currently have a selection?
    bool fHasSelection;

    // Drag start position.
    QPoint fDragStartPos;

    // Time of last double-click event.
    QTime fLastDoubleClick;

    // Stop tracking links.
    void
    fInvalidateLinkTracking();

    // Returns the text currently selected in this window.
    QString
    fMySelectedText();

    void
    fHandleDoubleOrTripleClick( QMouseEvent* e, bool tripleClick );

    void
    fSyncClipboard();

  protected:
    // Are we in text selection mode?
    bool inSelectMode;

    // The window we're embeded in.
    class CHtmlSysWinQt* parentSysWin;

    // Our parent's formatter, for easy access.
    class CHtmlFormatter* formatter;

    // Holds the widget that currently has an active selection range, or null
    // if there's no active selection.
    static DisplayWidget* curSelWidget;

    void
    paintEvent( QPaintEvent* e ) override;

    void
    mouseMoveEvent( QMouseEvent* e ) override;

    void
    leaveEvent( QEvent* e ) override;

    void
    mousePressEvent( QMouseEvent* e ) override;

    void
    mouseReleaseEvent( QMouseEvent* e ) override;

    void
    mouseDoubleClickEvent( QMouseEvent* e );

  public:
    DisplayWidget( class CHtmlSysWinQt* parent, class CHtmlFormatter* formatter );
    ~DisplayWidget() override;

    // When our parent's notify_clear_contents() is called, we need to know
    // about it so we can perform link tracking invalidation.
    void
    notifyClearContents()
    {
        // When clearing contents, the display items are already gone. Set them
        // Null so we won't try to access them.
        this->fClickedLink = this->fHoverLink = 0;
        this->fInvalidateLinkTracking();
    }

    // Clear the selection range.
    virtual void
    clearSelection();

    // Get the text contained in the currently active selection. Returns a
    // null string is there's currently no display widget with an active
    // selection.
    static QString
    selectedText();

    // Update link tracking for specified mouse position.  If the specified
    // position isNull(), it will be autodetected.
    //
    // TODO: What happens with multi-pointer systems?
    void
    updateLinkTracking( const QPoint& pos );
};


#endif
