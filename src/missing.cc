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
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <QString>
#include <QDebug>

#include "missing.h"

#ifndef _WIN32
int
memicmp( const char* s1, const char* s2, size_t len )
{	
	char* x1 = new char[len];
	char* x2 = new char[len];

	for (size_t i = 0; i < len; ++i) {
		x1[i] = std::tolower(s1[i]);
		x2[i] = std::tolower(s2[i]);
	}
	int ret = std::memcmp(x1, x2, len);
	delete[] x1;
	delete[] x2;
	return ret;
}
#endif


int
stricmp( const char* s1, const char* s2 )
{
	return QString::localeAwareCompare(QString(s1).toLower(), QString(s2).toLower());
}


int
strnicmp( const char* s1, const char* s2, size_t n )
{
	QString qs1(s1);
	QString qs2(s2);

	qs1.truncate(n);
	qs2.truncate(n);

	return QString::compare(qs1.toLower(), qs2.toLower());
}
