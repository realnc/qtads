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

#ifndef CONFIG_H
#define CONFIG_H

/* Recent versions of Qt define Q_BYTE_ORDER.  If defined, we use it to
 * decide which version of the endian-related Tads routines to use.  If
 * Q_BYTE_ORDER is not defined, we always use the PowerPC version, which
 * (desptite the name) are suited for both big-endian as well as little-
 * endian machines.  _M_IX86 is the little-endian version, and is faster
 * on Intel-compatibles than the generic PPC version.
 */
 /*
#ifdef Q_BYTE_ORDER
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define _M_PPC
#else
#define _M_PPC
#endif
#else
#define _M_PPC
#endif
*/

/* G++ (GNU C++) versions older than 3.0 didn't provide the 'and',
 * 'not' and 'or' keywords, so we define them as macros.
 */
#ifdef __cplusplus
#  ifdef __GNUC__
#    if __GNUC__ == 2
#      define not !
#      define and &&
#      define or ||
#    endif
#  endif
#endif

#endif /* CONFIG_H */
