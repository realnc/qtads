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

#include <QTimer>
#include <QMetaType>

#include "os.h"
#include "tadshtml.h"

#include "htmlqt.h"


// On OS X, SDL does weird stuff with main() and redefines it to SDLMain().
// We don't like that.
#ifdef Q_WS_MAC
#  ifdef main
#    undef main
#  endif
#endif
int main( int argc, char** argv )
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		qWarning("Unable to initialize sound system: %s", SDL_GetError());
		return 1;
	}

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

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
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

	CHtmlResType::add_basic_types();
	CHtmlSysFrameQt* app = new CHtmlSysFrameQt(argc, argv, "QTads", "2.0", "Nikos Chantziaras",
											   "qtads.sourceforge.net");
	qRegisterMetaType<char**>("char**");
	QMetaObject::invokeMethod(app, "main", Qt::QueuedConnection, Q_ARG(int, argc), Q_ARG(char**, argv));
	int ret = app->exec();

	delete app;
	Mix_ChannelFinished(0);
	Mix_HookMusicFinished(0);
	Mix_CloseAudio();
	SDL_Quit();
	return ret;
}
