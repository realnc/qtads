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
#include <QFileInfo>
#include <QBuffer>

#include "qtadsimage.h"
#include "sysimagejpeg.h"
#include "sysimagepng.h"
#include "sysimagemng.h"


/* Helper routine.  Loads any type of image from the specified offset inside
 * the given file and returns it.  Has the same semantics as the various
 * CHtmlSysImage*::create_*() routines.  The image type is specified in
 * 'imageType'.  It has the same format as the list returned by
 * QImageReader::supportedImageFormats() (like "JPG", "PNG", etc.)
 */
static CHtmlSysResource*
createImageFromFile( const CHtmlUrl* /*url*/, const textchar_t* filename, unsigned long seekpos,
                     unsigned long filesize, CHtmlSysWin* /*win*/, const QString& imageType )
{
    //qDebug() << "Loading" << imageType << "image from" << filename << "at offset" << seekpos
    //      << "with size" << filesize << "url:" << url->get_url();

    // Check if the file exists and is readable.
    QFileInfo inf(fnameToQStr(filename));
    if (not inf.exists() or not inf.isReadable()) {
        qWarning() << "ERROR:" << inf.filePath() << "doesn't exist or is unreadable";
        return 0;
    }

    // Open the file and seek to the specified position.
    QFile file(inf.filePath());
    if (not file.open(QIODevice::ReadOnly)) {
        qWarning() << "ERROR: Can't open file" << inf.filePath();
        return 0;
    }
    if (not file.seek(seekpos)) {
        qWarning() << "ERROR: Can't seek in file" << inf.filePath();
        file.close();
        return 0;
    }

    CHtmlSysResource* image = NULL;
    // Better get an error at compile-time using static_cast rather than an
    // abort at runtime using dynamic_cast.
    QTadsImage* cast = NULL;
    CHtmlSysImageMngQt* mngCast = NULL;

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
        return NULL;
    }

    // Load the image data.
    const QByteArray& data(file.read(filesize));
    file.close();
    if (data.isEmpty() or static_cast<unsigned long>(data.size()) < filesize) {
        qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << inf.filePath();
        delete image;
        return 0;
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
        return 0;
    }
    return image;
}


CHtmlSysResource*
CHtmlSysImageJpeg::create_jpeg( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                                unsigned long filesize, CHtmlSysWin* win )
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("JPG"));
}


CHtmlSysResource*
CHtmlSysImagePng::create_png( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                              unsigned long filesize, CHtmlSysWin* win )
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("PNG"));
}


CHtmlSysResource*
CHtmlSysImageMng::create_mng( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                              unsigned long filesize, CHtmlSysWin* win )
{
    return ::createImageFromFile(url, filename, seekpos, filesize, win, QString::fromLatin1("MNG"));
}
