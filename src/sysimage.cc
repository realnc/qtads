// This is copyrighted software. More information is at the end of this file.
#include <QBuffer>
#include <QFileInfo>

#include "qtadsimage.h"
#include "sysimagejpeg.h"
#include "sysimagemng.h"
#include "sysimagepng.h"

/* Helper routine.  Loads any type of image from the specified offset inside
 * the given file and returns it.  Has the same semantics as the various
 * CHtmlSysImage*::create_*() routines.  The image type is specified in
 * 'imageType'.  It has the same format as the list returned by
 * QImageReader::supportedImageFormats() (like "JPG", "PNG", etc.)
 */
static auto createImageFromFile(
    const CHtmlUrl* /*url*/, const textchar_t* filename, unsigned long seekpos,
    unsigned long filesize, CHtmlSysWin* /*win*/, const QString& imageType) -> CHtmlSysResource*
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
    if (imageType == QString::fromLatin1("JPG") or imageType == QString::fromLatin1("JPEG")) {
        image = new CHtmlSysImageJpegQt;
        cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImageJpegQt*>(image));
    } else if (imageType == QString::fromLatin1("PNG")) {
        image = new CHtmlSysImagePngQt;
        cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImagePngQt*>(image));
    } else if (imageType == QString::fromLatin1("MNG")) {
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

    if (imageType == QString::fromLatin1("MNG")) {
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

auto CHtmlSysImageJpeg::create_jpeg(
    const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos, unsigned long filesize,
    CHtmlSysWin* win) -> CHtmlSysResource*
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("JPG"));
}

auto CHtmlSysImagePng::create_png(
    const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos, unsigned long filesize,
    CHtmlSysWin* win) -> CHtmlSysResource*
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("PNG"));
}

auto CHtmlSysImageMng::create_mng(
    const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos, unsigned long filesize,
    CHtmlSysWin* win) -> CHtmlSysResource*
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("MNG"));
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
