// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "sysfont.h"
#include <QDate>

class Settings
{
public:
    Settings();
    void loadFromDisk();
    void saveToDisk();

    bool enableGraphics = true;
    bool enableSoundEffects = true;
    bool enableMusic = true;
    bool enableLinks = true;
    bool useSmoothScaling = true;

    QColor mainTextColor = Qt::black;
    QColor mainBgColor = Qt::white;
    QColor bannerTextColor = Qt::black;
    QColor bannerBgColor = Qt::lightGray;
    QColor inputColor = {70, 70, 70};

    bool underlineLinks = false;
    bool highlightLinks = true;
    QColor unvisitedLinkColor = Qt::blue;
    QColor hoveringLinkColor = Qt::red;
    QColor clickedLinkColor = Qt::cyan;

    CHtmlSysFontQt mainFont;
    CHtmlSysFontQt fixedFont;
    CHtmlSysFontQt serifFont;
    CHtmlSysFontQt sansFont;
    CHtmlSysFontQt scriptFont;
    CHtmlSysFontQt writerFont;
    CHtmlSysFontQt inputFont;
    bool useMainFontForInput = true;

    int ioSafetyLevelRead = 2;
    int ioSafetyLevelWrite = 2;

    QByteArray tads2Encoding = "windows-1252";
    bool pasteOnDblClk = true;
    bool softScrolling = true;
    bool askForGameFile = false;
    bool confirmRestartGame = true;
    bool confirmQuitGame = true;
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
    } updateFreq = UpdateDaily;
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
