// This is copyrighted software. More information is at the end of this file.
#include "dispwidget.h"

#include "htmlattr.h"
#include "htmldisp.h"
#include "htmlfmt.h"
#include "settings.h"
#include "syswininput.h"
#include <QClipboard>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QPaintEvent>
#include <QStatusBar>

DisplayWidget* DisplayWidget::curSelWidget = nullptr;

DisplayWidget::DisplayWidget(CHtmlSysWinQt& parent, CHtmlFormatter& formatter)
    : QWidget(&parent)
    , parentSysWin(parent)
    , formatter_(formatter)
{
    setForegroundRole(QPalette::Text);
    setBackgroundRole(QPalette::Base);
    setMouseTracking(true);
}

DisplayWidget::~DisplayWidget()
{
    // If we currently have an active selection, make sure no one tries to
    // access it anymore.
    if (curSelWidget == this) {
        qWinGroup->enableCopyAction(false);
        curSelWidget = nullptr;
    }
}

void DisplayWidget::fInvalidateLinkTracking()
{
    // If we're tracking links (hover/click), forget about them.
    if (fClickedLink != nullptr) {
        fClickedLink->set_clicked(&parentSysWin, CHtmlDispLink_none);
        fClickedLink = nullptr;
    }
    if (fHoverLink != nullptr) {
        fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_none);
        fHoverLink = nullptr;
    }
    unsetCursor();
    qWinGroup->statusBar()->setUpdatesEnabled(false);
    qWinGroup->statusBar()->clearMessage();
    qWinGroup->statusBar()->setUpdatesEnabled(true);
}

auto DisplayWidget::fMySelectedText() const -> QString
{
    unsigned long startOfs, endOfs;
    formatter_.get_sel_range(&startOfs, &endOfs);

    if (startOfs == endOfs) {
        return {};
    }

    CStringBuf buf(formatter_.get_chars_in_ofs_range(startOfs, endOfs));
    formatter_.extract_text(&buf, startOfs, endOfs);
    return QString::fromUtf8(buf.get());
}

void DisplayWidget::fHandleDoubleOrTripleClick(const QMouseEvent& e, const bool tripleClick)
{
    unsigned long start, end;
    const auto offset = formatter_.find_textofs_by_pos({e.x(), e.y()});
    if (tripleClick) {
        formatter_.get_line_limits(&start, &end, offset);
    } else {
        formatter_.get_word_limits(&start, &end, offset);
    }

    // If this is a Ctrl+double-click, and pasting is enabled in the settings,
    // paste the word into the current line input. Otherwise, just select the
    // word.
    if (qFrame->settings().pasteOnDblClk and not tripleClick
        and (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        CStringBuf strBuf;
        formatter_.extract_text(&strBuf, start, end);
        auto text = QString::fromUtf8(strBuf.get()).trimmed();
        if (not text.isEmpty()) {
            qFrame->gameWindow()->insertText(text + ' ');
        }
    } else if (~QApplication::keyboardModifiers() & Qt::ControlModifier) {
        formatter_.set_sel_range(start, end);
        fSyncClipboard();
        qWinGroup->enableCopyAction(true);
        inSelectMode = true;
        fHasSelection = true;
    }
}

void DisplayWidget::fSyncClipboard() const
{
    const auto text = fMySelectedText();
    if (text.isEmpty() or not QApplication::clipboard()->supportsSelection()) {
        return;
    }
    QApplication::clipboard()->setText(text, QClipboard::Selection);
}

void DisplayWidget::paintEvent(QPaintEvent* const e)
{
    // qDebug() << Q_FUNC_INFO << "called";

    // qDebug() << "repainting" << e->rect();
    const auto qRect = e->region().boundingRect();
    CHtmlRect cRect(
        qRect.left(), qRect.top(), qRect.left() + qRect.width(), qRect.top() + qRect.height());
    formatter_.draw(&cRect, false, nullptr);
}

void DisplayWidget::mouseMoveEvent(QMouseEvent* const e)
{
    // Avoid registering a triple click directly after the mouse was moved.
    fLastDoubleClick = {};

    if (e->buttons() & Qt::LeftButton) {
        // If we're tracking a selection, update the selection range.
        if (inSelectMode) {
            formatter_.set_sel_range(
                {fSelectOrigin.x(), fSelectOrigin.y()}, {e->pos().x(), e->pos().y()}, nullptr,
                nullptr);
            fSyncClipboard();
            return;
        }
        // We're not tracking a selection, but the mouse is inside of one.
        // If there's enough distance since the start of the drag, start a
        // drag event containing the selected text.
        if (fHasSelection
            and (e->pos() - fDragStartPos).manhattanLength() > QApplication::startDragDistance())
        {
            auto* const drag = new QDrag(this);
            auto* const mime = new QMimeData;
            mime->setText(fMySelectedText());
            drag->setMimeData(mime);
            drag->exec(Qt::CopyAction);
            fInvalidateLinkTracking();
            return;
        }
    }

    // This wasn't a selection event. Just update link tracking.
    updateLinkTracking(e->pos());
}

void DisplayWidget::leaveEvent(QEvent* /*const e*/)
{
    fInvalidateLinkTracking();
}

void DisplayWidget::mousePressEvent(QMouseEvent* const e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (fHoverLink != nullptr) {
        // We're hover-tracking a link. Click-track it if it's clickable.
        if (fHoverLink->is_clickable_link() and qFrame->settings().enableLinks) {
            // Draw all of the items involved in the link in the hilited state.
            fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_clicked);
            fClickedLink = fHoverLink;
        }
        return;
    }

    // We're not hover-tracking a link. Start selection mode if we're not
    // already in that mode.
    if (inSelectMode) {
        return;
    }

    // If there was a double-click previously and not too much time has
    // passed, then this is a triple-click.
    if (fLastDoubleClick.isValid()
        and fLastDoubleClick.msecsTo(QTime::currentTime()) <= QApplication::doubleClickInterval())
    {
        fHandleDoubleOrTripleClick(*e, true);
        return;
    }

    // We're not tracking a selection, but if we have a selection and the
    // mouse was inside it, then prepare for a drag start operation.
    unsigned long selStart, selEnd;
    formatter_.get_sel_range(&selStart, &selEnd);
    auto mousePos = formatter_.find_textofs_by_pos({e->pos().x(), e->pos().y()});
    if (mousePos > selStart and mousePos < selEnd) {
        fDragStartPos = e->pos();
        return;
    }

    inSelectMode = true;
    fSelectOrigin = e->pos();
    // Just in case we had a selection previously, clear it.
    clearSelection();
    // If another widget also has an active selection, clear it.
    if (curSelWidget != nullptr and curSelWidget != this) {
        curSelWidget->clearSelection();
    }
    // Remember that we're the widget with an active selection.
    curSelWidget = this;
    qWinGroup->enableCopyAction(true);
}

void DisplayWidget::mouseReleaseEvent(QMouseEvent* const e)
{
    if (e->button() == Qt::MiddleButton and QApplication::clipboard()->supportsSelection()) {
        qFrame->gameWindow()->insertText(QApplication::clipboard()->text(QClipboard::Selection));
        return;
    }

    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (inSelectMode) {
        // Releasing the button ends selection mode.
        inSelectMode = false;
        // If the selection is empty, there would be nothing to copy.
        if (fMySelectedText().isEmpty()) {
            qWinGroup->enableCopyAction(false);
        } else {
            fHasSelection = true;
        }
        return;
    }

    // We're not tracking a selection but if we do have selected text, clear
    // it.
    if (fHasSelection) {
        clearSelection();
    }

    if (fClickedLink == nullptr) {
        // We're not click-tracking a link; there's nothing else to do here.
        return;
    }

    // If we're still hovering over the clicked link, process it.
    if (fClickedLink == fHoverLink) {
        const auto* cmd = fClickedLink->href_.get_url();
        qFrame->gameWindow()->processCommand(
            cmd, strlen(cmd), fClickedLink->get_append(), not fClickedLink->get_noenter(),
            OS_CMD_NONE);
        // Put it back in hovering mode.
        if (qFrame->settings().highlightLinks) {
            fClickedLink->set_clicked(&parentSysWin, CHtmlDispLink_hover);
        }
        // Otherwise, if we're hovering over another link, put that one in hover mode.
    } else if (fHoverLink != nullptr) {
        fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_hover);
    }
    // Stop click-tracking it.
    fClickedLink = nullptr;
}

void DisplayWidget::mouseDoubleClickEvent(QMouseEvent* const e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    // Note the time this occurred, since we need to detect triple-clicks.
    fLastDoubleClick = QTime::currentTime();
    fHandleDoubleOrTripleClick(*e, false);
}

void DisplayWidget::clearSelection()
{
    // Clear the selection range in the formatter by setting both ends
    // of the range to the maximum text offset in the formatter's
    // display list.
    fHasSelection = false;
    const auto start = formatter_.get_text_ofs_max();
    formatter_.set_sel_range(start, start);
    // If we're the widget with the active selection, also disable the copy
    // action, since clearing the selection means there's nothing to copy.
    if (DisplayWidget::curSelWidget == this) {
        qWinGroup->enableCopyAction(false);
    }
}

auto DisplayWidget::selectedText() -> QString
{
    if (DisplayWidget::curSelWidget == nullptr) {
        // There's no active selection.
        return {};
    }
    return DisplayWidget::curSelWidget->fMySelectedText();
}

void DisplayWidget::updateLinkTracking(const QPoint mousePos)
{
    // Get the display object containing the position.
    CHtmlPoint docPos;
    // If specified mouse position is invalid, map it from the current global
    // position.
    if (mousePos.isNull()) {
        const auto pos = mapFromGlobal(QCursor::pos());
        docPos.set(pos.x(), pos.y());
    } else {
        docPos.set(mousePos.x(), mousePos.y());
    }
    const auto* const disp = formatter_.find_by_pos(docPos, true);

    // If there's nothing, no need to continue.
    if (disp == nullptr) {
        // If we were tracking anything, forget about it.
        if (fHoverLink != nullptr) {
            fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_none);
            fHoverLink = nullptr;
            unsetCursor();
            qWinGroup->statusBar()->setUpdatesEnabled(false);
            qWinGroup->statusBar()->clearMessage();
            qWinGroup->statusBar()->setUpdatesEnabled(true);
        }
        return;
    }

    // It could be a link.
    if (qFrame->settings().enableLinks) {
        auto* const link = disp->get_link(&formatter_, docPos.x, docPos.y);

        // If we're already tracking a hover over this link, we don't need to
        // do anything else.
        if (link == fHoverLink) {
            return;
        }

        // If we're tracking a hover and it's a new link, track this one and
        // forget about the previous one.
        if (link != fHoverLink) {
            if (fHoverLink != nullptr) {
                fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_none);
            }
            fHoverLink = link;
        }

        // If it's a clickable link, also change the mouse cursor shape and
        // hovering color.
        if (link != nullptr and link->is_clickable_link()) {
            setCursor(Qt::PointingHandCursor);
            // Only change its color if we're not click-tracking another link.
            if (qFrame->settings().highlightLinks and fClickedLink == nullptr) {
                link->set_clicked(&parentSysWin, CHtmlDispLink_hover);
            }
        }

        // If we found something that has ALT text, show it in the status bar.
        if (disp->get_alt_text() != nullptr and strlen(disp->get_alt_text()) > 0) {
            qWinGroup->statusBar()->setUpdatesEnabled(false);
            qWinGroup->statusBar()->showMessage(QString::fromUtf8(disp->get_alt_text()));
            qWinGroup->statusBar()->setUpdatesEnabled(true);
            return;
        }

        // It could be a clickable link without any ALT text.  In that case, show
        // its contents.
        if (link != nullptr and link->is_clickable_link()) {
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
    unsetCursor();
    if (fHoverLink != nullptr) {
        fHoverLink->set_clicked(&parentSysWin, CHtmlDispLink_none);
        fHoverLink = nullptr;
    }
}

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
