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
#include "syswingroup.h"
#include "globals.h"


void
QTadsSettings::loadFromDisk()
{
	QSettings sett;

	sett.beginGroup("media");
	this->enableGraphics = sett.value("graphics", true).toBool();
	this->enableDigitalSound = sett.value("sounds", true).toBool();
	this->enableMidiSound = sett.value("music", true).toBool();
	this->enableLinks = sett.value("links", true).toBool();
	sett.endGroup();

	sett.beginGroup("colors");
	this->mainBgColor = sett.value("mainbg", QColor(Qt::white)).value<QColor>();
	this->mainTextColor = sett.value("maintext", QColor(Qt::black)).value<QColor>();
	this->bannerBgColor = sett.value("bannerbg", QColor(Qt::lightGray)).value<QColor>();
	this->bannerTextColor = sett.value("bannertext", QColor(Qt::black)).value<QColor>();
	this->inputColor = sett.value("input", QColor(Qt::black)).value<QColor>();
	this->underlineLinks = sett.value("underlinelinks", false).toBool();
	this->highlightLinks = sett.value("highlightlinks", true).toBool();
	this->unvisitedLinkColor = sett.value("unvisitedlinks", QColor(Qt::blue)).value<QColor>();
	this->hoveringLinkColor = sett.value("hoveringlinks", QColor(Qt::red)).value<QColor>();
	this->clickedLinkColor = sett.value("clickedlinks", QColor(Qt::cyan)).value<QColor>();
	sett.endGroup();

	sett.beginGroup("fonts");
	this->mainFont.fromString(sett.value("main", "serif").toString());
	this->fixedFont.fromString(sett.value("fixed", "monospace").toString());
	this->serifFont.fromString(sett.value("serif", "serif").toString());
	this->sansFont.fromString(sett.value("sans", "sans-serif").toString());
	this->scriptFont.fromString(sett.value("script", "cursive").toString());
	this->writerFont.fromString(sett.value("typewriter", "monospace").toString());
	this->inputFont.fromString(sett.value("input", "serif").toString());
	sett.endGroup();

	this->appSize = sett.value("geometry/size", QSize(740, 540)).toSize();
}


void
QTadsSettings::saveToDisk()
{
	QSettings sett;

	sett.beginGroup("media");
	sett.setValue("graphics", this->enableGraphics);
	sett.setValue("sounds", this->enableDigitalSound);
	sett.setValue("music", this->enableMidiSound);
	sett.setValue("links", this->enableLinks);
	sett.endGroup();

	sett.beginGroup("colors");
	sett.setValue("mainbg", this->mainBgColor);
	sett.setValue("maintext", this->mainTextColor);
	sett.setValue("bannerbg", this->bannerBgColor);
	sett.setValue("bannertext", this->bannerTextColor);
	sett.setValue("input", this->inputColor);
	sett.setValue("underlinelinks", this->underlineLinks);
	sett.setValue("highlightlinks", this->highlightLinks);
	sett.setValue("unvisitedlinks", this->unvisitedLinkColor);
	sett.setValue("hoveringlinks", this->hoveringLinkColor);
	sett.setValue("clickedlinks", this->clickedLinkColor);
	sett.endGroup();

	sett.beginGroup("fonts");
	sett.setValue("main", this->mainFont.toString());
	sett.setValue("fixed", this->fixedFont.toString());
	sett.setValue("serif", this->serifFont.toString());
	sett.setValue("sans", this->sansFont.toString());
	sett.setValue("script", this->scriptFont.toString());
	sett.setValue("typewriter", this->writerFont.toString());
	sett.setValue("input", this->inputFont.toString());
	sett.endGroup();

	sett.setValue("geometry/size", qWinGroup->size());
	sett.sync();
}
