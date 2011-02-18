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
#include <QMetaType>
#include <QFileDialog>

#ifndef Q_WS_ANDROID
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_sound.h>
#endif

#include "settings.h"
#include "sysframe.h"
#include "qtadssound.h"
#include "syssoundmidi.h"


// On some platforms, SDL redefines main in order to provide a
// platform-specific main() implementation.  However, Qt handles this too,
// so things can get weird.  We need to make sure main is not redefined so
// that Qt can find our own implementation and SDL will not try to do
// platform-specific initialization work (like launching the Cocoa event-loop
// or setting up the application menu on OS X, or redirecting stdout and stderr
// to text files on Windows), which would break things.
#ifdef main
#  undef main
#endif

int main( int argc, char** argv )
{
	CHtmlResType::add_basic_types();
	CHtmlSysFrameQt* app = new CHtmlSysFrameQt(argc, argv, "QTads", "2.0", "Nikos Chantziaras",
											   "qtads.sourceforge.net");

	// Filename of the game to run.
	QString gameFileName;

	const QStringList& args = app->arguments();
	if (args.size() == 2) {
		if (QFile::exists(args.at(1))) {
			gameFileName = args.at(1);
		} else if (QFile::exists(args.at(1) + QString::fromAscii(".gam"))) {
			gameFileName = args.at(1) + QString::fromAscii(".gam");
		} else if (QFile::exists(args.at(1) + QString::fromAscii(".t3"))) {
			gameFileName = args.at(1) + QString::fromAscii(".t3");
		} else {
			qWarning() << "File" << args.at(1) << "not found.";
		}
	}

	if (gameFileName.isEmpty() and app->settings()->askForGameFile) {
		gameFileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the TADS game you wish to run"),
													QString::fromAscii(""),
													QObject::tr("TADS Games")
													+ QString::fromAscii("(*.gam *.Gam *.GAM *.t3 *.T3)"));
	}

#ifndef Q_WS_ANDROID
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		qWarning("Unable to initialize sound system: %s", SDL_GetError());
		return 1;
	}

	// Initialize SDL_sound.
	Sound_Init();

	// This will preload the needed codecs now instead of constantly loading
	// and unloading them each time a sound is played/stopped.  This is only
	// available in SDL_Mixer 1.2.10 and newer.
#if (MIX_MAJOR_VERSION > 1) \
	|| ((MIX_MAJOR_VERSION == 1) && (MIX_MINOR_VERSION > 2)) \
	|| ((MIX_MAJOR_VERSION == 1) && (MIX_MINOR_VERSION == 2) && (MIX_PATCHLEVEL > 9))
	int sdlFormats = MIX_INIT_OGG;
	if (Mix_Init((sdlFormats & sdlFormats) != sdlFormats)) {
		qWarning("Unable to load Ogg Vorbis support: %s", Mix_GetError());
		return 1;
	}
#endif

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
		qWarning("Unable to initialize audio mixer: %s", Mix_GetError());
		return 1;
	}
	Mix_AllocateChannels(16);
	Mix_ChannelFinished(QTadsSound::callback);
	Mix_HookMusicFinished(CHtmlSysSoundMidiQt::callback);

	/*
	int numtimesopened, frequency, channels;
	Uint16 format;
	numtimesopened=Mix_QuerySpec(&frequency, &format, &channels);
	if(numtimesopened) {
		printf("Mix_QuerySpec: %s\n",Mix_GetError());
	}
	else {
		char *format_str="Unknown";
		switch(format) {
			case AUDIO_U8: format_str="U8"; break;
			case AUDIO_S8: format_str="S8"; break;
			case AUDIO_U16LSB: format_str="U16LSB"; break;
			case AUDIO_S16LSB: format_str="S16LSB"; break;
			case AUDIO_U16MSB: format_str="U16MSB"; break;
			case AUDIO_S16MSB: format_str="S16MSB"; break;
		}
		qDebug("opened=%d times  frequency=%dHz  format=%s  channels=%d",
				numtimesopened, frequency, format_str, channels);
	}
	*/
	/*
	int max = Mix_GetNumChunkDecoders();
	for(int i = 0; i < max; ++i) {
		qDebug() << "Sample chunk decoder" << i << "is for" << Mix_GetChunkDecoder(i);
	}
	max = Mix_GetNumMusicDecoders();
	for(int i = 0; i < max; ++i) {
		qDebug() << "Sample chunk decoder" << i << "is for" << Mix_GetMusicDecoder(i);
	}
	*/
#endif

	QMetaObject::invokeMethod(app, "main", Qt::QueuedConnection, Q_ARG(QString, gameFileName));
	int ret = app->exec();

	delete app;
#ifndef Q_WS_ANDROID
	Mix_ChannelFinished(0);
	Mix_HookMusicFinished(0);
	// Close the audio device as many times as it was opened.
	// We disable this for now since it results in a crash in some systems.
	// It looks like a clash between SDL_mixer and SDL_sound.
	/*
	int opened = Mix_QuerySpec(0, 0, 0);
	for (int i = 0; i < opened; ++i) {
		Mix_CloseAudio();
	}
	*/
	Sound_Quit();
	SDL_Quit();
#endif
	return ret;
}

