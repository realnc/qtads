// This is copyrighted software. More information is at the end of this file.
#include "aboutqtadsdialog.h"
#include "ui_aboutqtadsdialog.h"

#include "globals.h"
#include "htmlver.h"
#include "trd.h"
#include "vmvsn.h"

AboutQtadsDialog::AboutQtadsDialog(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::AboutQtadsDialog)
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
        TADS Copyright © 2013 Michael J. Roberts<br>
        QTads Copyright © 2019 Nikos Chantziaras
        </p>

        <p>
        QTads is free software: you can redistribute it and/or modify it under the terms of the
        <a href="https://realnc.github.io/qtads/gpl-3.0-standalone.html">GNU General Public
        License</a> as published by the Free Software Foundation, either version 3 of the License,
        or (at your option) any later version.
        </p>
    )")
                                .arg(QLatin1String(QTADS_VERSION)));

    // Construct a string holding all version info.
    QString str;
    str += QString::fromLatin1("<p><table border=\"0\" width=\"100%\"><tr><td>");
    str += tr("QTads:") + QString::fromLatin1("</td><td>") + QString::fromLatin1(QTADS_VERSION)
           + QString::fromLatin1("<br></td></tr><tr><td>") + tr("HTML TADS:")
           + QString::fromLatin1("</td><td>\t") + QString::fromLatin1(HTMLTADS_VERSION)
           + QString::fromLatin1("</td></tr><tr><td>") + tr("TADS 2 virtual machine:")
           + QString::fromLatin1("</td><td>\t") + QString::fromLatin1(TADS_RUNTIME_VERSION)
           + QString::fromLatin1("</td></tr><tr><td>") + tr("TADS 3 virtual machine:")
           + QString::fromLatin1("</td><td>\t") + QString::fromLatin1(T3VM_VSN_STRING)
           + QString::fromLatin1(" (") + QString::fromLatin1(T3VM_IDENTIFICATION)
           + QString::fromLatin1(")<br></td></tr><tr><td>") + tr("Qt build version:")
           + QString::fromLatin1("</td><td>") + QString::fromLatin1(QT_VERSION_STR)
           + QString::fromLatin1("</td></tr><tr><td>") + tr("Qt runtime version:")
           + QString::fromLatin1("</td><td>") + QString::fromLatin1(qVersion())
           + QString::fromLatin1("</td></tr></table></p>");
    ui->versionInfoLabel->setText(str);
}

AboutQtadsDialog::~AboutQtadsDialog()
{
    delete ui;
}

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
