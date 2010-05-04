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

#include <QKeyEvent>
#include <QScrollBar>
#include <QTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QUrl>

#include "htmlqt.h"
#include "qtadsdispwidgetinput.h"

#include "htmlfmt.h"
#include "htmlinp.h"
#include "htmltags.h"
#include "htmlprs.h"


CHtmlSysWinInputQt::CHtmlSysWinInputQt( CHtmlFormatter* formatter, QWidget* parent )
: CHtmlSysWinQt(formatter, 0, parent), fInputReady(false),
  fAcceptInput(false), fSingleKeyInput(false), fLastKeyEvent(Qt::Key_Any)
{
	this->dispWidget = new QTadsDisplayWidgetInput(this, formatter);
	this->fCastDispWidget = static_cast<QTadsDisplayWidgetInput*>(this->dispWidget);
	this->setWidget(this->dispWidget);
}


void
CHtmlSysWinInputQt::fStartLineInput( CHtmlInputBuf* tadsBuffer, CHtmlTagTextInput* tag )
{
	this->fInputReady = false;
	this->fAcceptInput = true;
	this->fSingleKeyInput = false;
	this->fTag = tag;
	this->fTadsBuffer = tadsBuffer;
	tadsBuffer->setbuf(tadsBuffer->getbuf(), 1000, 0);
	tadsBuffer->set_sel_range(0, 0, 0);
}


void
CHtmlSysWinInputQt::fStartKeypressInput()
{
	this->fInputReady = false;
	this->fAcceptInput = true;
	this->fSingleKeyInput = true;
	this->fHrefEvent.clear();
}


void
CHtmlSysWinInputQt::processCommand( const textchar_t* cmd, size_t len, int append, int enter, int os_cmd_id )
{
	// If the command starts with "http:", "ftp:", "news:" "mailto:", or
	// "telnet:", try to open it in the external application that handles it.
	if (strnicmp(cmd, "http:", 5) == 0
		|| strnicmp(cmd, "ftp:", 4) == 0
		|| strnicmp(cmd, "news:", 5) == 0
		|| strnicmp(cmd, "mailto:", 7) == 0
		|| strnicmp(cmd, "telnet:", 7) == 0)
	{
		QDesktopServices::openUrl(QUrl(cmd));
		return;
	}

	// If we're not currently accepting input, ignore this.
	if (not this->fAcceptInput) {
		return;
	}

	// If we're waiting for a single key-press event and the command isn't some
	// sort of special OS_CMD command, it's an HREF event.
	if (this->fAcceptInput and this->fSingleKeyInput and os_cmd_id == OS_CMD_NONE) {
		this->fHrefEvent = QString::fromUtf8(cmd);
		return;
	}

	// If we're not in APPEND mode, clear out the current command; otherwise,
	// make sure we're at the end of the current text.
	if (!append) {
		this->fTadsBuffer->del_line();
	} else {
		this->fTadsBuffer->end_of_line(false);
	}

	// Add the command string.
	this->fTadsBuffer->add_string(cmd, len, true);

	// If 'enter' is true, indicate that we've finished reading the command, so
	// that getInput() will return the new command as its result; otherwise,
	// let the player continue editing this command.
	if (enter) {
		this->fInputReady = true;
		this->fAcceptInput = false;
	}
}


void
CHtmlSysWinInputQt::resizeEvent( QResizeEvent* event )
{
	CHtmlSysWinQt::resizeEvent(event);
	if (this->fCastDispWidget->isCursorVisible()) {
		this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);
	}
}


void
CHtmlSysWinInputQt::keyPressEvent ( QKeyEvent* e )
{
	//qDebug() << Q_FUNC_INFO;

	//qDebug() << "Key pressed:" << hex << event->key();

	if (not this->fAcceptInput) {
		QScrollArea::keyPressEvent(e);
		return;
	}

	if (this->fSingleKeyInput) {
		this->singleKeyPressEvent(e);
		return;
	}

	if (e->matches(QKeySequence::MoveToStartOfLine)) {
		this->fTadsBuffer->start_of_line(false);
	} else if (e->matches(QKeySequence::MoveToEndOfLine)) {
		this->fTadsBuffer->end_of_line(false);
#if QT_VERSION >= 0x040500
	} else if (e->matches(QKeySequence::InsertParagraphSeparator)) {
#else
	} else if (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return) {
#endif
		this->fInputReady = true;
		this->fAcceptInput = false;
		this->fTadsBuffer->add_hist();
		return;
	} else if (e->matches(QKeySequence::Delete)) {
		this->fTadsBuffer->del_right();
	} else if (e->matches(QKeySequence::DeleteEndOfWord)) {
		this->fTadsBuffer->move_right(true, true);
		this->fTadsBuffer->del_selection();
	} else if (e->matches(QKeySequence::DeleteStartOfWord)) {
		this->fTadsBuffer->move_left(true, true);
		this->fTadsBuffer->del_selection();
	} else if (e->matches(QKeySequence::MoveToPreviousChar)) {
		this->fTadsBuffer->move_left(false, false);
	} else if (e->matches(QKeySequence::MoveToNextChar)) {
		this->fTadsBuffer->move_right(false, false);
	} else if (e->matches(QKeySequence::MoveToPreviousWord)) {
		this->fTadsBuffer->move_left(false, true);
	} else if (e->matches(QKeySequence::MoveToNextWord)) {
		this->fTadsBuffer->move_right(false, true);
	} else if (e->matches(QKeySequence::MoveToPreviousLine)) {
		this->fTadsBuffer->select_prev_hist();
	} else if (e->matches(QKeySequence::MoveToNextLine)) {
		this->fTadsBuffer->select_next_hist();
	} else if (e->matches(QKeySequence::SelectPreviousChar)) {
		this->fTadsBuffer->move_left(true, false);
	} else if (e->matches(QKeySequence::SelectNextChar)) {
		this->fTadsBuffer->move_right(true, false);
	} else if (e->matches(QKeySequence::SelectPreviousWord)) {
		this->fTadsBuffer->move_left(true, true);
	} else if (e->matches(QKeySequence::SelectNextWord)) {
		this->fTadsBuffer->move_right(true, true);
	} else if (e->matches(QKeySequence::SelectStartOfLine) or e->matches(QKeySequence::SelectStartOfBlock)) {
		this->fTadsBuffer->start_of_line(true);
	} else if (e->matches(QKeySequence::SelectEndOfLine) or e->matches(QKeySequence::SelectEndOfBlock)) {
		this->fTadsBuffer->end_of_line(true);
	} else if (e->matches(QKeySequence::SelectAll)) {
		this->fTadsBuffer->start_of_line(false);
		this->fTadsBuffer->end_of_line(true);
	} else if (e->matches(QKeySequence::Undo)) {
		this->fTadsBuffer->undo();
	} else if (e->matches(QKeySequence::Paste)) {
		const QString& clipStr = QApplication::clipboard()->text();
		this->fTadsBuffer->add_string(clipStr.toUtf8().constData(), clipStr.toUtf8().size(), true);
	} else if (e->key() == Qt::Key_Backspace) {
		this->fTadsBuffer->backspace();
	} else {
		if (e->text().isEmpty()) {
			return;
		}
		this->fTadsBuffer->add_string(e->text().toUtf8().constData(), e->text().toUtf8().length(), true);
	}

	this->fTag->setlen(static_cast<CHtmlFormatterInput*>(this->formatter_), this->fTadsBuffer->getlen());
	if (this->fTag->ready_to_format()) {
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->formatter_);
	}
	this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);
	//static_cast<CHtmlSysWinQt*>(this->widget())->do_formatting(false, true, false);
}


void
CHtmlSysWinInputQt::singleKeyPressEvent( QKeyEvent* event )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(this->fAcceptInput);
	Q_ASSERT(this->fSingleKeyInput);

	this->fLastKeyEvent = static_cast<Qt::Key>(0);
	this->fLastKeyText = 0;

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
		this->fLastKeyEvent = static_cast<Qt::Key>(event->key());
		break;

	  default:
		// If the keypress doesn't correspond to exactly one character, ignore
		// it.
		if (event->text().size() != 1) {
			QScrollArea::keyPressEvent(event);
			return;
		}
		this->fLastKeyText = event->text().at(0);
	}

	this->fAcceptInput = false;
	this->fSingleKeyInput = false;
	this->fInputReady = true;
}


bool
CHtmlSysWinInputQt::getInput( CHtmlInputBuf* tadsBuffer )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(tadsBuffer != 0);

	this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
	CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(this->formatter_);
	formatter->prepare_for_input();
	while (formatter->more_to_do()) {
		formatter->do_formatting();
	}

	CHtmlTagTextInput* tag = formatter->begin_input(tadsBuffer->getbuf(), 0);
	if (tag->ready_to_format()) {
		tag->format(this, this->formatter_);
	}
	tadsBuffer->show_caret();
	this->fCastDispWidget->setCursorVisible(true);
	this->fCastDispWidget->updateCursorPos(formatter, tadsBuffer, tag);
	this->fStartLineInput(tadsBuffer, tag);
	while (qFrame->gameRunning() and not this->fInputReady) {
		qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
	}
	if (not qFrame->gameRunning()) {
		return false;
	}
	tadsBuffer->hide_caret();
	this->fCastDispWidget->setCursorVisible(false);
	formatter->end_input();

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
	this->formatter_->add_line_to_disp_height();

	// Flush the newline, and update the window immediately, in case the
	// operation takes a while to complete.
	qFrame->flush_txtbuf(true, true);
	return true;
}


int
CHtmlSysWinInputQt::getKeypress( unsigned long timeout, bool useTimeout, bool* timedOut )
{
	//qDebug() << Q_FUNC_INFO;
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
	CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(this->formatter_);
	formatter->prepare_for_input();
	while (formatter->more_to_do()) {
		formatter->do_formatting();
	}

	// Scroll to bottom.
	this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());

	// Clear any pending HREF event.
	this->fHrefEvent.clear();

	extKey = 0;
	this->fStartKeypressInput();
	if (useTimeout) {
		QTime t;
		t.start();
		while (static_cast<unsigned long>(t.elapsed()) < timeout and qFrame->gameRunning()
			   and not this->fInputReady) {
			qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents, timeout - t.elapsed());
		}
	} else while (not this->fInputReady and this->fHrefEvent.isEmpty() and qFrame->gameRunning()) {
		qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
	}

	if (not qFrame->gameRunning()) {
		// Game is quitting.
		return -3;
	}

	// If there was an HREF event, tell the caller.
	if (not this->fHrefEvent.isEmpty()) {
		return -2;
	}

	// If we're using a timeout and it expired, tell the caller.
	if (useTimeout and qFrame->gameRunning() and not this->fInputReady) {
		Q_ASSERT(timedOut != 0);
		*timedOut = true;
		return -1;
	}

	if (this->fLastKeyEvent != 0) {
		switch (this->fLastKeyEvent) {
		  case Qt::Key_Escape:   return 27;
		  // A Tab is not an extended character, but Tads requires that
		  // it is handled as one.
		  case Qt::Key_Tab:      extKey = CMD_TAB; break;
		  case Qt::Key_Return:
		  case Qt::Key_Enter:    return 13;
		  case Qt::Key_Down:     extKey = CMD_DOWN; break;
		  case Qt::Key_Up:       extKey = CMD_UP; break;
		  case Qt::Key_Left:     extKey = CMD_LEFT; break;
		  case Qt::Key_Right:    extKey = CMD_RIGHT; break;
		  case Qt::Key_Home:     extKey = CMD_HOME; break;
		  case Qt::Key_Backspace: return 8;
		  case Qt::Key_F1:       extKey = CMD_F1; break;
		  case Qt::Key_F2:       extKey = CMD_F2; break;
		  case Qt::Key_F3:       extKey = CMD_F3; break;
		  case Qt::Key_F4:       extKey = CMD_F4; break;
		  case Qt::Key_F5:       extKey = CMD_F5; break;
		  case Qt::Key_F6:       extKey = CMD_F6; break;
		  case Qt::Key_F7:       extKey = CMD_F7; break;
		  case Qt::Key_F8:       extKey = CMD_F8; break;
		  case Qt::Key_F9:       extKey = CMD_F9; break;
		  case Qt::Key_F10:      extKey = CMD_F10; break;
		  case Qt::Key_Delete:   extKey = CMD_DEL; break;
		  case Qt::Key_PageDown: extKey = CMD_PGDN; break;
		  case Qt::Key_PageUp:   extKey = CMD_PGUP; break;
		  case Qt::Key_End:      extKey = CMD_END; break;
		  default:
			  // If we got here, something went wrong.  Just report a
			  // space.
			  qWarning() << Q_FUNC_INFO << "unrecognized key event in switch:" << hex << this->fLastKeyEvent;
			  return ' ';
		}
	} else {
		// It's a textual key press.
		return this->fLastKeyText.unicode();
	}

	// Prepare to return the extended key-code on
	// our next call.
	done = false;
	return 0;
}
