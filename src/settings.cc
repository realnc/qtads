// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QFileInfo>
#include <QSettings>

#include "globals.h"
#include "settings.h"
#include "syswingroup.h"

static auto fontForStyleHint(const QFont::StyleHint hint) -> CHtmlSysFontQt
{
    CHtmlSysFontQt f;
    f.setStyleHint(hint);
    f.setFamily(f.defaultFamily());
    f.setPointSize(f.pointSize() + 4);
    return f;
}

Settings::Settings()
    : mainFont(fontForStyleHint(QFont::Serif))
    , fixedFont(fontForStyleHint(QFont::Monospace))
    , serifFont(fontForStyleHint(QFont::Serif))
    , sansFont(fontForStyleHint(QFont::SansSerif))
    , scriptFont(fontForStyleHint(QFont::Cursive))
    , writerFont(fontForStyleHint(QFont::TypeWriter))
    , inputFont(fontForStyleHint(QFont::Serif))
{
#ifdef NO_AUDIO
    bool enableSoundEffects = false;
    bool enableMusic = false;
#endif
}

void Settings::loadFromDisk()
{
    QSettings sett;

    sett.beginGroup("media");
    enableGraphics = sett.value("graphics", enableGraphics).toBool();
#ifndef NO_AUDIO
    enableSoundEffects = sett.value("sounds", enableSoundEffects).toBool();
    enableMusic = sett.value("music", enableMusic).toBool();
#endif
    enableLinks = sett.value("links", enableLinks).toBool();
    useSmoothScaling = sett.value("smoothImageScaling", useSmoothScaling).toBool();
    sett.endGroup();

    sett.beginGroup("colors");
    mainBgColor = sett.value("mainbg", mainBgColor).value<QColor>();
    mainTextColor = sett.value("maintext", mainTextColor).value<QColor>();
    bannerBgColor = sett.value("bannerbg", bannerBgColor).value<QColor>();
    bannerTextColor = sett.value("bannertext", bannerTextColor).value<QColor>();
    inputColor = sett.value("input", inputColor).value<QColor>();
    underlineLinks = sett.value("underlinelinks", underlineLinks).toBool();
    highlightLinks = sett.value("highlightlinks", highlightLinks).toBool();
    unvisitedLinkColor = sett.value("unvisitedlinks", unvisitedLinkColor).value<QColor>();
    hoveringLinkColor = sett.value("hoveringlinks", hoveringLinkColor).value<QColor>();
    clickedLinkColor = sett.value("clickedlinks", clickedLinkColor).value<QColor>();
    sett.endGroup();

    sett.beginGroup("fonts");
    mainFont.fromString(sett.value("main", mainFont).toString());
    fixedFont.fromString(sett.value("fixed", fixedFont).toString());
    serifFont.fromString(sett.value("serif", serifFont).toString());
    sansFont.fromString(sett.value("sans", sansFont).toString());
    scriptFont.fromString(sett.value("script", scriptFont).toString());
    writerFont.fromString(sett.value("typewriter", writerFont).toString());
    inputFont.fromString(sett.value("input", inputFont).toString());
    useMainFontForInput = sett.value("useMainFontForInput", useMainFontForInput).toBool();
    sett.endGroup();

    sett.beginGroup("misc");
    ioSafetyLevelRead = sett.value("ioSafetyLevelRead", ioSafetyLevelRead).toInt();
    ioSafetyLevelWrite = sett.value("ioSafetyLevelWrite", ioSafetyLevelWrite).toInt();
    tads2Encoding = sett.value("tads2encoding", tads2Encoding).toByteArray();
    pasteOnDblClk = sett.value("pasteondoubleclick", pasteOnDblClk).toBool();
    softScrolling = sett.value("softscrolling", softScrolling).toBool();
    askForGameFile = sett.value("askforfileatstart", askForGameFile).toBool();
    confirmRestartGame = sett.value("confirmrestartgame", confirmRestartGame).toBool();
    confirmQuitGame = sett.value("confirmquitgame", confirmQuitGame).toBool();
    textWidth = sett.value("linewidth", textWidth).toInt();
    lastFileOpenDir = sett.value("lastFileOpenDir", "").toString();
    sett.endGroup();

    sett.beginGroup("recent");
    recentGamesList = sett.value("games", QStringList()).toStringList();
    Q_ASSERT(recentGamesList.size() <= recentGamesCapacity);
    // Remove any files that don't exist or aren't readable.
    for (int i = 0; i < recentGamesList.size(); ++i) {
        QFileInfo file(recentGamesList.at(i));
        if (not file.exists() or not(file.isFile() or file.isSymLink()) or not file.isReadable()) {
            recentGamesList.removeAt(i);
            --i;
        }
    }
    sett.endGroup();

    appGeometry = sett.value("geometry/appgeometry", QByteArray()).toByteArray();
    lastUpdateDate = sett.value("update/lastupdatedate", QDate()).toDate();
    updateFreq = static_cast<UpdateFreq>(sett.value("update/updatefreq", updateFreq).toInt());
}

void Settings::saveToDisk()
{
    QSettings sett;

    sett.beginGroup("media");
    sett.setValue("graphics", enableGraphics);
#ifndef NO_AUDIO
    sett.setValue("sounds", enableSoundEffects);
    sett.setValue("music", enableMusic);
#endif
    sett.setValue("links", enableLinks);
    sett.setValue("smoothImageScaling", useSmoothScaling);
    sett.endGroup();

    sett.beginGroup("colors");
    sett.setValue("mainbg", mainBgColor);
    sett.setValue("maintext", mainTextColor);
    sett.setValue("bannerbg", bannerBgColor);
    sett.setValue("bannertext", bannerTextColor);
    sett.setValue("input", inputColor);
    sett.setValue("underlinelinks", underlineLinks);
    sett.setValue("highlightlinks", highlightLinks);
    sett.setValue("unvisitedlinks", unvisitedLinkColor);
    sett.setValue("hoveringlinks", hoveringLinkColor);
    sett.setValue("clickedlinks", clickedLinkColor);
    sett.endGroup();

    sett.beginGroup("fonts");
    sett.setValue("main", mainFont.toString());
    sett.setValue("fixed", fixedFont.toString());
    sett.setValue("serif", serifFont.toString());
    sett.setValue("sans", sansFont.toString());
    sett.setValue("script", scriptFont.toString());
    sett.setValue("typewriter", writerFont.toString());
    sett.setValue("input", inputFont.toString());
    sett.setValue("useMainFontForInput", useMainFontForInput);
    sett.endGroup();

    sett.beginGroup("misc");
    sett.setValue("ioSafetyLevelRead", ioSafetyLevelRead);
    sett.setValue("ioSafetyLevelWrite", ioSafetyLevelWrite);
    sett.setValue("tads2encoding", tads2Encoding);
    sett.setValue("pasteondoubleclick", pasteOnDblClk);
    sett.setValue("softscrolling", softScrolling);
    sett.setValue("askforfileatstart", askForGameFile);
    sett.setValue("confirmrestartgame", confirmRestartGame);
    sett.setValue("confirmquitgame", confirmQuitGame);
    sett.setValue("linewidth", textWidth);
    sett.setValue("lastFileOpenDir", lastFileOpenDir);
    sett.endGroup();

    sett.beginGroup("recent");
    sett.setValue("games", recentGamesList);
    sett.endGroup();

    sett.setValue("geometry/appgeometry", qWinGroup->saveGeometry());
    sett.setValue("update/lastupdatedate", lastUpdateDate);
    sett.setValue("update/updatefreq", updateFreq);
    sett.sync();
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
