// This is copyrighted software. More information is at the end of this file.
#include "dispwidgetinput.h"

#include "htmldisp.h"
#include "htmlfmt.h"
#include "htmlinp.h"
#include "htmltags.h"
#include "settings.h"
#include "syswin.h"
#include "syswingroup.h"
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

DisplayWidgetInput::DisplayWidgetInput(
    CHtmlSysWinQt& parent, CHtmlFormatter& formatter, CHtmlInputBuf& tadsBuffer)
    : DisplayWidget(parent, formatter)
    , fHeight(QFontMetrics(qFrame->settings().inputFont).height() - 1)
    , fTadsBuffer(tadsBuffer)
{
    connect(&fBlinkTimer, &QTimer::timeout, this, &DisplayWidgetInput::fBlinkCursor);
    resetCursorBlinking();

    // We need to check whether the application lost focus.
    connect(qApp, &QApplication::focusChanged, this, &DisplayWidgetInput::fHandleFocusChange);
}

void DisplayWidgetInput::paintEvent(QPaintEvent* const e)
{
    // qDebug() << Q_FUNC_INFO << "called";

    DisplayWidget::paintEvent(e);
    QPainter painter(this);
    if (fCursorVisible and fBlinkVisible) {
        painter.drawLine(fCursorPos.x(), fCursorPos.y(), fCursorPos.x(), fCursorPos.y() + fHeight);
    }
}

void DisplayWidgetInput::resizeEvent(QResizeEvent* const e)
{
    DisplayWidget::resizeEvent(e);
    QTimer::singleShot(0, this, [this] { updateCursorPos(formatter_, true, false); });
}

void DisplayWidgetInput::mousePressEvent(QMouseEvent* const e)
{
    DisplayWidget::mousePressEvent(e);
    if (not fInpTag or not e->isAccepted()) {
        return;
    }

    // We have a current input tag. If the mouse press was inside it, update
    // the input caret position.
    const auto offs = formatter_.find_textofs_by_pos(CHtmlPoint(e->x(), e->y()));
    if (offs >= fInpTag->get_text_ofs()) {
        fTadsBuffer.set_caret(offs - fInpTag->get_text_ofs());
        updateCursorPos(formatter_, false, true);
    }
}

void DisplayWidgetInput::mouseMoveEvent(QMouseEvent* const e)
{
    DisplayWidget::mouseMoveEvent(e);

    if (not inSelectMode or (~e->buttons() & Qt::LeftButton) or not fInpTag) {
        return;
    }

    // If we have a selection and it extends into the input tag, sync the
    // selection range with the tag.
    unsigned long selStart, selEnd;
    formatter_.get_sel_range(&selStart, &selEnd);
    if (selEnd < fInpTag->get_text_ofs()) {
        // The selection doesn't extend into our input tag.
        return;
    }

    // Figure out where to put the caret.
    const auto offs = formatter_.find_textofs_by_pos({e->x(), e->y()});
    const auto caretPos = offs > fInpTag->get_text_ofs() ? offs - fInpTag->get_text_ofs() : 0;

    // If there's no selection, just update the caret position. Otherwise,
    // also sync the input tag's selection with the formatter's.
    if (selStart == selEnd) {
        fTadsBuffer.set_caret(caretPos);
    } else {
        const auto inpSelStart =
            selStart > fInpTag->get_text_ofs() ? selStart - fInpTag->get_text_ofs() : 0;
        const auto inpSelEnd = selEnd - fInpTag->get_text_ofs();
        fTadsBuffer.set_sel_range(inpSelStart, inpSelEnd, caretPos);
    }
    updateCursorPos(formatter_, true, false);
}

void DisplayWidgetInput::fBlinkCursor()
{
    fBlinkVisible = not fBlinkVisible;
    update(
        fCursorPos.x() - 5, fCursorPos.y() - 5, fCursorPos.x() + 5, fCursorPos.y() + fHeight + 5);
}

void DisplayWidgetInput::fHandleFocusChange(const QWidget* const old, const QWidget* const now)
{
    if (now == nullptr) {
        // The application window lost focus.  Disable cursor blinking.
        fBlinkTimer.stop();
#ifdef Q_OS_MAC
        // On the Mac, when applications lose focus the cursor must be disabled.
        if (fBlinkVisible) {
            fBlinkCursor();
        }
#else
        // On all other systems we assume the cursor must stay visible.
        if (not fBlinkVisible) {
            fBlinkCursor();
        }
#endif
    } else if (old == nullptr and now != nullptr) {
        // The application window gained focus.  Reset cursor blinking.
        resetCursorBlinking();
    }
}

void DisplayWidgetInput::updateCursorPos(
    CHtmlFormatter& formatter, const bool keepSelection, const bool updateFormatterSelection)
{
    // Ignore the call if there's currently no active tag.
    if (fInpTag == nullptr) {
        return;
    }

    // Reset the blink timer.
    if (fBlinkTimer.isActive()) {
        fBlinkTimer.start();
    }

    // Blink-out first to ensure the cursor won't stay visible at the previous
    // position after we move it.
    if (fBlinkVisible) {
        fBlinkCursor();
    }

    const auto cursorPos =
        formatter.get_text_pos(fInpTag->get_text_ofs() + fTadsBuffer.get_caret());
    fCursorPos = QPoint(cursorPos.x, cursorPos.y);
    // If there's another window with an active selection, moving the cursor
    // must clear it, unless we've been explicitly told otherwise.
    if (not keepSelection and DisplayWidget::curSelWidget and DisplayWidget::curSelWidget != this) {
        DisplayWidget::curSelWidget->clearSelection();
    }

    // Update the selection range in the formatter. If there's actually any
    // text selected, then mark us as the active selection widget and enable
    // the "Copy" action.
    if (updateFormatterSelection) {
        size_t start, end, caret;
        const auto inp_txt_ofs = fInpTag->get_text_ofs();
        fTadsBuffer.get_sel_range(&start, &end, &caret);
        if (start != end) {
            formatter.set_sel_range(start + inp_txt_ofs, end + inp_txt_ofs);
            DisplayWidget::curSelWidget = this;
            qWinGroup->enableCopyAction(true);
        }
    }

    // Blink-in.
    if (not fBlinkVisible) {
        fBlinkCursor();
    }
}

void DisplayWidgetInput::resetCursorBlinking()
{
    // Start the timer unless cursor blinking is disabled.
    if (QApplication::cursorFlashTime() > 1) {
        fBlinkTimer.start(QApplication::cursorFlashTime() / 2);
    }
}

void DisplayWidgetInput::clearSelection()
{
    DisplayWidget::clearSelection();
    if (fTadsBuffer.has_sel_range()) {
        fTadsBuffer.set_sel_range(0, 0, fTadsBuffer.get_caret());
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
