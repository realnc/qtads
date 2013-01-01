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

    sett.beginGroup(QString::fromLatin1("media"));
    this->enableGraphics = sett.value(QString::fromLatin1("graphics"), true).toBool();
#ifndef Q_OS_ANDROID
    this->enableSoundEffects = sett.value(QString::fromLatin1("sounds"), true).toBool();
    this->enableMusic = sett.value(QString::fromLatin1("music"), true).toBool();
#else
    this->enableSoundEffects = sett.value(QString::fromLatin1("sounds"), false).toBool();
    this->enableMusic = sett.value(QString::fromLatin1("music"), false).toBool();
#endif
    this->enableLinks = sett.value(QString::fromLatin1("links"), true).toBool();
    this->useSmoothScaling = sett.value(QString::fromLatin1("smoothImageScaling"), true).toBool();
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("colors"));
    this->mainBgColor = sett.value(QString::fromLatin1("mainbg"), QColor(Qt::white)).value<QColor>();
    this->mainTextColor = sett.value(QString::fromLatin1("maintext"), QColor(Qt::black)).value<QColor>();
    this->bannerBgColor = sett.value(QString::fromLatin1("bannerbg"), QColor(Qt::lightGray)).value<QColor>();
    this->bannerTextColor = sett.value(QString::fromLatin1("bannertext"), QColor(Qt::black)).value<QColor>();
    this->inputColor = sett.value(QString::fromLatin1("input"), QColor(70, 70, 70)).value<QColor>();
    this->underlineLinks = sett.value(QString::fromLatin1("underlinelinks"), false).toBool();
    this->highlightLinks = sett.value(QString::fromLatin1("highlightlinks"), true).toBool();
    this->unvisitedLinkColor = sett.value(QString::fromLatin1("unvisitedlinks"), QColor(Qt::blue)).value<QColor>();
    this->hoveringLinkColor = sett.value(QString::fromLatin1("hoveringlinks"), QColor(Qt::red)).value<QColor>();
    this->clickedLinkColor = sett.value(QString::fromLatin1("clickedlinks"), QColor(Qt::cyan)).value<QColor>();
    sett.endGroup();

#ifdef Q_OS_MAC
    const QString& DEFAULT_SERIF = QString::fromLatin1("Georgia,15");
    const QString& DEFAULT_SANS = QString::fromLatin1("Helvetica,15");
    const QString& DEFAULT_MONO = QString::fromLatin1("Andale Mono,15");
    const QString& DEFAULT_SCRIPT = QString::fromLatin1("Apple Chancery,17");
#else
#ifdef Q_OS_WIN32
    const QString& DEFAULT_SERIF = QString::fromLatin1("Times New Roman,12");
    const QString& DEFAULT_SANS = QString::fromLatin1("Verdana,12");
    const QString& DEFAULT_MONO = QString::fromLatin1("Courier New,12");
    const QString& DEFAULT_SCRIPT = QString::fromLatin1("Comic Sans MS,12");
#else
#ifdef Q_OS_ANDROID
    const QString& DEFAULT_SERIF = QString::fromLatin1("Droid Serif");
    const QString& DEFAULT_SANS = QString::fromLatin1("Droid Sans");
    const QString& DEFAULT_MONO = QString::fromLatin1("Droid Sans Mono");
    const QString& DEFAULT_SCRIPT = QString::fromLatin1("Droid Serif");
#else
    const QString& DEFAULT_SERIF = QString::fromLatin1("serif");
    const QString& DEFAULT_SANS = QString::fromLatin1("sans-serif");
    const QString& DEFAULT_MONO = QString::fromLatin1("monospace");
    const QString& DEFAULT_SCRIPT = QString::fromLatin1("cursive");
#endif
#endif
#endif
    sett.beginGroup(QString::fromLatin1("fonts"));
    this->mainFont.fromString(sett.value(QString::fromLatin1("main"), DEFAULT_SERIF).toString());
    this->fixedFont.fromString(sett.value(QString::fromLatin1("fixed"), DEFAULT_MONO).toString());
    this->serifFont.fromString(sett.value(QString::fromLatin1("serif"), DEFAULT_SERIF).toString());
    this->sansFont.fromString(sett.value(QString::fromLatin1("sans"), DEFAULT_SANS).toString());
    this->scriptFont.fromString(sett.value(QString::fromLatin1("script"), DEFAULT_SCRIPT).toString());
    this->writerFont.fromString(sett.value(QString::fromLatin1("typewriter"), DEFAULT_MONO).toString());
    this->inputFont.fromString(sett.value(QString::fromLatin1("input"), DEFAULT_SERIF).toString());
    this->useMainFontForInput = sett.value(QString::fromLatin1("useMainFontForInput"), true).toBool();
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("misc"));
    this->ioSafetyLevelRead = sett.value(QString::fromLatin1("ioSafetyLevelRead"), 2).toInt();
    this->ioSafetyLevelWrite = sett.value(QString::fromLatin1("ioSafetyLevelWrite"), 2).toInt();
    this->tads2Encoding = sett.value(QString::fromLatin1("tads2encoding"), QByteArray("windows-1252")).toByteArray();
    this->softScrolling = sett.value(QString::fromLatin1("softscrolling"), true).toBool();
    this->askForGameFile = sett.value(QString::fromLatin1("askforfileatstart"), false).toBool();
    this->confirmRestartGame = sett.value(QString::fromLatin1("confirmrestartgame"), true).toBool();
    this->confirmQuitGame = sett.value(QString::fromLatin1("confirmquitgame"), true).toBool();
    this->lastFileOpenDir = sett.value(QString::fromLatin1("lastFileOpenDir"), QString::fromLatin1("")).toString();
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("recent"));
    this->recentGamesList = sett.value(QString::fromLatin1("games"), QStringList()).toStringList();
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

    this->appSize = sett.value(QString::fromLatin1("geometry/size"), QSize(740, 540)).toSize();
    this->lastUpdateDate = sett.value(QString::fromLatin1("update/lastupdatedate"), QDate()).toDate();
    this->updateFreq = static_cast<UpdateFreq>(
        sett.value(QString::fromLatin1("update/updatefreq"), UpdateDaily).toInt()
    );
}


void
Settings::saveToDisk()
{
    QSettings sett;

    sett.beginGroup(QString::fromLatin1("media"));
    sett.setValue(QString::fromLatin1("graphics"), this->enableGraphics);
    sett.setValue(QString::fromLatin1("sounds"), this->enableSoundEffects);
    sett.setValue(QString::fromLatin1("music"), this->enableMusic);
    sett.setValue(QString::fromLatin1("links"), this->enableLinks);
    sett.setValue(QString::fromLatin1("smoothImageScaling"), this->useSmoothScaling);
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("colors"));
    sett.setValue(QString::fromLatin1("mainbg"), this->mainBgColor);
    sett.setValue(QString::fromLatin1("maintext"), this->mainTextColor);
    sett.setValue(QString::fromLatin1("bannerbg"), this->bannerBgColor);
    sett.setValue(QString::fromLatin1("bannertext"), this->bannerTextColor);
    sett.setValue(QString::fromLatin1("input"), this->inputColor);
    sett.setValue(QString::fromLatin1("underlinelinks"), this->underlineLinks);
    sett.setValue(QString::fromLatin1("highlightlinks"), this->highlightLinks);
    sett.setValue(QString::fromLatin1("unvisitedlinks"), this->unvisitedLinkColor);
    sett.setValue(QString::fromLatin1("hoveringlinks"), this->hoveringLinkColor);
    sett.setValue(QString::fromLatin1("clickedlinks"), this->clickedLinkColor);
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("fonts"));
    sett.setValue(QString::fromLatin1("main"), this->mainFont.toString());
    sett.setValue(QString::fromLatin1("fixed"), this->fixedFont.toString());
    sett.setValue(QString::fromLatin1("serif"), this->serifFont.toString());
    sett.setValue(QString::fromLatin1("sans"), this->sansFont.toString());
    sett.setValue(QString::fromLatin1("script"), this->scriptFont.toString());
    sett.setValue(QString::fromLatin1("typewriter"), this->writerFont.toString());
    sett.setValue(QString::fromLatin1("input"), this->inputFont.toString());
    sett.setValue(QString::fromLatin1("useMainFontForInput"), this->useMainFontForInput);
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("misc"));
    sett.setValue(QString::fromLatin1("ioSafetyLevelRead"), this->ioSafetyLevelRead);
    sett.setValue(QString::fromLatin1("ioSafetyLevelWrite"), this->ioSafetyLevelWrite);
    sett.setValue(QString::fromLatin1("tads2encoding"), this->tads2Encoding);
    sett.setValue(QString::fromLatin1("softscrolling"), this->softScrolling);
    sett.setValue(QString::fromLatin1("askforfileatstart"), this->askForGameFile);
    sett.setValue(QString::fromLatin1("confirmrestartgame"), this->confirmRestartGame);
    sett.setValue(QString::fromLatin1("confirmquitgame"), this->confirmQuitGame);
    sett.setValue(QString::fromLatin1("lastFileOpenDir"), this->lastFileOpenDir);
    sett.endGroup();

    sett.beginGroup(QString::fromLatin1("recent"));
    sett.setValue(QString::fromLatin1("games"), this->recentGamesList);
    sett.endGroup();

    sett.setValue(QString::fromLatin1("geometry/size"), qWinGroup->size());
    sett.setValue(QString::fromLatin1("update/lastupdatedate"), this->lastUpdateDate);
    sett.setValue(QString::fromLatin1("update/updatefreq"), this->updateFreq);
    sett.sync();
}
