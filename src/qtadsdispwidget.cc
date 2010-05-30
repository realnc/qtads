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
#include "qtadssettings.h"


QTadsDisplayWidget::QTadsDisplayWidget( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
  : QWidget(parent), fHoverLink(0), fClickedLink(0), parentSysWin(parent), formatter(formatter)
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
	this->formatter->draw(&cRect, false, 0);
}


void
QTadsDisplayWidget::mouseMoveEvent( QMouseEvent* e )
{
	// Get the display object containing the position.
	CHtmlPoint pos;
	pos.set(e->pos().x(), e->pos().y());
	CHtmlDisp* disp = this->formatter->find_by_pos(pos, true);

	// If there's nothing, no need to continue.
	if (disp == 0) {
		// If we were tracking anything, forget about it.
		if (this->fHoverLink != 0) {
			this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
			this->fHoverLink = 0;
		}
		this->unsetCursor();
		qWinGroup->statusBar()->clearMessage();
		return;
	}

	// It could be a link.
	if (qFrame->settings()->enableLinks) {
		CHtmlDispLink* link = disp->get_link(this->formatter, pos.x, pos.y);

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
		if (disp->get_alt_text() != 0 and qstrlen(disp->get_alt_text()) > 0) {
			qWinGroup->statusBar()->showMessage(disp->get_alt_text());
			return;
		}

		// It could be a clickable link without any ALT text.  In that case, show
		// its contents.
		if (link != 0 and link->is_clickable_link()) {
			qWinGroup->statusBar()->showMessage(link->href_.get_url());
			return;
		}
	}

	// We don't know what it was.  Clear status bar message, reset cursor shape
	// and forget about any link we were tracking.
	qWinGroup->statusBar()->clearMessage();
	this->unsetCursor();
	if (this->fHoverLink != 0) {
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
		this->fHoverLink = 0;
	}
}


void
QTadsDisplayWidget::leaveEvent( QEvent* e )
{
	// If we're tracking a link, forget it.
	if (this->fHoverLink != 0 or this->fClickedLink != 0) {
		if (this->fClickedLink != 0) {
			this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
			this->fClickedLink = 0;
		}
		if (this->fHoverLink != 0) {
			this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
			this->fHoverLink = 0;
		}
		this->unsetCursor();
		qWinGroup->statusBar()->clearMessage();
	}
}


void
QTadsDisplayWidget::mousePressEvent( QMouseEvent* e )
{
	if (this->fHoverLink == 0) {
		// We're not hover-tracking a link; there's nothing to do here.
		return;
	}

	// If we're hover-tracking a clickable link here, also click-track it.
	if (this->fHoverLink->is_clickable_link() and qFrame->settings()->enableLinks) {
		// Draw all of the items involved in the link in the hilited state.
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_clicked);
		this->fClickedLink = this->fHoverLink;
	}
}


void
QTadsDisplayWidget::mouseReleaseEvent( QMouseEvent* e )
{
	if (this->fClickedLink == 0) {
		// We're not click-tracking a link; there's nothing to do here.
		return;
	}

	if (this->fClickedLink == this->fHoverLink) {
		// We're still hovering over the clicked link; process it.
		const textchar_t* cmd = this->fClickedLink->href_.get_url();
		qFrame->gameWindow()->processCommand(cmd, qstrlen(cmd), this->fClickedLink->get_append(),
											 not this->fClickedLink->get_noenter(), OS_CMD_NONE);
		// Put it back in hovering mode.
		if (qFrame->settings()->highlightLinks) {
			this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
		}
	} else if (this->fHoverLink != 0) {
		// We're hovering over another link; put it in hover mode.
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
	}
	// Stop click-tracking it.
	this->fClickedLink = 0;
}
