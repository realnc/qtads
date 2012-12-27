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
#ifndef SYSWINGROUP_H
#define SYSWINGROUP_H

#include <QMainWindow>
#include <QScrollArea>
#include <QNetworkReply>

#include "htmlsys.h"
#include "config.h"


class QTadsFrame: public QFrame {
    Q_OBJECT

  protected:
    void
    resizeEvent( QResizeEvent* e ) override;

  public:
    QTadsFrame( QWidget* parent )
    : QFrame(parent)
    { }
};


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysWinGroupQt: public QMainWindow, public CHtmlSysWinGroup {
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
    class QNetworkAccessManager* fNetManager;
    class QNetworkReply* fReply;
    QString fGameFileFromDropEvent;

    // If this is set, we won't bother the user with a dialog if the
    // update checker finds no available updates.
    bool fSilentIfNoUpdates;

    bool
    fAskQuitGameDialog();

    bool
    fAskRestartGameDialog();

  private slots:
    void
    fCheckForUpdates();

    void
    fReplyFinished( QNetworkReply* reply );

    void
    fErrorOccurred( QNetworkReply::NetworkError code );

    void
    fShowGameInfoDialog();

    void
    fHideGameInfoDialog();

    void
    fShowConfDialog();

    void
    fHideConfDialog();

    void
    fShowAboutGame();

    void
    fHideAboutGame();

    void
    fShowAboutQtads();

    void
    fHideAboutQtads();

    void
    fOpenNewGame();

    void
    fRecentGameTriggered( QAction* action );

    void
    fEndCurrentGame();

    void
    fRestartCurrentGame();

    void
    fNotifyGameQuitting();

    void
    fNotifyGameStarting();

    void
    fRunDropEventFile();

  protected:
    void
    closeEvent( QCloseEvent* e ) override;

    void
    dragEnterEvent( QDragEnterEvent* e ) override;

    void
    dropEvent( QDropEvent* e ) override;

  public:
    CHtmlSysWinGroupQt();
    ~CHtmlSysWinGroupQt() override;

    CHtmlSysWinAboutBoxQt*
    createAboutBox( class CHtmlFormatter* formatter );

    void
    deleteAboutBox();

    CHtmlSysWinAboutBoxQt*
    aboutBox()
    { return this->fAboutBox; }

    void
    updateRecentGames();

    void
    checkForUpdates();

#ifdef Q_OS_MAC
    // Handler for FileOpen events.  They only occur in OS X.
    bool
    handleFileOpenEvent( class QFileOpenEvent* e );
#endif

    //
    // CHtmlSysWinGroup interface implementation.
    //
    oshtml_charset_id_t
    get_default_win_charset() const override;

    size_t
    xlat_html4_entity( textchar_t* result, size_t result_size, unsigned int charval,
                       oshtml_charset_id_t* charset, int* changed_charset ) override;
};


#endif
