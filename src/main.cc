// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QFileDialog>
#include <QTimer>

#include "qtadssound.h"
#include "settings.h"
#include "sysframe.h"

// On some platforms, SDL redefines main in order to provide a
// platform-specific main() implementation.  However, Qt handles this too,
// so things can get weird.  We need to make sure main is not redefined so
// that Qt can find our own implementation and SDL will not try to do
// platform-specific initialization work (like launching the Cocoa event-loop
// or setting up the application menu on OS X, or redirecting stdout and stderr
// to text files on Windows), which would break things.
#if !defined(NO_AUDIO) && defined(main)
#undef main
#endif

int main(int argc, char** argv)
{
    CHtmlResType::add_basic_types();
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    CHtmlSysFrameQt* app = new CHtmlSysFrameQt(argc, argv, "QTads", QTADS_VERSION,
                                               "Nikos Chantziaras", "nikos.chantziaras.qtads");
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setDesktopFileName(QStringLiteral("nikos.chantziaras.qtads"));
#endif

    // Filename of the game to run.
    QString gameFileName;

    const QStringList& args = app->arguments();
    if (args.size() == 2) {
        if (QFile::exists(args.at(1))) {
            gameFileName = args.at(1);
        } else if (QFile::exists(args.at(1) + QString::fromLatin1(".gam"))) {
            gameFileName = args.at(1) + QString::fromLatin1(".gam");
        } else if (QFile::exists(args.at(1) + QString::fromLatin1(".t3"))) {
            gameFileName = args.at(1) + QString::fromLatin1(".t3");
        } else {
            qWarning() << "File" << args.at(1) << "not found.";
        }
    }

    if (gameFileName.isEmpty() and app->settings()->askForGameFile) {
        gameFileName = QFileDialog::getOpenFileName(
            0, QObject::tr("Choose the TADS game you wish to run"), QString::fromLatin1(""),
            QObject::tr("TADS Games") + QString::fromLatin1("(*.gam *.Gam *.GAM *.t3 *.T3)"));
    }

#ifndef NO_AUDIO
    if (not initSound()) {
        delete app;
        return 1;
    }
#endif

    QTimer::singleShot(0, app, [app, gameFileName] { app->entryPoint(gameFileName); });
    int ret = CHtmlSysFrameQt::exec();

    delete app;
    quitSound();
    return ret;
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
