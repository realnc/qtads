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

/* This is a replacement for tads3/derived/vmuni_cs.cpp and
 * tads3/vmuni.h.  The T3 VM's Unicode data tables are huge; we don't
 * want to use them since Qt provides its own tables.  We simply provide
 * trivial replacements for the functions and let Qt do the work.
 */
#ifndef VMUNI_H
#define VMUNI_H

#include <QChar>

/* Character types.  Types are mutually exclusive, so a character has
 * exactly one type.
 */
#define T3_CTYPE_UNDEF  0                        /* character isn't defined */
#define T3_CTYPE_ALPHA  1           /* alphabetic, with no case information */
#define T3_CTYPE_UPPER  2                          /* upper-case alphabetic */
#define T3_CTYPE_LOWER  3                          /* lower-case alphabetic */
#define T3_CTYPE_DIGIT  4                                          /* digit */
#define T3_CTYPE_SPACE  5                          /* horizontal whitespace */
#define T3_CTYPE_PUNCT  6                                    /* punctuation */
#define T3_CTYPE_VSPACE 7                            /* vertical whitespace */
#define T3_CTYPE_NONE   8       /* character doesn't fit any other category */

/* Get the character type.
 */
inline unsigned char
t3_get_chartype( wchar_t ch )
{
    const QChar c(ch);

    switch(c.category()) {
      case QChar::NoCategory:
        return T3_CTYPE_UNDEF;

      case QChar::Letter_Titlecase:
      case QChar::Letter_Modifier:
      case QChar::Letter_Other:
        return T3_CTYPE_ALPHA;

      case QChar::Letter_Uppercase:
        return T3_CTYPE_UPPER;

      case QChar::Letter_Lowercase:
        return T3_CTYPE_LOWER;

      case QChar::Number_DecimalDigit:
        return T3_CTYPE_DIGIT;

      case QChar::Separator_Space:
        return T3_CTYPE_SPACE;

      case QChar::Punctuation_Connector:
      case QChar::Punctuation_Dash:
      case QChar::Punctuation_Open:
      case QChar::Punctuation_Close:
      case QChar::Punctuation_InitialQuote:
      case QChar::Punctuation_FinalQuote:
      case QChar::Punctuation_Other:
        return T3_CTYPE_PUNCT;

      case QChar::Separator_Line:
      case QChar::Separator_Paragraph:
        return T3_CTYPE_VSPACE;

      case QChar::Other_Control:
        if (ch == 0x0B) {
            /* TADS-specific \b character -> vertical whitespace */
            return T3_CTYPE_VSPACE;
        }
        if (ch == 0x0E || ch == 0x0F) {
            /* TADS-specific \v and \^ -> device controls */
            return T3_CTYPE_NONE;
        }
        if (ch == 0x15) {
            /* TADS-specific '\ ' quoted space -> ordinary character */
            return T3_CTYPE_NONE;
        }

      default:
        return T3_CTYPE_NONE;
    }
}

/* Alphabetic?
 */
inline int
t3_is_alpha( wchar_t ch )
{
    return QChar(ch).isLetter();
}

/* Uppercase?
 */
inline int
t3_is_upper( wchar_t ch )
{
    return QChar(ch).isUpper();
}

/* Lowercase?
 */
inline int
t3_is_lower( wchar_t ch )
{
    return QChar(ch).isLower();
}

/* Digit?
 */
inline int
t3_is_digit( wchar_t ch )
{
    return QChar(ch).isDigit();
}

/* Whitespace?
 */
inline int
t3_is_space( wchar_t ch )
{
    // *Not* QChar.isSpace(), as it includes vertical space.
    return QChar(ch).category() == QChar::Separator_Space;
}

/* Vertical whitespace?
 */
inline int
t3_is_vspace( wchar_t ch )
{
    return t3_get_chartype(ch) == T3_CTYPE_VSPACE;
}

/* Any whitespace, horizontal or vertical?
 */
inline int
t3_is_whitespace( wchar_t ch )
{
    int t = t3_get_chartype(ch);
    return t == T3_CTYPE_SPACE or t == T3_CTYPE_VSPACE;
}

/* Punctuation?
 */
inline int
t3_is_punct( wchar_t ch )
{
    return QChar(ch).isPunct();
}

/* Is it a defined unicode character?
 */
inline int
t3_is_unichar( wchar_t ch )
{
    return (t3_get_chartype(ch) != T3_CTYPE_UNDEF);
}

/* Convert to upper case.
 */
inline wchar_t
t3_to_upper( wchar_t ch )
{
    return QChar::toUpper(sizeof(ch) > 2 ? static_cast<uint>(ch) : static_cast<ushort>(ch));
}

/* Convert to lower case.
 */
inline wchar_t
t3_to_lower( wchar_t ch )
{
    return QChar::toLower(sizeof(ch) > 2 ? static_cast<uint>(ch) : static_cast<ushort>(ch));
}

#endif // VMUNI_H
