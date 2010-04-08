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

#include <QSettings>

#include "qtadssettings.h"


void
QTadsSettings::loadFromDisk()
{
	QSettings sett;
	sett.beginGroup("fonts");
	this->mainFont.fromString(sett.value("main", "serif").toString());
	this->fixedFont.fromString(sett.value("fixed", "monospace").toString());
	this->serifFont.fromString(sett.value("serif", "serif").toString());
	this->sansFont.fromString(sett.value("sans", "sans-serif").toString());
	this->scriptFont.fromString(sett.value("script", "cursive").toString());
	this->writerFont.fromString(sett.value("typewriter", "monospace").toString());
	this->inputFont.fromString(sett.value("input", "serif").toString());
	//this->inputFont.setBold(sett.value("inputbold", false).toBool());
	//this->inputFont.setItalic(sett.value("inputitalic", false).toBool());
	sett.endGroup();

	/*
	qDebug() << "mainFont: " << QFontInfo(this->mainFont).family()
			<< "\nmainFont pt:" << QFontInfo(this->mainFont).pointSize()
			<< "\nmainFont px:" << QFontInfo(this->mainFont).pixelSize()
			<< "\nmainFont height:" << QFontMetrics(this->mainFont).height()
			<< "\nscriptFont: " << QFontInfo(this->scriptFont).family()
			<< "\nscriptFont pt:" << QFontInfo(this->scriptFont).pointSize()
			<< "\nscriptFont px:" << QFontInfo(this->scriptFont).pixelSize()
			<< "\nscriptFont height:" << QFontMetrics(this->scriptFont).height()
			;
	*/
}


void
QTadsSettings::saveToDisk()
{
	QSettings sett;
	sett.beginGroup("fonts");
	sett.setValue("main", this->mainFont.toString());
	sett.setValue("fixed", this->fixedFont.toString());
	sett.setValue("serif", this->serifFont.toString());
	sett.setValue("sans", this->sansFont.toString());
	sett.setValue("script", this->scriptFont.toString());
	sett.setValue("typewriter", this->writerFont.toString());
	sett.setValue("input", this->inputFont.toString());
	//sett.setValue("inputbold", this->inputFont.bold());
	//sett.setValue("inputitalic", this->inputFont.italic());
	sett.endGroup();
	sett.sync();
}
