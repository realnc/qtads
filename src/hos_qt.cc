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

#include "hos_qt.h"

#include <QDebug>
#include <QDateTime>


#ifdef TADSHTML_DEBUG
void
os_dbg_sys_msg( const textchar_t* msg )
{
	qDebug() << msg;
}
#endif


/* Get the next character in a string.
 *
 * FIXME: UTF-8
 */
textchar_t*
os_next_char( oshtml_charset_id_t /*id*/, const textchar_t* p, size_t /*len*/ )
{
	return const_cast<textchar_t*>(p+1);
}


/* Get the previous character in a string.
 *
 * FIXME: UTF-8
 */
textchar_t*
os_prev_char( oshtml_charset_id_t /*id*/, const textchar_t* p, const textchar_t* pstart )
{
	if (pstart == p) {
		return const_cast<textchar_t*>(p);
	}
	return const_cast<textchar_t*>(p-1);
}


/* Get the current system time.
 */
os_timer_t
os_get_time()
{
	return os_get_sys_clock_ms();
}
