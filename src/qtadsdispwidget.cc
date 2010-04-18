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

#include "htmlattr.h"
#include "htmltags.h"
#include "htmlqt.h"
#include "htmlfmt.h"
#include "tadshtml.h"
#include "htmlinp.h"
#include "htmldisp.h"
#include "htmlurl.h"

#include "qtadsdispwidget.h"


QTadsDisplayWidget::QTadsDisplayWidget( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
 : QWidget(parent), fParentSysWin(parent), fFormatter(formatter), fCursorPos(0, 0), fLastCursorPos(0, 0),
   fCursorVisible(false), fBlinkVisible(false), fBlinkTimer(new QTimer(this))
{
	this->setForegroundRole(QPalette::Text);
	this->setBackgroundRole(QPalette::Base);
	connect(this->fBlinkTimer, SIGNAL(timeout()), this, SLOT(fBlinkCursor()));
	this->fBlinkTimer->start(QApplication::cursorFlashTime() / 2);

	// Enable mouse tracking, since we need to change the mouse cursor shape
	// when hovering over hyperlinks.
	this->setMouseTracking(true);
}


void
QTadsDisplayWidget::paintEvent( QPaintEvent* e )
{
	//qDebug() << Q_FUNC_INFO << "called";

	//qDebug() << "repainting" << e->rect();
	const QRect& qRect = e->region().boundingRect();
	CHtmlRect cRect(qRect.left(), qRect.top(), qRect.left() + qRect.width(), qRect.top() + qRect.height());
	this->fFormatter->draw(&cRect, false, 0);
	QPainter painter(this);
	if (this->fCursorVisible) {
		if (this->fBlinkVisible) {
			// Blink out.
		} else {
			// Blink in.
			painter.drawLine(this->fCursorPos.x(), this->fCursorPos.y()+2,
							 this->fCursorPos.x(), this->fCursorPos.y()+18);
		}
	}
}

/*
void
QTadsDisplayWidget::mouseMoveEvent( QMouseEvent* e )
{
	qDebug() << e->pos();
}
*/

void
QTadsDisplayWidget::mousePressEvent( QMouseEvent* e )
{
	// Get the display containing the position.
	CHtmlPoint pos;
	pos.set(e->pos().x(), e->pos().y());
	CHtmlDisp* disp = this->fFormatter->find_by_pos(pos, true);

	// Get the link from the display item, if there is one.
	CHtmlDispLink* link = 0;
	if (disp != 0) {
		link = disp->get_link(this->fFormatter, pos.x, pos.y);
	}

	// If we found a link, process it.
	// FIXME: We should track the link and process it at mouseReleaseEvent().
	if (link != 0 and link->is_clickable_link()) {
		// Draw all of the items involved in the link in the hilited state.
		link->set_clicked(this->fParentSysWin, CHtmlDispLink_clicked);
		const textchar_t* cmd = link->href_.get_url();
		qFrame->gameWindow()->processCommand(cmd, qstrlen(cmd), link->get_append(),
											 not link->get_noenter(), OS_CMD_NONE);
	}
}


void
QTadsDisplayWidget::fBlinkCursor()
{
	this->fBlinkVisible = not this->fBlinkVisible;
	this->update(this->fCursorPos.x(), this->fCursorPos.y(), this->fCursorPos.x()+1, this->fCursorPos.y()+15);
}


void
QTadsDisplayWidget::updateCursorPos( CHtmlFormatter* formatter, CHtmlInputBuf* tadsBuffer, CHtmlTagTextInput* tag )
{
	CHtmlPoint cursorPos;
	this->fParentSysWin->do_formatting(false, false, false);
	cursorPos = formatter->get_text_pos(tag->get_text_ofs() + tadsBuffer->get_caret());
	//qDebug() << "Moving cursor to x:" << cursorPos.x << "y:" << cursorPos.y;
	//qDebug() << "caret position:" << tadsBuffer->get_caret();
	this->moveCursorPos(QPoint(cursorPos.x, cursorPos.y));

	// Update the selection range in the formatter.
	size_t start, end, caret;
	unsigned long inp_txt_ofs = tag->get_text_ofs();
	tadsBuffer->get_sel_range(&start, &end, &caret);
	formatter->set_sel_range(start + inp_txt_ofs, end + inp_txt_ofs);

	this->update();
}
