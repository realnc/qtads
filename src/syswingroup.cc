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
#include <QMessageBox>
#include <QUrl>

#include "syswininput.h"
#include "syswinaboutbox.h"
#include "confdialog.h"
#include "settings.h"
#include "gameinfodialog.h"
#include "aboutqtadsdialog.h"


void
CHtmlSysWinGroupQt::QTadsFrame::resizeEvent( QResizeEvent* e )
{
	qFrame->reformatBanners(true, true, false);
}


CHtmlSysWinGroupQt::CHtmlSysWinGroupQt()
: fConfDialog(0), fGameInfoDialog(0), fAboutBoxDialog(0), fAboutBox(0), fAboutQtadsDialog(0)
{
	//qDebug() << Q_FUNC_INFO << "called";
	Q_ASSERT(qWinGroup == 0);

	// Make sure we can drag&drop (files in our case) into the main window.
	this->setAcceptDrops(true);

	// We make our menu bar parentless so it will be shared by all our windows
	// in Mac OS X.
	QMenuBar* menuBar = new QMenuBar(0);

	// "File" menu.
	QMenu* menu = menuBar->addMenu(tr("&File"));
	QAction* act = new QAction(tr("&Open New Game"), this);
#if QT_VERSION >= 0x040600
	act->setIcon(QIcon::fromTheme("document-open"));
#endif
	act->setShortcuts(QKeySequence::Open);
	menu->addAction(act);
	connect(act, SIGNAL(triggered()), this, SLOT(fOpenNewGame()));
	act = new QAction(tr("&Recent Games"), this);
#if QT_VERSION >= 0x040600
	act->setIcon(QIcon::fromTheme("document-open-recent"));
#endif
	this->fRecentGamesMenu = new QMenu("Recent Games", this);
	act->setMenu(this->fRecentGamesMenu);
	menu->addAction(act);
	connect(this->fRecentGamesMenu, SIGNAL(triggered(QAction*)), this, SLOT(fRecentGameTriggered(QAction*)));
	this->fGameInfoAction = new QAction(tr("Game &Information"), this);
#if QT_VERSION >= 0x040600
	this->fGameInfoAction->setIcon(QIcon::fromTheme("document-properties"));
#endif
	menu->addAction(this->fGameInfoAction);
	this->fGameInfoAction->setEnabled(false);
	connect(this->fGameInfoAction, SIGNAL(triggered()), this, SLOT(fShowGameInfoDialog()));
	this->fRestartCurrentGameAction = new QAction(tr("Re&start Current Game"), this);
#if QT_VERSION >= 0x040600
	this->fRestartCurrentGameAction->setIcon(QIcon::fromTheme("view-refresh"));
#endif
	this->fRestartCurrentGameAction->setShortcut(QKeySequence("Ctrl+R"));
	menu->addAction(this->fRestartCurrentGameAction);
	this->fRestartCurrentGameAction->setEnabled(false);
	connect(this->fRestartCurrentGameAction, SIGNAL(triggered()), this, SLOT(fRestartCurrentGame()));
	this->fEndCurrentGameAction = new QAction(tr("&End Current Game"), this);
#if QT_VERSION >= 0x040600
	this->fEndCurrentGameAction->setIcon(QIcon::fromTheme("process-stop"));
#endif
	this->fEndCurrentGameAction->setShortcuts(QKeySequence::Close);
	menu->addAction(this->fEndCurrentGameAction);
	this->fEndCurrentGameAction->setEnabled(false);
	connect(this->fEndCurrentGameAction, SIGNAL(triggered()), this, SLOT(fEndCurrentGame()));
	menu->addSeparator();
	act = new QAction(tr("&Quit"), this);
#if QT_VERSION >= 0x040600
	act->setIcon(QIcon::fromTheme("application-exit"));
	act->setShortcuts(QKeySequence::Quit);
#endif
	menu->addAction(act);
	connect(act, SIGNAL(triggered()), this, SLOT(close()));

	// "Edit" menu.
	menu = menuBar->addMenu(tr("&Edit"));
	act = new QAction(tr("&Preferences"), this);
#if QT_VERSION >= 0x040600
	act->setIcon(QIcon::fromTheme("configure"));
	act->setShortcuts(QKeySequence::Preferences);
#endif
	menu->addAction(act);
	connect(act, SIGNAL(triggered()), this, SLOT(fShowConfDialog()));

	// "Help" menu.
	menu = menuBar->addMenu(tr("&Help"));
	this->fAboutGameAction = new QAction(tr("&About This Game"), this);
	this->fAboutGameAction->setMenuRole(QAction::ApplicationSpecificRole);
	this->fAboutGameAction->setEnabled(false);
	menu->addAction(this->fAboutGameAction);
	connect(this->fAboutGameAction, SIGNAL(triggered()), this, SLOT(fShowAboutGame()));
	this->fAboutQtadsAction = new QAction(tr("A&bout QTads"), this);
#if QT_VERSION >= 0x040600
	this->fAboutQtadsAction->setIcon(QIcon::fromTheme("help-about"));
#endif
	menu->addAction(this->fAboutQtadsAction);
	connect(this->fAboutQtadsAction, SIGNAL(triggered()), this, SLOT(fShowAboutQtads()));

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

	// Receive notification when a game is about to quit/start so we can
	// enable/disable related actions.
	connect(qFrame, SIGNAL(gameQuitting()), this, SLOT(fNotifyGameQuitting()));
	connect(qFrame, SIGNAL(gameHasQuit()), this, SLOT(fNotifyGameQuitting()));
	connect(qFrame, SIGNAL(gameStarting()), this, SLOT(fNotifyGameStarting()));

	qWinGroup = this;
}


CHtmlSysWinGroupQt::~CHtmlSysWinGroupQt()
{
	Q_ASSERT(qWinGroup != 0);
	qWinGroup = 0;
}


bool
CHtmlSysWinGroupQt::fAskQuitGameDialog()
{
	if (not qFrame->gameRunning()) {
		return true;
	}
	if (QMessageBox::question(0, tr("End Current Game") + " - " + qFrame->applicationName(),
							  tr("If you didn't save the current game, all progress will be lost. Do you wish"
								 " to quit the game?"), QMessageBox::Yes | QMessageBox::Cancel,
							  QMessageBox::Cancel)
		== QMessageBox::Yes)
	{
		return true;
	}
	return false;
}


bool
CHtmlSysWinGroupQt::fAskRestartGameDialog()
{
	if (not qFrame->gameRunning()) {
		return true;
	}
	if (QMessageBox::question(0, tr("Restart Current Game") + " - " + qFrame->applicationName(),
							  tr("If you didn't save the current game, all progress will be lost. Do you wish"
								 " to restart the game?"), QMessageBox::Yes | QMessageBox::Cancel,
							  QMessageBox::Cancel)
		== QMessageBox::Yes)
	{
		return true;
	}
	return false;
}


void
CHtmlSysWinGroupQt::fShowGameInfoDialog()
{
	// If the dialog is already open, simply activate and raise it.
	if (this->fGameInfoDialog != 0) {
		this->fGameInfoDialog->activateWindow();
		this->fGameInfoDialog->raise();
		return;
	}
	this->fGameInfoDialog = new GameInfoDialog(qFrame->gameFile(), this);
	this->fGameInfoDialog->setWindowTitle(tr("Game Information"));
	connect(this->fGameInfoDialog, SIGNAL(finished(int)), this, SLOT(fHideGameInfoDialog()));
	this->fGameInfoDialog->show();
}


void
CHtmlSysWinGroupQt::fHideGameInfoDialog()
{
	if (this->fGameInfoDialog != 0) {
		this->fGameInfoDialog->deleteLater();
		this->fGameInfoDialog = 0;
	}
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
	this->fConfDialog = new ConfDialog(this);
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
CHtmlSysWinGroupQt::fShowAboutQtads()
{
	// If the dialog is already open, simply activate and raise it.
	if (this->fAboutQtadsDialog != 0) {
		this->fAboutQtadsDialog->activateWindow();
		this->fAboutQtadsDialog->raise();
		return;
	}

	this->fAboutQtadsDialog = new AboutQtadsDialog(this);
	connect(this->fAboutQtadsDialog, SIGNAL(finished(int)), this, SLOT(fHideAboutQtads()));
#ifdef Q_WS_MAC
	// Similar bug to the config dialog one.  Again, 4 pixels higher fixes it.
	this->fAboutQtadsDialog->layout()->activate();
	this->fAboutQtadsDialog->setMinimumHeight(this->fAboutQtadsDialog->minimumHeight() + 4);
#endif
	this->fAboutQtadsDialog->show();
}


void
CHtmlSysWinGroupQt::fHideAboutQtads()
{
	if (this->fAboutQtadsDialog != 0) {
		this->fAboutQtadsDialog->deleteLater();
		this->fAboutQtadsDialog = 0;
	}
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
	if (not this->fAskQuitGameDialog()) {
		return;
	}
	qFrame->setNextGame(action->text().replace("&&", "&"));
}


void
CHtmlSysWinGroupQt::fEndCurrentGame()
{
	if (this->fAskQuitGameDialog()) {
		qFrame->setGameRunning(false);
	}
}


void
CHtmlSysWinGroupQt::fRestartCurrentGame()
{
	if (this->fAskRestartGameDialog()) {
		qFrame->setNextGame(qFrame->gameFile());
	}
}


void
CHtmlSysWinGroupQt::fNotifyGameQuitting()
{
	this->fGameInfoAction->setEnabled(false);
	this->fRestartCurrentGameAction->setEnabled(false);
	this->fEndCurrentGameAction->setEnabled(false);
}


void
CHtmlSysWinGroupQt::fNotifyGameStarting()
{
	this->fHideGameInfoDialog();
	this->fGameInfoAction->setEnabled(GameInfoDialog::gameHasMetaInfo(qFrame->gameFile()));
	this->fRestartCurrentGameAction->setEnabled(true);
	this->fEndCurrentGameAction->setEnabled(true);
}


void
CHtmlSysWinGroupQt::closeEvent( QCloseEvent* e )
{
	if (qFrame->gameRunning()) {
		if (QMessageBox::question(0, tr("Quit Interpreter") + " - " + qFrame->applicationName(),
								  tr("A game is currently running. Abandon the game and quit the interpreter?"),
								  QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel)
			== QMessageBox::Yes)
		{
			qFrame->setGameRunning(false);
			e->accept();
		} else {
			e->ignore();
		}
	}
}


void
CHtmlSysWinGroupQt::dragEnterEvent( QDragEnterEvent* e )
{
	// Only accept the event if there is exactly one URL which points to a
	// local file.
	if (e->mimeData()->hasUrls() and e->mimeData()->urls().size() == 1
		and not e->mimeData()->urls().at(0).toLocalFile().isEmpty())
	{
		e->acceptProposedAction();
	}
}


void
CHtmlSysWinGroupQt::dropEvent( QDropEvent* e )
{
	if (this->fAskQuitGameDialog()) {
		qFrame->setNextGame(e->mimeData()->urls().at(0).toLocalFile());
		e->acceptProposedAction();
	} else {
		e->ignore();
	}
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
