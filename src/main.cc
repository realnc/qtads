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
#include <QDebug>
#include <QMetaType>
#include <QFileDialog>

#include "settings.h"
#include "sysframe.h"
#include "qtadssound.h"

// Static Qt4 builds on OS X need the text codec plugins.
#if defined(STATIC_QT) and defined(Q_OS_MAC) and QT_VERSION < 0x050000
    #include <QtPlugin>
    Q_IMPORT_PLUGIN(qcncodecs)
    Q_IMPORT_PLUGIN(qjpcodecs)
    Q_IMPORT_PLUGIN(qtwcodecs)
    Q_IMPORT_PLUGIN(qkrcodecs)
#endif

// On some platforms, SDL redefines main in order to provide a
// platform-specific main() implementation.  However, Qt handles this too,
// so things can get weird.  We need to make sure main is not redefined so
// that Qt can find our own implementation and SDL will not try to do
// platform-specific initialization work (like launching the Cocoa event-loop
// or setting up the application menu on OS X, or redirecting stdout and stderr
// to text files on Windows), which would break things.
#ifdef main
#  undef main
#endif


int main( int argc, char** argv )
{
    CHtmlResType::add_basic_types();
    CHtmlSysFrameQt* app = new CHtmlSysFrameQt(argc, argv, "QTads", "2.0", "Nikos Chantziaras",
                                               "qtads.sourceforge.net");

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
        gameFileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the TADS game you wish to run"),
                                                    QString::fromLatin1(""),
                                                    QObject::tr("TADS Games")
                                                    + QString::fromLatin1("(*.gam *.Gam *.GAM *.t3 *.T3)"));
    }

    if (not initSound()) {
        delete app;
        return 1;
    }

    QMetaObject::invokeMethod(app, "entryPoint", Qt::QueuedConnection, Q_ARG(QString, gameFileName));
    int ret = app->exec();

    delete app;
    quitSound();
    return ret;
}
