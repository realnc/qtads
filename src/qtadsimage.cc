// This is copyrighted software. More information is at the end of this file.
#include "qtadsimage.h"

#include "settings.h"
#include "sysimagejpeg.h"
#include "sysimagemng.h"
#include "sysimagepng.h"
#include "syswin.h"
#include <QBuffer>
#include <QFileInfo>
#include <QPainter>

void QTadsImage::drawFromPaintEvent(
    class CHtmlSysWin* win, class CHtmlRect* pos, htmlimg_draw_mode_t mode)
{
    QPainter painter(static_cast<CHtmlSysWinQt*>(win)->widget());
    if (mode == HTMLIMG_DRAW_CLIP) {
        // Clip mode.  Only draw the part of the image that would fit.  If the
        // image is smaller than pos, adjust the drawing area to avoid scaling.
        int targetWidth;
        int targetHeight;
        if (width() > pos->right - pos->left) {
            targetWidth = pos->right - pos->left;
        } else {
            targetWidth = width();
        }
        if (height() > pos->bottom - pos->top) {
            targetHeight = pos->bottom - pos->top;
        } else {
            targetHeight = height();
        }
        painter.drawImage(pos->left, pos->top, *this, 0, 0, targetWidth, targetHeight);
        return;
    }

    if (mode == HTMLIMG_DRAW_STRETCH) {
        // If the image doesn't fit exactly, scale it. Use the "smooth"
        // transformation mode (which uses a bilinear filter) if enabled in
        // the settings.
        Qt::TransformationMode mode =
            qFrame->settings().useSmoothScaling ? Qt::SmoothTransformation : Qt::FastTransformation;
        if (width() != pos->right - pos->left or height() != pos->bottom - pos->top) {
            painter.drawImage(
                QPoint(pos->left, pos->top),
                scaled(
                    pos->right - pos->left, pos->bottom - pos->top, Qt::IgnoreAspectRatio, mode));
        } else {
            painter.drawImage(QPoint(pos->left, pos->top), *this);
        }
        return;
    }

    // If we get here, 'mode' must have been HTMLIMG_DRAW_TILE.
    Q_ASSERT(mode == HTMLIMG_DRAW_TILE);
    QPixmap pix(QPixmap::fromImage(*this));
    painter.drawTiledPixmap(
        pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top, pix);
}

auto createImageFromFile(
    const CHtmlUrl* /*const url*/, const textchar_t* const filename, const unsigned long seekpos,
    const unsigned long filesize, CHtmlSysWin* /*const win*/, const QString& imageType)
    -> CHtmlSysResource*
{
    // qDebug() << "Loading" << imageType << "image from" << filename << "at offset" << seekpos
    //      << "with size" << filesize << "url:" << url->get_url();

    // Check if the file exists and is readable.
    QFileInfo inf(fnameToQStr(filename));
    if (not inf.exists() or not inf.isReadable()) {
        qWarning() << "ERROR:" << inf.filePath() << "doesn't exist or is unreadable";
        return nullptr;
    }

    // Open the file and seek to the specified position.
    QFile file(inf.filePath());
    if (not file.open(QIODevice::ReadOnly)) {
        qWarning() << "ERROR: Can't open file" << inf.filePath();
        return nullptr;
    }
    if (not file.seek(seekpos)) {
        qWarning() << "ERROR: Can't seek in file" << inf.filePath();
        file.close();
        return nullptr;
    }

    CHtmlSysResource* image = nullptr;
    // Better get an error at compile-time using static_cast rather than an
    // abort at runtime using dynamic_cast.
    QTadsImage* cast = nullptr;
    CHtmlSysImageMngQt* mngCast = nullptr;

    // Create an object of the appropriate class for the specified image type.
    // Also cast the object to a QTadsImage so we can loadFromData() later on.
    if (imageType == "JPG" or imageType == "JPEG") {
        image = new CHtmlSysImageJpegQt;
        cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImageJpegQt*>(image));
    } else if (imageType == "PNG") {
        image = new CHtmlSysImagePngQt;
        cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImagePngQt*>(image));
    } else if (imageType == "MNG") {
        image = new CHtmlSysImageMngQt;
        mngCast = static_cast<CHtmlSysImageMngQt*>(image);
    } else {
        qWarning() << "ERROR: Unknown image type" << imageType;
        file.close();
        return nullptr;
    }

    // Load the image data.
    const QByteArray& data(file.read(filesize));
    file.close();
    if (data.isEmpty() or static_cast<unsigned long>(data.size()) < filesize) {
        qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << inf.filePath();
        delete image;
        return nullptr;
    }

    if (imageType == "MNG") {
        QBuffer* buf = new QBuffer(mngCast);
        buf->setData(data);
        buf->open(QBuffer::ReadOnly);
        mngCast->setFormat("MNG");
        mngCast->setDevice(buf);
        mngCast->start();
    } else if (not cast->loadFromData(data, imageType.toLatin1())) {
        qWarning() << "ERROR: Could not parse image data";
        delete image;
        return nullptr;
    }
    return image;
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
