// This is copyrighted software. More information is at the end of this file.
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QScreen>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <Qt>

#include "aboutqtadsdialog.h"
#include "confdialog.h"
#include "dispwidget.h"
#include "gameinfodialog.h"
#include "settings.h"
#include "syswinaboutbox.h"
#include "syswininput.h"

void QTadsFrame::resizeEvent(QResizeEvent*)
{
    qFrame->reformatBanners(true, true, false);
}

void QTadsFrame::dragEnterEvent(QDragEnterEvent* e)
{
    CHtmlSysWinInputQt* gameWindow = qFrame->gameWindow();
    // Only accept text. URLs are handled by the main window instead.
    if (e->mimeData()->hasText() and not e->mimeData()->hasUrls()
        and (gameWindow and gameWindow->acceptsText()))
    {
        e->acceptProposedAction();
    }
}

void QTadsFrame::dropEvent(QDropEvent* e)
{
    qFrame->gameWindow()->insertText(e->mimeData()->text());
}

CHtmlSysWinGroupQt::CHtmlSysWinGroupQt()
    : fConfDialog(nullptr)
    , fGameInfoDialog(nullptr)
    , fAboutBoxDialog(nullptr)
    , fAboutBox(nullptr)
    , fAboutQtadsDialog(nullptr)
    , fNetManager(nullptr)
    , fHttpRedirectCount(0)
    , fWantsToQuit(false)
    , fSilentIfNoUpdates(false)
{
    // qDebug() << Q_FUNC_INFO << "called";
    Q_ASSERT(qWinGroup == nullptr);

    // Make sure we can drag&drop (files in our case) into the main window.
    setAcceptDrops(true);

    // We make our menu bar parentless so it will be shared by all our windows
    // in Mac OS X.
    QMenuBar* menuBar = new QMenuBar(nullptr);

    // "Game" menu.
    QMenu* menu = menuBar->addMenu(tr("&Game"));
    QAction* act = new QAction(tr("&Open") + QString::fromLatin1("..."), this);
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("document-open")));
    act->setShortcuts(QKeySequence::Open);
    menu->addAction(act);
    connect(act, &QAction::triggered, this, &CHtmlSysWinGroupQt::fOpenNewGame);
    act = new QAction(tr("Open &Recent"), this);
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("document-open-recent")));
    fRecentGamesMenu = new QMenu(this);
    act->setMenu(fRecentGamesMenu);
    menu->addAction(act);
    connect(fRecentGamesMenu, &QMenu::triggered, this, &CHtmlSysWinGroupQt::fRecentGameTriggered);
    fRestartCurrentGameAction = new QAction(tr("Re&start"), this);
    fRestartCurrentGameAction->setIcon(QIcon::fromTheme(QString::fromLatin1("view-refresh")));
    fRestartCurrentGameAction->setShortcut(QKeySequence(QString::fromLatin1("Ctrl+R")));
    menu->addAction(fRestartCurrentGameAction);
    fRestartCurrentGameAction->setEnabled(false);
    connect(
        fRestartCurrentGameAction, &QAction::triggered, this,
        &CHtmlSysWinGroupQt::fRestartCurrentGame);
    fEndCurrentGameAction = new QAction(tr("Qui&t"), this);
    fEndCurrentGameAction->setMenuRole(QAction::NoRole);
    fEndCurrentGameAction->setIcon(QIcon::fromTheme(QString::fromLatin1("process-stop")));
    fEndCurrentGameAction->setShortcuts(QKeySequence::Close);
    menu->addAction(fEndCurrentGameAction);
    fEndCurrentGameAction->setEnabled(false);
    connect(fEndCurrentGameAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::fEndCurrentGame);
    menu->addSeparator();
    fAboutGameAction = new QAction(tr("&About This Game"), this);
    fAboutGameAction->setMenuRole(QAction::NoRole);
    fAboutGameAction->setEnabled(false);
    menu->addAction(fAboutGameAction);
    connect(fAboutGameAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::fShowAboutGame);
    fGameInfoAction = new QAction(tr("View Metadata"), this);
    fGameInfoAction->setIcon(QIcon::fromTheme(QString::fromLatin1("document-properties")));
    menu->addAction(fGameInfoAction);
    fGameInfoAction->setEnabled(false);
    connect(fGameInfoAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::fShowGameInfoDialog);
    menu->addSeparator();
    act = new QAction(tr("&Quit QTads"), this);
    act->setMenuRole(QAction::QuitRole);
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("application-exit")));
    act->setShortcuts(QKeySequence::Quit);
    menu->addAction(act);
    connect(act, &QAction::triggered, this, &QWidget::close);

    // "Edit" menu.
    menu = menuBar->addMenu(tr("&Edit"));
    fCopyAction = new QAction(tr("&Copy"), this);
    fCopyAction->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-copy")));
    fCopyAction->setShortcuts(QKeySequence::Copy);
    fCopyAction->setEnabled(false);
    menu->addAction(fCopyAction);
    connect(fCopyAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::copyToClipboard);
    fPasteAction = new QAction(tr("&Paste"), this);
    fPasteAction->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-paste")));
    fPasteAction->setShortcuts(QKeySequence::Paste);
    fPasteAction->setDisabled(true);
    menu->addAction(fPasteAction);
    connect(fPasteAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::pasteFromClipboard);
    connect(
        QApplication::clipboard(), &QClipboard::dataChanged, this,
        &CHtmlSysWinGroupQt::updatePasteAction);
    menu->addSeparator();
    act = new QAction(tr("&Preferences..."), this);
    act->setIcon(QIcon::fromTheme(QString::fromLatin1("configure")));
    act->setShortcuts(QKeySequence::Preferences);
    menu->addAction(act);
    connect(act, &QAction::triggered, this, &CHtmlSysWinGroupQt::fShowConfDialog);

    // "Help" menu.
    menu = menuBar->addMenu(tr("&Help"));
    fAboutQtadsAction = new QAction(tr("A&bout QTads"), this);
    fAboutQtadsAction->setIcon(QIcon::fromTheme(QString::fromLatin1("help-about")));
    menu->addAction(fAboutQtadsAction);
    connect(fAboutQtadsAction, &QAction::triggered, this, &CHtmlSysWinGroupQt::fShowAboutQtads);
    act = new QAction(tr("&Check for Updates"), this);
    act->setMenuRole(QAction::ApplicationSpecificRole);
    menu->addAction(act);
    connect(act, &QAction::triggered, this, &CHtmlSysWinGroupQt::fCheckForUpdates);

    setMenuBar(menuBar);

    // Create a default status bar.
    statusBar();

    // Set up our central widget.
    fFrame = new QTadsFrame(this);
    fFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    fFrame->setLineWidth(0);
    fFrame->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(fFrame);

    // Use a sane minimum size; by default Qt would allow us to be resized
    // to almost zero.
    setMinimumSize(QApplication::primaryScreen()->availableSize() / 3);

    // Receive notification when a game is about to quit/start so we can
    // enable/disable related actions.
    connect(qFrame, &CHtmlSysFrameQt::gameQuitting, this, &CHtmlSysWinGroupQt::fNotifyGameQuitting);
    connect(qFrame, &CHtmlSysFrameQt::gameHasQuit, this, &CHtmlSysWinGroupQt::fNotifyGameQuitting);
    connect(qFrame, &CHtmlSysFrameQt::gameStarting, this, &CHtmlSysWinGroupQt::fNotifyGameStarting);

    qWinGroup = this;
}

CHtmlSysWinGroupQt::~CHtmlSysWinGroupQt()
{
    Q_ASSERT(qWinGroup != nullptr);
    qWinGroup = nullptr;
}

auto CHtmlSysWinGroupQt::fAskQuitGameDialog() -> bool
{
    if (not qFrame->settings()->confirmQuitGame or not qFrame->gameRunning()) {
        return true;
    }

    QMessageBox* msgBox = new QMessageBox(
        QMessageBox::Question,
        tr("Quit Current Game") + QString::fromLatin1(" - ") + qFrame->applicationName(),
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

auto CHtmlSysWinGroupQt::fAskRestartGameDialog() -> bool
{
    if (not qFrame->settings()->confirmRestartGame or not qFrame->gameRunning()) {
        return true;
    }

    QMessageBox* msgBox = new QMessageBox(
        QMessageBox::Question,
        tr("Restart Current Game") + QString::fromLatin1(" - ") + qFrame->applicationName(),
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

static auto sendNetRequest(QNetworkAccessManager* nam, const QUrl& url) -> QNetworkReply*
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

void CHtmlSysWinGroupQt::fCheckForUpdates()
{
    // If there's already an update check in progress, don't do anything.
    if (fNetManager != nullptr) {
        return;
    }

    fNetManager = new QNetworkAccessManager(this);
    connect(
        fNetManager, &QNetworkAccessManager::finished, this, &CHtmlSysWinGroupQt::fReplyFinished);

    fHttpRedirectCount = 0;
    fReply = sendNetRequest(
        fNetManager, QUrl(QString::fromLatin1("https://realnc.github.io/qtads/currentversion")));
}

static void showUpdateErrorMsg(const QString& detailedText)
{
    QMessageBox* msg = new QMessageBox(
        QMessageBox::Critical, QObject::tr("Check for Updates - Error"),
        QObject::tr("It was not possible to retrieve update information. Please try again later,"
                    " as the problem might be temporary. If the problem persists, feel free to"
                    " contact the author."));
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setDetailedText(detailedText);
    msg->show();
}

void CHtmlSysWinGroupQt::fReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showUpdateErrorMsg(fReply->errorString());
        fNetManager->deleteLater();
        fReply->deleteLater();
        fNetManager = nullptr;
        return;
    }

    // If we get an HTTP redirect, retry the request with the new URL.
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302) {
        ++fHttpRedirectCount;
        // If we got more than 5 redirects by now, something's wrong. Abort.
        if (fHttpRedirectCount > 5) {
            showUpdateErrorMsg(tr("Too many HTTP redirects"));
            fNetManager->deleteLater();
            fReply->deleteLater();
            fNetManager = nullptr;
            return;
        }
        QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (newUrl.isRelative()) {
            newUrl = reply->url().resolved(newUrl);
        }
        reply->deleteLater();
        fReply = sendNetRequest(fNetManager, newUrl);
        return;
    }

    // If we get here, then anything else than an HTTP 200 status is an error.
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        QString errMsg = tr("Expected HTTP status code 200, got:\n");
        errMsg += reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        QString httpReason =
            reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if (not httpReason.isEmpty()) {
            errMsg += QString::fromLatin1(" (") + httpReason + QString::fromLatin1(")");
        }
        showUpdateErrorMsg(errMsg);
        fNetManager->deleteLater();
        fReply->deleteLater();
        fNetManager = nullptr;
        return;
    }

    // Convert current version to hex.
    QString str(QString::fromLatin1(QTADS_VERSION));
    // If this is a git snapshot, strip the " git".
    if (str.endsWith(QString::fromLatin1(" git"), Qt::CaseInsensitive)) {
        str.chop(4);
    }
    QStringList strList = str.split(QChar::fromLatin1('.'));
    int curVersion =
        QT_VERSION_CHECK(strList.at(0).toInt(), strList.at(1).toInt(), strList.at(2).toInt());

    // Do the same with the retrieved version.
    str = QString::fromUtf8(reply->readLine(10));
    int newVersion = 0;
    if (str.length() > 3) {
        // Chop the newline at the end, if there is one.
        if (str.endsWith(QChar::fromLatin1('\n'))) {
            str.chop(1);
        }
        strList = str.split(QChar::fromLatin1('.'));
        newVersion =
            QT_VERSION_CHECK(strList.at(0).toInt(), strList.at(1).toInt(), strList.at(2).toInt());
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
        msgBox->setText(
            tr("A newer version of QTads is available. Do you want to visit the download page?"));
        msgBox->setInformativeText(
            tr("Note that this is only a check for new versions. Nothing will be downloaded"
               " or installed automatically."));
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        if (msgBox->exec() == QMessageBox::Yes) {
            QDesktopServices::openUrl(
                QUrl(QString::fromLatin1("https://realnc.github.io/qtads/#downloads")));
        }
    } else if (not fSilentIfNoUpdates) {
#ifdef Q_OS_MAC
        msgBox->setIconPixmap(QPixmap(QString::fromLatin1(":/qtads_72x72.png")));
#else
        msgBox->setIcon(QMessageBox::Information);
#endif
        msgBox->setText(tr("This version of QTads is up to date."));
        msgBox->exec();
    }
    fSilentIfNoUpdates = false;
    fNetManager->deleteLater();
    fReply->deleteLater();
    fNetManager = nullptr;
    qFrame->settings()->lastUpdateDate = QDate::currentDate();
}

void CHtmlSysWinGroupQt::fShowGameInfoDialog()
{
    // If the dialog is already open, simply activate and raise it.
    if (fGameInfoDialog != nullptr) {
        fGameInfoDialog->activateWindow();
        fGameInfoDialog->raise();
        return;
    }
    fGameInfoDialog = new GameInfoDialog(qStrToFname(qFrame->gameFile()), this);
    fGameInfoDialog->setWindowTitle(tr("Game Information"));
    connect(fGameInfoDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fHideGameInfoDialog);
    connect(fGameInfoDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fActivateWindow);
    fGameInfoDialog->show();
}

void CHtmlSysWinGroupQt::fHideGameInfoDialog()
{
    if (fGameInfoDialog != nullptr) {
        fGameInfoDialog->deleteLater();
        fGameInfoDialog = nullptr;
    }
}

void CHtmlSysWinGroupQt::fShowConfDialog()
{
    // If the dialog is already open, simply activate and raise it.
    if (fConfDialog != nullptr) {
        fConfDialog->activateWindow();
        fConfDialog->raise();
        return;
    }
    fConfDialog = new ConfDialog(this);
    fConfDialog->setWindowTitle(tr("QTads Preferences"));
    connect(fConfDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fHideConfDialog);
    connect(fConfDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fActivateWindow);
    fConfDialog->show();
}

void CHtmlSysWinGroupQt::fHideConfDialog()
{
    if (fConfDialog != nullptr) {
        fConfDialog->deleteLater();
        fConfDialog = nullptr;
    }
}

void CHtmlSysWinGroupQt::fShowAboutGame()
{
    if (fAboutBox == nullptr) {
        return;
    }
    if (fAboutBoxDialog != nullptr and fAboutBoxDialog->isVisible()) {
        fAboutBoxDialog->activateWindow();
        fAboutBoxDialog->raise();
        return;
    }
    if (fAboutBoxDialog == nullptr) {
        fAboutBoxDialog = new QDialog(
            this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        fAboutBoxDialog->setWindowTitle(tr("About This Game"));
        fAboutBox->setParent(fAboutBoxDialog);
        QVBoxLayout* layout = new QVBoxLayout(fAboutBoxDialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(fAboutBox);
        connect(fAboutBoxDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fHideAboutGame);
    }
    fAboutBoxDialog->resize(fAboutBox->size());
    connect(fAboutBoxDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fActivateWindow);
    fAboutBoxDialog->show();
}

void CHtmlSysWinGroupQt::fHideAboutGame()
{
    // Destroy the dialog, but not the about box HTML banner.  We reparent
    // the banner so it won't be automatically deleted by its parent.
    fAboutBox->setParent(nullptr);
    fAboutBoxDialog->hide();
    fAboutBoxDialog->deleteLater();
    fAboutBoxDialog = nullptr;
}

void CHtmlSysWinGroupQt::fShowAboutQtads()
{
    // If the dialog is already open, simply activate and raise it.
    if (fAboutQtadsDialog != nullptr) {
        fAboutQtadsDialog->activateWindow();
        fAboutQtadsDialog->raise();
        return;
    }

    fAboutQtadsDialog = new AboutQtadsDialog(this);
    connect(fAboutQtadsDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fHideAboutQtads);
#ifdef Q_OS_MAC
    // Similar bug to the config dialog one.  Again, 4 pixels higher fixes it.
    fAboutQtadsDialog->layout()->activate();
    fAboutQtadsDialog->setMinimumHeight(fAboutQtadsDialog->minimumHeight() + 4);
#endif
    connect(fAboutQtadsDialog, &QDialog::finished, this, &CHtmlSysWinGroupQt::fActivateWindow);
    fAboutQtadsDialog->show();
}

void CHtmlSysWinGroupQt::fHideAboutQtads()
{
    if (fAboutQtadsDialog != nullptr) {
        fAboutQtadsDialog->deleteLater();
        fAboutQtadsDialog = nullptr;
    }
}

void CHtmlSysWinGroupQt::fOpenNewGame()
{
    QString fname = QFileDialog::getOpenFileName(
        nullptr, tr("Choose the TADS game you wish to run"), qFrame->settings()->lastFileOpenDir,
        tr("TADS Games") + QString::fromLatin1(" (*.gam *.Gam *.GAM *.t3 *.T3)"));
    activateWindow();
    if (not fname.isEmpty()) {
        qFrame->settings()->lastFileOpenDir = QFileInfo(fname).absolutePath();
        qFrame->setNextGame(std::move(fname));
    }
}

void CHtmlSysWinGroupQt::fRecentGameTriggered(QAction* action)
{
    if (fAskQuitGameDialog()) {
        qFrame->setNextGame(action->statusTip());
    }
}

void CHtmlSysWinGroupQt::fEndCurrentGame()
{
    if (fAskQuitGameDialog()) {
        qFrame->setGameRunning(false);
    }
}

void CHtmlSysWinGroupQt::fRestartCurrentGame()
{
    if (fAskRestartGameDialog()) {
        qFrame->setNextGame(qFrame->gameFile());
    }
}

void CHtmlSysWinGroupQt::fNotifyGameQuitting()
{
    fGameInfoAction->setEnabled(false);
    fRestartCurrentGameAction->setEnabled(false);
    fEndCurrentGameAction->setEnabled(false);
}

void CHtmlSysWinGroupQt::fNotifyGameStarting()
{
    fHideGameInfoDialog();
    fGameInfoAction->setEnabled(GameInfoDialog::gameHasMetaInfo(qStrToFname(qFrame->gameFile())));
    fRestartCurrentGameAction->setEnabled(true);
    fEndCurrentGameAction->setEnabled(true);
}

void CHtmlSysWinGroupQt::closeEvent(QCloseEvent* e)
{
    if (not qFrame->gameRunning()) {
        return;
    }

    QMessageBox* msgBox = new QMessageBox(
        QMessageBox::Question, tr("Quit QTads"),
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
        fWantsToQuit = true;
        e->accept();
    } else {
        e->ignore();
    }
}

void CHtmlSysWinGroupQt::dragEnterEvent(QDragEnterEvent* e)
{
    // Only accept the event if there is exactly one URL which points to a
    // local file.
    if (e->mimeData()->hasUrls() and e->mimeData()->urls().size() == 1
        and not e->mimeData()->urls().at(0).toLocalFile().isEmpty())
    {
        e->acceptProposedAction();
    }
}

void CHtmlSysWinGroupQt::dropEvent(QDropEvent* e)
{
    e->acceptProposedAction();
    fGameFileFromDropEvent = e->mimeData()->urls().at(0).toLocalFile();
    QTimer::singleShot(100, this, SLOT(fRunDropEventFile()));
}

void CHtmlSysWinGroupQt::copyToClipboard()
{
    const QString& selectedText = DisplayWidget::selectedText();
    if (not selectedText.isEmpty()) {
        QApplication::clipboard()->setText(selectedText);
    }
}

void CHtmlSysWinGroupQt::pasteFromClipboard()
{
    qFrame->gameWindow()->insertText(QApplication::clipboard()->text());
}

void CHtmlSysWinGroupQt::fRunDropEventFile()
{
    if (fAskQuitGameDialog()) {
        qFrame->setNextGame(fGameFileFromDropEvent);
    }
}

void CHtmlSysWinGroupQt::updatePasteAction()
{
    fPasteAction->setDisabled(
        QApplication::clipboard()->text().isEmpty() or not qFrame->gameWindow());
}

auto CHtmlSysWinGroupQt::createAboutBox(class CHtmlFormatter* formatter) -> CHtmlSysWinAboutBoxQt*
{
    // If there's already an "about" box, destroy it first.
    if (fAboutBoxDialog != nullptr) {
        fHideAboutGame();
    } else {
        fAboutGameAction->setEnabled(true);
    }

    // We will reparent the banner when we show the actual dialog.
    fAboutBox = new CHtmlSysWinAboutBoxQt(formatter, nullptr);
    // Only set the width to something comfortable.  The height will be
    // calculated later when set_banner_size() is called on the about box.
    fAboutBox->resize(500, 0);
    return fAboutBox;
}

void CHtmlSysWinGroupQt::deleteAboutBox()
{
    if (fAboutBox == nullptr) {
        return;
    }
    if (fAboutBoxDialog != nullptr) {
        fHideAboutGame();
    }
    delete fAboutBox;
    fAboutBox = nullptr;
    fAboutGameAction->setEnabled(false);
}

void CHtmlSysWinGroupQt::updateRecentGames()
{
    // We simply clear the menu of all items and re-populate it.
    fRecentGamesMenu->clear();

    // If the list is empty, disable the menu.
    if (qFrame->settings()->recentGamesList.isEmpty()) {
        fRecentGamesMenu->setEnabled(false);
        return;
    }

    // The list is not empty.  If the menu was disabled, enable it.
    if (not fRecentGamesMenu->isEnabled()) {
        fRecentGamesMenu->setEnabled(true);
    }

    // Populate it.
    const QStringList& list = qFrame->settings()->recentGamesList;
    for (int i = 0; i < list.size(); ++i) {
        QString gameName = GameInfoDialog::getMetaInfo(qStrToFname(list.at(i))).plainGameName;
        if (gameName.isEmpty()) {
            gameName = QFileInfo(list.at(i)).fileName();
        }
        gameName = gameName.replace(QString::fromLatin1("&"), QString::fromLatin1("&&"));
        QAction* act = fRecentGamesMenu->addAction(gameName);
        // Elide the text in case it's too long.
        act->setText(QFontMetrics(act->font()).elidedText(gameName, Qt::ElideRight, 300));
        act->setStatusTip(QString(list.at(i)));
    }
}

void CHtmlSysWinGroupQt::checkForUpdates()
{
    fSilentIfNoUpdates = true;
    fCheckForUpdates();
}

void CHtmlSysWinGroupQt::enableCopyAction(bool f)
{
    fCopyAction->setEnabled(f);
}

#ifdef Q_OS_MAC
bool CHtmlSysWinGroupQt::handleFileOpenEvent(class QFileOpenEvent* e)
{
    if (e->file().isEmpty()) {
        e->ignore();
        return false;
    }
    e->accept();
    fGameFileFromDropEvent = e->file();
    QTimer::singleShot(100, this, SLOT(fRunDropEventFile()));
    return true;
}
#endif

auto CHtmlSysWinGroupQt::get_default_win_charset() const -> oshtml_charset_id_t
{
    // qDebug() << Q_FUNC_INFO << "called";

    return 0;
}

auto CHtmlSysWinGroupQt::xlat_html4_entity(
    textchar_t* result, size_t result_size, unsigned int charval, oshtml_charset_id_t*,
    int* changed_charset) -> size_t
{
    // qDebug() << Q_FUNC_INFO << "called";
    Q_ASSERT(result != nullptr);

    // HTML4 entities are Unicode characters, which means the QChar(uint) ctor
    // will do the right thing.
    QString s = QString(QChar(charval));
    strcpy(result, s.toUtf8());
    if (changed_charset != nullptr) {
        *changed_charset = false;
    }
    return s.toUtf8().length();
}

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
