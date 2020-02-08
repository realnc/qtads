// This is copyrighted software. More information is at the end of this file.

// Do not use "#pragma once" here, stuff might break. TADS does weird things in
// its include order and sometimes includes the same header multiple times with
// different preprocessor defitions.

/* This file should be included ONLY by html_os.h, which serves as the
 * switchboard for OS header inclusion.  Do not include this file directly from
 * from any other file.
 */
#ifndef TADSHTML_H
#include "tadshtml.h"
#endif

#ifndef HOS_QT_H
#define HOS_QT_H

#include <chrono>

/* ------------------------------------------------------------------------ */
/*
 *   System debugging console messages
 */

/*
 *   Write a debug message to the system console.  This is only used when
 *   TADSHTML_DEBUG is defined, and even then is only used to display
 *   internal debugging messages.  The system code can provide an empty
 *   implementation, if desired, and should not even need to include a
 *   definition when TADSHTML_DEBUG isn't defined when the system is
 *   compiled.
 */
#ifdef TADSHTML_DEBUG
void os_dbg_sys_msg(const textchar_t* msg);
#endif

/* ------------------------------------------------------------------------ */
/*
 *   Character set identifier type.  The font descriptor uses this type to
 *   contain a description of the character set to use in selecting a
 *   font.  On systems that use code pages or a similar mechanism, this
 *   should contain an identifier for the desired code page.
 *
 *   On Windows, we use the xxx_CHARSET identifiers to select font code
 *   pages.  Other systems may use a font ID or font family ID, code page
 *   number, script ID, or other system-specific type; the correct type to
 *   use depends on how the system chooses the character set to display
 *   when drawing text.
 */
// Qt always uses Unicode, so this is pretty much a dummy definition.
typedef unsigned int oshtml_charset_id_t;

/* get the system default character set */
oshtml_charset_id_t os_get_default_charset();

/* ------------------------------------------------------------------------ */
/*
 *   Advance a character string pointer to the next character in the string.
 *   The character string is in the character set identified by 'id'.
 *
 *   For systems that use plain ASCII or other single-byte character sets,
 *   this can simply always return (p+1).
 *
 *   For systems that use or can use multi-byte character sets, this must
 *   advance 'p' by the number of bytes taken up by the character to which
 *   'p' points.  On some systems, single-byte and double-byte characters
 *   can be mixed within the same character set; in such cases, this
 *   function must look at the actual character that 'p' points to and
 *   figure out how big that specific character is.
 *
 *   Note that this function takes a 'const' pointer and returns a non-const
 *   pointer to the same string, so the pointer passed is stripped of its
 *   const-ness.  This isn't great, but (short of templates) is the only
 *   convenient way to allow both const and non-const uses of this function.
 *   Callers should take care not to misuse the const removal.  (It's not
 *   like this is a security breach or anything; the caller can always use
 *   an ordinary type-cast if they really want to remove the const-ness.)
 */
textchar_t* os_next_char(oshtml_charset_id_t /*id*/, const textchar_t* p, size_t /*len*/);

/*
 *   Decrement a character string pointer to point to the previous character
 *   in the string.  The character set is identified by 'id'.  'p' is the
 *   current character pointer, and 'pstart' is a pointer to the start of the
 *   string.
 *
 *   The return value should be a pointer to the character before the
 *   character 'p' points to, but never before the start of the string.  Note
 *   that 'p' might actually point just past the end of the string, so no
 *   assumptions should be made about what 'p' currently points to.  However,
 *   it *is* safe to assume that 'p' points to a valid character *boundary* -
 *   that is, 'p' must never be at a byte position in the middle of a
 *   multi-byte character but should always be at a byte that starts a
 *   character OR at the byte just after the end of the string...
 */
textchar_t* os_prev_char(oshtml_charset_id_t /*id*/, const textchar_t* p, const textchar_t* pstart);

/*
 *   Determine if the character at the given string position is a word
 *   character - i.e., a character that's part of a written word.  The exact
 *   set of included characters is up to the platform, since this is for UI
 *   purposes, for things like selecting a word at a time in displayed text.
 *   Word characters usually include alphabetic characters, digits, and
 *   hyphens.
 *
 *   'p' is a pointer to the character of interest (if it's a multi-byte
 *   character, 'p' points to the first byte of the character), and 'len' is
 *   the number of bytes after 'p' in the overall string (which might contain
 *   additional characters following the character of interest, so 'len'
 *   isn't necessary the number of bytes in the single character at 'p').
 */
int os_is_word_char(oshtml_charset_id_t id, const textchar_t* p, size_t len);

/* ------------------------------------------------------------------------ */
/*
 *   System timer.  The system timer is used to measure short time
 *   differences.  The timer should provide resolution as high as
 *   possible; in practice, we'll use it to measure intervals of about a
 *   second, so a precision of about a tenth of a second is adequate.
 *   Since we only use the timer to measure intervals, the timer's zero
 *   point is unimportant (i.e., it doesn't need to relate to any
 *   particular time zone).
 */

/* system timer datatype */
using os_timer_t = std::chrono::milliseconds::rep;

/* get the current system time value */
os_timer_t os_get_time();

/*
 *   Get the number of timer units per second.  For Windows and Unix, the
 *   timer indicates milliseconds, so there are 1000 units per second.
 */
// We use chrono::milliseconds on all platforms.
#define OS_TIMER_UNITS_PER_SECOND 1000

/* ------------------------------------------------------------------------ */
/*
 *   Huge memory block manipulation - for use with blocks of memory over
 *   64k on 16-bit machines.  For 32-bit (and larger) architectures, huge
 *   blocks are the same as normal blocks, so these macros have trivial
 *   implementations.
 */

/*
 *   Huge pointer type.  For a given datatype, this should construct an
 *   appropriate type declaration for a huge pointer to that datatype.  On
 *   win32 and Unix, this just returns a plain pointer to the type.
 */
#define OS_HUGEPTR(type) type*

/*
 *   Allocate a huge (>64k) buffer.  On win32/Unix, we can use the normal
 *   memory allocator; on 16-bit systems, this would have to make a
 *   special OS call to do the allocation.
 *
 *   Note that we use th_malloc(), not the plain malloc().  th_malloc() is
 *   defined in the portable HTML TADS code; it provides a debug version
 *   of malloc that we can use to track memory allocations to ensure there
 *   are no memory leaks.  All 32-bit (and larger) platforms should use
 *   this same definition for os_alloc_huge().
 */
#define os_alloc_huge(siz) th_malloc(siz)

/*
 *   Increment a "huge" pointer by a given amount.  On win32/Unix,
 *   ordinary pointers are huge, so we just do the normal arithmetic.  On
 *   16-bit systems, this may have to do some extra work.
 */
#define os_add_huge(ptr, amt) ((ptr) + (amt))

/*
 *   Free a huge pointer.  On systems that run Qt, we use the standard
 *   memory allocator.
 *
 *   Note that we use th_free(), not the plain free(); see the explanation
 *   of the use of th_malloc() above.
 */
#define os_free_huge(ptr) th_free(ptr)

/*
 *   Memory alignment: align a size to the worst-case alignment
 *   requirement for this system.  For most systems, 4-byte alignment is
 *   adequate, but some systems (such as 64-bit hardware) may have a
 *   larger worst-case alignment requirement.
 */
// We assume 8-byte alignment instead of 4-byte, since we need to support
// 64-bit compilers.
#define os_align_size(siz) (((siz) + 7) & ~7)

#endif /* HOS_QT_H */

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
