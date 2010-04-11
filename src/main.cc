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

#include <iostream>
#include <QScrollArea>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

#include "os.h"
#include "trd.h"

#include "t3std.h"
#include "vmvsn.h"
#include "vmmain.h"
#include "vmconsol.h"
#include "vmmaincn.h"
#include "vmhost.h"
#include "resload.h"

#include "tadshtml.h"
#include "htmlprs.h"
#include "htmlfmt.h"
#include "htmlrf.h"
#include "htmlqt.h"

#include "qtadshostifc.h"


int main( int argc, char** argv )
{
	// Filename of the game to run.
	QString gameFileName;

	if (argc == 2) {
		if (QFile::exists(QString::fromLocal8Bit(argv[1]))) {
			gameFileName = QString::fromLocal8Bit(argv[1]);
		} else if (QFile::exists(QString::fromLocal8Bit(argv[1]) + ".gam")) {
			gameFileName = QString::fromLocal8Bit(argv[1]) + ".gam";
		} else if (QFile::exists(QString::fromLocal8Bit(argv[1]) + ".t3")) {
			gameFileName = QString::fromLocal8Bit(argv[1]) + ".t3";
		} else {
			std::cerr << "File `" << argv[1] << "' not found." << std::endl;
			return 2;
		}
	}

	CHtmlResType::add_basic_types();
	CHtmlSysFrameQt* app = new CHtmlSysFrameQt(argc, argv, "QTads", "2.0", "Nikos Chantziaras",
											   "qtads.sourceforge.net");
	CHtmlSysFrame::set_frame_obj(app);

	if (gameFileName.isEmpty()) {
		gameFileName = QFileDialog::getOpenFileName(0, "Choose the TADS game you wish to run", "",
													"TADS Games (*.gam *.Gam *.GAM *.t3 *.T3)");
	}

	QDir::setCurrent(QFileInfo(gameFileName).path());
	if (vm_get_game_type(QFileInfo(gameFileName).fileName().toLocal8Bit(), 0, 0, 0, 0) == VM_GGT_TADS2) {
		app->runT2Game(QFileInfo(gameFileName).fileName());
	} else if (vm_get_game_type(QFileInfo(gameFileName).fileName().toLocal8Bit(), 0, 0, 0, 0) == VM_GGT_TADS3) {
		app->runT3Game(QFileInfo(gameFileName).fileName());
	} else {
		std::cerr << gameFileName.toLocal8Bit().constData() << " is not a TADS game file.\n";
	}

	CHtmlSysFrame::set_frame_obj(0);
	delete app;
}
