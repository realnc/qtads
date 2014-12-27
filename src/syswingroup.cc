/* Copyright (C) 2013 Nikos Chantziaras.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */
#include <Qt>
#include <QDebug>
#include <QTimer>
#include <QMenuBar>
#include <QCloseEvent>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>
#include <QClipboard>
#include <QMimeData>

#include "syswininput.h"
#include "syswinaboutbox.h"
#include "confdialog.h"
#include "settings.h"
#include "gameinfodialog.h"
#include "aboutqtadsdialog.h"
#include "dispwidget.h"


void
QTadsFrame::resizeEvent( QResizeEvent* )
{
    qFrame->reformatBanners(true, true, false);
}


void
QTadsFrame::dragEnterEvent( QDragEnterEvent* e )
{
    // Only accept text. URLs are handled by the main window instead, for
    // opening game files through drag&drop.
    if (e->mimeData()->hasText() and not e->mimeData()->hasUrls()
        and qFrame->gameWindow()->acceptsText())
    {
        e->acceptProposedAction();
    }
}


void
QTadsFrame::dropEvent( QDropEvent* e )
{
    qFrame->gameWindow()->insertText(e->mimeData()->text());
}


CHtmlSysWinGroupQt::CHtmlSysWinGroupQt()
    : fConfDialog(0),
      fGameInfoDialog(0),
      fAboutBoxDialog(0),
      fAboutBox(0),
      fAboutQtadsDialog(0),
      fNetManager(0),
      fHttpRedirectCount(0),
      fWantsToQuit(false),
      fSilentIfNoUpdates(false)
{
    //qDebug() << Q_FUNC_INFO << "called";
    Q_ASSERT(qWinGroup == 0);

    // Make sure we can drag&drop (files in our case) into the main window.
    this->setAcceptDrops(true);

    // We make our menu bar parentless so it will be shared by all our windows
    // in Mac OS X.
    QMenuBar* menuBar = new QMenuBar(0);

    // "Game" menu.
    QMenu* menu = menuBar->addMenu(tr("&Game"));
    QAction* act = new QAction(tr("&Open") + QString::fromLatin1("..."), this);
#if QT_VERSION >= 0x040600
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("document-open")));
#endif
    act->setShortcuts(QKeySequence::Open);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(fOpenNewGame()));
    act = new QAction(tr("Open &Recent"), this);
#if QT_VERSION >= 0x040600
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("document-open-recent")));
#endif
    this->fRecentGamesMenu = new QMenu(this);
    act->setMenu(this->fRecentGamesMenu);
    menu->addAction(act);
    connect(this->fRecentGamesMenu, SIGNAL(triggered(QAction*)), this, SLOT(fRecentGameTriggered(QAction*)));
    this->fRestartCurrentGameAction = new QAction(tr("Re&start"), this);
#if QT_VERSION >= 0x040600
    this->fRestartCurrentGameAction->setIcon(QIcon::fromTheme(QString::fromLatin1("view-refresh")));
#endif
    this->fRestartCurrentGameAction->setShortcut(QKeySequence(QString::fromLatin1("Ctrl+R")));
    menu->addAction(this->fRestartCurrentGameAction);
    this->fRestartCurrentGameAction->setEnabled(false);
    connect(this->fRestartCurrentGameAction, SIGNAL(triggered()), this, SLOT(fRestartCurrentGame()));
    this->fEndCurrentGameAction = new QAction(tr("Qui&t"), this);
    this->fEndCurrentGameAction->setMenuRole(QAction::NoRole);
#if QT_VERSION >= 0x040600
    this->fEndCurrentGameAction->setIcon(QIcon::fromTheme(QString::fromLatin1("process-stop")));
#endif
    this->fEndCurrentGameAction->setShortcuts(QKeySequence::Close);
    menu->addAction(this->fEndCurrentGameAction);
    this->fEndCurrentGameAction->setEnabled(false);
    connect(this->fEndCurrentGameAction, SIGNAL(triggered()), this, SLOT(fEndCurrentGame()));
    menu->addSeparator();
    this->fAboutGameAction = new QAction(tr("&About This Game"), this);
    this->fAboutGameAction->setMenuRole(QAction::NoRole);
    this->fAboutGameAction->setEnabled(false);
    menu->addAction(this->fAboutGameAction);
    connect(this->fAboutGameAction, SIGNAL(triggered()), this, SLOT(fShowAboutGame()));
    this->fGameInfoAction = new QAction(tr("View Metadata"), this);
#if QT_VERSION >= 0x040600
    this->fGameInfoAction->setIcon(QIcon::fromTheme(QString::fromLatin1("document-properties")));
#endif
    menu->addAction(this->fGameInfoAction);
    this->fGameInfoAction->setEnabled(false);
    connect(this->fGameInfoAction, SIGNAL(triggered()), this, SLOT(fShowGameInfoDialog()));
    menu->addSeparator();
    act = new QAction(tr("&Quit QTads"), this);
    act->setMenuRole(QAction::QuitRole);
#if QT_VERSION >= 0x040600
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("application-exit")));
    act->setShortcuts(QKeySequence::Quit);
#endif
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(close()));

    // "Edit" menu.
    menu = menuBar->addMenu(tr("&Edit"));
    this->fCopyAction = new QAction(tr("&Copy"), this);
#if QT_VERSION >= 0x040600
    this->fCopyAction->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-copy")));
    this->fCopyAction->setShortcuts(QKeySequence::Copy);
#endif
    this->fCopyAction->setEnabled(false);
    menu->addAction(this->fCopyAction);
    connect(this->fCopyAction, SIGNAL(triggered()), SLOT(copyToClipboard()));
    this->fPasteAction = new QAction(tr("&Paste"), this);
#if QT_VERSION >= 0x040600
    this->fPasteAction->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-paste")));
    this->fPasteAction->setShortcuts(QKeySequence::Paste);
#endif
    this->fPasteAction->setDisabled(true);
    menu->addAction(this->fPasteAction);
    connect(this->fPasteAction, SIGNAL(triggered()), SLOT(pasteFromClipboard()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(updatePasteAction()));
    menu->addSeparator();
    act = new QAction(tr("&Preferences..."), this);
#if QT_VERSION >= 0x040600
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("configure")));
    act->setShortcuts(QKeySequence::Preferences);
#endif
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(fShowConfDialog()));

    // "Help" menu.
    menu = menuBar->addMenu(tr("&Help"));
    this->fAboutQtadsAction = new QAction(tr("A&bout QTads"), this);
#if QT_VERSION >= 0x040600
    this->fAboutQtadsAction->setIcon(QIcon::fromTheme(QString::fromLatin1("help-about")));
#endif
    menu->addAction(this->fAboutQtadsAction);
    connect(this->fAboutQtadsAction, SIGNAL(triggered()), this, SLOT(fShowAboutQtads()));
    act = new QAction(tr("&Check for Updates"), this);
    act->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(fCheckForUpdates()));

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
    if (not qFrame->settings()->confirmQuitGame or not qFrame->gameRunning()) {
        return true;
    }

    QMessageBox* msgBox = new QMessageBox(QMessageBox::Question,
                                          tr("Quit Current Game") + QString::fromLatin1(" - ")
                                          + qFrame->applicationName(),
                                          tr("If you didn't save the current game, all progress will"
                                             " be lost. Do you wish to quit the game?"),
                                          QMessageBox::Yes | QMessageBox::Cancel, this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setDefaultButton(QMessageBox::Cancel);
#ifdef Q_OS_MAC
    msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
    // This presents the dialog as a sheet in OS X.
    msgBox->setWindowModality(Qt::WindowModal);
#endif

    if (msgBox->exec() == QMessageBox::Yes) {
        return true;
    }
    return false;
}


bool
CHtmlSysWinGroupQt::fAskRestartGameDialog()
{
    if (not qFrame->settings()->confirmRestartGame or not qFrame->gameRunning()) {
        return true;
    }

    QMessageBox* msgBox = new QMessageBox(QMessageBox::Question,
                                          tr("Restart Current Game") + QString::fromLatin1(" - ")
                                          + qFrame->applicationName(),
                                          tr("If you didn't save the current game, all progress will be lost."
                                             " Do you wish to restart the game?"),
                                          QMessageBox::Yes | QMessageBox::Cancel, this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setDefaultButton(QMessageBox::Cancel);
#ifdef Q_OS_MAC
    msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
    // This presents the dialog as a sheet in OS X.
    msgBox->setWindowModality(Qt::WindowModal);
#endif

    if (msgBox->exec() == QMessageBox::Yes) {
        return true;
    }
    return false;
}


static QNetworkReply*
sendNetRequest( QNetworkAccessManager* nam, const QUrl& url )
{
    QByteArray userAgent = "QTads/";
    userAgent += QByteArray(QTADS_VERSION).replace(' ', '_');
    userAgent += " Qt/";
    userAgent += qVersion();

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", userAgent);
    req.setRawHeader("Connection", "close");

    return nam->get(req);
}


void
CHtmlSysWinGroupQt::fCheckForUpdates()
{
    // If there's already an update check in progress, don't do anything.
    if (this->fNetManager != 0) {
        return;
    }

    this->fNetManager = new QNetworkAccessManager(this);
    connect(this->fNetManager, SIGNAL(finished(QNetworkReply*)), SLOT(fReplyFinished(QNetworkReply*)));

    this->fHttpRedirectCount = 0;
    this->fReply = sendNetRequest(this->fNetManager,
                                  QUrl(QString::fromLatin1("http://qtads.sourceforge.net/currentversion")));
}


static void
showUpdateErrorMsg( const QString& detailedText )
{
    QMessageBox* msg = new QMessageBox(
                QMessageBox::Critical, QObject::tr("Check for Updates - Error"),
                QObject::tr("It was not possible to retrieve update information. Please try again later,"
                            " as the problem might be temporary. If the problem persists, feel free to"
                            " contact the author.")
    );
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setDetailedText(detailedText);
    msg->show();
}


void
CHtmlSysWinGroupQt::fReplyFinished( QNetworkReply* reply )
{
    if (reply->error() != QNetworkReply::NoError) {
        showUpdateErrorMsg(this->fReply->errorString());
        this->fNetManager->deleteLater();
        this->fReply->deleteLater();
        this->fNetManager = 0;
        return;
    }

    // If we get an HTTP redirect, retry the request with the new URL.
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302) {
        ++this->fHttpRedirectCount;
        // If we got more than 5 redirects by now, something's wrong. Abort.
        if (this->fHttpRedirectCount > 5) {
            showUpdateErrorMsg(tr("Too many HTTP redirects"));
            this->fNetManager->deleteLater();
            this->fReply->deleteLater();
            this->fNetManager = 0;
            return;
        }
        QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (newUrl.isRelative()) {
            newUrl = reply->url().resolved(newUrl);
        }
        reply->deleteLater();
        this->fReply = sendNetRequest(this->fNetManager, newUrl);
        return;
    }

    // If we get here, then anything else than an HTTP 200 status is an error.
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        QString errMsg = tr("Expected HTTP status code 200, got:\n");
        errMsg += reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        QString httpReason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if (not httpReason.isEmpty()) {
            errMsg += QString::fromLatin1(" (") + httpReason + QString::fromLatin1(")");
        }
        showUpdateErrorMsg(errMsg);
        this->fNetManager->deleteLater();
        this->fReply->deleteLater();
        this->fNetManager = 0;
        return;
    }

    // Convert current version to hex.
    QString str(QString::fromLatin1(QTADS_VERSION));
    // If this is a git snapshot, strip the " git".
    if (str.endsWith(QString::fromLatin1(" git"), Qt::CaseInsensitive)) {
        str.chop(4);
    }
    QStringList strList = str.split(QChar::fromLatin1('.'));
    int curVersion = QT_VERSION_CHECK(strList.at(0).toInt(), strList.at(1).toInt(), strList.at(2).toInt());

    // Do the same with the retrieved version.
    str = QString::fromUtf8(reply->readLine(10));
    int newVersion = 0;
    if (str.length() > 3) {
        // Chop the newline at the end, if there is one.
        if (str.endsWith(QChar::fromLatin1('\n'))) {
            str.chop(1);
        }
        strList = str.split(QChar::fromLatin1('.'));
        newVersion = QT_VERSION_CHECK(strList.at(0).toInt(), strList.at(1).toInt(), strList.at(2).toInt());
    }
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setTextFormat(Qt::RichText);
    msgBox->setWindowTitle(tr("Check for Updates"));
    if (newVersion > curVersion) {
        // There's a new version available.  Retrieve the rest of the remote
        // file.  For security, provide a sane upper limit of characters to
        // read.
        QString text;
        while (reply->canReadLine() and text.length() < 2500) {
            text.append(QString::fromUtf8(reply->readLine(100)));
        }
        // Remove that last newline.
        if (text.endsWith(QString::fromLatin1("\n"))) {
            text.truncate(text.length() - 1);
        }
        if (text.length() > 2) {
            msgBox->setDetailedText(text);
        }
#ifdef Q_OS_MAC
        msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
#else
        msgBox->setIcon(QMessageBox::Question);
#endif
        msgBox->setText(tr("A newer version of QTads is available. Do you want to visit the download page?"));
        msgBox->setInformativeText(tr("Note that this is only a check for new versions. Nothing will be downloaded"
                                      " or installed automatically."));
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        if (msgBox->exec() == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl(QString::fromLatin1("http://qtads.sourceforge.net/downloads.shtml")));
        }
    } else if (not this->fSilentIfNoUpdates) {
#ifdef Q_OS_MAC
        msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
#else
        msgBox->setIcon(QMessageBox::Information);
#endif
        msgBox->setText(tr("This version of QTads is up to date."));
        msgBox->exec();
    }
    this->fSilentIfNoUpdates = false;
    this->fNetManager->deleteLater();
    this->fReply->deleteLater();
    this->fNetManager = 0;
    qFrame->settings()->lastUpdateDate = QDate::currentDate();
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
    this->fGameInfoDialog = new GameInfoDialog(qStrToFname(qFrame->gameFile()), this);
    this->fGameInfoDialog->setWindowTitle(tr("Game Information"));
    connect(fGameInfoDialog, SIGNAL(finished(int)), SLOT(fHideGameInfoDialog()));
    connect(fGameInfoDialog, SIGNAL(finished(int)), SLOT(fActivateWindow()));
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
#ifdef Q_OS_MAC
    // There's a bug in Qt for OS X that results in a visual glitch with
    // QFontComboBox widgets inside QFormLayouts.  Making the dialog 4 pixels
    // higher fixes it.
    //
    // See: http://bugreports.qt.nokia.com/browse/QTBUG-10460
    this->fConfDialog->layout()->activate();
    this->fConfDialog->setMinimumHeight(this->fConfDialog->minimumHeight() + 4);
#endif
    connect(fConfDialog, SIGNAL(finished(int)), SLOT(fActivateWindow()));
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
    if (this->fAboutBox == 0) {
        return;
    }
    if (this->fAboutBoxDialog != 0 and this->fAboutBoxDialog->isVisible()) {
        this->fAboutBoxDialog->activateWindow();
        this->fAboutBoxDialog->raise();
        return;
    }
    if (this->fAboutBoxDialog == 0) {
        this->fAboutBoxDialog = new QDialog(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
        this->fAboutBoxDialog->setWindowTitle(tr("About This Game"));
        this->fAboutBox->setParent(this->fAboutBoxDialog);
        QVBoxLayout* layout = new QVBoxLayout(this->fAboutBoxDialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(this->fAboutBox);
        connect(this->fAboutBoxDialog, SIGNAL(finished(int)), SLOT(fHideAboutGame()));
    }
    this->fAboutBoxDialog->resize(this->fAboutBox->size());
    connect(fAboutBoxDialog, SIGNAL(finished(int)), SLOT(fActivateWindow()));
    this->fAboutBoxDialog->show();
}


void
CHtmlSysWinGroupQt::fHideAboutGame()
{
    // Destroy the dialog, but not the about box HTML banner.  We reparent
    // the banner so it won't be automatically deleted by its parent.
    this->fAboutBox->setParent(0);
    this->fAboutBoxDialog->hide();
    this->fAboutBoxDialog->deleteLater();
    this->fAboutBoxDialog = 0;
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
#ifdef Q_OS_MAC
    // Similar bug to the config dialog one.  Again, 4 pixels higher fixes it.
    this->fAboutQtadsDialog->layout()->activate();
    this->fAboutQtadsDialog->setMinimumHeight(this->fAboutQtadsDialog->minimumHeight() + 4);
#endif
    connect(fAboutQtadsDialog, SIGNAL(finished(int)), SLOT(fActivateWindow()));
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
    const QString& fname = QFileDialog::getOpenFileName(0, tr("Choose the TADS game you wish to run"),
                                                        qFrame->settings()->lastFileOpenDir,
                                                        tr("TADS Games")
                                                        + QString::fromLatin1(" (*.gam *.Gam *.GAM *.t3 *.T3)"));
    this->activateWindow();
    if (not fname.isEmpty()) {
        qFrame->settings()->lastFileOpenDir = QFileInfo(fname).absolutePath();
        qFrame->setNextGame(fname);
    }
}


void
CHtmlSysWinGroupQt::fRecentGameTriggered( QAction* action )
{
    if (this->fAskQuitGameDialog()) {
        qFrame->setNextGame(action->statusTip());
    }
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
    this->fGameInfoAction->setEnabled(GameInfoDialog::gameHasMetaInfo(qStrToFname(qFrame->gameFile())));
    this->fRestartCurrentGameAction->setEnabled(true);
    this->fEndCurrentGameAction->setEnabled(true);
}


void
CHtmlSysWinGroupQt::closeEvent( QCloseEvent* e )
{
    if (not qFrame->gameRunning()) {
        return;
    }

    QMessageBox* msgBox = new QMessageBox(QMessageBox::Question,
                                          tr("Quit QTads"),
                                          tr("A game is currently running. Abandon the game and quit the interpreter?"),
                                          QMessageBox::Yes | QMessageBox::Cancel, this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setDefaultButton(QMessageBox::Cancel);
#ifdef Q_OS_MAC
    msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
    // This presents the dialog as a sheet in OS X.
    msgBox->setWindowModality(Qt::WindowModal);
#endif

    if (not qFrame->settings()->confirmQuitGame or msgBox->exec() == QMessageBox::Yes) {
        qFrame->setGameRunning(false);
        // Make sure the VM knows that we're closing.
        this->fWantsToQuit = true;
        e->accept();
    } else {
        e->ignore();
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
    e->acceptProposedAction();
    this->fGameFileFromDropEvent = e->mimeData()->urls().at(0).toLocalFile();
    QTimer::singleShot(100, this, SLOT(fRunDropEventFile()));
}


void
CHtmlSysWinGroupQt::copyToClipboard()
{
    const QString& selectedText = DisplayWidget::selectedText();
    if (not selectedText.isEmpty()) {
        QApplication::clipboard()->setText(selectedText);
    }
}


void
CHtmlSysWinGroupQt::pasteFromClipboard()
{
    qFrame->gameWindow()->insertText(QApplication::clipboard()->text());
}


void
CHtmlSysWinGroupQt::fRunDropEventFile()
{
    if (this->fAskQuitGameDialog()) {
        qFrame->setNextGame(this->fGameFileFromDropEvent);
    }
}


void
CHtmlSysWinGroupQt::updatePasteAction()
{
    this->fPasteAction->setDisabled(QApplication::clipboard()->text().isEmpty()
                                    or not qFrame->gameWindow());
}


CHtmlSysWinAboutBoxQt*
CHtmlSysWinGroupQt::createAboutBox( class CHtmlFormatter* formatter )
{
    // If there's already an "about" box, destroy it first.
    if (this->fAboutBoxDialog != 0) {
        this->fHideAboutGame();
    } else {
        this->fAboutGameAction->setEnabled(true);
    }

    // We will reparent the banner when we show the actual dialog.
    this->fAboutBox = new CHtmlSysWinAboutBoxQt(formatter, 0);
    // Only set the width to something comfortable.  The height will be
    // calculated later when set_banner_size() is called on the about box.
    this->fAboutBox->resize(500, 0);
    return this->fAboutBox;
}


void
CHtmlSysWinGroupQt::deleteAboutBox()
{
    if (this->fAboutBox == 0) {
        return;
    }
    if (this->fAboutBoxDialog != 0) {
        this->fHideAboutGame();
    }
    delete this->fAboutBox;
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
    const QStringList& list = qFrame->settings()->recentGamesList;
    for (int i = 0; i < list.size(); ++i) {
        QString gameName = GameInfoDialog::getMetaInfo(qStrToFname(list.at(i))).plainGameName;
        if (gameName.isEmpty()) {
            gameName = QFileInfo(list.at(i)).fileName();
        }
        gameName = gameName.replace(QString::fromLatin1("&"), QString::fromLatin1("&&"));
        QAction* act = this->fRecentGamesMenu->addAction(gameName);
        // Elide the text in case it's too long.
        act->setText(QFontMetrics(act->font()).elidedText(gameName, Qt::ElideRight, 300));
        act->setStatusTip(QString(list.at(i)));
    }
}

void
CHtmlSysWinGroupQt::checkForUpdates()
{
    this->fSilentIfNoUpdates = true;
    this->fCheckForUpdates();
}


void
CHtmlSysWinGroupQt::enableCopyAction( bool f )
{
    this->fCopyAction->setEnabled(f);
}


#ifdef Q_OS_MAC
bool
CHtmlSysWinGroupQt::handleFileOpenEvent( class QFileOpenEvent* e )
{
    if (e->file().isEmpty()) {
        e->ignore();
        return false;
    }
    e->accept();
    this->fGameFileFromDropEvent = e->file();
    QTimer::singleShot(100, this, SLOT(fRunDropEventFile()));
    return true;
}
#endif


oshtml_charset_id_t
CHtmlSysWinGroupQt::get_default_win_charset() const
{
    //qDebug() << Q_FUNC_INFO << "called";

    return 0;
}


size_t
CHtmlSysWinGroupQt::xlat_html4_entity( textchar_t* result, size_t result_size, unsigned int charval,
                                       oshtml_charset_id_t*, int* changed_charset )
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
