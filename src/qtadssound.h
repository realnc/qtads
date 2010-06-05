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
#ifndef QTADSSOUND_H
#define QTADSSOUND_H

#include <QObject>
#include <QTime>

#include "tadshtml.h"


/* Provides the common code for all three types of digitized sound (WAV,
 * Ogg Vorbis and MP3).
 */
class QTadsSound: public QObject {
	Q_OBJECT

  public:
	enum SoundType { WAV, OGG, MPEG };

  private:
	struct Mix_Chunk* fChunk;
	int fChannel;
	SoundType fType;
	bool fPlaying;

	// TADS callback to invoke on stop.
	void (*fDone_func)(void*, int repeat_count);

	// CTX to pass to the TADS callback.
	void* fDone_func_ctx;

	// How many times we repeated the sound.
	int fRepeats;

	// How many times should we repeat the sound.
	// 0 means repeat forever.
	int fRepeatsWanted;

	// How long has the sound been playing.
	QTime fTime;

	// Total length of the sound in milliseconds.
	unsigned fLength;

	// All QTadsMediaObjects that currently exist.  We need this in order to
	// implement the SDL_Mixer callback (which in turn needs to call the TADS
	// callback) that is invoked after a channel has stopped playing.  That
	// callback has to be a static member (C++ methods can't be C callbacks),
	// and since there's no 'this' pointer in static member functions, it needs
	// to invoke the TADS callback based on the channel number.
	static QList<QTadsSound*> fObjList;

  public:
	QTadsSound( QObject* parent, struct Mix_Chunk* chunk, SoundType type );

	virtual
	~QTadsSound();

	// The SDL_Mixer callbacks.
	static void callback( int channel );
	static void effectCallback( int chan, void* stream, int len, void* udata );

	int
	startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat, int vol );

	void
	cancelPlaying( bool sync );

	static class CHtmlSysSound*
	createSound( const class CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
				 unsigned long filesize, class CHtmlSysWin* win, SoundType type );
};


#endif
