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

#include <QColorDialog>
#include <QSignalMapper>
#include <QPushButton>
#include <QCheckBox>
#include <QTextCodec>

#include "confdialog.h"
#include "ui_confdialog.h"
#include "globals.h"
#include "settings.h"
#include "sysframe.h"
#include "syswingroup.h"


ConfDialog::ConfDialog( CHtmlSysWinGroupQt* parent )
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
      ui(new Ui::ConfDialog)
{
    ui->setupUi(this);
    Settings* sett = qFrame->settings();
    sett->loadFromDisk();

#ifdef Q_OS_MAC
    // On the Mac, make the color selection buttons smaller so that they
    // become square instead of round.
    QSize macSize(48, 24);
    ui->mainBgColorButton->setFixedSize(macSize);
    ui->mainTextColorButton->setFixedSize(macSize);
    ui->bannerBgColorButton->setFixedSize(macSize);
    ui->bannerTextColorButton->setFixedSize(macSize);
    ui->inputColorButton->setFixedSize(macSize);
    ui->linkUnvisitedColorButton->setFixedSize(macSize);
    ui->linkHoveringColorButton->setFixedSize(macSize);
    ui->linkClickedColorButton->setFixedSize(macSize);
#endif

    ui->allowGraphicsCheckBox->setChecked(sett->enableGraphics);
#ifdef NO_AUDIO
    ui->allowSoundEffectsCheckBox->setDisabled(true);
    ui->allowSoundEffectsCheckBox->hide();
    ui->allowMusicCheckBox->setDisabled(true);
    ui->allowMusicCheckBox->hide();
#else
    ui->allowSoundEffectsCheckBox->setChecked(sett->enableSoundEffects);
    ui->allowMusicCheckBox->setChecked(sett->enableMusic);
#endif
    ui->allowLinksCheckBox->setChecked(sett->enableLinks);
    ui->smoothScalingCheckBox->setChecked(sett->useSmoothScaling);

    ui->mainBgColorButton->setColor(sett->mainBgColor);
    ui->mainTextColorButton->setColor(sett->mainTextColor);
    ui->bannerBgColorButton->setColor(sett->bannerBgColor);
    ui->bannerTextColorButton->setColor(sett->bannerTextColor);
    ui->inputColorButton->setColor(sett->inputColor);
    ui->linkUnvisitedColorButton->setColor(sett->unvisitedLinkColor);
    ui->linkHoveringColorButton->setColor(sett->hoveringLinkColor);
    ui->linkClickedColorButton->setColor(sett->clickedLinkColor);

    ui->underlineLinksCheckBox->setChecked(sett->underlineLinks);
    ui->highlightLinksCheckBox->setChecked(sett->highlightLinks);

    ui->mainFontSizeSpinBox->setValue(sett->mainFont.pointSize());
    ui->fixedFontSizeSpinBox->setValue(sett->fixedFont.pointSize());
    ui->serifFontSizeSpinBox->setValue(sett->serifFont.pointSize());
    ui->sansFontSizeSpinBox->setValue(sett->sansFont.pointSize());
    ui->scriptFontSizeSpinBox->setValue(sett->scriptFont.pointSize());
    ui->writerFontSizeSpinBox->setValue(sett->writerFont.pointSize());
    if (sett->useMainFontForInput) {
        ui->inputFontSizeSpinBox->setValue(sett->mainFont.pointSize());
        ui->inputFontSizeSpinBox->setEnabled(false);
    } else {
        ui->inputFontSizeSpinBox->setValue(sett->inputFont.pointSize());
    }

    ui->mainFontBox->setCurrentFont(sett->mainFont);
    ui->fixedFontBox->setCurrentFont(sett->fixedFont);
    ui->serifFontBox->setCurrentFont(sett->serifFont);
    ui->sansFontBox->setCurrentFont(sett->sansFont);
    ui->scriptFontBox->setCurrentFont(sett->scriptFont);
    ui->writerFontBox->setCurrentFont(sett->writerFont);
    if (sett->useMainFontForInput) {
        ui->inputFontBox->setCurrentFont(sett->mainFont);
        ui->inputFontBox->setEnabled(false);
    } else {
        ui->inputFontBox->setCurrentFont(sett->inputFont);
    }
    ui->useMainFontCheckBox->setChecked(sett->useMainFontForInput);
    connect(ui->useMainFontCheckBox, SIGNAL(toggled(bool)), ui->inputFontBox, SLOT(setDisabled(bool)));
    connect(ui->useMainFontCheckBox, SIGNAL(toggled(bool)), ui->inputFontSizeSpinBox, SLOT(setDisabled(bool)));
    ui->inputFontItalicCheckBox->setChecked(sett->inputFont.italic());
    ui->inputFontBoldCheckBox->setChecked(sett->inputFont.bold());

    switch (sett->ioSafetyLevelRead) {
      case 0: ui->safetyRead0RadioButton->setChecked(true); break;
      case 2: ui->safetyRead2RadioButton->setChecked(true); break;
      case 4: ui->safetyRead4RadioButton->setChecked(true); break;
      default: ui->safetyRead2RadioButton->setChecked(true); break;
    }
    switch (sett->ioSafetyLevelWrite) {
      case 0: ui->safetyWrite0RadioButton->setChecked(true); break;
      case 2: ui->safetyWrite2RadioButton->setChecked(true); break;
      case 4: ui->safetyWrite4RadioButton->setChecked(true); break;
      default: ui->safetyWrite2RadioButton->setChecked(true); break;
    }

    const QList<QByteArray>& aliases = QTextCodec::availableCodecs();
    QList<QByteArray> codecs;
    for (int i = 0; i < aliases.size(); ++i) {
        const QByteArray codecName = QTextCodec::codecForName(aliases.at(i))->name();
        // Only allow some of the possible sets, otherwise we would get a big
        // list with most of the encodings being irrelevant.  The only Unicode
        // encoding we allow is UTF-8, since it's a single-byte character set
        // and therefore can be used by TADS 2 games (though I'm not aware of
        // any that actually use UTF-8.)
        if (codecName == "UTF-8"
            or codecName.startsWith("windows-")
            or codecName.startsWith("ISO-")
            or codecName.startsWith("KOI8-")
            or codecName.startsWith("IBM")
            or codecName.startsWith("EUC-")
            or codecName.startsWith("jisx020")
            or codecName.startsWith("cp949"))
        {
            codecs.append(codecName);
        }
    }
    qSort(codecs);
    for (int i = 0; i < codecs.size(); ++i) {
        if (ui->encodingComboBox->findText(QString::fromLatin1(codecs.at(i))) == -1) {
            ui->encodingComboBox->addItem(QString::fromLatin1(codecs.at(i)));
        }
    }
    ui->encodingComboBox->setCurrentIndex(ui->encodingComboBox->findText(QString::fromLatin1(sett->tads2Encoding)));

    QString txt(QKeySequence(Qt::CTRL).toString(QKeySequence::NativeText));
    if (txt.endsWith(QChar::fromLatin1('+'))) {
        txt.truncate(txt.length() - 1);
    }
    ui->pasteOnDblClkCheckBox->setText(tr("%1 + double-click pastes current word").arg(txt));
    ui->pasteOnDblClkCheckBox->setChecked(sett->pasteOnDblClk);
    ui->softScrollCheckBox->setChecked(sett->softScrolling);
    ui->askForGameFileCheckBox->setChecked(sett->askForGameFile);
    ui->confirmRestartCheckBox->setChecked(sett->confirmRestartGame);
    ui->confirmQuitCheckBox->setChecked(sett->confirmQuitGame);

    switch (sett->updateFreq) {
      case Settings::UpdateOnEveryStart: ui->updateOnStartRadioButton->setChecked(true); break;
      case Settings::UpdateDaily: ui->updateDailyRadioButton->setChecked(true); break;
      case Settings::UpdateWeekly: ui->updateWeeklyRadioButton->setChecked(true); break;
      default: ui->updateNeverRadioButton->setChecked(true); break;
    }

#ifdef Q_OS_MAC
    // On Mac OS X, the dialog should not have any buttons, and settings
    // changes should apply instantly.
    fMakeInstantApply();
    ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
    QDialogButtonBox::ButtonLayout layoutPolicy
        = QDialogButtonBox::ButtonLayout(ui->buttonBox->style()->styleHint(QStyle::SH_DialogButtonLayout));
    if (layoutPolicy == QDialogButtonBox::GnomeLayout) {
        // On Gnome (and other Gtk-based environments, like XFCE), we follow
        // Gnome standards. We only provide a "Close" button and settings
        // changes should apply instantly.
        fMakeInstantApply();
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


ConfDialog::~ConfDialog()
{
    delete ui;
}


void ConfDialog::changeEvent(QEvent *e)
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
ConfDialog::fMakeInstantApply()
{
    connect(ui->mainFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->fixedFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->serifFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->sansFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->scriptFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->writerFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->inputFontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(fApplySettings()));
    connect(ui->useMainFontCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));

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
#ifndef NO_AUDIO
    connect(ui->allowSoundEffectsCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->allowMusicCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
#endif
    connect(ui->allowLinksCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->smoothScalingCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));

    connect(ui->mainTextColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->mainBgColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->bannerTextColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->bannerBgColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->inputColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->linkUnvisitedColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->linkHoveringColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));
    connect(ui->linkClickedColorButton, SIGNAL(changed(QColor)), this, SLOT(fApplySettings()));

    // Because these are radio buttons, we connect the clicked() instead of
    // the toggled() signal.  The toggled() signal would result in the slot
    // getting called twice, once for the button that gets unchecked and once
    // for the button that gets checked.  The clicked() signal is only emitted
    // when a button gets checked.
    connect(ui->safetyRead0RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->safetyRead2RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->safetyRead4RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->safetyWrite0RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->safetyWrite2RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->safetyWrite4RadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->updateOnStartRadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->updateDailyRadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->updateWeeklyRadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));
    connect(ui->updateNeverRadioButton, SIGNAL(clicked()), this, SLOT(fApplySettings()));

    connect(ui->encodingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fApplySettings()));
    connect(ui->pasteOnDblClkCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->softScrollCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->askForGameFileCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->confirmRestartCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
    connect(ui->confirmQuitCheckBox, SIGNAL(toggled(bool)), this, SLOT(fApplySettings()));
}


void
ConfDialog::fApplySettings()
{
    if (ui->useMainFontCheckBox->isChecked()) {
        ui->inputFontBox->setCurrentFont(ui->mainFontBox->currentFont());
        ui->inputFontSizeSpinBox->setValue(ui->mainFontSizeSpinBox->value());
    }

    Settings* sett = qFrame->settings();

    sett->enableGraphics = ui->allowGraphicsCheckBox->isChecked();
#ifndef NO_AUDIO
    sett->enableSoundEffects = ui->allowSoundEffectsCheckBox->isChecked();
    sett->enableMusic = ui->allowMusicCheckBox->isChecked();
#endif
    sett->enableLinks = ui->allowLinksCheckBox->isChecked();
    sett->useSmoothScaling = ui->smoothScalingCheckBox->isChecked();

    sett->mainBgColor = ui->mainBgColorButton->color();
    sett->mainTextColor = ui->mainTextColorButton->color();
    sett->bannerBgColor = ui->bannerBgColorButton->color();
    sett->bannerTextColor = ui->bannerTextColorButton->color();
    sett->inputColor = ui->inputColorButton->color();
    sett->unvisitedLinkColor = ui->linkUnvisitedColorButton->color();
    sett->hoveringLinkColor = ui->linkHoveringColorButton->color();
    sett->clickedLinkColor = ui->linkClickedColorButton->color();

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
    sett->useMainFontForInput = ui->useMainFontCheckBox->isChecked();

    if (ui->safetyRead0RadioButton->isChecked()) {
        sett->ioSafetyLevelRead = 0;
    } else if (ui->safetyRead2RadioButton->isChecked()) {
        sett->ioSafetyLevelRead = 2;
    } else if (ui->safetyRead4RadioButton->isChecked()) {
        sett->ioSafetyLevelRead = 4;
    }
    if (ui->safetyWrite0RadioButton->isChecked()) {
        sett->ioSafetyLevelWrite = 0;
    } else if (ui->safetyWrite2RadioButton->isChecked()) {
        sett->ioSafetyLevelWrite = 2;
    } else if (ui->safetyWrite4RadioButton->isChecked()) {
        sett->ioSafetyLevelWrite = 4;
    }
    sett->tads2Encoding = ui->encodingComboBox->currentText().toLatin1();
    sett->pasteOnDblClk = ui->pasteOnDblClkCheckBox->isChecked();
    sett->softScrolling = ui->softScrollCheckBox->isChecked();
    sett->askForGameFile = ui->askForGameFileCheckBox->isChecked();
    sett->confirmRestartGame = ui->confirmRestartCheckBox->isChecked();
    sett->confirmQuitGame = ui->confirmQuitCheckBox->isChecked();
    if (ui->updateOnStartRadioButton->isChecked()) {
        sett->updateFreq = Settings::UpdateOnEveryStart;
    } else if (ui->updateDailyRadioButton->isChecked()) {
        sett->updateFreq = Settings::UpdateDaily;
    } else if (ui->updateWeeklyRadioButton->isChecked()) {
        sett->updateFreq = Settings::UpdateWeekly;
    } else {
        sett->updateFreq = Settings::UpdateNever;
    }

    // Notify the application that preferences have changed.
    qFrame->notifyPreferencesChange(sett);

    sett->saveToDisk();
}
