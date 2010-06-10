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
#include <QPushButton>

#include "qtadsconfdialog.h"
#include "ui_qtadsconfdialog.h"
#include "globals.h"
#include "qtadssettings.h"
#include "sysframe.h"
#include "syswingroup.h"


QTadsConfDialog::QTadsConfDialog( CHtmlSysWinGroupQt* parent )
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), ui(new Ui::QTadsConfDialog),
	fInstantApply(false)
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
	this->fTmpInputColor = qFrame->settings()->inputColor;
	ui->inputColorButton->setStyleSheet(buttonStyle + this->fTmpInputColor.name() + "}");
	this->fTmpUnvisitedLinkColor = qFrame->settings()->unvisitedLinkColor;
	ui->linkUnvisitedColorButton->setStyleSheet(buttonStyle + this->fTmpUnvisitedLinkColor.name() + "}");
	this->fTmpHoveringLinkColor = qFrame->settings()->hoveringLinkColor;
	ui->linkHoveringColorButton->setStyleSheet(buttonStyle + this->fTmpHoveringLinkColor.name() + "}");
	this->fTmpClickedLinkColor = qFrame->settings()->clickedLinkColor;
	ui->linkClickedColorButton->setStyleSheet(buttonStyle + this->fTmpClickedLinkColor.name() + "}");

	ui->underlineLinksCheckBox->setChecked(sett->underlineLinks);
	ui->highlightLinksCheckBox->setChecked(sett->highlightLinks);

	ui->mainFontSizeSpinBox->setValue(sett->mainFont.pointSize());
	ui->fixedFontSizeSpinBox->setValue(sett->fixedFont.pointSize());
	ui->serifFontSizeSpinBox->setValue(sett->serifFont.pointSize());
	ui->sansFontSizeSpinBox->setValue(sett->sansFont.pointSize());
	ui->scriptFontSizeSpinBox->setValue(sett->scriptFont.pointSize());
	ui->writerFontSizeSpinBox->setValue(sett->writerFont.pointSize());
	ui->inputFontSizeSpinBox->setValue(sett->inputFont.pointSize());

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
	sigMapper->setMapping(ui->inputColorButton, 8);
	connect(ui->mainTextColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->mainBgColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->bannerTextColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->bannerBgColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->inputColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkUnvisitedColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkHoveringColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(ui->linkClickedColorButton, SIGNAL(clicked()), sigMapper, SLOT(map()));
	connect(sigMapper, SIGNAL(mapped(int)), this, SLOT(fSelectColor(int)));

#ifdef Q_WS_MAC
	// On Mac OS X, the dialog should not have any buttons, and settings
	// changes should apply instantly.
	this->fMakeInstantApply();
	ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
	QDialogButtonBox::ButtonLayout layoutPolicy =
			QDialogButtonBox::ButtonLayout(ui->buttonBox->style()->styleHint(QStyle::SH_DialogButtonLayout));
	if (layoutPolicy == QDialogButtonBox::GnomeLayout) {
		// On Gnome (and other Gtk-based environments, like XFCE), we follow
		// Gnome standards. We only provide a "Close" button and settings
		// changes should apply instantly.
		this->fMakeInstantApply();
		ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
	} else {
		// Assume KDE/MS Windows standards. No instant apply, and use OK/Apply/Cancel
		// buttons.
		ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
		connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(fApplySettings()));
		connect(this, SIGNAL(accepted()), this, SLOT(fApplySettings()));
	}
#endif
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
QTadsConfDialog::fMakeInstantApply()
{
	this->fInstantApply = true;

	connect(ui->mainFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->fixedFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->serifFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->sansFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->scriptFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->writerFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->inputFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
	connect(ui->mainFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->fixedFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->serifFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->sansFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->scriptFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->writerFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->inputFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fApplySettings()));
	connect(ui->inputFontItalicCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->inputFontBoldCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->underlineLinksCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->highlightLinksCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->allowGraphicsCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->allowDigitalCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->allowMidiCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
	connect(ui->allowLinksCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
}


void
QTadsConfDialog::fApplySettings()
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
	sett->inputColor = this->fTmpInputColor;
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

	sett->mainFont.setPointSize(ui->mainFontSizeSpinBox->value());
	sett->fixedFont.setPointSize(ui->fixedFontSizeSpinBox->value());
	sett->serifFont.setPointSize(ui->serifFontSizeSpinBox->value());
	sett->sansFont.setPointSize(ui->sansFontSizeSpinBox->value());
	sett->scriptFont.setPointSize(ui->scriptFontSizeSpinBox->value());
	sett->writerFont.setPointSize(ui->writerFontSizeSpinBox->value());
	sett->inputFont.setPointSize(ui->inputFontSizeSpinBox->value());

	// Notify the application that preferences have changed.
	qFrame->notifyPreferencesChange(sett);

	sett->saveToDisk();
}


void
QTadsConfDialog::fSelectColor( int i )
{
	QToolButton* button;
	QColor* tmpColor;
	QColor newColor;

	switch (i) {
	  case 1:
		button = ui->mainTextColorButton;
		tmpColor = &this->fTmpMainTextColor;
		newColor = this->fTmpMainTextColor;
		break;
	  case 2:
		button = ui->mainBgColorButton;
		tmpColor = &this->fTmpMainBgColor;
		newColor = this->fTmpMainBgColor;
		break;
	  case 3:
		button = ui->bannerTextColorButton;
		tmpColor = &this->fTmpBannerTextColor;
		newColor = this->fTmpBannerTextColor;
		break;
	  case 4:
		button = ui->bannerBgColorButton;
		tmpColor = &this->fTmpBannerBgColor;
		newColor = this->fTmpBannerBgColor;
		break;
	  case 5:
		button = ui->linkUnvisitedColorButton;
		tmpColor = &this->fTmpUnvisitedLinkColor;
		newColor = this->fTmpUnvisitedLinkColor;
		break;
	  case 6:
		button = ui->linkHoveringColorButton;
		tmpColor = &this->fTmpHoveringLinkColor;
		newColor = this->fTmpHoveringLinkColor;
		break;
	  case 7:
		button = ui->linkClickedColorButton;
		tmpColor = &this->fTmpClickedLinkColor;
		newColor = this->fTmpClickedLinkColor;
		break;
	  case 8:
		button = ui->inputColorButton;
		tmpColor = &this->fTmpInputColor;
		newColor = this->fTmpInputColor;
		break;
	}

	newColor = QColorDialog::getColor(newColor);
	if (not newColor.isValid()) {
		// User canceled the dialog.
		return;
	}

	*tmpColor = newColor;
	button->setStyleSheet(QByteArray("*{background:") + newColor.name() + "}");

	if (this->fInstantApply) {
		this->fApplySettings();
	}
}
