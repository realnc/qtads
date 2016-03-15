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
/*
 * Byte conversion functions for little-endian CPUs.
 */
#ifndef H_QT_LE_H
#define H_QT_LE_H

#include <stdalign.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Round a size up to worst-case alignment boundary. */
static inline size_t osrndsz(const size_t siz)
{
    return (siz + alignof(intmax_t) - 1) & ~(alignof(intmax_t) - 1);
}


/* Round a pointer up to worst-case alignment boundary. */
static inline const void* osrndpt(const void* p)
{
    return (void*)(((uintptr_t)p + alignof(void*) - 1) & ~(alignof(void*) - 1));
}


/* Read an unaligned portable unsigned 2-byte value, returning int. */
static inline int osrp2(const void* p)
{
    uint16_t tmp;
    memcpy(&tmp, p, 2);
    return tmp;
}


/* Read an unaligned portable signed 2-byte value, returning int. */
static inline int osrp2s(const void* p)
{
    int16_t tmp;
    memcpy(&tmp, p, 2);
    return tmp;
}


/* Write unsigned int to unaligned portable 2-byte value. */
static inline void oswp2(void* p, const unsigned i)
{
    const uint16_t tmp = i;
    memcpy(p, &tmp, 2);
}


/* Write signed int to unaligned portable 2-byte value. */
static inline void oswp2s(void* p, const int i)
{
    oswp2(p, i);
}


/* Read an unaligned unsigned portable 4-byte value, returning long. */
static inline unsigned long osrp4(const void* p)
{
    uint32_t tmp;
    memcpy(&tmp, p, 4);
    return tmp;
}


/* Read an unaligned signed portable 4-byte value, returning long. */
static inline long osrp4s(const void *p)
{
    int32_t tmp;
    memcpy(&tmp, p, 4);
    return tmp;
}


/* Write an unsigned long to an unaligned portable 4-byte value. */
static inline void oswp4(void* p, const unsigned long l)
{
    const uint32_t tmp = l;
    memcpy(p, &tmp, 4);
}


/* Write a signed long to an unaligned portable 4-byte value. */
static inline void oswp4s(void* p, const long l)
{
    const int32_t tmp = l;
    memcpy(p, &tmp, 4);
}

#ifdef __cplusplus
}
#endif
#endif
