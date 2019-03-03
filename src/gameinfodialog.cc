// This is copyrighted software. More information is at the end of this file.
#include "gameinfodialog.h"
#include "ui_gameinfodialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "globals.h"
#include "htmlfmt.h"
#include "htmlrf.h"
#include "settings.h"
#include "sysframe.h"
#include "syswininput.h"

void QTadsGameInfoEnum::tads_enum_game_info(const char* name, const char* val)
{
    const QString& valStr = QString::fromUtf8(val);
    const QString& nameStr = QString::fromUtf8(name).toLower();
    const QString& htmlValStr = valStr.toHtmlEscaped();

    if (nameStr == QString::fromLatin1("name")) {
        gameName = QString::fromLatin1("<b><center><font size=\"+1\">") + htmlValStr
                   + QString::fromLatin1("</font></center></b><p>");
        plainGameName = valStr;
    } else if (nameStr == QString::fromLatin1("headline")) {
        headline =
            QString::fromLatin1("<center>") + htmlValStr + QString::fromLatin1("</center><p>");
    } else if (nameStr == QString::fromLatin1("byline")) {
        byLine = QString::fromLatin1("<i><center>") + htmlValStr
                 + QString::fromLatin1("</center></i><p>");
    } else if (nameStr == QString::fromLatin1("htmlbyline")) {
        htmlByLine =
            QString::fromLatin1("<i><center>") + valStr + QString::fromLatin1("</center></i><p>");
    } else if (nameStr == QString::fromLatin1("authoremail")) {
        email = valStr;
    } else if (nameStr == QString::fromLatin1("desc")) {
        desc = htmlValStr;
        desc.replace(QString::fromLatin1("\\n"), QString::fromLatin1("<p>"));
    } else if (nameStr == QString::fromLatin1("htmldesc")) {
        htmlDesc = valStr;
    } else if (nameStr == QString::fromLatin1("version")) {
        version = valStr;
    } else if (nameStr == QString::fromLatin1("firstpublished")) {
        published = valStr;
    } else if (nameStr == QString::fromLatin1("releasedate")) {
        date = valStr;
    } else if (nameStr == QString::fromLatin1("language")) {
        lang = valStr;
    } else if (nameStr == QString::fromLatin1("series")) {
        series = valStr;
    } else if (nameStr == QString::fromLatin1("seriesnumber")) {
        seriesNumber = valStr;
    } else if (nameStr == QString::fromLatin1("genre")) {
        genre = valStr;
    } else if (nameStr == QString::fromLatin1("forgiveness")) {
        forgiveness = valStr;
    } else if (nameStr == QString::fromLatin1("licensetype")) {
        license = valStr;
    } else if (nameStr == QString::fromLatin1("copyingrules")) {
        copyRules = valStr;
    } else if (nameStr == QString::fromLatin1("ifid")) {
        ifid = valStr;
    }
}

static void insertTableRow(QTableWidget* table, const QString& text1, const QString& text2)
{
    table->insertRow(table->rowCount());
    QTableWidgetItem* item = new QTableWidgetItem(text1);
    item->setFlags(Qt::ItemIsEnabled);
    table->setItem(table->rowCount() - 1, 0, item);
    item = new QTableWidgetItem(text2);
    item->setFlags(Qt::ItemIsEnabled);
    table->setItem(table->rowCount() - 1, 1, item);
}

static QImage loadCoverArtImage()
{
    CHtmlResFinder* resFinder = qFrame->gameWindow()->get_formatter()->get_res_finder();

    // Look for a cover art resource. We try four different resource names.
    // The first two (PNG and JPG with a ".system/" prefix) are defined in the
    // current cover art standard.  The other two were defined in the older
    // standard which did not use a prefix.
    QByteArray coverArtResName;
    bool coverArtFound = false;
    const char coverArtPaths[][21] = {".system/CoverArt.png", ".system/CoverArt.jpg",
                                      "CoverArt.png", "CoverArt.jpg"};
    for (int i = 0; i < 4 and not coverArtFound; ++i) {
        coverArtResName = coverArtPaths[i];
        if (resFinder->resfile_exists(coverArtResName.constData(), coverArtResName.length())) {
            coverArtFound = true;
        }
    }

    if (not coverArtFound) {
        return QImage();
    }

    CStringBuf strBuf;
    unsigned long offset;
    unsigned long size;
    resFinder->get_file_info(&strBuf, coverArtResName.constData(), coverArtResName.length(),
                             &offset, &size);

    // Check if the file exists and is readable.
    QFileInfo inf(fnameToQStr(strBuf.get()));
    if (not inf.exists() or not inf.isReadable()) {
        qWarning() << "ERROR:" << inf.filePath() << "doesn't exist or is unreadable";
        return QImage();
    }

    // Open the file and seek to the specified position.
    QFile file(inf.filePath());
    if (not file.open(QIODevice::ReadOnly)) {
        qWarning() << "ERROR: Can't open file" << inf.filePath();
        return QImage();
    }
    if (not file.seek(offset)) {
        qWarning() << "ERROR: Can't seek in file" << inf.filePath();
        file.close();
        return QImage();
    }

    // Load the image data.
    const QByteArray& data(file.read(size));
    file.close();
    if (data.isEmpty() or static_cast<unsigned long>(data.size()) < size) {
        qWarning() << "ERROR: Could not read" << size << "bytes from file" << inf.filePath();
        return QImage();
    }
    QImage image;
    if (not image.loadFromData(data)) {
        qWarning() << "ERROR: Could not parse image data";
        return QImage();
    }

    // If we got here, all went well.  Return the image scaled to a
    // 200 pixels width if it's too large.  Otherwise, return it as-is.
    if (image.width() > 200) {
        Qt::TransformationMode mode = qFrame->settings()->useSmoothScaling
                                          ? Qt::SmoothTransformation
                                          : Qt::FastTransformation;
        return image.scaledToWidth(200, mode);
    } else {
        return image;
    }
}

GameInfoDialog::GameInfoDialog(const QByteArray& fname, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::GameInfoDialog)
{
    ui->setupUi(this);

    CTadsGameInfo info;
    QTadsGameInfoEnum cb;

    // Try to load the cover art.  If there is one, insert it into the text
    // browser as the "CoverArt" resource.
    const QImage& image = loadCoverArtImage();
    if (not image.isNull()) {
        ui->description->document()->addResource(QTextDocument::ImageResource,
                                                 QUrl(QString::fromLatin1("CoverArt")), image);
        resize(width(), height() + image.height());
    }

    info.read_from_file(fname.constData());
    info.enum_values(&cb);

    // Fill out the description.
    QString tmp;
    if (not image.isNull()) {
        tmp += QString::fromLatin1("<center><img src=\"CoverArt\"></center><p>");
    }
    tmp += cb.gameName + cb.headline + (cb.htmlByLine.isEmpty() ? cb.byLine : cb.htmlByLine)
           + (cb.htmlDesc.isEmpty() ? cb.desc : cb.htmlDesc);
    ui->description->setHtml(tmp);

    // Fill out the table.
    ui->table->setColumnCount(2);
    if (not cb.genre.isEmpty()) {
        insertTableRow(ui->table, tr("Genre"), cb.genre);
    }

    if (not cb.version.isEmpty()) {
        insertTableRow(ui->table, tr("Version"), cb.version);
    }

    if (not cb.forgiveness.isEmpty()) {
        insertTableRow(ui->table, tr("Forgiveness"), cb.forgiveness);
    }

    if (not cb.series.isEmpty()) {
        insertTableRow(ui->table, tr("Series"), cb.series);
    }

    if (not cb.seriesNumber.isEmpty()) {
        insertTableRow(ui->table, tr("Series Number"), cb.seriesNumber);
    }

    if (not cb.date.isEmpty()) {
        insertTableRow(ui->table, tr("Date"), cb.date);
    }

    if (not cb.published.isEmpty()) {
        insertTableRow(ui->table, tr("First Published"), cb.published);
    }

    if (not cb.email.isEmpty()) {
        insertTableRow(ui->table, tr("Author email"), cb.email);
    }

    if (not cb.lang.isEmpty()) {
        insertTableRow(ui->table, tr("Language"), cb.lang);
    }

    if (not cb.license.isEmpty()) {
        insertTableRow(ui->table, tr("License Type"), cb.license);
    }

    if (not cb.copyRules.isEmpty()) {
        insertTableRow(ui->table, tr("Copying Rules"), cb.copyRules);
    }

    if (not cb.ifid.isEmpty()) {
        insertTableRow(ui->table, tr("IFID"), cb.ifid);
    }

    ui->table->resizeColumnsToContents();
    ui->table->resizeRowsToContents();
    ui->table->setShowGrid(false);
    // There's no point in having the tableview widget be any higher than the
    // sum of all rows.  This will also make sure that the widget is not shown
    // at all in case we have zero rows.
    int topMargin;
    int bottomMargin;
    ui->table->getContentsMargins(0, &topMargin, 0, &bottomMargin);
    int maxHeight = ui->table->rowCount() * ui->table->rowHeight(0);
    if (maxHeight > 0) {
        // Only add the margins to the maximum height if we're going to show
        // the table at all.
        maxHeight += topMargin + bottomMargin;
        if (maxHeight < ui->table->minimumSizeHint().height()) {
            // Do not make it smaller than the minimum size hint, otherwise we'll
            // have a messed-up scrollbar.
            maxHeight = ui->table->minimumSizeHint().height();
        }
    }
    ui->table->setMaximumHeight(maxHeight);
}

GameInfoDialog::~GameInfoDialog()
{
    delete ui;
}

bool GameInfoDialog::gameHasMetaInfo(const QByteArray& fname)
{
    CTadsGameInfo info;
    return info.read_from_file(fname.constData());
}

QTadsGameInfoEnum GameInfoDialog::getMetaInfo(const QByteArray& fname)
{
    CTadsGameInfo info;
    QTadsGameInfoEnum cb;
    info.read_from_file(fname.constData());
    info.enum_values(&cb);
    return cb;
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
