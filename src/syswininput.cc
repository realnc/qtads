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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */

#include <QClipboard>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollBar>
#include <QStatusBar>
#include <QTextCodec>
#include <QTime>
#include <QTimer>
#include <QUrl>

#include "dispwidgetinput.h"
#include "settings.h"
#include "syswininput.h"

#include "htmlfmt.h"
#include "htmlinp.h"
#include "htmlprs.h"
#include "htmltags.h"

CHtmlSysWinInputQt::CHtmlSysWinInputQt(CHtmlFormatter* formatter, QWidget* parent)
    : CHtmlSysWinQt(formatter, parent)
    , fInputMode(NoInput)
    , fInputReady(false)
    , fRestoreFromCancel(false)
    , fLastKeyEvent(Qt::Key_Any)
    , fTag(0)
{
    fInputBuffer = new textchar_t[1024];
    fInputBufferSize = 1024;
    fTadsBuffer = new CHtmlInputBuf(fInputBuffer, 1024, 100);
    fTadsBuffer->set_utf8_mode(true);

    // Replace the default display widget with an input display widget.
    formatter_->unset_win();
    delete dispWidget;
    dispWidget = new DisplayWidgetInput(this, formatter, fTadsBuffer);
    fCastDispWidget = static_cast<DisplayWidgetInput*>(dispWidget);
    setWidget(dispWidget);
    formatter_->set_win(this, &margins);

    QPalette p(palette());
    p.setColor(QPalette::Base, qFrame->settings()->mainBgColor);
    p.setColor(QPalette::Text, qFrame->settings()->mainTextColor);
    setPalette(p);
}

CHtmlSysWinInputQt::~CHtmlSysWinInputQt()
{
    delete fTadsBuffer;
    delete[] fInputBuffer;
}

void CHtmlSysWinInputQt::fStartKeypressInput()
{
    fInputReady = false;
    fInputMode = SingleKeyInput;
    fHrefEvent.clear();
}

void CHtmlSysWinInputQt::fProcessPagePauseQueue()
{
    if (fPagePauseQueue.isEmpty()) {
        return;
    }

    QLabel moreText(tr("*** MORE ***  [press space or enter to continue]"));
    // Display a permanent QLabel instead of a temporary message.  This allows
    // other status bar messages (like when hovering over hyperlinks) to
    // temporary remove the MORE text instead of replacing it.
    moreText.setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    moreText.setLineWidth(0);
    moreText.setContentsMargins(0, 0, 0, 0);
    qWinGroup->statusBar()->setUpdatesEnabled(false);
    qWinGroup->statusBar()->addWidget(&moreText);
    qWinGroup->statusBar()->setUpdatesEnabled(true);
    fInputMode = PagePauseInput;
    while (fInputMode == PagePauseInput and qFrame->gameRunning()) {
        qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
    }
    qWinGroup->statusBar()->setUpdatesEnabled(false);
    qWinGroup->statusBar()->removeWidget(&moreText);
    qWinGroup->statusBar()->setUpdatesEnabled(true);
}

void CHtmlSysWinInputQt::fUpdateInputFormatter()
{
    if (fTag == 0) {
        return;
    }
    fTag->setlen(static_cast<CHtmlFormatterInput*>(formatter_), fTadsBuffer->getlen());
    if (fTag->ready_to_format()) {
        fTag->format(static_cast<CHtmlSysWinQt*>(this), formatter_);
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    }
    fCastDispWidget->updateCursorPos(formatter_, false, true);
}

void CHtmlSysWinInputQt::setCursorHeight(unsigned height)
{
    fCastDispWidget->setCursorHeight(height);
    fCastDispWidget->updateCursorPos(formatter_, true, true);
}

void CHtmlSysWinInputQt::processCommand(const textchar_t* cmd, size_t len, int append, int enter,
                                        int os_cmd_id)
{
    if (not qFrame->gameRunning()) {
        return;
    }

    // If the command starts with "http:", "ftp:", "news:" "mailto:", or
    // "telnet:", try to open it in the external application that handles it.
    if (strnicmp(cmd, "http:", 5) == 0 || strnicmp(cmd, "ftp:", 4) == 0) {
        // Parse http and ftp URLs in strict mode.
        QDesktopServices::openUrl(QUrl::fromEncoded(cmd, QUrl::StrictMode));
        return;
    }
    if (strnicmp(cmd, "news:", 5) == 0 || strnicmp(cmd, "mailto:", 7) == 0
        || strnicmp(cmd, "telnet:", 7) == 0) {
        // Parse news, mailto and telnet URLs in tolerant mode.
        QDesktopServices::openUrl(QUrl::fromEncoded(cmd, QUrl::TolerantMode));
        return;
    }

    // If we're not currently accepting input, ignore this.
    if (fInputMode == NoInput or fInputMode == PagePauseInput) {
        return;
    }

    // If we're waiting for a single key-press event and the command isn't some
    // sort of special OS_CMD command, it's an HREF event.
    if (fInputMode == SingleKeyInput and os_cmd_id == OS_CMD_NONE) {
        // If the HREF string is empty, use a single space so that we know that
        // an HREF event actually occured.
        if (cmd[0] == '\0') {
            fHrefEvent = QChar::fromLatin1(' ');
        } else {
            fHrefEvent = QString::fromUtf8(cmd);
        }
        return;
    }

    // If we're not in APPEND mode, clear out the current command; otherwise,
    // make sure we're at the end of the current text.
    if (!append) {
        fTadsBuffer->del_line();
    } else {
        fTadsBuffer->end_of_line(false);
    }

    // Add the command string.
    fTadsBuffer->add_string(cmd, len, true);
    fTag->setlen(static_cast<CHtmlFormatterInput*>(formatter_), fTadsBuffer->getlen());
    if (fTag->ready_to_format()) {
        fTag->format(static_cast<CHtmlSysWinQt*>(this), formatter_);
    }
    fCastDispWidget->updateCursorPos(formatter_, false, true);

    // If 'enter' is true, indicate that we've finished reading the command, so
    // that getInput() will return the new command as its result; otherwise,
    // let the player continue editing this command.
    if (enter) {
        fInputReady = true;
        fInputMode = NoInput;
        emit inputReady();
    }
}

void CHtmlSysWinInputQt::resizeEvent(QResizeEvent* event)
{
    CHtmlSysWinQt::resizeEvent(event);
    if (fCastDispWidget->isCursorVisible()) {
        fCastDispWidget->updateCursorPos(formatter_, true, true);
    }
}

void CHtmlSysWinInputQt::keyPressEvent(QKeyEvent* e)
{
    // qDebug() << Q_FUNC_INFO;

    // qDebug() << "Key pressed:" << hex << e->key();

    if (fInputMode == NoInput or not qFrame->gameRunning()) {
        QScrollArea::keyPressEvent(e);
        return;
    }

    if (fInputMode == SingleKeyInput) {
        singleKeyPressEvent(e);
        return;
    }

    if (fInputMode == PagePauseInput) {
        if (e->key() == Qt::Key_Space) {
            // Scroll down by a page.
            fPagePauseQueue.head()->scrollDown(true, false);
#if QT_VERSION >= 0x040500
        } else if (e->matches(QKeySequence::InsertParagraphSeparator)) {
#else
        } else if (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return) {
#endif
            // Scroll down by a line.
            fPagePauseQueue.head()->scrollDown(true, true);
        }
        return;
    }

    if (e->matches(QKeySequence::MoveToStartOfLine)
        or e->matches(QKeySequence::MoveToStartOfBlock)) {
        fTadsBuffer->start_of_line(false);
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToEndOfLine)
               or e->matches(QKeySequence::MoveToEndOfBlock)) {
        fTadsBuffer->end_of_line(false);
        fCastDispWidget->clearSelection();
#if QT_VERSION >= 0x040500
    } else if (e->matches(QKeySequence::InsertParagraphSeparator)) {
#else
    } else if (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return) {
#endif
        fInputReady = true;
        fInputMode = NoInput;
        fTadsBuffer->add_hist();
        fCastDispWidget->clearSelection();
        emit inputReady();
        return;
    } else if (e->matches(QKeySequence::Delete)) {
        fTadsBuffer->del_right();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::DeleteEndOfWord)) {
        fTadsBuffer->move_right(true, true);
        fTadsBuffer->del_selection();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::DeleteStartOfWord)) {
        fTadsBuffer->move_left(true, true);
        fTadsBuffer->del_selection();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToPreviousChar)) {
        fTadsBuffer->move_left(false, false);
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToNextChar)) {
        fTadsBuffer->move_right(false, false);
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToPreviousWord)) {
        fTadsBuffer->move_left(false, true);
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToNextWord)) {
        fTadsBuffer->move_right(false, true);
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToPreviousLine)) {
        fTadsBuffer->select_prev_hist();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::MoveToNextLine)) {
        fTadsBuffer->select_next_hist();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::Find)) {
        fTadsBuffer->select_prev_hist_prefix();
        fCastDispWidget->clearSelection();
    } else if (e->matches(QKeySequence::SelectPreviousChar)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->move_left(true, false);
    } else if (e->matches(QKeySequence::SelectNextChar)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->move_right(true, false);
    } else if (e->matches(QKeySequence::SelectPreviousWord)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->move_left(true, true);
    } else if (e->matches(QKeySequence::SelectNextWord)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->move_right(true, true);
    } else if (e->matches(QKeySequence::SelectStartOfLine)
               or e->matches(QKeySequence::SelectStartOfBlock)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->start_of_line(true);
    } else if (e->matches(QKeySequence::SelectEndOfLine)
               or e->matches(QKeySequence::SelectEndOfBlock)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->end_of_line(true);
    } else if (e->matches(QKeySequence::SelectAll)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->start_of_line(false);
        fTadsBuffer->end_of_line(true);
    } else if (e->matches(QKeySequence::Undo)) {
        if (not fTadsBuffer->has_sel_range()) {
            fCastDispWidget->clearSelection();
        }
        fTadsBuffer->undo();
    } else if (e->key() == Qt::Key_Backspace) {
        fTadsBuffer->backspace();
        fCastDispWidget->clearSelection();
    } else {
        QString strToAdd = e->text();
        if (strToAdd.isEmpty() or not strToAdd.at(0).isPrint()) {
            QScrollArea::keyPressEvent(e);
            return;
        }
        insertText(strToAdd);
    }
    fUpdateInputFormatter();
}

void CHtmlSysWinInputQt::inputMethodEvent(QInputMethodEvent* e)
{
    if (fInputMode == NoInput or not qFrame->gameRunning() or fInputMode == PagePauseInput
        or e->commitString().isEmpty()) {
        QScrollArea::inputMethodEvent(e);
        return;
    }

    if (fInputMode == SingleKeyInput) {
        fLastKeyEvent = static_cast<Qt::Key>(0);
        fLastKeyText = 0;
        // If the keypress doesn't correspond to exactly one character, ignore
        // it.
        if (e->commitString().size() != 1) {
            QScrollArea::inputMethodEvent(e);
            return;
        }
        fLastKeyText = e->commitString().at(0);
        fInputMode = NoInput;
        fInputReady = true;
        emit inputReady();
        return;
    }
    insertText(e->commitString());
    fUpdateInputFormatter();
}

void CHtmlSysWinInputQt::singleKeyPressEvent(QKeyEvent* event)
{
    // qDebug() << Q_FUNC_INFO;
    Q_ASSERT(fInputMode == SingleKeyInput);

    fLastKeyEvent = static_cast<Qt::Key>(0);
    fLastKeyText = 0;

    switch (event->key()) {
    case 0:
    case Qt::Key_unknown:
        QScrollArea::keyPressEvent(event);
        return;

    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backspace:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Delete:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
        fLastKeyEvent = static_cast<Qt::Key>(event->key());
        break;

    default:
        // If the keypress doesn't correspond to exactly one character, ignore
        // it.
        if (event->text().size() != 1) {
            QScrollArea::keyPressEvent(event);
            return;
        }
        fLastKeyText = event->text().at(0);
    }

    fInputMode = NoInput;
    fInputReady = true;
    emit inputReady();
}

void CHtmlSysWinInputQt::getInput(textchar_t* buf, size_t buflen, unsigned long timeout,
                                  bool useTimeout, bool* timedOut)
{
    // qDebug() << Q_FUNC_INFO;
    Q_ASSERT(buf != 0);

    CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(formatter_);

    bool resuming = fTag != 0;

    // Correct any ill-formed HTML prior to input.
    formatter->prepare_for_input();

    if (resuming) {
        // We're resuming; reuse our existing input tag with the new buffer.
        fTag->change_buf(formatter, fTadsBuffer->getbuf());
        fTag->format(static_cast<CHtmlSysWinQt*>(this), formatter_);
        // We treat canceled inputs with reset=false as if they were resumes.
        // The difference is that in that case, we need to restore the cursor.
        if (fRestoreFromCancel) {
            fCastDispWidget->setCursorVisible(true);
            fCastDispWidget->updateCursorPos(formatter, false, true);
            fRestoreFromCancel = false;
        }
    } else {
        // Since we're not resuming, make sure that we've formatted all
        // available input and tell the formatter to begin a new input.
        while (formatter->more_to_do()) {
            formatter->do_formatting();
        }
        fTadsBuffer->setbuf(fInputBuffer,
                            buflen > fInputBufferSize ? fInputBufferSize - 1 : buflen - 1, 0);
        CHtmlTagTextInput* tag = formatter->begin_input(fTadsBuffer->getbuf(), 0);
        fTag = tag;
        fCastDispWidget->setInputTag(tag);
        if (tag->ready_to_format()) {
            tag->format(this, formatter_);
        }
        fTadsBuffer->show_caret();
        fCastDispWidget->setCursorVisible(true);
        fCastDispWidget->updateCursorPos(formatter, false, true);
        fTadsBuffer->set_sel_range(0, 0, 0);
    }

    // If we have banners waiting in page-pause mode, process them first.
    fProcessPagePauseQueue();

    fInputReady = false;
    fInputMode = NormalInput;

    // Reset the MORE prompt position to this point, since the user has seen
    // everything up to here.
    lastInputHeight = formatter_->get_max_y_pos();

    if (useTimeout) {
        QEventLoop idleLoop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, SIGNAL(timeout()), &idleLoop, SLOT(quit()));
        connect(qFrame, SIGNAL(gameQuitting()), &idleLoop, SLOT(quit()));
        connect(this, SIGNAL(inputReady()), &idleLoop, SLOT(quit()));
        timer.start(timeout);
        idleLoop.exec();
        if (timedOut != 0 and not fInputReady and qFrame->gameRunning()) {
            *timedOut = true;
            fInputMode = NoInput;
            return;
        }
    } else
        while (qFrame->gameRunning() and not fInputReady) {
            qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
        }

    // We're finished with input.
    cancelInput(true);

    // If input exceeds the buffer size, make sure we don't overflow.
    int len = fTadsBuffer->getlen() > buflen ? buflen : fTadsBuffer->getlen();

    // For TADS 3, we use the result as-is; it's already in UTF-8.  For TADS 2,
    // we will need to use the prefered encoding.
    if (qFrame->tads3()) {
        strncpy(buf, fTadsBuffer->getbuf(), len);
    } else {
        QTextCodec* codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);
        strncpy(buf,
                codec->fromUnicode(QString::fromUtf8(fTadsBuffer->getbuf(), fTadsBuffer->getlen()))
                    .constData(),
                len);
    }
    buf[len] = '\0';
}

void CHtmlSysWinInputQt::cancelInput(bool reset)
{
    if (fTag == 0) {
        // There's nothing to cancel.
        return;
    }

    // Remember if we are at the bottom of the output.
    bool wasAtBottom = verticalScrollBar()->value() == verticalScrollBar()->maximum();

    fTadsBuffer->hide_caret();
    fCastDispWidget->setCursorVisible(false);
    static_cast<CHtmlFormatterInput*>(formatter_)->end_input();

    // Add the line-break after the command.
    if (qFrame->get_parser()->get_obey_markups()) {
        // we're in parsed mode, so write our sequence as HTML
        qFrame->display_output("<br>", 4);
    } else {
        // we're in literal mode, so write out a literal newline
        qFrame->display_output("\n", 1);
    }

    // Tell the formatter to add an extra line's worth of spacing, to ensure
    // that we have some immediate visual feedback (in the form of scrolling
    // the window up a line) when the user presses the Enter key.
    formatter_->add_line_to_disp_height();

    // Flush the newline, and update the window immediately, in case the
    // operation takes a while to complete.
    if (wasAtBottom) {
        qFrame->flush_txtbuf(true, true);
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
        qFrame->advanceEventLoop();
    }

    // Done with the tag.
    if (reset) {
        fTag = 0;
        fCastDispWidget->setInputTag(0);
        fRestoreFromCancel = false;
    } else {
        fRestoreFromCancel = true;
    }
}

int CHtmlSysWinInputQt::getKeypress(unsigned long timeout, bool useTimeout, bool* timedOut)
{
    // qDebug() << Q_FUNC_INFO;
    // If 'done' is false, it means that we have been previously called and
    // returned 0, so this time we should return the extended key-code we
    // stored last time in 'extKey'.
    static bool done = true;
    static int extKey;

    if (not done) {
        // We have a pending return from our last call.  Prepare to do a
        // normal read on our next call and return the pending result.
        done = true;
        return extKey;
    }

    // Prepare the formatter for input and format all remaining lines.
    CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(formatter_);
    formatter->prepare_for_input();
    while (formatter->more_to_do()) {
        formatter->do_formatting();
    }

    // scrollDown();

    // If we have banners waiting in page-pause mode, process them first.
    fProcessPagePauseQueue();

    // Clear any pending HREF event.
    fHrefEvent.clear();

    extKey = 0;
    fStartKeypressInput();
    // Reset the MORE prompt position to this point, since the user has seen
    // everything up to here.
    lastInputHeight = formatter_->get_max_y_pos();
    if (useTimeout) {
        QEventLoop idleLoop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, SIGNAL(timeout()), &idleLoop, SLOT(quit()));
        connect(qFrame, SIGNAL(gameQuitting()), &idleLoop, SLOT(quit()));
        connect(this, SIGNAL(inputReady()), &idleLoop, SLOT(quit()));
        timer.start(timeout);
        idleLoop.exec();
    } else
        while (not fInputReady and fHrefEvent.isEmpty() and qFrame->gameRunning()) {
            qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
        }

    if (not qFrame->gameRunning()) {
        // Game is quitting.
        return -3;
    }

    // If there was an HREF event, tell the caller.
    if (not fHrefEvent.isEmpty()) {
        return -2;
    }

    // If we're using a timeout and it expired, tell the caller.
    if (useTimeout and qFrame->gameRunning() and not fInputReady) {
        Q_ASSERT(timedOut != 0);
        *timedOut = true;
        return -1;
    }

    if (fLastKeyEvent != 0) {
        switch (fLastKeyEvent) {
        case Qt::Key_Escape:
            return 27;
        // A Tab is not an extended character, but Tads requires that
        // it is handled as one.
        case Qt::Key_Tab:
            extKey = CMD_TAB;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return 13;
        case Qt::Key_Down:
            extKey = CMD_DOWN;
            break;
        case Qt::Key_Up:
            extKey = CMD_UP;
            break;
        case Qt::Key_Left:
            extKey = CMD_LEFT;
            break;
        case Qt::Key_Right:
            extKey = CMD_RIGHT;
            break;
        case Qt::Key_Home:
            extKey = CMD_HOME;
            break;
        case Qt::Key_Backspace:
            return 8;
        case Qt::Key_F1:
            extKey = CMD_F1;
            break;
        case Qt::Key_F2:
            extKey = CMD_F2;
            break;
        case Qt::Key_F3:
            extKey = CMD_F3;
            break;
        case Qt::Key_F4:
            extKey = CMD_F4;
            break;
        case Qt::Key_F5:
            extKey = CMD_F5;
            break;
        case Qt::Key_F6:
            extKey = CMD_F6;
            break;
        case Qt::Key_F7:
            extKey = CMD_F7;
            break;
        case Qt::Key_F8:
            extKey = CMD_F8;
            break;
        case Qt::Key_F9:
            extKey = CMD_F9;
            break;
        case Qt::Key_F10:
            extKey = CMD_F10;
            break;
        case Qt::Key_Delete:
            extKey = CMD_DEL;
            break;
        case Qt::Key_PageDown:
            extKey = CMD_PGDN;
            break;
        case Qt::Key_PageUp:
            extKey = CMD_PGUP;
            break;
        case Qt::Key_End:
            extKey = CMD_END;
            break;
        default:
            // If we got here, something went wrong.  Just report a
            // space.
            qWarning() << Q_FUNC_INFO << "unrecognized key event in switch:" << hex
                       << fLastKeyEvent;
            return ' ';
        }
    } else {
        // It's a textual key press.
        return fLastKeyText.unicode();
    }

    // Prepare to return the extended key-code on
    // our next call.
    done = false;
    return 0;
}

void CHtmlSysWinInputQt::addToPagePauseQueue(CHtmlSysWinQt* banner)
{
    if (not fPagePauseQueue.contains(banner)) {
        fPagePauseQueue.enqueue(banner);
    }
}

void CHtmlSysWinInputQt::removeFromPagePauseQueue(CHtmlSysWinQt* banner)
{
    if (fPagePauseQueue.isEmpty() or not fPagePauseQueue.contains(banner)) {
        return;
    }
    fPagePauseQueue.removeOne(banner);
    if (fPagePauseQueue.isEmpty() and fInputMode == PagePauseInput) {
        fInputMode = NoInput;
    }
}

void CHtmlSysWinInputQt::insertText(QString str)
{
    if (fInputMode != NormalInput or str.isEmpty() or not qFrame->gameRunning()) {
        return;
    }
    // Replace tabs and newlines with spaces.
    for (int i = 0; i < str.length(); ++i) {
        if (str.at(i) == QChar::fromLatin1('\t') or str.at(i) == QChar::fromLatin1('\n')) {
            str[i] = QChar::fromLatin1(' ');
        }
    }
    const QByteArray& utfTxt = str.toUtf8();
    // Try to add it to the input buffer. Do not allow truncation; it
    // won't work with UTF-8 strings and we'll crash if it doesn't fit.
    if (not fTadsBuffer->add_string(utfTxt.constData(), utfTxt.length(), false)) {
        // It didn't fit.  Try to add the largest part of it that fits.
        int i = str.length();
        QString subStr;
        QByteArray subUtf;
        do {
            --i;
            subStr = str.left(i);
            subUtf = subStr.toUtf8();
        } while (not fTadsBuffer->add_string(subUtf.constData(), subUtf.length(), false) and i > 1);
    }
    fCastDispWidget->clearSelection();
    fUpdateInputFormatter();
}

void CHtmlSysWinInputQt::set_html_input_color(HTML_color_t clr, int use_default)
{
    // qDebug() << Q_FUNC_INFO;

    if (use_default) {
        const QColor& def = qFrame->settings()->inputColor;
        qFrame->inputColor(HTML_make_color(def.red(), def.green(), def.blue()));
    } else {
        qFrame->inputColor(map_system_color(clr));
    }
}
