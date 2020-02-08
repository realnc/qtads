// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "gameinfo.h"
#include <QDialog>

/* Implementation of the game information enumerator callback interface.
 * See tads3/gameinfo.h for details.
 */
class QTadsGameInfoEnum: public CTadsGameInfo_enum
{
public:
    QString gameName;
    QString plainGameName; // Game name but without any HTML markup.
    QString headline;
    QString byLine;
    QString htmlByLine;
    QString email;
    QString desc;
    QString htmlDesc;
    QString version;
    QString published;
    QString date;
    QString lang;
    QString series;
    QString seriesNumber;
    QString genre;
    QString forgiveness;
    QString license;
    QString copyRules;
    QString ifid;

    void tads_enum_game_info(const char* name, const char* val) override;
};

namespace Ui {
class GameInfoDialog;
}

class GameInfoDialog: public QDialog
{
    Q_OBJECT

public:
    explicit GameInfoDialog(const QByteArray& fname, QWidget* parent = 0);
    ~GameInfoDialog() override;

    // Checks whether a game file contains any embedded meta information.
    static bool gameHasMetaInfo(const QByteArray& fname);

    static QTadsGameInfoEnum getMetaInfo(const QByteArray& fname);

private:
    Ui::GameInfoDialog* ui;
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
