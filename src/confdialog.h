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
#ifndef CONFDIALOG_H
#define CONFDIALOG_H

#include <QDialog>

#include "config.h"


namespace Ui {
    class ConfDialog;
}

class ConfDialog: public QDialog {
    Q_OBJECT

  public:
    ConfDialog( class CHtmlSysWinGroupQt* parent = 0 );
    ~ConfDialog() override;

  protected:
    void
    changeEvent( QEvent* e ) override;

  private:
    Ui::ConfDialog* ui;

    // Makes the dialog's controls apply instantly when they change.
    void
    fMakeInstantApply();

  private slots:
    void
    fApplySettings();
};


#endif
