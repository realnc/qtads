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
#include <QPaintEvent>
#include <QStatusBar>

#include "htmlattr.h"
#include "htmlqt.h"
#include "htmlfmt.h"
#include "htmldisp.h"

#include "qtadsdispwidget.h"


QTadsDisplayWidget::QTadsDisplayWidget( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
 : QWidget(parent), fParentSysWin(parent), fFormatter(formatter)
{
	this->setForegroundRole(QPalette::Text);
	this->setBackgroundRole(QPalette::Base);

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
}


void
QTadsDisplayWidget::mouseMoveEvent( QMouseEvent* e )
{
	// Get the display object containing the position.
	CHtmlPoint pos;
	pos.set(e->pos().x(), e->pos().y());
	CHtmlDisp* disp = this->fFormatter->find_by_pos(pos, true);

	// If there's nothing, no need to continue.
	if (disp == 0) {
		this->unsetCursor();
		qWinGroup->statusBar()->clearMessage();
		return;
	}

	// It could be a link.
	CHtmlDispLink* link = disp->get_link(this->fFormatter, pos.x, pos.y);

	// If we found something that has ALT text, show it in the status bar.
	if (disp->get_alt_text() != 0 and qstrlen(disp->get_alt_text()) > 0) {
		qWinGroup->statusBar()->showMessage(disp->get_alt_text());
		// If it's a clickable link, also change the mouse cursor shape.
		if (link != 0 and link->is_clickable_link()) {
			this->setCursor(Qt::PointingHandCursor);
		}
		return;
	}

	// It could be a clickable link without any ALT text.  In that case, show
	// its contents.
	if (link != 0 and link->is_clickable_link()) {
		qWinGroup->statusBar()->showMessage(link->href_.get_url());
		this->setCursor(Qt::PointingHandCursor);
		return;
	}

	// We don't know what it was.  Clear status bar message and reset cursor
	// shape.
	qWinGroup->statusBar()->clearMessage();
	this->unsetCursor();
}


void
QTadsDisplayWidget::mousePressEvent( QMouseEvent* e )
{
	// Get the display object containing the position.
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
