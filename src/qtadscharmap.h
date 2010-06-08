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

/* Some of these routines are used *a lot* by the T3VM.  Therefore, we
 * implement them here instead of in qtadscharmap.cc, so that the compiler's
 * optimizer can inline them.
 *
 * One might think that they can't be inlined, since they're virtual.  This
 * isn't true; a decent optimizer can sometimes inline even virtual functions.
 */
#ifndef QTADSCHARMAP_H
#define QTADSCHARMAP_H

#include <cstring>

#include <QTextCodec>
#include <QByteArray>
#include <QString>
#include <QDebug>

#include "charmap.h"


/* QTadsCharmapToLocal translates UTF-8 encoded unicode to the local character
 * set.  A 7-bit ASCII fallback is used when no real mapping exists.  For
 * example, if the local charset lacks the AE ligature, the ASCII fallback will
 * expand it to "AE".  Same goes for other unmappable characters; "(c)" for the
 * circled copyright-symbol, and ASCII quotes instead of curly quotes, for
 * example.
 */
class QTadsCharmapToLocal: public CCharmapToLocal
{
  private:
	// Our Unicode -> Local codec.
	QTextCodec* fLocalCodec;
	// Our Unicode -> 7bit ASCII fall-back codec.
	CCharmapToLocalASCII fASCIICodec;

  public:
	QTadsCharmapToLocal()
	// Ask Qt for a suitable codec.  Note that the returned pointer refers to
	// an already existing codec; we are not allowed to delete it.
	: fLocalCodec(QTextCodec::codecForLocale())
	{ }

	// Convert a character from Unicode to the local character set.
	virtual size_t
	map( wchar_t unicode_char, char** output_ptr, size_t* output_buf_len ) const;

	// Determine if the given Unicode character has a mapping to the local
	// character set.
	//
	// We also check if it's mappable by the ASCII fall-back, which means that
	// we consider the character to be mappable even if no real 1:1 mapping
	// exists (in other words, if an expansion exists, we claim it's mappable).
	virtual int
	is_mappable( wchar_t unicode_char ) const;

	// TODO: Provide custom implementations for the (commented out) methods
	// below for more efficiency.  The problem with these methods is that they
	// don't allow an ASCII fall-back.  The default implementation of these
	// methods (in our base class), simply calls map() for each individual
	// character.  This is slower than letting Qt map the whole string at once,
	// but it allows the ASCII fall-back to work.  I can't think of a way to
	// work around this problem, so I'll not implement them for now.

	// Convert a UTF-8 string with a given byte length to the local character
	// set.
	//virtual size_t
	//map_utf8( char* dest, size_t dest_len, utf8_ptr src, size_t src_byte_len, size_t* src_bytes_used ) const;

	// Convert a null-terminated UTF-8 string to the local character set.
	//virtual size_t
	//map_utf8z( char* dest, size_t dest_len, utf8_ptr src ) const;
};


/* QTadsCharmapToUni translates from the local character set to UTF-8 encoded
 * Unicode.
 */
class QTadsCharmapToUni: public CCharmapToUni
{
  private:
	// Our Local -> Unicode codec.
	QTextCodec* fLocalCodec;

  public:
	QTadsCharmapToUni()
	// Ask Qt for a suitable codec.  Note that the returned pointer refers to
	// an already existing codec; we are not allowed to delete it.
	: fLocalCodec(QTextCodec::codecForLocale())
	{ }

	// Determine if the given byte sequence forms a complete character.
	virtual int
	is_complete_char( const char* p, size_t len ) const;

	// Convert a string from the local character set to Unicode (UTF-8).
	virtual size_t
	map( char** output_ptr, size_t* output_buf_len, const char* input_ptr, size_t input_len ) const;

	// Convert a string from the local character set to Unicode (UTF-8).
	// TODO: Implement partial_len.
	virtual size_t
	map2( char** output_ptr, size_t* output_buf_len, const char* input_ptr, size_t input_len,
		  size_t* partial_len ) const;

	// Read characters from a file into a buffer, translating the characters to
	// UTF-8.
	virtual size_t
	read_file( osfildef* fp, char* buf, size_t bufl, unsigned long read_limit );

	// Only defined here because the linker barks.
	virtual void
	set_mapping( wchar_t /*uni_code_pt*/, wchar_t /*local_code_pt*/ )
	{
		qWarning("QTadsCharmapToUni::set_mapping() called");
	}
};


/* --------------------------------------------------------------------
 * QTadsCharmapToLocal inline implementations.
 */
inline size_t
QTadsCharmapToLocal::map( wchar_t unicode_char, char** output_ptr, size_t* output_buf_len ) const
{
	// If Qt can't map, use the ASCII fall-back.
	if (not this->fLocalCodec->canEncode(unicode_char)) {
		return this->fASCIICodec.map(unicode_char, output_ptr, output_buf_len);
	}

	// Get the mapping of the character and store its length.
	const QByteArray& res = this->fLocalCodec->fromUnicode(QChar(unicode_char));
	const uint& len = res.length();

	if (*output_buf_len < len) {
		// There's no room in the output-buffer.
		*output_buf_len = 0;
	} else {
		// Copy the mapping to the output-buffer.
		std::memcpy(*output_ptr, res, len);
		*output_ptr += len;
		*output_buf_len -= len;
	}
	return len;
}

inline int
QTadsCharmapToLocal::is_mappable( wchar_t unicode_char ) const
{
	return this->fLocalCodec->canEncode(unicode_char) or this->fASCIICodec.is_mappable(unicode_char);
}


/* --------------------------------------------------------------------
 * QTadsCharmapToUni inline implementations.
 */
inline int
QTadsCharmapToUni::is_complete_char( const char* p, size_t len ) const
{
	// We create a decoder and try to decode.  If the result is empty, the
	// decoder buffered the sequence which means that it was not a complete
	// character.
	//
	// TODO: Test if it really works; the Qt docs say that this decoder is
	// stateless.
	QTextDecoder* decoder = this->fLocalCodec->makeDecoder();
	if (decoder->toUnicode(p, len).isEmpty()) {
		delete decoder;
		return false;
	}
	delete decoder;
	return true;
}

inline size_t
QTadsCharmapToUni::map( char** output_ptr, size_t* output_buf_len, const char* input_ptr, size_t input_len ) const
{
	// Map the string.
	const QByteArray& mapped = this->fLocalCodec->toUnicode(input_ptr, input_len).toUtf8();

	if (static_cast<size_t>(mapped.length()) >= *output_buf_len) {
		// The result totally fills or exceeds the output buffer; don't write
		// past the buffer's boundary.
		std::strncpy(*output_ptr, mapped, *output_buf_len);
		*output_ptr += *output_buf_len;
		*output_buf_len = 0;
	} else {
		// No special precautions needed, since the result is smaller than the
		// buffer's capacity; the terminating '\0' fits too.
		std::strcpy(*output_ptr, mapped);
		*output_ptr += mapped.length();
		*output_buf_len -= mapped.length();
	}
	return mapped.length();
}

inline size_t
QTadsCharmapToUni::map2( char** output_ptr, size_t* output_buf_len, const char* input_ptr, size_t input_len,
						 size_t* partial_len ) const
{
	qDebug() << Q_FUNC_INFO;
	// No idea how to implement that one.
	*partial_len = 0;
	return this->map(output_ptr, output_buf_len, input_ptr, input_len);
}


#endif // QTADSCHARMAP_H
