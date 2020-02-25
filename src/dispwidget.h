// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include <QDebug>
#include <QTime>
#include <QWidget>

class CHtmlDispLink;
class CHtmlFormatter;
class CHtmlSysWinQt;

/* The widget where CHtmlSysWin* performs actual paint operations.  It also
 * handles mouse events.
 */
class DisplayWidget: public QWidget
{
    Q_OBJECT

public:
    DisplayWidget(CHtmlSysWinQt& parent, CHtmlFormatter& formatter_);
    ~DisplayWidget() override;

    // When our parent's notify_clear_contents() is called, we need to know
    // about it so we can perform link tracking invalidation.
    void notifyClearContents()
    {
        // When clearing contents, the display items are already gone. Set them
        // Null so we won't try to access them.
        fClickedLink = fHoverLink = nullptr;
        fInvalidateLinkTracking();
    }

    virtual void clearSelection();
    static auto selectedText() -> QString;
    void updateLinkTracking(QPoint pos);

protected:
    bool inSelectMode = false;
    CHtmlSysWinQt& parentSysWin;
    CHtmlFormatter& formatter_;

    // Holds the widget that currently has an active selection range, or null
    // if there's no active selection.
    static DisplayWidget* curSelWidget;

    void paintEvent(QPaintEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    // We track the current link the mouse is currently hovering over and the
    // link over which the mouse button has been pressed but not released yet.
    CHtmlDispLink* fHoverLink = nullptr;
    CHtmlDispLink* fClickedLink = nullptr;

    QPoint fSelectOrigin;
    bool fHasSelection = false;
    QPoint fDragStartPos;
    QTime fLastDoubleClick;

    void fInvalidateLinkTracking();
    auto fMySelectedText() const -> QString;
    void fHandleDoubleOrTripleClick(const QMouseEvent& e, bool tripleClick);
    void fSyncClipboard() const;
};

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
