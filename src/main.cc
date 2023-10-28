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

auto main(int argc, char** argv) -> int
{
    CHtmlResType::add_basic_types();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // No need to enable High Dpi scaling because it's always on
#elif QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    CHtmlSysFrameQt* app =
        new CHtmlSysFrameQt(argc, argv, "QTads", QTADS_VERSION, "Nikos Chantziaras", {});
#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setDesktopFileName("nikos.chantziaras.qtads");
#endif

    // Filename of the game to run.
    QString gameFileName;
    bool embed = false;

    const QStringList& args = app->arguments();
    if (args.size() >= 1) {
        bool prevNonFlagArgument = false;

        for (int i = 1; i < args.size(); ++i) {
            const auto& arg = args.at(i);

            if (!arg.startsWith("-")) {
                if (prevNonFlagArgument) {
                    qWarning() << "It looks like you specified more than one non-flag command-line"
                               << "argument at" << arg
                               << "but QTADS can only accept one game file, so only the"
                               << "first non flag argument will be used.";
                } else if (gameFileName.isNull()) {
                    if (QFile::exists(arg)) {
                        gameFileName = arg;
                    } else if (QFile::exists(arg + ".gam")) {
                        gameFileName = arg + ".gam";
                    } else if (QFile::exists(arg + ".t3")) {
                        gameFileName = arg + ".t3";
                    } else {
                        qWarning() << "File" << arg << "not found.";
                    }
                }

                prevNonFlagArgument = true;
            } else if (arg == "--help" || arg == "-h") {
                qInfo() << "qtads [OPTIONS] [FILE]\n"
                        << "\t--help\t\tThis help message\n"
                        << "\t--embed\t\tPrint out the window id on startup so that qtads can be "
                           "embedded\n"
                        << "\t-h\t\tSame as --help\n"
                        << "\t-e\t\tSame as --embed";
            } else if (arg == "--embed" || arg == "-e") {
                embed = true;
            } else {
                qWarning() << "Unrecognized command line argument " << arg << ".";
                return 1;
            }
        }
    }

    if (gameFileName.isEmpty() and app->settings().askForGameFile) {
        gameFileName = QFileDialog::getOpenFileName(
            nullptr, QObject::tr("Choose the TADS game you wish to run"), {},
            QObject::tr("TADS Games") + "(*.gam *.Gam *.GAM *.t3 *.T3)");
    }

#ifndef NO_AUDIO
    if (not initSound()) {
        delete app;
        return 1;
    }
#endif

    QTimer::singleShot(
        0, app, [app, embed, gameFileName] { app->entryPoint(gameFileName, embed); });
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
