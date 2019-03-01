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
#ifndef SETTINGS_H
#define SETTINGS_H

#include "sysfont.h"
#include <QDate>

class Settings
{
public:
    void loadFromDisk();

    void saveToDisk();

    bool enableGraphics;
    bool enableSoundEffects;
    bool enableMusic;
    bool enableLinks;
    bool useSmoothScaling;

    QColor mainTextColor;
    QColor mainBgColor;
    QColor bannerTextColor;
    QColor bannerBgColor;
    QColor inputColor;

    bool underlineLinks;
    bool highlightLinks;
    QColor unvisitedLinkColor;
    QColor hoveringLinkColor;
    QColor clickedLinkColor;

    CHtmlSysFontQt mainFont;
    CHtmlSysFontQt fixedFont;
    CHtmlSysFontQt serifFont;
    CHtmlSysFontQt sansFont;
    CHtmlSysFontQt scriptFont;
    CHtmlSysFontQt writerFont;
    CHtmlSysFontQt inputFont;
    bool useMainFontForInput;

    int ioSafetyLevelRead;
    int ioSafetyLevelWrite;

    QByteArray tads2Encoding;
    bool pasteOnDblClk;
    bool softScrolling;
    bool askForGameFile;
    bool confirmRestartGame;
    bool confirmQuitGame;
    QString lastFileOpenDir;

    QStringList recentGamesList;
    static const int recentGamesCapacity = 10;

    QByteArray appGeometry;
    QDate lastUpdateDate;
    enum UpdateFreq
    {
        UpdateOnEveryStart,
        UpdateDaily,
        UpdateWeekly,
        UpdateNever
    } updateFreq;
};

#endif
