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

/* This file provides some non-standard functions that are considered
 * standard by the portable Tads code and thus aren't in the OS-layer.
 */
#ifndef MISSING_H
#define MISSING_H

#include <stddef.h> /* For size_t. */

#ifdef __cplusplus
extern "C" {
#endif

/* Counted-length case-insensitive string comparison.
 *
 * This function always compares 'len' characters, regardless if a '\0'
 * character has been detected.
 *
 * This function is already available on MS Windows.
 */
#ifndef _WIN32
int
memicmp( const char* s1, const char* s2, size_t len );
#endif

/* Case-insensitive string comparison.
 */
int
stricmp( const char* s1, const char* s2 );

/* Length-limited case-insensitive string comparison.
 *
 * This function compares at most 'n' characters, or until a '\0'
 * character has been detected.
 */
int
strnicmp( const char* s1, const char* s2, size_t n );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MISSING_H */
