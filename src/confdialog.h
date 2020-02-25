// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include <QDialog>
#include <memory>

class CHtmlSysWinGroupQt;
class Settings;
namespace Ui {
class ConfDialog;
}

class ConfDialog final: public QDialog
{
    Q_OBJECT

public:
    ConfDialog(CHtmlSysWinGroupQt* parent = nullptr);
    ~ConfDialog() override;

private:
    std::unique_ptr<Ui::ConfDialog> ui;
    bool is_instant_apply_ = false;
    bool disable_apply_ = false;

    void fMakeInstantApply();

private slots:
    void fLoadSettings(const Settings& sett);
    void fApplySettings();
    void fRestoreDefaults();
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
