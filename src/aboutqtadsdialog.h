// This is copyrighted software. More information is at the end of this file.
#ifndef ABOUTQTADSDIALOG_H
#define ABOUTQTADSDIALOG_H

#include <QDialog>

#include "config.h"

namespace Ui {
class AboutQtadsDialog;
}

class AboutQtadsDialog: public QDialog
{
    Q_OBJECT

public:
    explicit AboutQtadsDialog(QWidget* parent = 0);
    ~AboutQtadsDialog() override;

private:
    Ui::AboutQtadsDialog* ui;
};

#endif // ABOUTQTADSDIALOG_H

/*
    Copyright 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2018, 2019 Nikos
    Chantziaras.

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
