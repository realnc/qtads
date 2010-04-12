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

#include "htmlqt.h"
#include "qtadsdispwidget.h"

#include "htmlfmt.h"
#include "htmlinp.h"
#include "htmltags.h"
#include "htmlprs.h"


CHtmlSysWinInputQt::CHtmlSysWinInputQt( CHtmlFormatter* formatter, QWidget* parent )
: CHtmlSysWinQt(formatter, parent), fInputReady(false), fAcceptInput(false), fSingleKeyInput(false),
  fLastKeyEvent(Qt::Key_Any)
{ }


void
CHtmlSysWinInputQt::keyPressEvent ( QKeyEvent* event )
{
	//qDebug() << Q_FUNC_INFO;

	//qDebug() << "Key pressed:" << hex << event->key();

	if (not this->fAcceptInput) {
		QScrollArea::keyPressEvent(event);
		return;
	}

	if (this->fSingleKeyInput) {
		this->singleKeyPressEvent(event);
		return;
	}

	switch (event->key()) {
	  case Qt::Key_Enter:
	  case Qt::Key_Return:
		this->fInputReady = true;
		this->fAcceptInput = false;
		this->fTadsBuffer->add_hist();
		return;

	  case Qt::Key_Backspace:
		if (event->modifiers().testFlag(Qt::ControlModifier)) {
			this->fTadsBuffer->move_left(true, true);
			this->fTadsBuffer->del_selection();
		} else {
			this->fTadsBuffer->backspace();
		}
		break;

	  case Qt::Key_Delete:
		if (event->modifiers().testFlag(Qt::ControlModifier)) {
			this->fTadsBuffer->move_right(true, true);
			this->fTadsBuffer->del_selection();
		} else {
			this->fTadsBuffer->del_right();
		}
		break;

	  case Qt::Key_Home:
		this->fTadsBuffer->start_of_line(event->modifiers().testFlag(Qt::ShiftModifier));
		break;

	  case Qt::Key_End:
		this->fTadsBuffer->end_of_line(event->modifiers().testFlag(Qt::ShiftModifier));
		break;

	  case Qt::Key_Left:
		this->fTadsBuffer->move_left(event->modifiers().testFlag(Qt::ShiftModifier),
									 event->modifiers().testFlag(Qt::ControlModifier));
		break;

	  case Qt::Key_Right:
		this->fTadsBuffer->move_right(event->modifiers().testFlag(Qt::ShiftModifier),
									  event->modifiers().testFlag(Qt::ControlModifier));
		break;

	  case Qt::Key_Up:
		this->fTadsBuffer->select_prev_hist();
		break;

	  case Qt::Key_Down:
		this->fTadsBuffer->select_next_hist();
		break;

	  default:
		if (event->text().isEmpty()) {
			return;
		}
		this->fTadsBuffer->add_string(event->text().toUtf8().constData(), event->text().toUtf8().length(), true);
	}

	this->fTag->setlen(static_cast<CHtmlFormatterInput*>(this->fFormatter), this->fTadsBuffer->getlen());
	if (this->fTag->ready_to_format()) {
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->fFormatter);
	}
	this->fDispWidget->updateCursorPos(this->fFormatter, this->fTadsBuffer, this->fTag);
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
	this->fDispWidget->setCursorVisible(true);
	this->fDispWidget->updateCursorPos(formatter, tadsBuffer, tag);
	this->startLineInput(tadsBuffer, tag, formatter);
	while (qFrame->gameRunning() and not this->inputReady()) {
		qApp->sendPostedEvents();
		qApp->processEvents(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
		qApp->sendPostedEvents();
	}
	if (not qFrame->gameRunning()) {
		return false;
	}
	tadsBuffer->hide_caret();
	this->fDispWidget->setCursorVisible(false);
	formatter->end_input();

	// Add the line-break after the command.
	if (qFrame->get_parser()->get_obey_markups()) {
		// we're in parsed mode, so write our sequence as HTML
		qFrame->display_output("<br>", 4);
	} else {
		// we're in literal mode, so write out a literal newline
		qFrame->display_output("\n", 1);
	}

	// Flush the newline, and update the window immediately, in case the
	// operation takes a while to complete.
	qFrame->flush_txtbuf(true, true);

	// Tell the formatter to add an extra line's worth of spacing, to ensure
	// that we have some immediate visual feedback (in the form of scrolling
	// the window up a line) when the user presses the Enter key.
	this->formatter_->add_line_to_disp_height();
	return true;
}


int
CHtmlSysWinInputQt::getKeypress( int timeout, bool* timedOut )
{
	//qDebug() << Q_FUNC_INFO;
	// If 'done' is false, it means that we have been previously called and
	// returned 0, so this time we should return the extended key-code we
	// stored last time in 'extKey'.
	static bool done = true;
	static int extKey;

	if (done) {
		CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(this->formatter_);
		formatter->prepare_for_input();
		while (formatter->more_to_do()) {
			formatter->do_formatting();
		}
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
		extKey = 0;
		this->startKeypressInput();
		while (qFrame->gameRunning() and not this->inputReady()) {
			qApp->sendPostedEvents();
			qApp->processEvents(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
			qApp->sendPostedEvents();
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
				  qWarning() << Q_FUNC_INFO << "unrecognized key event in switch";
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

	// We have a pending return from our last call.  Prepare to do a
	// normal read on our next call and return the pending result.
	done = true;
	return extKey;
}


void
CHtmlSysWinInputQt::startLineInput( CHtmlInputBuf* tadsBuffer, CHtmlTagTextInput* tag, CHtmlFormatter* formatter )
{
	this->fInputReady = false;
	this->fAcceptInput = true;
	this->fSingleKeyInput = false;
	this->fTag = tag;
	this->fFormatter = formatter;
	this->fTadsBuffer = tadsBuffer;
	tadsBuffer->setbuf(tadsBuffer->getbuf(), 1000, 0);
	tadsBuffer->set_sel_range(0, 0, 0);
}


void
CHtmlSysWinInputQt::startKeypressInput()
{
	this->fInputReady = false;
	this->fAcceptInput = true;
	this->fSingleKeyInput = true;
}
