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

#include <QSettings>
#include <QDebug>
#include <QFileInfo>

#include "settings.h"
#include "syswingroup.h"
#include "globals.h"


void
Settings::loadFromDisk()
{
    QSettings sett;

    sett.beginGroup(QString::fromAscii("media"));
    this->enableGraphics = sett.value(QString::fromAscii("graphics"), true).toBool();
#ifndef Q_OS_ANDROID
    this->enableSoundEffects = sett.value(QString::fromAscii("sounds"), true).toBool();
    this->enableMusic = sett.value(QString::fromAscii("music"), true).toBool();
#else
    this->enableSoundEffects = sett.value(QString::fromAscii("sounds"), false).toBool();
    this->enableMusic = sett.value(QString::fromAscii("music"), false).toBool();
#endif
    this->enableLinks = sett.value(QString::fromAscii("links"), true).toBool();
    this->useSmoothScaling = sett.value(QString::fromAscii("smoothImageScaling"), true).toBool();
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("colors"));
    this->mainBgColor = sett.value(QString::fromAscii("mainbg"), QColor(Qt::white)).value<QColor>();
    this->mainTextColor = sett.value(QString::fromAscii("maintext"), QColor(Qt::black)).value<QColor>();
    this->bannerBgColor = sett.value(QString::fromAscii("bannerbg"), QColor(Qt::lightGray)).value<QColor>();
    this->bannerTextColor = sett.value(QString::fromAscii("bannertext"), QColor(Qt::black)).value<QColor>();
    this->inputColor = sett.value(QString::fromAscii("input"), QColor(70, 70, 70)).value<QColor>();
    this->underlineLinks = sett.value(QString::fromAscii("underlinelinks"), false).toBool();
    this->highlightLinks = sett.value(QString::fromAscii("highlightlinks"), true).toBool();
    this->unvisitedLinkColor = sett.value(QString::fromAscii("unvisitedlinks"), QColor(Qt::blue)).value<QColor>();
    this->hoveringLinkColor = sett.value(QString::fromAscii("hoveringlinks"), QColor(Qt::red)).value<QColor>();
    this->clickedLinkColor = sett.value(QString::fromAscii("clickedlinks"), QColor(Qt::cyan)).value<QColor>();
    sett.endGroup();

#ifdef Q_WS_MAC
    const QString& DEFAULT_SERIF = QString::fromAscii("Georgia,15");
    const QString& DEFAULT_SANS = QString::fromAscii("Helvetica,15");
    const QString& DEFAULT_MONO = QString::fromAscii("Andale Mono,15");
    const QString& DEFAULT_SCRIPT = QString::fromAscii("Apple Chancery,17");
#else
#ifdef Q_WS_WIN
    const QString& DEFAULT_SERIF = QString::fromAscii("Times New Roman,12");
    const QString& DEFAULT_SANS = QString::fromAscii("Verdana,12");
    const QString& DEFAULT_MONO = QString::fromAscii("Courier New,12");
    const QString& DEFAULT_SCRIPT = QString::fromAscii("Comic Sans MS,12");
#else
#ifdef Q_OS_ANDROID
    const QString& DEFAULT_SERIF = QString::fromAscii("Droid Serif");
    const QString& DEFAULT_SANS = QString::fromAscii("Droid Sans");
    const QString& DEFAULT_MONO = QString::fromAscii("Droid Sans Mono");
    const QString& DEFAULT_SCRIPT = QString::fromAscii("Droid Serif");
#else
    const QString& DEFAULT_SERIF = QString::fromAscii("serif");
    const QString& DEFAULT_SANS = QString::fromAscii("sans-serif");
    const QString& DEFAULT_MONO = QString::fromAscii("monospace");
    const QString& DEFAULT_SCRIPT = QString::fromAscii("cursive");
#endif
#endif
#endif
    sett.beginGroup(QString::fromAscii("fonts"));
    this->mainFont.fromString(sett.value(QString::fromAscii("main"), DEFAULT_SERIF).toString());
    this->fixedFont.fromString(sett.value(QString::fromAscii("fixed"), DEFAULT_MONO).toString());
    this->serifFont.fromString(sett.value(QString::fromAscii("serif"), DEFAULT_SERIF).toString());
    this->sansFont.fromString(sett.value(QString::fromAscii("sans"), DEFAULT_SANS).toString());
    this->scriptFont.fromString(sett.value(QString::fromAscii("script"), DEFAULT_SCRIPT).toString());
    this->writerFont.fromString(sett.value(QString::fromAscii("typewriter"), DEFAULT_MONO).toString());
    this->inputFont.fromString(sett.value(QString::fromAscii("input"), DEFAULT_SERIF).toString());
    this->useMainFontForInput = sett.value(QString::fromAscii("useMainFontForInput"), true).toBool();
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("misc"));
    this->ioSafetyLevelRead = sett.value(QString::fromAscii("ioSafetyLevelRead"), 2).toInt();
    this->ioSafetyLevelWrite = sett.value(QString::fromAscii("ioSafetyLevelWrite"), 2).toInt();
    this->tads2Encoding = sett.value(QString::fromAscii("tads2encoding"), QByteArray("windows-1252")).toByteArray();
    this->softScrolling = sett.value(QString::fromAscii("softscrolling"), true).toBool();
    this->askForGameFile = sett.value(QString::fromAscii("askforfileatstart"), false).toBool();
    this->confirmRestartGame = sett.value(QString::fromAscii("confirmrestartgame"), true).toBool();
    this->confirmQuitGame = sett.value(QString::fromAscii("confirmquitgame"), true).toBool();
    this->lastFileOpenDir = sett.value(QString::fromAscii("lastFileOpenDir"), QString::fromAscii("")).toString();
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("recent"));
    this->recentGamesList = sett.value(QString::fromAscii("games"), QStringList()).toStringList();
    Q_ASSERT(this->recentGamesList.size() <= this->recentGamesCapacity);
    // Remove any files that don't exist or aren't readable.
    for (int i = 0; i < this->recentGamesList.size(); ++i) {
        QFileInfo file(this->recentGamesList.at(i));
        if (not file.exists() or not (file.isFile() or file.isSymLink()) or not file.isReadable()) {
            this->recentGamesList.removeAt(i);
            --i;
        }
    }
    sett.endGroup();

    this->appSize = sett.value(QString::fromAscii("geometry/size"), QSize(740, 540)).toSize();
    this->lastUpdateDate = sett.value(QString::fromAscii("update/lastupdatedate"), QDate()).toDate();
    this->updateFreq = static_cast<UpdateFreq>(
        sett.value(QString::fromAscii("update/updatefreq"), UpdateDaily).toInt()
    );
}


void
Settings::saveToDisk()
{
    QSettings sett;

    sett.beginGroup(QString::fromAscii("media"));
    sett.setValue(QString::fromAscii("graphics"), this->enableGraphics);
    sett.setValue(QString::fromAscii("sounds"), this->enableSoundEffects);
    sett.setValue(QString::fromAscii("music"), this->enableMusic);
    sett.setValue(QString::fromAscii("links"), this->enableLinks);
    sett.setValue(QString::fromAscii("smoothImageScaling"), this->useSmoothScaling);
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("colors"));
    sett.setValue(QString::fromAscii("mainbg"), this->mainBgColor);
    sett.setValue(QString::fromAscii("maintext"), this->mainTextColor);
    sett.setValue(QString::fromAscii("bannerbg"), this->bannerBgColor);
    sett.setValue(QString::fromAscii("bannertext"), this->bannerTextColor);
    sett.setValue(QString::fromAscii("input"), this->inputColor);
    sett.setValue(QString::fromAscii("underlinelinks"), this->underlineLinks);
    sett.setValue(QString::fromAscii("highlightlinks"), this->highlightLinks);
    sett.setValue(QString::fromAscii("unvisitedlinks"), this->unvisitedLinkColor);
    sett.setValue(QString::fromAscii("hoveringlinks"), this->hoveringLinkColor);
    sett.setValue(QString::fromAscii("clickedlinks"), this->clickedLinkColor);
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("fonts"));
    sett.setValue(QString::fromAscii("main"), this->mainFont.toString());
    sett.setValue(QString::fromAscii("fixed"), this->fixedFont.toString());
    sett.setValue(QString::fromAscii("serif"), this->serifFont.toString());
    sett.setValue(QString::fromAscii("sans"), this->sansFont.toString());
    sett.setValue(QString::fromAscii("script"), this->scriptFont.toString());
    sett.setValue(QString::fromAscii("typewriter"), this->writerFont.toString());
    sett.setValue(QString::fromAscii("input"), this->inputFont.toString());
    sett.setValue(QString::fromAscii("useMainFontForInput"), this->useMainFontForInput);
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("misc"));
    sett.setValue(QString::fromAscii("ioSafetyLevelRead"), this->ioSafetyLevelRead);
    sett.setValue(QString::fromAscii("ioSafetyLevelWrite"), this->ioSafetyLevelWrite);
    sett.setValue(QString::fromAscii("tads2encoding"), this->tads2Encoding);
    sett.setValue(QString::fromAscii("softscrolling"), this->softScrolling);
    sett.setValue(QString::fromAscii("askforfileatstart"), this->askForGameFile);
    sett.setValue(QString::fromAscii("confirmrestartgame"), this->confirmRestartGame);
    sett.setValue(QString::fromAscii("confirmquitgame"), this->confirmQuitGame);
    sett.setValue(QString::fromAscii("lastFileOpenDir"), this->lastFileOpenDir);
    sett.endGroup();

    sett.beginGroup(QString::fromAscii("recent"));
    sett.setValue(QString::fromAscii("games"), this->recentGamesList);
    sett.endGroup();

    sett.setValue(QString::fromAscii("geometry/size"), qWinGroup->size());
    sett.setValue(QString::fromAscii("update/lastupdatedate"), this->lastUpdateDate);
    sett.setValue(QString::fromAscii("update/updatefreq"), this->updateFreq);
    sett.sync();
}
