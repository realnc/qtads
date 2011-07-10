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

/* Tads 3 uses so called "character mapping files" for mappings between
 * various character sets.  Since Qt provides its own mappings between
 * Unicode (Qt's native charset) and other charsets, we don't want to
 * use the Tads 3 character mapping file mechanism.  Therefore, we
 * provide a drop-in replacement for tads3/charmap.cpp, which is the
 * part of the T3VM that deals with mapping files.  Our implementation
 * is much simpler (trivial, actually), since we delegate all the
 * mapping work to Qt.  No matter what the current charset looks like,
 * Qt will always get the mappings right.
 */
#include <cstdlib>
#include <cstring>

#include "t3std.h"
#include "os.h"
#include "utf8.h"
#include "resload.h"
#include "charmap.h"
#include "vmdatasrc.h"

#include "qtadscharmap.h"


/* Create a mapper and load a mapping file.
 *
 * In our own implementation, we simply return a QTadsCharmapToLocal, no
 * matter which mapper is requested.  A special exception is "utf8",
 * where we return a trivial UTF-8 to UTF-8 "do nothing" mapper.
 */
CCharmapToLocal*
CCharmapToLocal::load( CResLoader*, const char* table_name )
{
    if (std::strcmp(table_name, "utf8") == 0) {
        return new CCharmapToLocalUTF8;
    }
    return new QTadsCharmapToLocal;
}

/* Create an appropriate mapping object for the given mapping file.
 *
 * In our own implementation, we simply return a QTadsCharmapToUni, no
 * matter which mapper is requested.  A special exception is "utf8",
 * where we return a trivial UTF-8 to UTF-8 "do nothing" mapper.
 */
CCharmapToUni*
CCharmapToUni::load( class CResLoader*, const char* table_name )
{
    if (std::strcmp(table_name, "utf8") == 0) {
        return new CCharmapToUniUTF8;
    }
    return new QTadsCharmapToUni;
}


/* ==================================================================
 * The rest of the code is the same as in tads3/charmap.cpp.  Code
 * not needed by our own implementation has been removed.
 * ================================================================== */

/* ------------------------------------------------------------------------ */
/*
 *   Special built-in mapper to 7-bit ASCII.  This is available as a last
 *   resort when no external mapping file can be found.
 */

/*
 *   create a plain ascii translator
 */
CCharmapToLocalASCII::CCharmapToLocalASCII()
{
    unsigned char *dst;
    wchar_t *exp_dst;
    size_t siz;
    size_t exp_siz;
    struct ascii_map_t
    {
        wchar_t uni;
        char asc[5];
    };
    ascii_map_t *p;
    static ascii_map_t ascii_mapping[] =
    {
        /* regular ASCII characters */
        { 1, { 1 } },
        { 2, { 2 } },
        { 3, { 3 } },
        { 4, { 4 } },
        { 5, { 5 } },
        { 6, { 6 } },
        { 7, { 7 } },
        { 8, { 8 } },
        { 9, { 9 } },
        { 10, { 10 } },
        { 11, { 11 } },
        { 12, { 12 } },
        { 13, { 13 } },
        { 14, { 14 } },
        { 15, { 15 } },
        { 16, { 16 } },
        { 17, { 17 } },
        { 18, { 18 } },
        { 19, { 19 } },
        { 20, { 20 } },
        { 21, { 21 } },
        { 22, { 22 } },
        { 23, { 23 } },
        { 24, { 24 } },
        { 25, { 25 } },
        { 26, { 26 } },
        { 27, { 27 } },
        { 28, { 28 } },
        { 29, { 29 } },
        { 30, { 30 } },
        { 31, { 31 } },
        { 32, { 32 } },
        { 33, { 33 } },
        { 34, { 34 } },
        { 35, { 35 } },
        { 36, { 36 } },
        { 37, { 37 } },
        { 38, { 38 } },
        { 39, { 39 } },
        { 40, { 40 } },
        { 41, { 41 } },
        { 42, { 42 } },
        { 43, { 43 } },
        { 44, { 44 } },
        { 45, { 45 } },
        { 46, { 46 } },
        { 47, { 47 } },
        { 48, { 48 } },
        { 49, { 49 } },
        { 50, { 50 } },
        { 51, { 51 } },
        { 52, { 52 } },
        { 53, { 53 } },
        { 54, { 54 } },
        { 55, { 55 } },
        { 56, { 56 } },
        { 57, { 57 } },
        { 58, { 58 } },
        { 59, { 59 } },
        { 60, { 60 } },
        { 61, { 61 } },
        { 62, { 62 } },
        { 63, { 63 } },
        { 64, { 64 } },
        { 65, { 65 } },
        { 66, { 66 } },
        { 67, { 67 } },
        { 68, { 68 } },
        { 69, { 69 } },
        { 70, { 70 } },
        { 71, { 71 } },
        { 72, { 72 } },
        { 73, { 73 } },
        { 74, { 74 } },
        { 75, { 75 } },
        { 76, { 76 } },
        { 77, { 77 } },
        { 78, { 78 } },
        { 79, { 79 } },
        { 80, { 80 } },
        { 81, { 81 } },
        { 82, { 82 } },
        { 83, { 83 } },
        { 84, { 84 } },
        { 85, { 85 } },
        { 86, { 86 } },
        { 87, { 87 } },
        { 88, { 88 } },
        { 89, { 89 } },
        { 90, { 90 } },
        { 91, { 91 } },
        { 92, { 92 } },
        { 93, { 93 } },
        { 94, { 94 } },
        { 95, { 95 } },
        { 96, { 96 } },
        { 97, { 97 } },
        { 98, { 98 } },
        { 99, { 99 } },
        { 100, { 100 } },
        { 101, { 101 } },
        { 102, { 102 } },
        { 103, { 103 } },
        { 104, { 104 } },
        { 105, { 105 } },
        { 106, { 106 } },
        { 107, { 107 } },
        { 108, { 108 } },
        { 109, { 109 } },
        { 110, { 110 } },
        { 111, { 111 } },
        { 112, { 112 } },
        { 113, { 113 } },
        { 114, { 114 } },
        { 115, { 115 } },
        { 116, { 116 } },
        { 117, { 117 } },
        { 118, { 118 } },
        { 119, { 119 } },
        { 120, { 120 } },
        { 121, { 121 } },
        { 122, { 122 } },
        { 123, { 123 } },
        { 124, { 124 } },
        { 125, { 125 } },
        { 126, { 126 } },
        { 127, { 127 } },

        /* Latin-1 accented characters and symbols */
        { 353, "s" },
        { 352, "S" },
        { 8218, "\'" },
        { 8222, "\"" },
        { 8249, "<" },
        { 338, "OE" },
        { 8216, "\'" },
        { 8217, "\'" },
        { 8220, "\"" },
        { 8221, "\"" },
        { 8211, "-" },
        { 8212, "--" },
        { 8482, "(tm)" },
        { 8250, ">" },
        { 339, "oe" },
        { 376, "Y" },
        { 162, "c" },
        { 163, "L" },
        { 165, "Y" },
        { 166, "|" },
        { 169, "(c)" },
        { 170, "a" },
        { 173, " " },
        { 174, "(R)" },
        { 175, "-" },
        { 177, "+/-" },
        { 178, "2" },
        { 179, "3" },
        { 180, "\'" },
        { 181, "u" },
        { 182, "P" },
        { 183, "*" },
        { 184, "," },
        { 185, "1" },
        { 186, "o" },
        { 171, "<<" },
        { 187, ">>" },
        { 188, "1/4" },
        { 189, "1/2" },
        { 190, "3/4" },
        { 192, "A" },
        { 193, "A" },
        { 194, "A" },
        { 195, "A" },
        { 196, "A" },
        { 197, "A" },
        { 198, "AE" },
        { 199, "C" },
        { 200, "E" },
        { 201, "E" },
        { 202, "E" },
        { 203, "E" },
        { 204, "I" },
        { 205, "I" },
        { 206, "I" },
        { 207, "I" },
        { 209, "N" },
        { 210, "O" },
        { 211, "O" },
        { 212, "O" },
        { 213, "O" },
        { 214, "O" },
        { 215, "x" },
        { 216, "O" },
        { 217, "U" },
        { 218, "U" },
        { 219, "U" },
        { 220, "U" },
        { 221, "Y" },
        { 223, "ss" },
        { 224, "a" },
        { 225, "a" },
        { 226, "a" },
        { 227, "a" },
        { 228, "a" },
        { 229, "a" },
        { 230, "ae" },
        { 231, "c" },
        { 232, "e" },
        { 233, "e" },
        { 234, "e" },
        { 235, "e" },
        { 236, "i" },
        { 237, "i" },
        { 238, "i" },
        { 239, "i" },
        { 241, "n" },
        { 242, "o" },
        { 243, "o" },
        { 244, "o" },
        { 245, "o" },
        { 246, "o" },
        { 247, "/" },
        { 248, "o" },
        { 249, "u" },
        { 250, "u" },
        { 251, "u" },
        { 252, "u" },
        { 253, "y" },
        { 255, "y" },
        { 710, "^" },
        { 732, "~" },

        /* math symbols */
        { 402, "f" },

        /* other symbols */
        { 8226, "*" },

        /* arrows */
        { 8592, "<-" },
        { 8594, "->" },

        /* several capital Greek letters look a lot like Roman letters */
        { 913, "A" },
        { 914, "B" },
        { 918, "Z" },
        { 919, "H" },
        { 921, "I" },
        { 922, "K" },
        { 924, "M" },
        { 925, "N" },
        { 927, "O" },
        { 929, "P" },
        { 932, "T" },
        { 933, "Y" },
        { 935, "X" },

        /* Latin-2 accented characters */
        { 260, "A" },
        { 321, "L" },
        { 317, "L" },
        { 346, "S" },
        { 350, "S" },
        { 356, "T" },
        { 377, "Z" },
        { 381, "Z" },
        { 379, "Z" },
        { 261, "a" },
        { 731, "o" },
        { 322, "l" },
        { 318, "l" },
        { 347, "s" },
        { 351, "s" },
        { 357, "t" },
        { 378, "z" },
        { 733, "\"" },
        { 382, "z" },
        { 380, "z" },
        { 340, "R" },
        { 258, "A" },
        { 313, "L" },
        { 262, "C" },
        { 268, "C" },
        { 280, "E" },
        { 282, "E" },
        { 270, "D" },
        { 272, "D" },
        { 323, "N" },
        { 327, "N" },
        { 336, "O" },
        { 344, "R" },
        { 366, "U" },
        { 368, "U" },
        { 354, "T" },
        { 341, "r" },
        { 259, "a" },
        { 314, "l" },
        { 263, "c" },
        { 269, "c" },
        { 281, "e" },
        { 283, "e" },
        { 271, "d" },
        { 273, "d" },
        { 324, "n" },
        { 328, "n" },
        { 337, "o" },
        { 345, "r" },
        { 367, "u" },
        { 369, "u" },
        { 355, "t" },
        { 0, { 0 } }
    };

    /* determine how much space we'll need in the translation array */
    for (p = ascii_mapping, siz = 0, exp_siz = 0 ; p->uni != 0 ; ++p)
    {
        /* we need space for this mapping string, plus a length prefix byte */
        siz += strlen(p->asc) + 1;

        /*
         *   if this is a multi-character expansion, count it in the
         *   expansion array size
         */
        if (strlen(p->asc) > 1)
            exp_siz += strlen(p->asc) + 1;
    }

    /* add in space for the default entry */
    siz += 2;

    /* allocate the translation array */
    xlat_array_ = (unsigned char *)t3malloc(siz);

    /*
     *   allocate the expansion array; allocate one extra entry for the null
     *   mapping at index zero
     */
    exp_array_ = (wchar_t *)t3malloc((exp_siz + 1) * sizeof(wchar_t));

    /*
     *   start at element 1 of the expansion array (element zero is reserved
     *   to indicate the null mapping)
     */
    dst = xlat_array_;
    exp_dst = exp_array_ + 1;

    /*
     *   Add the zeroeth entry, which serves as the default mapping for
     *   characters that aren't otherwise mappable.
     */
    set_mapping(0, 0);
    *dst++ = 1;
    *dst++ = '?';

    /* set up the arrays */
    for (p = ascii_mapping ; p->uni != 0 ; ++p)
    {
        size_t len;

        /* set the mapping's offset in the translation array */
        set_mapping(p->uni, dst - xlat_array_);

        /* get the length of this mapping */
        len = strlen(p->asc);

        /* set this mapping's length */
        *dst++ = (unsigned char)len;

        /* copy the mapping */
        memcpy(dst, p->asc, len);

        /* move past the mapping in the translation array */
        dst += len;

        /* add the expansion mapping if necessary */
        if (len > 1)
        {
            size_t i;

            /* add an expansion mapping */
            set_exp_mapping(p->uni, exp_dst - exp_array_);

            /* set the length prefix */
            *exp_dst++ = (wchar_t)len;

            /* add the mapping */
            for (i = 0 ; i < len ; ++i)
                *exp_dst++ = (wchar_t)p->asc[i];
        }
    }
}

/*
 *   create the translator
 */
CCharmapToLocal::CCharmapToLocal()
{
    /* no mapping sub-tables yet */
    memset(map_, 0, sizeof(map_));
    memset(exp_map_, 0, sizeof(exp_map_));

    /* no translation or expansion arrays yet */
    xlat_array_ = 0;
    exp_array_ = 0;
}

/*
 *   delete the translator
 */
CCharmapToLocal::~CCharmapToLocal()
{
    size_t i;

    /* delete the translation array */
    if (xlat_array_ != 0)
        t3free(xlat_array_);

    /* delete the expansion array */
    if (exp_array_ != 0)
        t3free(exp_array_);

    /* delete any mapping tables we've allocated */
    for (i = 0 ; i < sizeof(map_)/sizeof(map_[0]) ; ++i)
    {
        /* delete this mapping if allocated */
        if (map_[i] != 0)
            t3free(map_[i]);
    }

    /* delete any expansion mapping tables */
    for (i = 0 ; i < sizeof(exp_map_)/sizeof(exp_map_[0]) ; ++i)
    {
        /* delete this expansion mapping if allocated */
        if (exp_map_[i] != 0)
            t3free(exp_map_[i]);
    }
}

/*
 *   Set a mapping
 */
void CCharmapToLocal::set_mapping(wchar_t unicode_char,
                                  unsigned int xlat_offset)
{
    int master_idx;

    /* get the master table index for this unicode character */
    master_idx = (int)((unicode_char >> 8) & 0xff);

    /* if there's no sub-table here yet, create one */
    if (map_[master_idx] == 0)
    {
        int i;

        /* allocate it */
        map_[master_idx] =
            (unsigned int *)t3malloc(256 * sizeof(unsigned int));

        /*
         *   Set each entry to the default character, so that it will
         *   produce valid results if no mapping is ever specified for the
         *   character.  The default character is always at offset zero in
         *   the translation array.
         */
        for (i = 0 ; i < 256 ; ++i)
            map_[master_idx][i] = 0;
    }

    /* set the mapping for the character's entry in the sub-table */
    map_[master_idx][unicode_char & 0xff] = xlat_offset;
}

/*
 *   Set an expansion mapping
 */
void CCharmapToLocal::set_exp_mapping(wchar_t unicode_char,
                                      unsigned int exp_offset)
{
    int master_idx;

    /* get the master table index for this unicode character */
    master_idx = (int)((unicode_char >> 8) & 0xff);

    /* if there's no sub-table here yet, create one */
    if (exp_map_[master_idx] == 0)
    {
        int i;

        /* allocate it */
        exp_map_[master_idx] =
            (unsigned int *)t3malloc(256 * sizeof(unsigned int));

        /*
         *   Set each entry to the default character, so that it will produce
         *   valid results if no mapping is ever specified for the character.
         *   The default character is always at offset zero in the expansion
         *   array.
         */
        for (i = 0 ; i < 256 ; ++i)
            exp_map_[master_idx][i] = 0;
    }

    /* set the mapping for the character's entry in the sub-table */
    exp_map_[master_idx][unicode_char & 0xff] = exp_offset;
}

/*
 *   Map a UTF-8 string of known byte length to the local character set
 */
size_t CCharmapToLocal::map_utf8(char *dest, size_t dest_len,
                                 utf8_ptr src, size_t src_byte_len,
                                 size_t *src_bytes_used) const
{
    utf8_ptr src_start;
    size_t cur_total;
    char *srcend;

    /* remember where we started */
    src_start = src;

    /* compute where the source buffer ends */
    srcend = src.getptr() + src_byte_len;

    /* copy characters until we reach the end of the source string */
    for (cur_total = 0 ; src.getptr() < srcend ; src.inc())
    {
        char mapbuf[10];
        size_t maplen = sizeof(mapbuf);
        char *mapp = mapbuf;

        /* map this character */
        maplen = map(src.getch(), &mapp, &maplen);

        /* determine how to store the character */
        if (dest == 0)
        {
            /* we're just counting */
        }
        else if (dest_len >= maplen)
        {
            /* we have room for it - add it in */
            memcpy(dest, mapbuf, maplen);

            /* advance past it */
            dest += maplen;
            dest_len -= maplen;
        }
        else
        {
            /* there's no more room - stop now */
            break;
        }

        /* add this into the total */
        cur_total += maplen;
    }

    /* if the caller wants to know how much space we used, tell them */
    if (src_bytes_used != 0)
        *src_bytes_used = src.getptr() - src_start.getptr();

    /* return the total length of the result */
    return cur_total;
}

/*
 *   Map a null-terminated UTF-8 string to the local character set
 */
size_t CCharmapToLocal::map_utf8z(char *dest, size_t dest_len,
                                  utf8_ptr src) const
{
    size_t cur_total;

    /* copy characters until we find the terminating null */
    for (cur_total = 0 ; src.getch() != 0 ; src.inc())
    {
        /*
         *   map this character into the output, if it will fit, but in
         *   any case count the space it needs in the output
         */
        cur_total += map(src.getch(), &dest, &dest_len);
    }

    /*
     *   add a null terminator if there's room, but don't count it in the
     *   result length
     */
    map(0, &dest, &dest_len);

    /* return the total length of the result */
    return cur_total;
}

/*
 *   Map a null-terminated UTF-8 string to the local character set, escaping
 *   characters that aren't part of the local character set.
 */
size_t CCharmapToLocal::map_utf8z_esc(
    char *dest, size_t dest_len, utf8_ptr src,
    size_t (*esc_fn)(wchar_t, char **, size_t *)) const
{
    size_t cur_total;

    /* copy characters until we find the terminating null */
    for (cur_total = 0 ; src.getch() != 0 ; src.inc())
    {
        wchar_t ch = src.getch();

        /* if this character is mappable, map it; otherwise, escape it */
        if (is_mappable(src.getch()))
        {
            /* map the character */
            cur_total += map(ch, &dest, &dest_len);
        }
        else
        {
            /* we can't map it, so let the escape callback handle it */
            cur_total += (*esc_fn)(ch, &dest, &dest_len);
        }
    }

    /*
     *   add a null terminator if there's room, but don't count it in the
     *   result length
     */
    map(0, &dest, &dest_len);

    /* return the total length of the result */
    return cur_total;
}

/*
 *   Escape callback for map_utf8z_esc() - prepares source-code-style
 *   'backslash' escape sequences for unmappable characters.
 */
size_t CCharmapToLocal::source_esc_cb(wchar_t ch, char **dest, size_t *len)
{
    char buf[7];
    size_t copylen;

    /* prepare our own representation */
    sprintf(buf, "\\u%04x", (unsigned int)ch);

    /* copy the whole thing if possible, but limit to the available space */
    copylen = 6;
    if (copylen > *len)
        copylen = *len;

    /* copy the bytes */
    memcpy(*dest, buf, copylen);

    /* advance the buffer pointers */
    *dest += copylen;
    *len -= copylen;

    /* return the full space needed */
    return 6;
}

/*
 *   Map to UTF8
 */
size_t CCharmapToLocal::map_utf8(char *dest, size_t dest_len,
                                 const char *src, size_t src_byte_len,
                                 size_t *src_bytes_used) const
{
    utf8_ptr src_ptr;

    /* set up the source UTF-8 pointer */
    src_ptr.set((char *)src);

    /* map it and return the result */
    return map_utf8(dest, dest_len, src_ptr, src_byte_len, src_bytes_used);
}

/*
 *   Load the character set translation table
 */
void CCharmapToLocal::load_table(osfildef *fp)
{
    ulong startpos;
    ulong ofs;
    uchar buf[256];
    uint cnt;
    ulong xbytes;
    ulong xchars;
    uint next_ofs;

    /* note the initial seek position */
    startpos = osfpos(fp);

    /* read the first entry, which gives the offset of the to-local table */
    if (osfrb(fp, buf, 4))
        return;
    ofs = t3rp4u(buf);

    /* seek to the to-local table */
    osfseek(fp, startpos + ofs, OSFSK_SET);

    /* read the number of entries and number of bytes needed */
    if (osfrb(fp, buf, 6))
        return;
    cnt = osrp2(buf);
    xbytes = t3rp4u(buf + 2);

    /*
     *   Allocate space for the translation table.  Note that we cannot
     *   handle translation tables bigger than the maximum allowed in a
     *   single allocation unit on the operating system.
     */
    if (xbytes > OSMALMAX)
        return;
    xlat_array_ = (unsigned char *)t3malloc(xbytes);
    if (xlat_array_ == 0)
        return;

    /*
     *   Read each mapping
     */
    for (next_ofs = 0 ; cnt > 0 ; --cnt)
    {
        wchar_t codept;
        uint xlen;

        /* read the code point and translation length */
        if (osfrb(fp, buf, 3))
            return;

        /* decode the code point and translation length */
        codept = osrp2(buf);
        xlen = (unsigned int)buf[2];

        /* assign the mapping */
        set_mapping(codept, next_ofs);

        /* store the translation length */
        xlat_array_[next_ofs++] = buf[2];

        /* read the translation bytes */
        if (osfrb(fp, xlat_array_ + next_ofs, xlen))
            return;

        /* skip past the translation bytes we've read */
        next_ofs += xlen;
    }

    /*
     *   Next, read the expansions, if present.
     *
     *   If we find the $EOF marker, it means it's an old-format file without
     *   the separate expansion definitions.  Otherwise, we'll have the
     *   expansion entry count and the aggregate number of unicode characters
     *   in all of the expansions.
     */
    if (osfrb(fp, buf, 6) || memcmp(buf, "$EOF", 4) == 0)
        return;

    /* decode the expansion entry count and aggregate length */
    cnt = osrp2(buf);
    xchars = t3rp4u(buf + 2);

    /*
     *   add one entry so that we can leave index zero unused, to indicate
     *   unmapped characters
     */
    ++xchars;

    /* add one array slot per entry, for the length prefix slots */
    xchars += cnt;

    /* allocate space for the expansions */
    exp_array_ = (wchar_t *)t3malloc(xchars * sizeof(wchar_t));
    if (exp_array_ == 0)
        return;

    /*
     *   read the mappings; start loading them at index 1, since we want to
     *   leave index 0 unused so that it can indicate unused mappings
     */
    for (next_ofs = 1 ; cnt > 0 ; --cnt)
    {
        wchar_t codept;
        uint xlen;
        size_t i;

        /* read this entry's unicode value and expansion character length */
        if (osfrb(fp, buf, 3))
            return;

        /* decode the code point and expansion length */
        codept = osrp2(buf);
        xlen = (uint)buf[2];

        /* assign the expansion mapping */
        set_exp_mapping(codept, next_ofs);

        /* set the length prefix */
        exp_array_[next_ofs++] = (wchar_t)xlen;

        /* read and store the expansion characters */
        for (i = 0 ; i < xlen ; ++i)
        {
            /* read this translation */
            if (osfrb(fp, buf, 2))
                return;

            /* decode and store this translation */
            exp_array_[next_ofs++] = osrp2(buf);
        }
    }
}

/*
 *   Write to a file
 */
int CCharmapToLocal::write_file(CVmDataSource *fp,
                                const char *buf, size_t bufl)
{
    utf8_ptr p;

    /* set up to read from the buffer */
    p.set((char *)buf);

    /* map and write one buffer-full at a time */
    while (bufl > 0)
    {
        char conv_buf[256];
        size_t conv_len;
        size_t used_src_len;

        /* map as much as we can fit into our buffer */
        conv_len = map_utf8(conv_buf, sizeof(conv_buf), p, bufl,
                            &used_src_len);

        /* write out this chunk */
        if (fp->write(conv_buf, conv_len))
            return 1;

        /* advance past this chunk in the input */
        p.set(p.getptr() + used_src_len);
        bufl -= used_src_len;
    }

    /* no errors */
    return 0;
}


/* ------------------------------------------------------------------------ */
/*
 *   Character mapper - trivial UTF8-to-UTF8 conversion
 */

/*
 *   map a character
 */
size_t CCharmapToLocalUTF8::map(wchar_t unicode_char, char **output_ptr,
                                size_t *output_len) const
{
    size_t map_len;

    /* get the character size */
    map_len = utf8_ptr::s_wchar_size(unicode_char);

    /* if we don't have room for one more character, abort */
    if (*output_len < map_len)
    {
        *output_len = 0;
        return map_len;
    }

    /* store the mapping */
    utf8_ptr::s_putch(*output_ptr, unicode_char);

    /* increment the pointer by the number of characters we copied */
    *output_ptr += map_len;

    /* adjust the remaining output length */
    *output_len -= map_len;

    /* return the size of the result */
    return map_len;
}

/*
 *   Map a UTF-8 string of known byte length
 */
size_t CCharmapToLocalUTF8::map_utf8(char *dest, size_t dest_len,
                                     utf8_ptr src, size_t src_byte_len,
                                     size_t *src_bytes_used) const
{
    size_t copy_len;

    /*
     *   if they didn't give us a destination buffer, tell them how much
     *   space is needed for the copy - this is identical to the length of
     *   the source string since we make no changes to it
     */
    if (dest == 0)
    {
        if (src_bytes_used != 0)
            *src_bytes_used = 0;

        return src_byte_len;
    }

    /* copy as much as we can, up to the output buffer length */
    copy_len = src_byte_len;
    if (copy_len > dest_len)
        copy_len = dest_len;

    /*
     *   if the last byte we'd copy is a continuation byte, don't copy it
     *   so that we keep whole characters intact
     */
    if (copy_len > 0
        && utf8_ptr::s_is_continuation(src.getptr() + copy_len - 1))
    {
        /* don't copy this byte */
        --copy_len;

        /*
         *   check the previous byte as well, since a given character can
         *   be up to three bytes long (hence we might have two
         *   continuation bytes)
         */
        if (copy_len > 0
            && utf8_ptr::s_is_continuation(src.getptr() + copy_len - 1))
            --copy_len;
    }

    /* if we have an output buffer, copy the data */
    if (dest != 0)
        memcpy(dest, src.getptr(), copy_len);

    /* set the amount we copied, if the caller is interested */
    if (src_bytes_used != 0)
        *src_bytes_used = copy_len;

    /* return the number of bytes we put in the destination buffer */
    return copy_len;
}

/*
 *   Map a null-terminated UTF-8 string
 */
size_t CCharmapToLocalUTF8::map_utf8z(char *dest, size_t dest_len,
                                      utf8_ptr src) const
{
    size_t src_len;

    /* get the source length */
    src_len = strlen(src.getptr());

    /* copy the bytes */
    map_utf8(dest, dest_len, src, src_len, 0);

    /*
     *   if there's room for the null terminator (which takes up just one
     *   byte in UTF-8), add it
     */
    if (dest_len > src_len)
        *(dest + src_len) = '\0';

    /*
     *   return the amount of space needed to copy the whole string --
     *   this is identical to the source length, since we don't make any
     *   changes to it
     */
    return src_len;
}


/* ------------------------------------------------------------------------ */
/*
 *   Character mapper - Unicode to Single-byte
 */

/*
 *   map a character
 */
size_t CCharmapToLocalSB::map(wchar_t unicode_char, char **output_ptr,
                              size_t *output_len) const
{
    const unsigned char *mapping;
    size_t map_len;

    /* get the mapping */
    mapping = get_xlation(unicode_char, &map_len);

    /* if we don't have room for one more character, abort */
    if (*output_len < map_len)
    {
        *output_len = 0;
        return map_len;
    }

    /* copy the mapping */
    memcpy(*output_ptr, mapping, map_len);

    /* increment the pointer by the number of characters we copied */
    *output_ptr += map_len;

    /* adjust the remaining output length */
    *output_len -= map_len;

    /* return the size of the result */
    return map_len;
}

/*
 *   Map a UTF-8 string of known byte length to the local character set
 */
size_t CCharmapToLocalSB::map_utf8(char *dest, size_t dest_len,
                                   utf8_ptr src, size_t src_byte_len,
                                   size_t *src_bytes_used) const
{
    utf8_ptr src_start;
    size_t cur_total;
    char *srcend;

    /* remember where we started */
    src_start = src;

    /* compute where the source buffer ends */
    srcend = src.getptr() + src_byte_len;

    /* copy characters until we reach the end of the source string */
    for (cur_total = 0 ; src.getptr() < srcend ; src.inc())
    {
        const unsigned char *mapping;
        size_t map_len;

        /* get the mapping for this character */
        mapping = get_xlation(src.getch(), &map_len);

        /*
         *   if we have room, add it; otherwise, zero the output length
         *   remaining so we don't try to add anything more
         */
        if (dest == 0)
        {
            /* we're just counting */
        }
        else if (map_len <= dest_len)
        {
            /* add the sequence */
            memcpy(dest, mapping, map_len);

            /* adjust the output pointer and length remaining */
            dest += map_len;
            dest_len -= map_len;
        }
        else
        {
            /* it doesn't fit - stop now */
            break;
        }

        /* count the length in the total */
        cur_total += map_len;
    }

    /* if the caller wants to know how much space we used, tell them */
    if (src_bytes_used != 0)
        *src_bytes_used = src.getptr() - src_start.getptr();

    /* return the total length of the result */
    return cur_total;
}

/*
 *   Map a null-terminated UTF-8 string to the local character set
 */
size_t CCharmapToLocalSB::map_utf8z(char *dest, size_t dest_len,
                                    utf8_ptr src) const
{
    size_t cur_total;

    /* copy characters until we find the terminating null */
    for (cur_total = 0 ; src.getch() != 0 ; src.inc())
    {
        const unsigned char *mapping;
        size_t map_len;

        /* get the mapping for this character */
        mapping = get_xlation(src.getch(), &map_len);

        /*
         *   if we have room, add it; otherwise, zero the output length
         *   remaining so we don't try to add anything more
         */
        if (map_len <= dest_len)
        {
            /* add the sequence */
            memcpy(dest, mapping, map_len);

            /* adjust the output pointer and length remaining */
            dest += map_len;
            dest_len -= map_len;
        }
        else
        {
            /* it doesn't fit - zero the output length remaining */
            dest_len = 0;
        }

        /* count the length in the total */
        cur_total += map_len;
    }

    /*
     *   add a null terminator, if there's room, but don't count it in the
     *   output length
     */
    if (dest_len > 0)
        *dest = '\0';

    /* return the total length of the result */
    return cur_total;
}


/*
 *   load a mapping table
 */
void CCharmapToUni::load_table(osfildef *fp)
{
    uchar buf[256];
    uint entry_cnt;

    /* read the header and the local table header */
    if (osfrb(fp, buf, 6))
        return;

    /* get the local table size from the local table header */
    entry_cnt = osrp2(buf + 4);

    /* read the mappings */
    while (entry_cnt > 0)
    {
        size_t cur;
        const uchar *p;

        /* figure out how many entries we can read this time */
        cur = sizeof(buf)/4;
        if (cur > entry_cnt)
            cur = entry_cnt;

        /* read the entries */
        if (osfrb(fp, buf, cur*4))
            return;

        /* deduct this number from the remaining count */
        entry_cnt -= cur;

        /* scan the entries */
        for (p = buf ; cur > 0 ; p += 4, --cur)
        {
            /* map this entry */
            set_mapping(osrp2(p), osrp2(p+2));
        }
    }
}

/*
 *   Map a null-terminated string into a buffer
 */
size_t CCharmapToUni::map_str(char *outbuf, size_t outbuflen,
                              const char *input_str)
{
    size_t input_len;
    size_t output_len;

    /* get the length of the input string */
    input_len = strlen(input_str);

    /* map the string to the output buffer */
    output_len = map(&outbuf, &outbuflen, input_str, input_len);

    /* if there's space remaining in the output buffer, add the null byte */
    if (outbuflen != 0)
        *outbuf = '\0';

    /* return the number of bytes needed for the conversion */
    return output_len;
}

/*
 *   Validate a buffer of utf-8 characters
 */
void CCharmapToUni::validate(char *buf, size_t len)
{
    for ( ; len != 0 ; ++buf, --len)
    {
        /* check the type of the character */
        char c = *buf;
        if ((c & 0x80) == 0)
        {
            /* 0..127 are one-byte characters, so this is valid */
        }
        else if ((c & 0xC0) == 0x80)
        {
            /*
             *   This byte has the pattern 10xxxxxx, which makes it a
             *   continuation byte.  Since we didn't just come from a
             *   multi-byte intro byte, this is invalid.  Change this byte to
             *   '?'.
             */
            *buf = '?';
        }
        else if ((c & 0xE0) == 0xC0)
        {
            /*
             *   This byte has the pattern 110xxxxx, which makes it the first
             *   byte of a two-byte character sequence.  The next byte must
             *   have the pattern 10xxxxxx - if not, mark the current
             *   character as invalid, since it's not part of a valid
             *   sequence, and deal with the next byte separately.
             */
            if (len > 1 && (*(buf+1) & 0xC0) == 0x80)
            {
                /* we have a valid two-byte sequence - skip it */
                ++buf;
                --len;
            }
            else
            {
                /*
                 *   the next byte isn't a continuation, so the current byte
                 *   is invalid
                 */
                *buf = '?';
            }
        }
        else
        {
            /*
             *   This byte has the pattern 111xxxxx, which makes it the first
             *   byte of a three-byte sequence.  The next two bytes must be
             *   marked as continuation bytes.
             */
            if (len > 2
                && (*(buf+1) & 0xC0) == 0x80
                && (*(buf+2) & 0xC0) == 0x80)
            {
                /* we have a valid three-byte sequence - skip it */
                buf += 2;
                len -= 2;
            }
            else
            {
                /* this is not a valid three-byte sequence */
                *buf = '?';
            }
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Basic single-byte character set to UTF-8 mapper
 */

/*
 *   read from a single-byte file and translate to UTF-8
 */
size_t CCharmapToUniSB_basic::read_file(CVmDataSource *fp,
                                        char *buf, size_t bufl)
{
    size_t inlen;

    /*
     *   Compute how much to read from the file.  The input file is
     *   composed of single-byte characters, so only read up to one third
     *   of the buffer length; this will ensure that we can always fit
     *   what we read into the caller's buffer.
     */
    inlen = bufl / 3;

    /* in any case, we can't read more than our own buffer size */
    if (inlen > sizeof(inbuf_))
        inlen = sizeof(inbuf_);

    /* read from the file */
    inlen = fp->readc(inbuf_, inlen);

    /*
     *   Map data to the caller's buffer, and return the result.  We're
     *   certain that the data will fit in the caller's buffer: we're
     *   mapping only a third as many characters as we have bytes
     *   available, and each character can take up at most three bytes,
     *   hence the worst case is that we fill the buffer completely.
     *
     *   On the other hand, we may only fill the buffer to a third of its
     *   capacity, but this is okay too, since we're not required to give
     *   the caller everything they asked for.
     */
    return map(&buf, &bufl, inbuf_, inlen);
}

/* ------------------------------------------------------------------------ */
/*
 *   Plain ASCII local to UTF-8 mapper
 */

/*
 *   map a string from the single-byte local character set to UTF-8
 */
size_t CCharmapToUniASCII::map(char **outp, size_t *outlen,
                               const char *inp, size_t inlen) const
{
    size_t tot_outlen;

    /* we haven't written any characters to the output buffer yet */
    tot_outlen = 0;

    /* scan each character (character == byte) in the input string */
    for ( ; inlen > 0 ; --inlen, ++inp)
    {
        wchar_t uni;
        size_t csiz;

        /*
         *   map any character outside of the 7-bit range to U+FFFD, the
         *   Unicode REPLACEMENT CHARACTER, which is the standard way to
         *   represent characters that can't be mapped from an incoming
         *   character set
         */
        if (((unsigned char)*inp) > 127)
            uni = 0xfffd;
        else
            uni = ((wchar_t)(unsigned char)*inp);

        /* get the size of this character */
        csiz = utf8_ptr::s_wchar_size(uni);

        /* add it to the total output length */
        tot_outlen += csiz;

        /* if there's room, add it to our output buffer */
        if (*outlen >= csiz)
        {
            /* write it out */
            *outp += utf8_ptr::s_putch(*outp, uni);

            /* deduct it from the remaining output length */
            *outlen -= csiz;
        }
        else
        {
            /* there's no room - set the remaining output length to zero */
            *outlen = 0;
        }
    }

    /* return the total output length */
    return tot_outlen;
}

/* ------------------------------------------------------------------------ */
/*
 *   Trivial UTF8-to-UTF8 input mapper
 */

/*
 *   map a string
 */
size_t CCharmapToUniUTF8::map2(char **outp, size_t *outlen,
                               const char *inp, size_t inlen,
                               size_t *partial_len) const
{
    size_t copy_len;

    /*
     *   Make sure we copy only whole characters, by truncating the string
     *   to a length that includes only whole characters.
     */
    copy_len = utf8_ptr::s_trunc(inp, inlen);

    /*
     *   note the length of any partial characters at the end of the buffer
     *   for the caller - this is simply the difference between the original
     *   length and the truncated copy length, since the truncation length
     *   is simply the length excluding the partial last character bytes
     */
    *partial_len = inlen - copy_len;

    /* limit the copying to what will fit in the output buffer */
    if (copy_len > *outlen)
    {
        /* don't copy more than will fit, and don't copy partial characters */
        copy_len = utf8_ptr::s_trunc(inp, *outlen);

        /* we don't have enough room, so set the output size to zero */
        *outlen = 0;
    }
    else
    {
        /* we have room, so decrement the output size by the copy size */
        *outlen -= copy_len;
    }

    /* copy the data, if we have an output buffer */
    if (outp != 0)
    {
        /* copy the bytes */
        memcpy(*outp, inp, copy_len);

        /* validate that the bytes we copied are well-formed UTF-8 */
        validate(*outp, copy_len);

        /* advance the output pointer past the copied data */
        *outp += copy_len;
    }

    /*
     *   return the total input length -- the total output length is
     *   always identical to the input length, because we don't change
     *   anything
     */
    return inlen;
}

/*
 *   read a file
 */
size_t CCharmapToUniUTF8::read_file(CVmDataSource *fp,
                                    char *buf, size_t bufl)
{
    size_t read_len;
    char *last_start;
    size_t last_got_len;
    size_t last_need_len;

    /*
     *   Read directly from the file, up the buffer size minus two bytes.
     *   We want to leave two extra bytes so that we can read any extra
     *   continuation bytes for the last character, in order to ensure
     *   that we always read whole characters; in the worst case, the last
     *   character could be three bytes long, in which case we'd need to
     *   read two extra bytes.
     *
     *   If the available buffer size is less than three bytes, just read
     *   the number of bytes they asked for and don't bother trying to
     *   keep continuation sequences intact.
     */
    if (bufl < 3)
    {
        read_len = fp->readc(buf, bufl);
        validate(buf, read_len);
        return read_len;
    }

    /*
     *   read up to the buffer size, less two bytes for possible
     *   continuation bytes
     */
    read_len = fp->readc(buf, bufl - 2);

    /*
     *   if we didn't satisfy the entire request, we're at the end of the
     *   file, so there's no point in trying to finish off any
     *   continuation sequences - in this case, just return what we have
     */
    if (read_len < bufl - 2)
    {
        validate(buf, read_len);
        return read_len;
    }

    /*
     *   Check the last byte we read to see if there's another byte or two
     *   following.
     *
     *   If the last byte is a continuation byte, this is a bit trickier.
     *   We must back up to the preceding lead byte to figure out what we
     *   have in this case.
     */
    last_start = &buf[read_len - 1];
    last_got_len = 1;
    if (utf8_ptr::s_is_continuation(last_start))
    {
        /*
         *   if we only read one byte, simply return the one byte - we
         *   started in the middle of a sequence, so there's no way we can
         *   read a complete sequence
         */
        if (read_len == 1)
        {
            validate(buf, read_len);
            return read_len;
        }

        /* back up to the byte we're continuing from */
        --last_start;
        ++last_got_len;

        /*
         *   if this is another continuation byte, we've reached the maximum
         *   byte length of three for a single character, so there's no way
         *   we could need to read anything more
         */
        if (utf8_ptr::s_is_continuation(last_start))
        {
            validate(buf, read_len);
            return read_len;
        }
    }

    /*
     *   Okay: we have last_start pointing to the start of the last
     *   character, and last_got_len the number of bytes we actually have for
     *   that last character.  If the needed length differs from the length
     *   we actually have, we need to read more.
     */
    last_need_len = utf8_ptr::s_charsize(*last_start);
    if (last_need_len > last_got_len)
    {
        /*
         *   we need more than we actually read, so read the remaining
         *   characters
         */
        read_len += fp->readc(buf + read_len, last_need_len - last_got_len);
    }

    /* validate the buffer - ensure that it's well-formed UTF-8 */
    validate(buf, read_len);

    /* return the length we read */
    return read_len;
}

/* ------------------------------------------------------------------------ */
/*
 *   Basic UCS-2 to UTF-8 mapper
 */

/*
 *   Read from a file, translating to UTF-8 encoding
 */
size_t CCharmapToUniUcs2::read_file(CVmDataSource *fp,
                                    char *buf, size_t bufl)
{
    size_t inlen;

    /*
     *   Compute how much to read from the file.  The input file is composed
     *   of two-byte characters, so only read up to two thirds of the buffer
     *   length; this will ensure that we can always fit what we read into
     *   the caller's buffer.
     *
     *   Note that we divide by three first, then double the result, to
     *   ensure that we read an even number of bytes.  Each UCS-2 character
     *   is represented in exactly two bytes, so we must always read pairs of
     *   bytes to be sure we're reading whole characters.
     */
    inlen = bufl / 3;
    inlen *= 2;

    /* in any case, we can't read more than our own buffer size */
    if (inlen > sizeof(inbuf_))
        inlen = sizeof(inbuf_);

    /* read from the file */
    inlen = fp->readc(inbuf_, inlen);

    /*
     *   Map data to the caller's buffer, and return the result.  We're
     *   certain that the data will fit in the caller's buffer: we're
     *   mapping only a third as many characters as we have bytes
     *   available, and each character can take up at most three bytes,
     *   hence the worst case is that we fill the buffer completely.
     *
     *   On the other hand, we may only fill the buffer to a third of its
     *   capacity, but this is okay too, since we're not required to give
     *   the caller everything they asked for.
     */
    return map(&buf, &bufl, inbuf_, inlen);
}

/* ------------------------------------------------------------------------ */
/*
 *   UCS-2 little-endian to UTF-8 mapper
 */

/*
 *   map a string
 */
size_t CCharmapToUniUcs2Little::map(char **outp, size_t *outlen,
                                    const char *inp, size_t inlen) const
{
    size_t tot_outlen;

    /* we haven't written any characters to the output buffer yet */
    tot_outlen = 0;

    /* scan each character (character == byte pair) in the input string */
    for ( ; inlen > 1 ; inlen -= 2, inp += 2)
    {
        wchar_t uni;
        size_t csiz;

        /*
         *   read the little-endian two-byte value - no mapping is
         *   required, since UCS-2 uses the same code point assignments as
         *   UTF-8
         */
        uni = ((wchar_t)(unsigned char)*inp)
              + (((wchar_t)(unsigned char)*(inp + 1)) << 8);

        /* get the size of this character */
        csiz = utf8_ptr::s_wchar_size(uni);

        /* add it to the total output lenght */
        tot_outlen += csiz;

        /* if there's room, add it to our output buffer */
        if (*outlen >= csiz)
        {
            /* write it out */
            *outp += utf8_ptr::s_putch(*outp, uni);

            /* deduct it from the remaining output length */
            *outlen -= csiz;
        }
        else
        {
            /* there's no room - set the remaining output length to zero */
            *outlen = 0;
        }
    }

    /* return the total output length */
    return tot_outlen;
}

/* ------------------------------------------------------------------------ */
/*
 *   UCS-2 big-endian to UTF-8 mapper
 */

/*
 *   map a string
 */
size_t CCharmapToUniUcs2Big::map(char **outp, size_t *outlen,
                                 const char *inp, size_t inlen) const
{
    size_t tot_outlen;

    /* we haven't written any characters to the output buffer yet */
    tot_outlen = 0;

    /* scan each character (character == byte pair) in the input string */
    for ( ; inlen > 1 ; inlen -= 2, inp += 2)
    {
        wchar_t uni;
        size_t csiz;

        /*
         *   read the big-endian two-byte value - no mapping is required,
         *   since UCS-2 uses the same code point assignments as UTF-8
         */
        uni = (((wchar_t)(unsigned char)*inp) << 8)
              + ((wchar_t)(unsigned char)*(inp + 1));

        /* get the size of this character */
        csiz = utf8_ptr::s_wchar_size(uni);

        /* add it to the total output lenght */
        tot_outlen += csiz;

        /* if there's room, add it to our output buffer */
        if (*outlen >= csiz)
        {
            /* write it out */
            *outp += utf8_ptr::s_putch(*outp, uni);

            /* deduct it from the remaining output length */
            *outlen -= csiz;
        }
        else
        {
            /* there's no room - set the remaining output length to zero */
            *outlen = 0;
        }
    }

    /* return the total output length */
    return tot_outlen;
}
