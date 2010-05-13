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

#include "qtadsconfdialog.h"
#include "ui_qtadsconfdialog.h"
#include "qtadssettings.h"
#include "htmlqt.h"


QTadsConfDialog::QTadsConfDialog( CHtmlSysWinGroupQt* parent )
  : QDialog(parent), ui(new Ui::QTadsConfDialog)
{
	ui->setupUi(this);
	QTadsSettings* sett = qFrame->settings();
	sett->loadFromDisk();

	const QList<int>& sizeList = QFontDatabase::standardSizes();
	for (int i = 0; i < sizeList.size(); ++i) {
		const QString& item = QString::number(sizeList.at(i));
		ui->mainFontSizeComboBox->addItem(item);
		ui->fixedFontSizeComboBox->addItem(item);
		ui->serifFontSizeComboBox->addItem(item);
		ui->sansFontSizeComboBox->addItem(item);
		ui->scriptFontSizeComboBox->addItem(item);
		ui->writerFontSizeComboBox->addItem(item);
		ui->inputFontSizeComboBox->addItem(item);
	}

	ui->mainFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->mainFont.pointSize()));
	ui->fixedFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->fixedFont.pointSize()));
	ui->serifFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->serifFont.pointSize()));
	ui->sansFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->sansFont.pointSize()));
	ui->scriptFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->scriptFont.pointSize()));
	ui->writerFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->writerFont.pointSize()));
	ui->inputFontSizeComboBox->setCurrentIndex(sizeList.indexOf(sett->inputFont.pointSize()));

	ui->mainFontBox->setCurrentFont(sett->mainFont);
	ui->fixedFontBox->setCurrentFont(sett->fixedFont);
	ui->serifFontBox->setCurrentFont(sett->serifFont);
	ui->sansFontBox->setCurrentFont(sett->sansFont);
	ui->scriptFontBox->setCurrentFont(sett->scriptFont);
	ui->writerFontBox->setCurrentFont(sett->writerFont);
	ui->inputFontBox->setCurrentFont(sett->inputFont);
	ui->inputFontItalicCheckBox->setChecked(sett->inputFont.italic());
	ui->inputFontBoldCheckBox->setChecked(sett->inputFont.bold());

	connect(this, SIGNAL(accepted()), this, SLOT(applySettings()));
}


QTadsConfDialog::~QTadsConfDialog()
{
	delete ui;
}


void
QTadsConfDialog::applySettings()
{
	QTadsSettings* sett = qFrame->settings();

	sett->mainFont = ui->mainFontBox->currentFont();
	sett->fixedFont = ui->fixedFontBox->currentFont();
	sett->serifFont = ui->serifFontBox->currentFont();
	sett->sansFont = ui->sansFontBox->currentFont();
	sett->scriptFont = ui->scriptFontBox->currentFont();
	sett->writerFont = ui->writerFontBox->currentFont();
	sett->inputFont = ui->inputFontBox->currentFont();
	sett->inputFont.setBold(ui->inputFontBoldCheckBox->isChecked());
	sett->inputFont.setItalic(ui->inputFontItalicCheckBox->isChecked());

	sett->mainFont.setPointSize(ui->mainFontSizeComboBox->currentText().toInt());
	sett->fixedFont.setPointSize(ui->fixedFontSizeComboBox->currentText().toInt());
	sett->serifFont.setPointSize(ui->serifFontSizeComboBox->currentText().toInt());
	sett->sansFont.setPointSize(ui->sansFontSizeComboBox->currentText().toInt());
	sett->scriptFont.setPointSize(ui->scriptFontSizeComboBox->currentText().toInt());
	sett->writerFont.setPointSize(ui->writerFontSizeComboBox->currentText().toInt());
	sett->inputFont.setPointSize(ui->inputFontSizeComboBox->currentText().toInt());

	// Change the text cursor's height according to the new input font's height.
	qFrame->gameWindow()->setCursorHeight(QFontMetrics(sett->inputFont).height());

	qFrame->reformatBanners();

	sett->saveToDisk();
}


void QTadsConfDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
