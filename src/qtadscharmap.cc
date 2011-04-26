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
#include "qtadscharmap.h"
#include <QByteArray>

size_t
QTadsCharmapToUni::read_file( osfildef* fp, char* buf, size_t bufl, unsigned long read_limit )
{
    Q_ASSERT(bufl >= 3); // Required by the specs.
    Q_ASSERT(buf != 0);

    // Find out the maximum number of bytes to read.
    if (read_limit == 0) {
        // No limit specified; use the file's size as limit.
        long tmp = osfpos(fp);
        osfseek(fp, 0, OSFSK_END);
        read_limit = osfpos(fp) + 1;
        osfseek(fp, tmp, OSFSK_SET);
    }

    // We read only one byte at a time.
    char c;

    // We accumulate the bytes we read until we have a full character (the
    // input file's charset might be multi-byte).  We assume that the biggest
    // possible multi-byte character is 4 bytes long.  I don't know of any
    // charset in this world that uses more than 4 bytes to represent a single
    // character.
    QByteArray acc(4, '\0');

    // Current index in acc (also number of bytes it contains).
    int accInd = 0;

    // We store the translated byte-sequences here.
    QString res;

    // Did an error occur while reading from the file?
    bool eof = false;

    // Are we done translating?
    bool done = false;

    // Amount of bytes read from the file.
    unsigned long count = 0;

    while (not eof and not done and count < read_limit) {
        // Read one byte.
        if (osfrb(fp, &c, 1) != 0) {
            // Failed; must be EOF.  We don't care if it's really EOF or not;
            // if we can't read, we can't read, no matter why.
            eof = true;
        } else {
            ++count;
            // Accumulate the read byte.
            Q_ASSERT(accInd < 4);
            acc[accInd++] = c;
            if (this->is_complete_char(acc, accInd)) {
                // The accumulated sequence is a valid character; translate it
                // and reset the accumulator, but make sure that we won't
                // overflow the output buffer.
                const QString& tmp = this->fLocalCodec->toUnicode(acc, accInd);
                if (static_cast<size_t>(res.toUtf8().length() + tmp.toUtf8().length()) > bufl) {
                    // It would overflow.  Set the file's seek location back,
                    // so that our caller won't lose the accumulated bytes;
                    // we'll read them again in our next call.
                    osfseek(fp, -accInd, OSFSK_CUR);
                    // We can't proceed, so we're done.
                    done = true;
                } else {
                    // It fits.
                    res += tmp;
                    accInd = 0;
                }
            }
        }
    }

    if (eof and res.isEmpty()) {
        // Nothing was translated and we reached the end of the file.  Return 0
        // (EOF).  We don't care if there are still bytes in our accumulator;
        // there's nothing left to read from the file, so the accumulator will
        // never contain a valid sequence.
        return 0;
    }

    // Copy the result to the output buffer, without the terminating '\0' byte
    // (as this might overflow the buffer, and we're not required to
    // 0-terminate the result anyway).
    const QByteArray& tmp = res.toUtf8();
    std::strncpy(buf, tmp, tmp.length());
    return tmp.length();
}
