// This is copyrighted software. More information is at the end of this file.
#include "aboutqtadsdialog.h"
#include "ui_aboutqtadsdialog.h"

#include "globals.h"
#include "htmlver.h"
#include "trd.h"
#include "vmvsn.h"

AboutQtadsDialog::AboutQtadsDialog(QWidget* const parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    , ui(std::make_unique<Ui::AboutQtadsDialog>())
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    // On Mac OS X, the dialog should not have any buttons.
    ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
    // Show a "Close" button everywhere else.
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
#endif

    ui->aboutLabel->setText(tr(R"(
        <h1>QTads</h1>
        <br>%1
        <p><a href="https://realnc.github.io/qtads">https://realnc.github.io/qtads</a></p>

        <p>
        TADS Copyright © 2016 Michael J. Roberts<br>
        QTads Copyright © 2020 Nikos Chantziaras
        </p>

        <p>
        QTads is free software: you can redistribute it and/or modify it under the terms of the
        <a href="https://realnc.github.io/qtads/gpl-3.0-standalone.html">GNU General Public
        License</a> as published by the Free Software Foundation, either version 3 of the License,
        or (at your option) any later version.
        </p>
    )")
                                .arg(QTADS_VERSION));

    ui->versionInfoLabel->setText(
        "<p><table border=\"0\" width=\"100%\">"
        "<tr><td>QTads:</td><td>" QTADS_VERSION "<br></td></tr>"
        "<tr><td>HTML TADS:</td><td>\t" HTMLTADS_VERSION "</td></tr>"
        "<tr>"
            "<td>TADS 2 " + tr("virtual machine:") + "</td>"
            "<td>\t" TADS_RUNTIME_VERSION "</td>"
        "</tr>"
        "<tr>"
            "<td>TADS 3 " + tr("virtual machine:") + "</td>"
            "<td>\t" T3VM_VSN_STRING + " (" T3VM_IDENTIFICATION + ")<br></td>"
        "</tr>"
        "<tr><td>Qt " + tr("build version:") + "</td><td>" QT_VERSION_STR "</td></tr>"
        "<tr><td>Qt " + tr("runtime version:") + "</td><td>" + qVersion() + "</td></tr></table></p>"
    );
}

AboutQtadsDialog::~AboutQtadsDialog() = default;

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
