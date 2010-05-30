/* Copyright (C) 2010 Nikos Chantziaras.
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
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef QTADSCONFDIALOG_H
#define QTADSCONFDIALOG_H

#include <QDialog>


namespace Ui {
	class QTadsConfDialog;
}

class QTadsConfDialog: public QDialog {
	Q_OBJECT

  public:
	QTadsConfDialog( class CHtmlSysWinGroupQt* parent = 0 );
	~QTadsConfDialog();

  protected:
	void
	changeEvent( QEvent* e );

  private:
	Ui::QTadsConfDialog* ui;

  private slots:
	void
	applySettings();

	void
	selectColor( int i );
};


#endif
