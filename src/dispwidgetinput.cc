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

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>

#include "htmlfmt.h"
#include "htmltags.h"
#include "htmlinp.h"
#include "htmldisp.h"

#include "settings.h"
#include "syswingroup.h"
#include "syswin.h"
#include "dispwidgetinput.h"


DisplayWidgetInput::DisplayWidgetInput( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
: DisplayWidget(parent, formatter), fCursorPos(0, 0), fLastCursorPos(0, 0), fCursorVisible(false),
  fBlinkVisible(false), fBlinkTimer(new QTimer(this))
{
	connect(this->fBlinkTimer, SIGNAL(timeout()), this, SLOT(fBlinkCursor()));
	this->resetCursorBlinking();

	// Our initial height is the height of the current input font.
	this->fHeight = QFontMetrics(qFrame->settings()->inputFont).height();

	// We need to check whether the application lost focus.
	connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(fHandleFocusChange(QWidget*,QWidget*)));
}


void
DisplayWidgetInput::paintEvent( QPaintEvent* e )
{
	//qDebug() << Q_FUNC_INFO << "called";

	DisplayWidget::paintEvent(e);
	QPainter painter(this);
	if (this->fCursorVisible and this->fBlinkVisible) {
		painter.drawLine(this->fCursorPos.x(), this->fCursorPos.y(), this->fCursorPos.x(),
						 this->fCursorPos.y() + this->fHeight);
	}
}


void
DisplayWidgetInput::fBlinkCursor()
{
	this->fBlinkVisible = not this->fBlinkVisible;
	this->update(this->fCursorPos.x(), this->fCursorPos.y(), this->fCursorPos.x() + 1,
				 this->fCursorPos.y() + this->fHeight);
}


void
DisplayWidgetInput::fHandleFocusChange( QWidget* old, QWidget* now )
{
	if (now == 0) {
		// The application window lost focus.  Disable cursor blinking.
		this->fBlinkTimer->stop();
#ifdef Q_WS_MAC
		// On the Mac, when applications lose focus the cursor must be disabled.
		if (this->fBlinkVisible) {
			this->fBlinkCursor();
		}
#else
		// On all other systems we assume the cursor must stay visible.
		if (not this->fBlinkVisible) {
			this->fBlinkCursor();
		}
#endif
	} else if (old == 0 and now != 0) {
		// The application window gained focus.  Reset cursor blinking.
		this->resetCursorBlinking();
	}
}


void
DisplayWidgetInput::updateCursorPos( CHtmlFormatter* formatter, CHtmlInputBuf* tadsBuffer, CHtmlTagTextInput* tag )
{
	// Reset the blink timer.
	if (this->fBlinkTimer->isActive()) {
		this->fBlinkTimer->start();
	}

	// Blink-out first to ensure the cursor won't stay visible at the previous
	// position after we move it.
	if (this->fBlinkVisible) {
		this->fBlinkCursor();
	}

	this->parentSysWin->do_formatting(false, false, false);
	CHtmlPoint cursorPos = formatter->get_text_pos(tag->get_text_ofs() + tadsBuffer->get_caret());
	this->moveCursorPos(QPoint(cursorPos.x, cursorPos.y));

	// Update the selection range in the formatter.
	size_t start, end, caret;
	unsigned long inp_txt_ofs = tag->get_text_ofs();
	tadsBuffer->get_sel_range(&start, &end, &caret);
	formatter->set_sel_range(start + inp_txt_ofs, end + inp_txt_ofs);

	// Blink-in.
	if (not this->fBlinkVisible) {
		this->fBlinkCursor();
	}

	CHtmlDisp* disp = formatter->find_by_pos(cursorPos, false);
	if (disp != 0) {
		const CHtmlRect& itemRect = disp->get_pos();
		this->update(0, itemRect.top, this->width(), itemRect.bottom - itemRect.top);
	}
}


void
DisplayWidgetInput::resetCursorBlinking()
{
	// Start the timer unless cursor blinking is disabled.
	if (QApplication::cursorFlashTime() > 1) {
		this->fBlinkTimer->start(QApplication::cursorFlashTime() / 2);
	}
}
