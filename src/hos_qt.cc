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
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "hos_qt.h"

#include <QDebug>


#ifdef TADSHTML_DEBUG
void
os_dbg_sys_msg( const textchar_t* msg )
{
    qDebug() << msg;
}
#endif


/* Get the next character in a string.
 */
textchar_t*
os_next_char( oshtml_charset_id_t /*id*/, const textchar_t* p, size_t /*len*/ )
{
    // We always use UTF-8, so we only need to figure out how many bytes to
    // skip by looking at the first byte. We don't need to actually iterate
    // over them.
    if ((0x80 & *p) == 0x00) {
        p += 1;
    } else if ((0xE0 & *p) == 0xC0) {
        p += 2;
    } else if ((0xF0 & *p) == 0xE0) {
        p += 3;
    } else if ((0xF8 & *p) == 0xF0) {
        p += 4;
    } else {
        qWarning() << Q_FUNC_INFO << "corrupt UTF-8 sequence";
        p += 1;
    }
    return const_cast<textchar_t*>(p);
}


/* Get the previous character in a string.
 */
textchar_t*
os_prev_char( oshtml_charset_id_t /*id*/, const textchar_t* p, const textchar_t* pstart )
{
    // Move back one byte.
    --p;

    // Skip continuation bytes. Make sure we don't go past the beginning.
    // We need to iterate over every byte since it's not possible to determine
    // the length of the character without actually looking at the first byte.
    while (p != pstart and (*p & 0xC0) == 0x80) {
        --p;
    }
    return const_cast<textchar_t*>(p);
}


/* Get the current system time.
 */
os_timer_t
os_get_time()
{
    return os_get_sys_clock_ms();
}
