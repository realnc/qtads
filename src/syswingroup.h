// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QMainWindow>
#include <QNetworkReply>
#include <QScrollArea>

#include "config.h"
#include "htmlsys.h"

class QTadsFrame: public QFrame
{
    Q_OBJECT

protected:
    void resizeEvent(QResizeEvent* e) override;

    void dragEnterEvent(QDragEnterEvent* e) override;

    void dropEvent(QDropEvent* e) override;

public:
    QTadsFrame(QWidget* parent)
        : QFrame(parent)
    {
        setAcceptDrops(true);
    }
};

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysWinGroupQt: public QMainWindow, public CHtmlSysWinGroup
{
    Q_OBJECT

private:
    class ConfDialog* fConfDialog;
    class GameInfoDialog* fGameInfoDialog;
    QTadsFrame* fFrame;
    class QDialog* fAboutBoxDialog;
    class CHtmlSysWinAboutBoxQt* fAboutBox;
    class AboutQtadsDialog* fAboutQtadsDialog;
    class QMenu* fRecentGamesMenu;
    class QAction* fAboutGameAction;
    class QAction* fEndCurrentGameAction;
    class QAction* fRestartCurrentGameAction;
    class QAction* fGameInfoAction;
    class QAction* fAboutQtadsAction;
    class QAction* fCopyAction;
    class QAction* fPasteAction;
    class QNetworkAccessManager* fNetManager;
    class QNetworkReply* fReply;
    unsigned int fHttpRedirectCount;
    QString fGameFileFromDropEvent;

    // Are we trying to quit the application?
    bool fWantsToQuit;

    // If this is set, we won't bother the user with a dialog if the
    // update checker finds no available updates.
    bool fSilentIfNoUpdates;

    auto fAskQuitGameDialog() -> bool;

    auto fAskRestartGameDialog() -> bool;

private slots:
    void fCheckForUpdates();

    void fReplyFinished(QNetworkReply* reply);

    void fShowGameInfoDialog();

    void fHideGameInfoDialog();

    void fShowConfDialog();

    void fHideConfDialog();

    void fShowAboutGame();

    void fHideAboutGame();

    void fShowAboutQtads();

    void fHideAboutQtads();

    void fOpenNewGame();

    void fRecentGameTriggered(QAction* action);

    void fEndCurrentGame();

    void fRestartCurrentGame();

    void fNotifyGameQuitting();

    void fNotifyGameStarting();

    void fRunDropEventFile();

    void fActivateWindow()
    {
        activateWindow();
    }

protected:
    void closeEvent(QCloseEvent* e) override;

    void dragEnterEvent(QDragEnterEvent* e) override;

    void dropEvent(QDropEvent* e) override;

public slots:
    // Copy the current selection to the clipboard. No action is performed if
    // there's no selection in any of the game windows.
    void copyToClipboard();

    // Paste the current contents of the clipboard into the input window's
    // editor. Does nothing if the input window is not currently in line input
    // mode.
    void pasteFromClipboard();

    // Enable or disable the paste action according to whether there's text
    // in the clipboard and a game window currently exists.
    void updatePasteAction();

public:
    CHtmlSysWinGroupQt();
    ~CHtmlSysWinGroupQt() override;

    auto createAboutBox(class CHtmlFormatter* formatter) -> CHtmlSysWinAboutBoxQt*;

    void deleteAboutBox();

    auto aboutBox() -> CHtmlSysWinAboutBoxQt*
    {
        return fAboutBox;
    }

    void updateRecentGames();

    void checkForUpdates();

    auto wantsToQuit() const -> bool
    {
        return fWantsToQuit;
    }

    void enableCopyAction(bool f);

#ifdef Q_OS_MAC
    // Handler for FileOpen events.  They only occur in OS X.
    bool handleFileOpenEvent(class QFileOpenEvent* e);
#endif

    //
    // CHtmlSysWinGroup interface implementation.
    //
    auto get_default_win_charset() const -> oshtml_charset_id_t override;

    auto xlat_html4_entity(
        textchar_t* result, size_t result_size, unsigned int charval, oshtml_charset_id_t* charset,
        int* changed_charset) -> size_t override;
};

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
