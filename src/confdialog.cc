// This is copyrighted software. More information is at the end of this file.
#include <QCheckBox>
#include <QColorDialog>
#include <QPushButton>
#include <QSignalMapper>
#include <QTextCodec>
#include <algorithm>

#include "confdialog.h"
#include "globals.h"
#include "settings.h"
#include "sysframe.h"
#include "syswingroup.h"
#include "ui_confdialog.h"
#include "util.h"

ConfDialog::ConfDialog(CHtmlSysWinGroupQt* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint
#ifdef Q_OS_MAC
                          | Qt::Tool
#endif
              )
    , ui(new Ui::ConfDialog)
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
    connect(ui->useMainFontCheckBox, &QAbstractButton::toggled, ui->inputFontBox,
            &QWidget::setDisabled);
    connect(ui->useMainFontCheckBox, &QAbstractButton::toggled, ui->inputFontSizeSpinBox,
            &QWidget::setDisabled);
    ui->inputFontItalicCheckBox->setChecked(sett->inputFont.italic());
    ui->inputFontBoldCheckBox->setChecked(sett->inputFont.bold());

    switch (sett->ioSafetyLevelRead) {
    case 0:
        ui->safetyRead0RadioButton->setChecked(true);
        break;
    case 2:
        ui->safetyRead2RadioButton->setChecked(true);
        break;
    case 4:
        ui->safetyRead4RadioButton->setChecked(true);
        break;
    default:
        ui->safetyRead2RadioButton->setChecked(true);
        break;
    }
    switch (sett->ioSafetyLevelWrite) {
    case 0:
        ui->safetyWrite0RadioButton->setChecked(true);
        break;
    case 2:
        ui->safetyWrite2RadioButton->setChecked(true);
        break;
    case 4:
        ui->safetyWrite4RadioButton->setChecked(true);
        break;
    default:
        ui->safetyWrite2RadioButton->setChecked(true);
        break;
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
        if (codecName == "UTF-8" or codecName.startsWith("windows-") or codecName.startsWith("ISO-")
            or codecName.startsWith("KOI8-") or codecName.startsWith("IBM")
            or codecName.startsWith("EUC-") or codecName.startsWith("jisx020")
            or codecName.startsWith("cp949")) {
            codecs.append(codecName);
        }
    }
    std::sort(codecs.begin(), codecs.end());
    for (int i = 0; i < codecs.size(); ++i) {
        if (ui->encodingComboBox->findText(QString::fromLatin1(codecs.at(i))) == -1) {
            ui->encodingComboBox->addItem(QString::fromLatin1(codecs.at(i)));
        }
    }
    ui->encodingComboBox->setCurrentIndex(
        ui->encodingComboBox->findText(QString::fromLatin1(sett->tads2Encoding)));

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
    if (sett->textWidth > 0) {
        ui->limitWidthCheckBox->setChecked(true);
        ui->limitWidthSpinBox->setValue(sett->textWidth);
    } else {
        ui->limitWidthCheckBox->setChecked(false);
        ui->limitWidthSpinBox->setEnabled(false);
    }
    connect(ui->limitWidthCheckBox, &QCheckBox::toggled, ui->limitWidthSpinBox,
            &QSpinBox::setEnabled);

    switch (sett->updateFreq) {
    case Settings::UpdateOnEveryStart:
        ui->updateOnStartRadioButton->setChecked(true);
        break;
    case Settings::UpdateDaily:
        ui->updateDailyRadioButton->setChecked(true);
        break;
    case Settings::UpdateWeekly:
        ui->updateWeeklyRadioButton->setChecked(true);
        break;
    default:
        ui->updateNeverRadioButton->setChecked(true);
        break;
    }

#ifdef Q_OS_MAC
    // On Mac OS X, the dialog should not have any buttons, and settings
    // changes should apply instantly.
    fMakeInstantApply();
    ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
    QDialogButtonBox::ButtonLayout layoutPolicy = QDialogButtonBox::ButtonLayout(
        ui->buttonBox->style()->styleHint(QStyle::SH_DialogButtonLayout));
    if (layoutPolicy == QDialogButtonBox::GnomeLayout) {
        // On Gnome (and other Gtk-based environments, like XFCE), we follow
        // Gnome standards. We only provide a "Close" button and settings
        // changes should apply instantly.
        fMakeInstantApply();
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
    } else {
        // Assume KDE/MS Windows standards. No instant apply, and use OK/Apply/Cancel
        // buttons.
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply
                                          | QDialogButtonBox::Cancel);
        connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this,
                &ConfDialog::fApplySettings);
        connect(this, &QDialog::accepted, this, &ConfDialog::fApplySettings);
    }
#endif
}

ConfDialog::~ConfDialog()
{
    delete ui;
}

void ConfDialog::changeEvent(QEvent* e)
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

void ConfDialog::fMakeInstantApply()
{
    connect(ui->mainFontBox, &QFontComboBox::currentFontChanged, this, &ConfDialog::fApplySettings);
    connect(ui->fixedFontBox, &QFontComboBox::currentFontChanged, this,
            &ConfDialog::fApplySettings);
    connect(ui->serifFontBox, &QFontComboBox::currentFontChanged, this,
            &ConfDialog::fApplySettings);
    connect(ui->sansFontBox, &QFontComboBox::currentFontChanged, this, &ConfDialog::fApplySettings);
    connect(ui->scriptFontBox, &QFontComboBox::currentFontChanged, this,
            &ConfDialog::fApplySettings);
    connect(ui->writerFontBox, &QFontComboBox::currentFontChanged, this,
            &ConfDialog::fApplySettings);
    connect(ui->inputFontBox, &QFontComboBox::currentFontChanged, this,
            &ConfDialog::fApplySettings);
    connect(ui->useMainFontCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);

    connect(ui->mainFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->fixedFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->serifFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->sansFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->scriptFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->writerFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->inputFontSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);

    connect(ui->inputFontItalicCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->inputFontBoldCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->underlineLinksCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->highlightLinksCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->allowGraphicsCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
#ifndef NO_AUDIO
    connect(ui->allowSoundEffectsCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->allowMusicCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);
#endif
    connect(ui->allowLinksCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);
    connect(ui->smoothScalingCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);

    connect(ui->mainTextColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->mainBgColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->bannerTextColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->bannerBgColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->inputColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->linkUnvisitedColorButton, &KColorButton::changed, this,
            &ConfDialog::fApplySettings);
    connect(ui->linkHoveringColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);
    connect(ui->linkClickedColorButton, &KColorButton::changed, this, &ConfDialog::fApplySettings);

    // Because these are radio buttons, we connect the clicked() instead of
    // the toggled() signal.  The toggled() signal would result in the slot
    // getting called twice, once for the button that gets unchecked and once
    // for the button that gets checked.  The clicked() signal is only emitted
    // when a button gets checked.
    connect(ui->safetyRead0RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->safetyRead2RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->safetyRead4RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->safetyWrite0RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->safetyWrite2RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->safetyWrite4RadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->updateOnStartRadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->updateDailyRadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->updateWeeklyRadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);
    connect(ui->updateNeverRadioButton, &QAbstractButton::clicked, this,
            &ConfDialog::fApplySettings);

    connect(ui->encodingComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &ConfDialog::fApplySettings);
    connect(ui->pasteOnDblClkCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->softScrollCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);
    connect(ui->askForGameFileCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->confirmRestartCheckBox, &QAbstractButton::toggled, this,
            &ConfDialog::fApplySettings);
    connect(ui->confirmQuitCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);
    connect(ui->limitWidthCheckBox, &QAbstractButton::toggled, this, &ConfDialog::fApplySettings);
    connect(ui->limitWidthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &ConfDialog::fApplySettings);
}

void ConfDialog::fApplySettings()
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
    if (ui->limitWidthCheckBox->isChecked()) {
        sett->textWidth = ui->limitWidthSpinBox->value();
    } else {
        sett->textWidth = 0;
    }
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
