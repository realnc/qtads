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
#define T3_CTYPE_NONE   0  // Character doesn't fit any other category.
#define T3_CTYPE_ALPHA  1  // Alphabetic, with no case information.
#define T3_CTYPE_UPPER  2  // Upper-case alphabetic.
#define T3_CTYPE_LOWER  3  // Lower-case alphabetic.
#define T3_CTYPE_DIGIT  4  // Digit.
#define T3_CTYPE_SPACE  5  // Horizontal whitespace.
#define T3_CTYPE_PUNCT  6  // Punctuation.

/* Get the character type.
 */
inline unsigned char
t3_get_chartype( wchar_t ch )
{
    const QChar c(ch);

    switch(c.category()) {
      case QChar::Number_DecimalDigit:
        return T3_CTYPE_DIGIT;

      case QChar::Separator_Space:
        return T3_CTYPE_SPACE;

      case QChar::Letter_Uppercase:
        return T3_CTYPE_UPPER;

      case QChar::Letter_Lowercase:
        return T3_CTYPE_LOWER;

      case QChar::Letter_Titlecase:
      case QChar::Letter_Modifier:
      case QChar::Letter_Other:
        return T3_CTYPE_ALPHA;

      case QChar::Punctuation_Connector:
      case QChar::Punctuation_Dash:
      case QChar::Punctuation_Open:
      case QChar::Punctuation_Close:
      case QChar::Punctuation_InitialQuote:
      case QChar::Punctuation_FinalQuote:
      case QChar::Punctuation_Other:
        return T3_CTYPE_PUNCT;

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
    return QChar(ch).category() == QChar::Letter_Uppercase;
}

/* Lowercase?
 */
inline int
t3_is_lower( wchar_t ch )
{
    return QChar(ch).category() == QChar::Letter_Lowercase;
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

/* Punctuation?
 */
inline int
t3_is_punct( wchar_t ch )
{
    return QChar(ch).isPunct();
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
