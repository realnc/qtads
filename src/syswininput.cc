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
#include <QTimer>
#include <QTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QUrl>
#include <QTextCodec>

#include "settings.h"
#include "dispwidgetinput.h"
#include "syswininput.h"

#include "htmlfmt.h"
#include "htmlinp.h"
#include "htmltags.h"
#include "htmlprs.h"


CHtmlSysWinInputQt::CHtmlSysWinInputQt( CHtmlFormatter* formatter, QWidget* parent )
: CHtmlSysWinQt(formatter, 0, parent), fInputMode(NoInput), fInputReady(false), fRestoreFromCancel(false),
  fLastKeyEvent(Qt::Key_Any), fTag(0)
{
	this->dispWidget = new DisplayWidgetInput(this, formatter);
	this->fCastDispWidget = static_cast<DisplayWidgetInput*>(this->dispWidget);
	this->setWidget(this->dispWidget);

	this->fInputBuffer = new textchar_t[1024];
	this->fInputBufferSize = 1024;
	this->fTadsBuffer = new CHtmlInputBuf(this->fInputBuffer, 1024, 100);
	this->fTadsBuffer->set_utf8_mode(true);

	QPalette p(this->palette());
	p.setColor(QPalette::Base, qFrame->settings()->mainBgColor);
	p.setColor(QPalette::Text, qFrame->settings()->mainTextColor);
	this->setPalette(p);
}


CHtmlSysWinInputQt::~CHtmlSysWinInputQt()
{
	delete this->fTadsBuffer;
	delete[] this->fInputBuffer;
}

void
CHtmlSysWinInputQt::fStartKeypressInput()
{
	this->fInputReady = false;
	this->fInputMode = SingleKeyInput;
	this->fHrefEvent.clear();
}


void
CHtmlSysWinInputQt::setCursorHeight( unsigned height )
{
	this->fCastDispWidget->setCursorHeight(height);
}


void
CHtmlSysWinInputQt::processCommand( const textchar_t* cmd, size_t len, int append, int enter, int os_cmd_id )
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
	if (strnicmp(cmd, "news:", 5) == 0 || strnicmp(cmd, "mailto:", 7) == 0 || strnicmp(cmd, "telnet:", 7) == 0) {
		// Parse news, mailto and telnet URLs in tolerant mode.
		QDesktopServices::openUrl(QUrl::fromEncoded(cmd, QUrl::TolerantMode));
		return;
	}

	// If we're not currently accepting input, ignore this.
	if (this->fInputMode == NoInput) {
		return;
	}

	// If we're waiting for a single key-press event and the command isn't some
	// sort of special OS_CMD command, it's an HREF event.
	if (this->fInputMode == SingleKeyInput and os_cmd_id == OS_CMD_NONE) {
		// If the HREF string is empty, use a single space so that we know that
		// an HREF event actually occured.
		if (cmd[0] == '\0') {
			this->fHrefEvent = " ";
		} else {
			this->fHrefEvent = QString::fromUtf8(cmd);
		}
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
	this->fTag->setlen(static_cast<CHtmlFormatterInput*>(this->formatter_), this->fTadsBuffer->getlen());
	if (this->fTag->ready_to_format()) {
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->formatter_);
	}
	this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);

	// If 'enter' is true, indicate that we've finished reading the command, so
	// that getInput() will return the new command as its result; otherwise,
	// let the player continue editing this command.
	if (enter) {
		this->fInputReady = true;
		this->fInputMode = NoInput;
		emit inputReady();
	}
}


void
CHtmlSysWinInputQt::resizeEvent( QResizeEvent* event )
{
	if (this->fCastDispWidget->isCursorVisible()) {
		this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);
	}
}


void
CHtmlSysWinInputQt::keyPressEvent ( QKeyEvent* e )
{
	//qDebug() << Q_FUNC_INFO;

	//qDebug() << "Key pressed:" << hex << e->key();

	if (this->fInputMode == NoInput or not qFrame->gameRunning()) {
		QScrollArea::keyPressEvent(e);
		return;
	}

	if (this->fInputMode == SingleKeyInput) {
		this->singleKeyPressEvent(e);
		return;
	}

	if (this->fInputMode == PagePauseInput) {
		this->pagePauseKeyPressEvent(e);
		return;
	}

	if (e->matches(QKeySequence::MoveToStartOfLine) or e->matches(QKeySequence::MoveToStartOfBlock)) {
		this->fTadsBuffer->start_of_line(false);
	} else if (e->matches(QKeySequence::MoveToEndOfLine) or e->matches(QKeySequence::MoveToEndOfBlock)) {
		this->fTadsBuffer->end_of_line(false);
#if QT_VERSION >= 0x040500
	} else if (e->matches(QKeySequence::InsertParagraphSeparator)) {
#else
	} else if (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return) {
#endif
		this->fInputReady = true;
		this->fInputMode = NoInput;
		this->fTadsBuffer->add_hist();
		emit inputReady();
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
		if (e->text().isEmpty() or not e->text().at(0).isPrint()) {
			QScrollArea::keyPressEvent(e);
			return;
		}
		this->fTadsBuffer->add_string(e->text().toUtf8().constData(), e->text().toUtf8().length(), true);
	}

	this->fTag->setlen(static_cast<CHtmlFormatterInput*>(this->formatter_), this->fTadsBuffer->getlen());
	if (this->fTag->ready_to_format()) {
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->formatter_);
		this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
	}
	this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);
}


void
CHtmlSysWinInputQt::inputMethodEvent( QInputMethodEvent* e )
{
	if (this->fInputMode == NoInput or not qFrame->gameRunning() or this->fInputMode == PagePauseInput
		or e->commitString().isEmpty())
	{
		QScrollArea::inputMethodEvent(e);
		return;
	}

	if (this->fInputMode == SingleKeyInput) {
		this->fLastKeyEvent = static_cast<Qt::Key>(0);
		this->fLastKeyText = 0;
		// If the keypress doesn't correspond to exactly one character, ignore
		// it.
		if (e->commitString().size() != 1) {
			QScrollArea::inputMethodEvent(e);
			return;
		}
		this->fLastKeyText = e->commitString().at(0);
		this->fInputMode = NoInput;
		this->fInputReady = true;
		emit inputReady();
		return;
	}

	this->fTadsBuffer->add_string(e->commitString().toUtf8().constData(),
								  e->commitString().toUtf8().length(), true);
	this->fTag->setlen(static_cast<CHtmlFormatterInput*>(this->formatter_), this->fTadsBuffer->getlen());
	if (this->fTag->ready_to_format()) {
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->formatter_);
		this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
	}
	this->fCastDispWidget->updateCursorPos(this->formatter_, this->fTadsBuffer, this->fTag);
}


void
CHtmlSysWinInputQt::singleKeyPressEvent( QKeyEvent* event )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(this->fInputMode == SingleKeyInput);

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

	this->fInputMode = NoInput;
	this->fInputReady = true;
	emit inputReady();
}


void
CHtmlSysWinInputQt::pagePauseKeyPressEvent( QKeyEvent* e )
{
	if (e->key() == Qt::Key_Space) {
		this->fInputMode = NoInput;
	}
}


bool
CHtmlSysWinInputQt::getInput( textchar_t* buf, size_t buflen, unsigned long timeout, bool useTimeout,
							  bool* timedOut )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(buf != 0);

	CHtmlFormatterInput* formatter = static_cast<CHtmlFormatterInput*>(this->formatter_);

	bool resuming = this->fTag != 0;

	// Correct any ill-formed HTML prior to input.
	formatter->prepare_for_input();

	if (resuming) {
		// We're resuming; reuse our existing input tag with the new buffer.
		this->fTag->change_buf(formatter, this->fTadsBuffer->getbuf());
		this->fTag->format(static_cast<CHtmlSysWinQt*>(this), this->formatter_);
		// We treat canceled inputs with reset=false as if they were resumes.
		// The difference is that in that case, we need to restore the cursor.
		if (this->fRestoreFromCancel) {
			this->fCastDispWidget->setCursorVisible(true);
			this->fCastDispWidget->updateCursorPos(formatter, this->fTadsBuffer, this->fTag);
			this->fRestoreFromCancel = false;
		}
	} else {
		// Since we're not resuming, make sure we scroll down and that we've
		// formatted all available input and tell the formatter to begin a new
		// input.
		this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
		while (formatter->more_to_do()) {
			formatter->do_formatting();
		}
		this->fTadsBuffer->setbuf(this->fInputBuffer,
								  buflen > this->fInputBufferSize ? this->fInputBufferSize - 1 : buflen - 1,
								  0);
		CHtmlTagTextInput* tag = formatter->begin_input(this->fTadsBuffer->getbuf(), 0);
		this->fTag = tag;
		if (tag->ready_to_format()) {
			tag->format(this, this->formatter_);
		}
		this->fTadsBuffer->show_caret();
		this->fCastDispWidget->setCursorVisible(true);
		this->fCastDispWidget->updateCursorPos(formatter, this->fTadsBuffer, tag);
		this->fTadsBuffer->set_sel_range(0, 0, 0);
	}

	this->fInputReady = false;
	this->fInputMode = NormalInput;

	// Reset the MORE prompt position to this point, since the user has seen
	// everything up to here.
	this->lastInputHeight = this->formatter_->get_max_y_pos();

	if (useTimeout) {
		QEventLoop idleLoop;
		QTimer timer;
		timer.setSingleShot(true);
		connect(&timer, SIGNAL(timeout()), &idleLoop, SLOT(quit()));
		connect(qFrame, SIGNAL(gameQuitting()), &idleLoop, SLOT(quit()));
		connect(this, SIGNAL(inputReady()), &idleLoop, SLOT(quit()));
		timer.start(timeout);
		idleLoop.exec();
		if (timedOut != 0 and not this->fInputReady and qFrame->gameRunning()) {
			*timedOut = true;
			this->fInputMode = NoInput;
			return true;
		}
	} else while (qFrame->gameRunning() and not this->fInputReady) {
		qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
	}

	// We're finished with input.
	this->cancelInput(true);

	// If input exceeds the buffer size, make sure we don't overflow.
	int len = this->fTadsBuffer->getlen() > buflen ? buflen : this->fTadsBuffer->getlen();

	// For TADS 3, we use the result as-is; it's already in UTF-8.  For TADS 2,
	// we will need to use the prefered encoding.
	if (qFrame->tads3()) {
		strncpy(buf, this->fTadsBuffer->getbuf(), len);
	} else {
		QTextCodec* codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);
		strncpy(buf, codec->fromUnicode(QString::fromUtf8(this->fTadsBuffer->getbuf(),
														  this->fTadsBuffer->getlen())).constData(), len);
	}
	buf[len] = '\0';

	if (not qFrame->gameRunning()) {
		return false;
	}
	return true;
}


void
CHtmlSysWinInputQt::cancelInput( bool reset )
{
	if (this->fTag == 0) {
		// There's nothing to cancel.
		return;
	}

	this->fTadsBuffer->hide_caret();
	this->fCastDispWidget->setCursorVisible(false);
	static_cast<CHtmlFormatterInput*>(this->formatter_)->end_input();

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
	//this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
	//qFrame->advanceEventLoop();

	// Done with the tag.
	if (reset) {
		this->fTag = 0;
		this->fRestoreFromCancel = false;
	} else {
		this->fRestoreFromCancel = true;
	}
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

	this->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);

	// Clear any pending HREF event.
	this->fHrefEvent.clear();

	extKey = 0;
	this->fStartKeypressInput();
	// Reset the MORE prompt position to this point, since the user has seen
	// everything up to here.
	this->lastInputHeight = this->formatter_->get_max_y_pos();
	if (useTimeout) {
		QEventLoop idleLoop;
		QTimer timer;
		timer.setSingleShot(true);
		connect(&timer, SIGNAL(timeout()), &idleLoop, SLOT(quit()));
		connect(qFrame, SIGNAL(gameQuitting()), &idleLoop, SLOT(quit()));
		connect(this, SIGNAL(inputReady()), &idleLoop, SLOT(quit()));
		timer.start(timeout);
		idleLoop.exec();
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


void
CHtmlSysWinInputQt::pagePause()
{
	InputMode lastMode = this->fInputMode;
	this->fInputMode = PagePauseInput;
	while (this->fInputMode == PagePauseInput and qFrame->gameRunning()) {
		qFrame->advanceEventLoop(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
	}
	this->fInputMode = lastMode;
}


void
CHtmlSysWinInputQt::set_html_input_color(HTML_color_t clr, int use_default)
{
	//qDebug() << Q_FUNC_INFO;

	if (use_default) {
		const QColor& def = qFrame->settings()->inputColor;
		qFrame->inputColor(HTML_make_color(def.red(), def.green(), def.blue()));
	} else {
		qFrame->inputColor(this->map_system_color(clr));
	}
}
