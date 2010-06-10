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
#include <Qt>
#include <QDebug>
#include <QMenuBar>
#include <QCloseEvent>
#include <QFileDialog>
#include <QVBoxLayout>

#include "syswinaboutbox.h"
#include "qtadsconfdialog.h"
#include "qtadssettings.h"


void
CHtmlSysWinGroupQt::QTadsFrame::resizeEvent( QResizeEvent* e )
{
	if (qFrame->gameWindow() != 0) {
		qFrame->reformatBanners();
		qFrame->adjustBannerSizes();
	}
}


CHtmlSysWinGroupQt::CHtmlSysWinGroupQt()
: fConfDialog(0), fAboutBoxDialog(0), fAboutBox(0)
{
	//qDebug() << Q_FUNC_INFO << "called";
	Q_ASSERT(qWinGroup == 0);

	// We make our menu bar parentless so it will be shared by all our windows
	// in Mac OS X.
	QMenuBar* menuBar = new QMenuBar(0);

	// "File" menu.
	QMenu* fileMenu = menuBar->addMenu(tr("&File"));
	QAction* openAct = new QAction(tr("&Open New Game"), this);
	openAct->setShortcuts(QKeySequence::Open);
	fileMenu->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(fOpenNewGame()));
	QAction* recentAct = new QAction(tr("&Recent Games"), this);
	this->fRecentGamesMenu = new QMenu("Recent Games", this);
	recentAct->setMenu(this->fRecentGamesMenu);
	fileMenu->addAction(recentAct);
	connect(this->fRecentGamesMenu, SIGNAL(triggered(QAction*)), this, SLOT(fRecentGameTriggered(QAction*)));

	// "Edit" menu.
	QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
	QAction* settingsAct = new QAction(tr("&Preferences"), this);
#if QT_VERSION >= 0x040600
	settingsAct->setShortcuts(QKeySequence::Preferences);
#endif
	editMenu->addAction(settingsAct);
	connect(settingsAct, SIGNAL(triggered()), this, SLOT(fShowConfDialog()));

	// "Help" menu.
	QMenu* gameMenu = menuBar->addMenu(tr("&Help"));
	this->fAboutGameAction = new QAction(tr("&About This Game"), this);
	this->fAboutGameAction->setMenuRole(QAction::ApplicationSpecificRole);
	this->fAboutGameAction->setEnabled(false);
	gameMenu->addAction(this->fAboutGameAction);
	connect(this->fAboutGameAction, SIGNAL(triggered()), this, SLOT(fShowAboutGame()));

	this->setMenuBar(menuBar);

	// Create a default status bar.
	this->statusBar();

	// Set up our central widget.
	this->fFrame = new QTadsFrame(this);
	this->fFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	this->fFrame->setLineWidth(0);
	this->fFrame->setContentsMargins(0,0,0,0);
	this->setCentralWidget(this->fFrame);

	// Use a sane minimum size; by default Qt would allow us to be resized
	// to almost zero.
	this->setMinimumSize(240, 180);

	qWinGroup = this;
}


CHtmlSysWinGroupQt::~CHtmlSysWinGroupQt()
{
	qDebug() << Q_FUNC_INFO;

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
#ifdef Q_WS_MAC
	// There's a bug in Qt for OS X that results in a visual glitch with
	// QFontComboBox widgets inside QFormLayouts.  Making the dialog 4 pixels
	// higher fixes it.
	//
	// See: http://bugreports.qt.nokia.com/browse/QTBUG-10460
	this->fConfDialog->layout()->activate();
	this->fConfDialog->setMinimumHeight(this->fConfDialog->minimumHeight() + 4);
#endif
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
CHtmlSysWinGroupQt::fShowAboutGame()
{
	if (this->fAboutBoxDialog == 0) {
		return;
	}
	if (this->fAboutBoxDialog->isVisible()) {
		this->fAboutBoxDialog->activateWindow();
		this->fAboutBoxDialog->raise();
		return;
	}
	this->fAboutBoxDialog->resize(this->fAboutBox->size());
	this->fAboutBoxDialog->show();
}


void
CHtmlSysWinGroupQt::fOpenNewGame()
{
	const QString& fname = QFileDialog::getOpenFileName(0, "Choose the TADS game you wish to run", "",
														"TADS Games (*.gam *.Gam *.GAM *.t3 *.T3)");
	if (not fname.isEmpty()) {
		qFrame->setNextGame(fname);
	}
}


void
CHtmlSysWinGroupQt::fRecentGameTriggered( QAction* action )
{
	qFrame->setNextGame(action->text().replace("&&", "&"));
}


void
CHtmlSysWinGroupQt::closeEvent( QCloseEvent* e )
{
	qFrame->setGameRunning(false);
	e->accept();
}


CHtmlSysWinAboutBoxQt*
CHtmlSysWinGroupQt::createAboutBox( class CHtmlFormatter* formatter )
{
	// If there's already an "about" box, destroy it first.
	if (this->fAboutBoxDialog != 0) {
		this->fAboutBoxDialog->hide();
		delete this->fAboutBoxDialog;
	} else {
		this->fAboutGameAction->setEnabled(true);
	}

	this->fAboutBoxDialog = new QDialog(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	this->fAboutBoxDialog->setWindowTitle(tr("About This Game"));
	this->fAboutBox = new CHtmlSysWinAboutBoxQt(formatter, this->fAboutBoxDialog);
	QVBoxLayout* layout = new QVBoxLayout(this->fAboutBoxDialog);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(this->fAboutBox);

	// Only set the width to something comfortable.  The height will be
	// calculated later when set_banner_size() is called on the about box.
	this->fAboutBoxDialog->resize(500, 0);
	this->fAboutBoxDialog->layout()->activate();
	return this->fAboutBox;
}


void
CHtmlSysWinGroupQt::deleteAboutBox()
{
	if (this->fAboutBoxDialog == 0) {
		return;
	}
	this->fAboutBoxDialog->hide();
	delete this->fAboutBoxDialog;
	this->fAboutBoxDialog = 0;
	this->fAboutBox = 0;
	this->fAboutGameAction->setEnabled(false);
}


void
CHtmlSysWinGroupQt::updateRecentGames()
{
	// We simply clear the menu of all items and re-populate it.
	this->fRecentGamesMenu->clear();

	// If the list is empty, disable the menu.
	if (qFrame->settings()->recentGamesList.isEmpty()) {
		this->fRecentGamesMenu->setEnabled(false);
		return;
	}

	// The list is not empty.  If the menu was disabled, enable it.
	if (not this->fRecentGamesMenu->isEnabled()) {
		this->fRecentGamesMenu->setEnabled(true);
	}

	// Populate it.
	for (int i = 0; i < qFrame->settings()->recentGamesList.size(); ++i) {
		this->fRecentGamesMenu->addAction(QString(qFrame->settings()->recentGamesList.at(i)).replace("&", "&&"));
	}
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
