// This is copyrighted software. More information is at the end of this file.
#pragma once
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
    int textWidth = 70;

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
