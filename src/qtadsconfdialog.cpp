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

#include <QColorDialog>
#include <QSignalMapper>

#include "qtadsconfdialog.h"
#include "ui_qtadsconfdialog.h"
#include "qtadssettings.h"
#include "htmlqt.h"


QTadsConfDialog::QTadsConfDialog( CHtmlSysWinGroupQt* parent )
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), ui(new Ui::QTadsConfDialog)
{
	ui->setupUi(this);
	QTadsSettings* sett = qFrame->settings();
	sett->loadFromDisk();

	ui->allowGraphicsCheckBox->setChecked(sett->enableGraphics);
	ui->allowDigitalCheckBox->setChecked(sett->enableDigitalSound);
	ui->allowMidiCheckBox->setChecked(sett->enableMidiSound);
	ui->allowLinksCheckBox->setChecked(sett->enableLinks);

	const QByteArray buttonStyle("*{background:");
	this->fTmpMainBgColor = qFrame->settings()->mainBgColor;
	ui->mainBgColorButton->setStyleSheet(buttonStyle + this->fTmpMainBgColor.name() + "}");
	this->fTmpMainTextColor = qFrame->settings()->mainTextColor;
	ui->mainTextColorButton->setStyleSheet(buttonStyle + this->fTmpMainTextColor.name() + "}");
	this->fTmpBannerBgColor = qFrame->settings()->bannerBgColor;
	ui->bannerBgColorButton->setStyleSheet(buttonStyle + this->fTmpBannerBgColor.name() + "}");
	this->fTmpBannerTextColor = qFrame->settings()->bannerTextColor;
	ui->bannerTextColorButton->setStyleSheet(buttonStyle + this->fTmpBannerTextColor.name() + "}");
	this->fTmpUnvisitedLinkColor = qFrame->settings()->unvisitedLinkColor;
	ui->linkUnvisitedColorButton->setStyleSheet(buttonStyle + this->fTmpUnvisitedLinkColor.name() + "}");
	this->fTmpHoveringLinkColor = qFrame->settings()->hoveringLinkColor;
	ui->linkHoveringColorButton->setStyleSheet(buttonStyle + this->fTmpHoveringLinkColor.name() + "}");
	this->fTmpClickedLinkColor = qFrame->settings()->clickedLinkColor;
	ui->linkClickedColorButton->setStyleSheet(buttonStyle + this->fTmpClickedLinkColor.name() + "}");

	ui->underlineLinksCheckBox->setChecked(sett->underlineLinks);
	ui->highlightLinksCheckBox->setChecked(sett->highlightLinks);

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

	QSignalMapper* sigMapper = new QSignalMapper(this);
	sigMapper->setMapping(ui->mainTextColorButton, 1);
	sigMapper->setMapping(ui->mainBgColorButton, 2);
	sigMapper->setMapping(ui->bannerTextColorButton, 3);
	sigMapper->setMapping(ui->bannerBgColorButton, 4);
	sigMapper->setMapping(ui->linkUnvisitedColorButton, 5);
	sigMapper->setMapping(ui->linkHoveringColorButton, 6);
	sigMapper->setMapping(ui->linkClickedColorButton, 7);
	connect(ui->mainTextColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->mainBgColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->bannerTextColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->bannerBgColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkUnvisitedColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkHoveringColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkClickedColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(sigMapper, SIGNAL(mapped(int)), this, SLOT(selectColor(int)));

	connect(this, SIGNAL(accepted()), this, SLOT(applySettings()));
}


QTadsConfDialog::~QTadsConfDialog()
{
	delete ui;
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


void
QTadsConfDialog::applySettings()
{
	QTadsSettings* sett = qFrame->settings();

	sett->enableGraphics = ui->allowGraphicsCheckBox->isChecked();
	sett->enableDigitalSound = ui->allowDigitalCheckBox->isChecked();
	sett->enableMidiSound = ui->allowMidiCheckBox->isChecked();
	sett->enableLinks = ui->allowLinksCheckBox->isChecked();

	sett->mainBgColor = this->fTmpMainBgColor;
	sett->mainTextColor = this->fTmpMainTextColor;
	sett->bannerBgColor = this->fTmpBannerBgColor;
	sett->bannerTextColor = this->fTmpBannerTextColor;
	sett->unvisitedLinkColor = this->fTmpUnvisitedLinkColor;
	sett->hoveringLinkColor = this->fTmpHoveringLinkColor;
	sett->clickedLinkColor = this->fTmpClickedLinkColor;
	sett->underlineLinks = ui->underlineLinksCheckBox->isChecked();
	sett->highlightLinks = ui->highlightLinksCheckBox->isChecked();

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

	// Notify the application that preferences have changed.
	qFrame->notifyPreferencesChange(sett);

	sett->saveToDisk();
}


void
QTadsConfDialog::selectColor( int i )
{
	QColor res(QColorDialog::getColor());
	if (not res.isValid()) {
		// User canceled the dialog.
		return;
	}

	QToolButton* button;
	QColor* tmpColor;

	switch (i) {
	  case 1:
		button = ui->mainTextColorButton;
		tmpColor = &this->fTmpMainTextColor;
		break;
	  case 2:
		button = ui->mainBgColorButton;
		tmpColor = &this->fTmpMainBgColor;
		break;
	  case 3:
		button = ui->bannerTextColorButton;
		tmpColor = &this->fTmpBannerTextColor;
		break;
	  case 4:
		button = ui->bannerBgColorButton;
		tmpColor = &this->fTmpBannerBgColor;
		break;
	  case 5:
		button = ui->linkUnvisitedColorButton;
		tmpColor = &this->fTmpUnvisitedLinkColor;
		break;
	  case 6:
		button = ui->linkHoveringColorButton;
		tmpColor = &this->fTmpHoveringLinkColor;
		break;
	  case 7:
		button = ui->linkClickedColorButton;
		tmpColor = &this->fTmpClickedLinkColor;
		break;
	}

	*tmpColor = res;
	button->setStyleSheet(QByteArray("*{background:") + res.name() + "}");
}
