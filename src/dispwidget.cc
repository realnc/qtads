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

#include <QDebug>
#include <QPaintEvent>
#include <QStatusBar>
#include <QDrag>
#include <QMimeData>

#include "htmlattr.h"
#include "htmlfmt.h"
#include "htmldisp.h"

#include "dispwidget.h"
#include "settings.h"
#include "syswininput.h"


DisplayWidget* DisplayWidget::curSelWidget = 0;


DisplayWidget::DisplayWidget( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
    : QWidget(parent),
      fHoverLink(0),
      fClickedLink(0),
      fInSelectMode(false),
      fHasSelection(false),
      parentSysWin(parent),
      formatter(formatter)
{
    this->setForegroundRole(QPalette::Text);
    this->setBackgroundRole(QPalette::Base);

    // Enable mouse tracking, since we need to change the mouse cursor shape
    // when hovering over hyperlinks.
    this->setMouseTracking(true);
}


DisplayWidget::~DisplayWidget()
{
    // If we currently have an active selection, make sure no one tries to
    // access it anymore.
    if (DisplayWidget::curSelWidget == this) {
        qWinGroup->enableCopyAction(false);
        DisplayWidget::curSelWidget = 0;
    }
}


void
DisplayWidget::fInvalidateLinkTracking()
{
    // If we're tracking links (hover/click), forget about them.
    if (this->fClickedLink != 0) {
        this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
        this->fClickedLink = 0;
    }
    if (this->fHoverLink != 0) {
        this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
        this->fHoverLink = 0;
    }
    this->unsetCursor();
    qWinGroup->statusBar()->setUpdatesEnabled(false);
    qWinGroup->statusBar()->clearMessage();
    qWinGroup->statusBar()->setUpdatesEnabled(true);
}


QString
DisplayWidget::fMySelectedText()
{
    unsigned long startOfs, endOfs;
    this->formatter->get_sel_range(&startOfs, &endOfs);

    if (startOfs == endOfs) {
        // There's nothing selected.
        return QString();
    }

    // Figure out how much space we need.
    unsigned long len = this->formatter->get_chars_in_ofs_range(startOfs, endOfs);

    // Get the text in the internal format.
    CStringBuf buf(len);
    this->formatter->extract_text(&buf, startOfs, endOfs);
    return QString::fromUtf8(buf.get(), len);
}


void
DisplayWidget::paintEvent( QPaintEvent* e )
{
    //qDebug() << Q_FUNC_INFO << "called";

    //qDebug() << "repainting" << e->rect();
    const QRect& qRect = e->region().boundingRect();
    CHtmlRect cRect(qRect.left(), qRect.top(), qRect.left() + qRect.width(), qRect.top() + qRect.height());
    this->formatter->draw(&cRect, false, 0);
}


void
DisplayWidget::mouseMoveEvent( QMouseEvent* e )
{   
    if (e->buttons() & Qt::LeftButton) {
        // If we're tracking a selection, update the selection range.
        if (this->fInSelectMode) {
            this->formatter->set_sel_range(CHtmlPoint(this->fSelectOrigin.x(),
                                                      this->fSelectOrigin.y()),
                                           CHtmlPoint(e->pos().x(), e->pos().y()),
                                           0, 0);
            return;
        }
        // We're not tracking a selection, but the mouse is inside of one.
        // If there's enough distance since the start of the drag, start a
        // drag event containing the selected text.
        if (this->fHasSelection
            and (e->pos() - this->fDragStartPos).manhattanLength()
                > QApplication::startDragDistance())
        {
            QDrag* drag = new QDrag(this);
            QMimeData* mime = new QMimeData;
            mime->setText(this->fMySelectedText());
            drag->setMimeData(mime);
            drag->exec(Qt::CopyAction);
            this->fInvalidateLinkTracking();
            return;
        }
    }

    // This wasn't a selection event. Just update link tracking.
    this->updateLinkTracking(e->pos());
}


void
DisplayWidget::leaveEvent( QEvent* )
{
    this->fInvalidateLinkTracking();
}


void
DisplayWidget::mousePressEvent( QMouseEvent* e )
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (this->fHoverLink == 0) {
        // We're not hover-tracking a link. Start selection mode if we're not
        // already in that mode.
        if (this->fInSelectMode) {
            return;
        }

        // We're not tracking a selection, but if we have a selection and the
        // mouse was inside it, then prepare for a drag start operation.
        unsigned long selStart, selEnd;
        this->formatter->get_sel_range(&selStart, &selEnd);
        unsigned long mousePos = this->formatter->find_textofs_by_pos(CHtmlPoint(e->pos().x(),
                                                                                 e->pos().y()));
        if (mousePos > selStart and mousePos < selEnd) {
            this->fDragStartPos = e->pos();
            return;
        }

        this->fInSelectMode = true;
        this->fSelectOrigin = e->pos();
        // Just in case we had a selection previously, clear it.
        this->clearSelection();
        // If another widget also has an active selection, clear it.
        if (curSelWidget != 0 and curSelWidget != this) {
            curSelWidget->clearSelection();
        }
        // Remember that we're the widget with an active selection.
        curSelWidget = this;
        qWinGroup->enableCopyAction(true);
        return;
    }

    // We're hover-tracking a link. Click-track it if it's clickable.
    if (this->fHoverLink->is_clickable_link() and qFrame->settings()->enableLinks) {
        // Draw all of the items involved in the link in the hilited state.
        this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_clicked);
        this->fClickedLink = this->fHoverLink;
    }
}


void
DisplayWidget::mouseReleaseEvent( QMouseEvent* e )
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (this->fInSelectMode) {
        // Releasing the button ends selection mode.
        this->fInSelectMode = false;
        // If the selection is empty, there would be nothing to copy.
        if (this->fMySelectedText().isEmpty()) {
            qWinGroup->enableCopyAction(false);
        } else {
            this->fHasSelection = true;
        }
        return;
    }

    // We're not tracking a selection but if we do have selected text, clear
    // it.
    if (this->fHasSelection) {
        this->clearSelection();
    }

    if (this->fClickedLink == 0) {
        // We're not click-tracking a link; there's nothing else to do here.
        return;
    }

    // If we're still hovering over the clicked link, process it.
    if (this->fClickedLink == this->fHoverLink) {
        const textchar_t* cmd = this->fClickedLink->href_.get_url();
        qFrame->gameWindow()->processCommand(cmd, strlen(cmd), this->fClickedLink->get_append(),
                                             not this->fClickedLink->get_noenter(), OS_CMD_NONE);
        // Put it back in hovering mode.
        if (qFrame->settings()->highlightLinks) {
            this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
        }
    // Otherwise, if we're hovering over another link, put that one in hover mode.
    } else if (this->fHoverLink != 0) {
        this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
    }
    // Stop click-tracking it.
    this->fClickedLink = 0;
}


void
DisplayWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    // Get the offsets of the current word.
    unsigned long start, end, offs;
    offs = this->formatter->find_textofs_by_pos(CHtmlPoint(e->x(), e->y()));
    this->formatter->get_word_limits(&start, &end, offs);

    // If this is a Ctrl+double-click, and pasting is enabled in the settings,
    // paste the word into the current line input. Otherwise, just select the
    // word.
    if (qFrame->settings()->pasteOnDblClk
        and (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        CStringBuf strBuf;
        this->formatter->extract_text(&strBuf, start, end);
        QString txt(QString::fromUtf8(strBuf.get()).trimmed());
        if (not txt.isEmpty()) {
            qFrame->gameWindow()->insertText(txt + QChar::fromLatin1(' '));
        }
    } else {
        this->formatter->set_sel_range(start, end);
        qWinGroup->enableCopyAction(true);
        this->fInSelectMode = true;
        this->fHasSelection = true;
    }
}


void
DisplayWidget::clearSelection()
{
    // Clear the selection range in the formatter by setting both ends
    // of the range to the maximum text offset in the formatter's
    // display list.
    this->fHasSelection = false;
    unsigned long start = this->formatter->get_text_ofs_max();
    this->formatter->set_sel_range(start, start);
    // If we're the widget with the active selection, also disable the copy
    // action, since clearing the selection means there's nothing to copy.
    if (DisplayWidget::curSelWidget == this) {
        qWinGroup->enableCopyAction(false);
    }
}


QString
DisplayWidget::selectedText()
{
    if (DisplayWidget::curSelWidget == 0) {
        // There's no active selection.
        return QString();
    }
    return DisplayWidget::curSelWidget->fMySelectedText();
}


void
DisplayWidget::updateLinkTracking( const QPoint& mousePos )
{
    // Get the display object containing the position.
    CHtmlPoint docPos;
    // If specified mouse position is invalid, map it from the current global
    // position.
    if (mousePos.isNull()) {
        const QPoint pos(this->mapFromGlobal(QCursor::pos()));
        docPos.set(pos.x(), pos.y());
    } else {
        docPos.set(mousePos.x(), mousePos.y());
    }
    CHtmlDisp* disp = this->formatter->find_by_pos(docPos, true);

    // If there's nothing, no need to continue.
    if (disp == 0) {
        // If we were tracking anything, forget about it.
        if (this->fHoverLink != 0) {
            this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
            this->fHoverLink = 0;
            this->unsetCursor();
            qWinGroup->statusBar()->setUpdatesEnabled(false);
            qWinGroup->statusBar()->clearMessage();
            qWinGroup->statusBar()->setUpdatesEnabled(true);
        }
        return;
    }

    // It could be a link.
    if (qFrame->settings()->enableLinks) {
        CHtmlDispLink* link = disp->get_link(this->formatter, docPos.x, docPos.y);

        // If we're already tracking a hover over this link, we don't need to
        // do anything else.
        if (link == this->fHoverLink) {
            return;
        }

        // If we're tracking a hover and it's a new link, track this one and
        // forget about the previous one.
        if (link != this->fHoverLink) {
            if (this->fHoverLink != 0) {
                this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
            }
            this->fHoverLink = link;
        }

        // If it's a clickable link, also change the mouse cursor shape and
        // hovering color.
        if (link != 0 and link->is_clickable_link()) {
            this->setCursor(Qt::PointingHandCursor);
            // Only change its color if we're not click-tracking another link.
            if (qFrame->settings()->highlightLinks and this->fClickedLink == 0) {
                link->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
            }
        }

        // If we found something that has ALT text, show it in the status bar.
        if (disp->get_alt_text() != 0 and strlen(disp->get_alt_text()) > 0) {
            qWinGroup->statusBar()->setUpdatesEnabled(false);
            qWinGroup->statusBar()->showMessage(QString::fromUtf8(disp->get_alt_text()));
            qWinGroup->statusBar()->setUpdatesEnabled(true);
            return;
        }

        // It could be a clickable link without any ALT text.  In that case, show
        // its contents.
        if (link != 0 and link->is_clickable_link()) {
            qWinGroup->statusBar()->setUpdatesEnabled(false);
            qWinGroup->statusBar()->showMessage(QString::fromUtf8(link->href_.get_url()));
            qWinGroup->statusBar()->setUpdatesEnabled(true);
            return;
        }
    }

    // We don't know what it was.  Clear status bar message, reset cursor shape
    // and forget about any link we were tracking.
    qWinGroup->statusBar()->setUpdatesEnabled(false);
    qWinGroup->statusBar()->clearMessage();
    qWinGroup->statusBar()->setUpdatesEnabled(true);
    this->unsetCursor();
    if (this->fHoverLink != 0) {
        this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
        this->fHoverLink = 0;
    }
}
