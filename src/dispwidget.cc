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
#include "htmlfmt.h"
#include "htmldisp.h"

#include "dispwidget.h"
#include "settings.h"
#include "syswininput.h"


DisplayWidget::DisplayWidget( CHtmlSysWinQt* parent, CHtmlFormatter* formatter )
: QWidget(parent), fHoverLink(0), fClickedLink(0), parentSysWin(parent), formatter(formatter)
{
	this->setForegroundRole(QPalette::Text);
	this->setBackgroundRole(QPalette::Base);

	// Enable mouse tracking, since we need to change the mouse cursor shape
	// when hovering over hyperlinks.
	this->setMouseTracking(true);
}


void
DisplayWidget::fInvalidateLinkTracking()
{
	// If we're tracking links (hover/click), forget about them.
	if (this->fClickedLink != 0) {
		this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
		this->fClickedLink = 0;
	}
	if (this->fHoverLink != 0) {
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
		this->fHoverLink = 0;
	}
	this->unsetCursor();
	qWinGroup->statusBar()->setUpdatesEnabled(false);
	qWinGroup->statusBar()->clearMessage();
	qWinGroup->statusBar()->setUpdatesEnabled(true);
}


void
DisplayWidget::paintEvent( QPaintEvent* e )
{
	//qDebug() << Q_FUNC_INFO << "called";

	//qDebug() << "repainting" << e->rect();
	const QRect& qRect = e->region().boundingRect();
	CHtmlRect cRect(qRect.left(), qRect.top(), qRect.left() + qRect.width(), qRect.top() + qRect.height());
	this->formatter->draw(&cRect, false, 0);
}


void
DisplayWidget::mouseMoveEvent( QMouseEvent* e )
{
	this->updateLinkTracking(e->pos());
}


void
DisplayWidget::leaveEvent( QEvent* e )
{
	this->fInvalidateLinkTracking();
}


void
DisplayWidget::mousePressEvent( QMouseEvent* e )
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
DisplayWidget::mouseReleaseEvent( QMouseEvent* e )
{
	if (this->fClickedLink == 0) {
		// We're not click-tracking a link; there's nothing to do here.
		return;
	}

	// If we're still hovering over the clicked link, process it.
	if (this->fClickedLink == this->fHoverLink) {
		const textchar_t* cmd = this->fClickedLink->href_.get_url();
		qFrame->gameWindow()->processCommand(cmd, strlen(cmd), this->fClickedLink->get_append(),
											 not this->fClickedLink->get_noenter(), OS_CMD_NONE);
		// Put it back in hovering mode.
		if (qFrame->settings()->highlightLinks) {
			this->fClickedLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
		}
	// Otherwise, if we're hovering over another link, put that one in hover mode.
	} else if (this->fHoverLink != 0) {
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_hover);
	}
	// Stop click-tracking it.
	this->fClickedLink = 0;
}


void
DisplayWidget::updateLinkTracking( const QPoint& mousePos )
{
	// Get the display object containing the position.
	CHtmlPoint docPos;
	// If specified mouse position is invalid, map it from the current global
	// position.
	if (mousePos.isNull()) {
		const QPoint pos(this->mapFromGlobal(QCursor::pos()));
		docPos.set(pos.x(), pos.y());
	} else {
		docPos.set(mousePos.x(), mousePos.y());
	}
	CHtmlDisp* disp = this->formatter->find_by_pos(docPos, true);

	// If there's nothing, no need to continue.
	if (disp == 0) {
		// If we were tracking anything, forget about it.
		if (this->fHoverLink != 0) {
			this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
			this->fHoverLink = 0;
			this->unsetCursor();
			qWinGroup->statusBar()->setUpdatesEnabled(false);
			qWinGroup->statusBar()->clearMessage();
			qWinGroup->statusBar()->setUpdatesEnabled(true);
		}
		return;
	}

	// It could be a link.
	if (qFrame->settings()->enableLinks) {
		CHtmlDispLink* link = disp->get_link(this->formatter, docPos.x, docPos.y);

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
		if (disp->get_alt_text() != 0 and strlen(disp->get_alt_text()) > 0) {
			qWinGroup->statusBar()->setUpdatesEnabled(false);
			qWinGroup->statusBar()->showMessage(QString::fromUtf8(disp->get_alt_text()));
			qWinGroup->statusBar()->setUpdatesEnabled(true);
			return;
		}

		// It could be a clickable link without any ALT text.  In that case, show
		// its contents.
		if (link != 0 and link->is_clickable_link()) {
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
	this->unsetCursor();
	if (this->fHoverLink != 0) {
		this->fHoverLink->set_clicked(this->parentSysWin, CHtmlDispLink_none);
		this->fHoverLink = 0;
	}
}
