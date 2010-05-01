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

#include <QMenuBar>
#include <QCloseEvent>

#include "htmlqt.h"
#include "qtadsconfdialog.h"


CHtmlSysWinGroupQt::CHtmlSysWinGroupQt()
: fConfDialog(0)
{
	//qDebug() << Q_FUNC_INFO << "called";
	Q_ASSERT(qWinGroup == 0);

	// We make our menu bar parentless so it will be shared by all our windows
	// in Mac OS X.
	QMenuBar* menuBar = new QMenuBar(0);
	QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
	QAction* settingsAct = new QAction(tr("&Settings"), menuBar);
	editMenu->addAction(settingsAct);
	connect(settingsAct, SIGNAL(triggered()), this, SLOT(fShowConfDialog()));
	this->setMenuBar(menuBar);

	// Create a default status bar.
	this->statusBar();

	qWinGroup = this;
}


CHtmlSysWinGroupQt::~CHtmlSysWinGroupQt()
{
	qDebug() << Q_FUNC_INFO << "called";

	Q_ASSERT(qWinGroup != 0);

	qWinGroup = 0;
	delete this->menuBar();
}


void
CHtmlSysWinGroupQt::fShowConfDialog()
{
	// If the dialog is already open, simply activate and raise it.
	if (this->fConfDialog != 0) {
		this->fConfDialog->activateWindow();
		this->fConfDialog->raise();
		return;
	}
	this->fConfDialog = new QTadsConfDialog(this);
	this->fConfDialog->setWindowTitle(tr("QTads Preferences"));
	connect(this->fConfDialog, SIGNAL(finished(int)), this, SLOT(fHideConfDialog()));
	this->fConfDialog->show();
}


void
CHtmlSysWinGroupQt::fHideConfDialog()
{
	if (this->fConfDialog != 0) {
		this->fConfDialog->deleteLater();
		this->fConfDialog = 0;
	}
}


void
CHtmlSysWinGroupQt::closeEvent( QCloseEvent* event )
{
	qFrame->setGameRunning(false);
	event->accept();
}


oshtml_charset_id_t
CHtmlSysWinGroupQt::get_default_win_charset() const
{
	//qDebug() << Q_FUNC_INFO << "called";

	return 0;
}


size_t
CHtmlSysWinGroupQt::xlat_html4_entity( textchar_t* result, size_t result_size, unsigned int charval,
									   oshtml_charset_id_t* charset, int* changed_charset )
{
	//qDebug() << Q_FUNC_INFO << "called";
	Q_ASSERT(result != 0);

	// HTML4 entities are Unicode characters, which means the QChar(uint) ctor
	// will do the right thing.
	QString s = QString(QChar(charval));
	strcpy(result, s.toUtf8());
	if (changed_charset != 0) {
		*changed_charset = false;
	}
	return s.toUtf8().length();
}
