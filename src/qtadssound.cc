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
#include <QDebug>
#include <QFileInfo>
#include <QTimer>
#include <SDL_mixer.h>
#include <SDL_sound.h>

#include "qtadssound.h"
#include "globals.h"
#include "sysframe.h"
#include "settings.h"
#include "syssoundwav.h"
#include "syssoundogg.h"
#include "syssoundmpeg.h"


QList<QTadsSound*> QTadsSound::fObjList;


QTadsSound::QTadsSound( QObject* parent, Mix_Chunk* chunk, SoundType type )
: QObject(parent), fChunk(chunk), fChannel(-1), fType(type), fPlaying(false), fFadeOut(0), fCrossFade(false),
  fDone_func(0), fDone_func_ctx(0), fRepeats(0), fRepeatsWanted(1)
{
	// FIXME: Calculate sound length in a safer way.
	this->fLength = (this->fChunk->alen * 8) / (2 * 16 * 44.1);
	//qDebug() << "Sound length:" << this->fLength;
	connect(this, SIGNAL(readyToLoop()), SLOT(fDoLoop()));
	connect(this, SIGNAL(readyToFadeOut()), SLOT(fPrepareFadeOut()));
	connect(&this->fFadeOutTimer, SIGNAL(timeout()), this, SLOT(fDoFadeOut()));

	// Make sure the timer only calls our fade-out slot *once* and then stops.
	this->fFadeOutTimer.setSingleShot(true);
}


QTadsSound::~QTadsSound()
{
	//qDebug() << Q_FUNC_INFO;
	this->fRepeatsWanted = -1;
	// If our chunk is still playing, halt it.
	if (Mix_GetChunk(this->fChannel) == this->fChunk and Mix_Playing(this->fChannel) == 1) {
		Mix_HaltChannel(this->fChannel);
	}
	// Wait till it finished before deleting it.
	if (Mix_GetChunk(this->fChannel) == this->fChunk) {
		while (Mix_Playing(this->fChannel) != 0) {
			SDL_Delay(10);
		}
	}
	// Free the raw audio data buffer if SDL_mixer didn't allocate it itself.
	if (not this->fChunk->allocated) {
		free(this->fChunk->abuf);
	}
	Mix_FreeChunk(this->fChunk);
}


void
QTadsSound::fDoFadeOut()
{
	Q_ASSERT(Mix_GetChunk(this->fChannel) == this->fChunk and Mix_Playing(this->fChannel) == 1);

	// If we need to do a crossfade, call the TADS callback now.
	if (this->fCrossFade and this->fDone_func) {
		this->fDone_func(this->fDone_func_ctx, this->fRepeats);
		// Make sure our SDL callback won't call the TADS callback a second
		// time.
		this->fDone_func = 0;
		this->fDone_func_ctx = 0;
	}
	Mix_FadeOutChannel(this->fChannel, this->fFadeOut);
}


void
QTadsSound::fDoLoop()
{
	// When playing again, we can't assume that we can use the same channel.
	this->fChannel = Mix_PlayChannel(-1, this->fChunk, 0);
	this->fTimePos.start();
	++this->fRepeats;

	// If this is the last iteration and we have a fade-out, set the fade-out
	// timer.
	if (this->fFadeOut > 0 and (this->fRepeatsWanted == -1 or this->fRepeats == this->fRepeatsWanted)) {
		this->fFadeOutTimer.start(this->fLength - this->fFadeOut);
	}
}


void
QTadsSound::fPrepareFadeOut()
{
	this->fFadeOutTimer.start(this->fLength - this->fFadeOut);
}


void
QTadsSound::callback( int channel )
{
	QTadsSound* mObj = 0;
	int index;
	// Find the object that uses the specified channel.
	for (int i = 0; i < fObjList.size() and mObj == 0; ++i) {
		if (fObjList.at(i)->fChannel == channel) {
			mObj = fObjList.at(i);
			index = i;
		}
	}

	if (mObj == 0) {
		return;
	}

	// If it's an infinite loop sound, or it has not reached the wanted repeat
	// count yet, play again.
	if ((mObj->fRepeatsWanted == 0) or (mObj->fRepeats < mObj->fRepeatsWanted)) {
		mObj->emitReadyToLoop();
		return;
	}

	// Remove the object from the list.  Since it can be included several
	// times, only remove the instance we associated to the channel we're
	// handling.
	fObjList.removeAt(index);

	// Sound has repeated enough times, or it has been halted.  In either case,
	// we need to invoke the TADS callback, if there is one.
	mObj->fPlaying = false;
	if (mObj->fDone_func) {
		mObj->fDone_func(mObj->fDone_func_ctx, mObj->fRepeats);
	}
}


int
QTadsSound::startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat, int vol,
						  int fadeIn, int fadeOut, bool crossFade )
{
	// Check if user disabled digital sound.
	if (not qFrame->settings()->enableDigitalSound) {
		return 1;
	}

	Q_ASSERT(not this->fPlaying);

	// Adjust volume if it exceeds min/max levels.
	if (vol < 0) {
		vol = 0;
	} else if (vol > 100) {
		vol = 100;
	}

	// Convert the TADS volume level semantics [0..100] to SDL volume
	// semantics [0..MIX_MAX_VOLUME].
	vol = (vol * MIX_MAX_VOLUME) / 100;

	// Set the volume level.
	Mix_VolumeChunk(this->fChunk, vol);

	this->fRepeatsWanted = repeat;
	if (fadeIn > 0) {
		this->fChannel = Mix_FadeInChannel(-1, this->fChunk, 0, fadeIn);
	} else {
		this->fChannel = Mix_PlayChannel(-1, this->fChunk, 0);
	}
	if (this->fChannel == -1) {
		qWarning() << "ERROR:" << Mix_GetError();
		Mix_SetError("");
		return 1;
	} else {
		this->fTimePos.start();
		this->fPlaying = true;
		this->fCrossFade = crossFade;
		this->fRepeats = 1;
		QTadsSound::fObjList.append(this);
		this->fDone_func = done_func;
		this->fDone_func_ctx = done_func_ctx;
		if (fadeOut > 0) {
			this->fFadeOut = fadeOut;
			if (repeat == 1) {
				// The sound should only be played once.  We need to set the
				// fade-out timer here since otherwise the sound won't get a
				// chance to fade-out.
				emit readyToFadeOut();
			}
		}
	}
	return 0;
}


void
QTadsSound::cancelPlaying( bool sync, int fadeOut, bool fadeOutInBg )
{
	if (not this->fPlaying) {
		return;
	}

	this->fRepeatsWanted = -1;

	if (not sync and fadeOut > 0) {
		if (fadeOutInBg and this->fDone_func) {
			// We need to do fade-out in the background; call the TADS callback
			// now.
			this->fDone_func(this->fDone_func_ctx, this->fRepeats);
			// Make sure our SDL callback won't call the TADS callback a second
			// time.
			this->fDone_func = 0;
			this->fDone_func_ctx = 0;
		}
		Mix_FadeOutChannel(this->fChannel, fadeOut);
	} else {
		Mix_HaltChannel(this->fChannel);
	}

	if (sync and Mix_GetChunk(this->fChannel) == this->fChunk) {
		// The operation needs to be synchronous; wait for the sound to finish.
		while (Mix_Playing(this->fChannel) != 0) {
			SDL_Delay(10);
		}
	}
}


void
QTadsSound::addCrossFade( int ms )
{
	this->fCrossFade = true;
	this->fFadeOut = ms;

	if (this->fPlaying) {
		this->fRepeatsWanted = -1;
		int timeFromNow = this->fLength - this->fFadeOut - this->fTimePos.elapsed();
		if (timeFromNow < 1) {
			timeFromNow = 1;
			this->fFadeOut = this->fLength - this->fTimePos.elapsed();
		}
		this->fFadeOutTimer.start(timeFromNow);
	}
}


CHtmlSysSound*
QTadsSound::createSound( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
						 unsigned long filesize, CHtmlSysWin* win, SoundType type )
{
	//qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
	//		<< "url:" << url->get_url();

	// Check if the file exists and is readable.
	QFileInfo inf(QString::fromLocal8Bit(filename));
	if (not inf.exists() or not inf.isReadable()) {
		qWarning() << "ERROR:" << inf.filePath() << "doesn't exist or is unreadable";
		return 0;
	}

	// Open the file and seek to the specified position.
	QFile file(inf.filePath());
	if (not file.open(QIODevice::ReadOnly)) {
		qWarning() << "ERROR: Can't open file" << inf.filePath();
		return 0;
	}
	if (not file.seek(seekpos)) {
		qWarning() << "ERROR: Can't seek in file" << inf.filePath();
		file.close();
		return 0;
	}
	QByteArray data(file.read(filesize));
	file.close();
	if (data.isEmpty() or static_cast<unsigned long>(data.size()) < filesize) {
		qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << inf.filePath();
		return 0;
	}

	// Create the RWops through which the data will be read later.
	SDL_RWops* rw = SDL_RWFromConstMem(data.constData(), data.size());
	if (rw == 0) {
		qWarning() << "ERROR:" << SDL_GetError();
		SDL_ClearError();
		return 0;
	}

	// The chunk that will hold the final, decoded sound.
	Mix_Chunk* chunk;

	// If it's an MP3 or WAV, we'll decode it with SDL_sound.  For Ogg Vorbis
	// we use SDL_mixer.  The reason is that SDL_mixer plays WAV at wrong
	// speeds if they're not 11, 22 or 44kHz (like 48kHz or 32kHz) and crashes
	// sometimes with MP3s.  SDL_sound can't cope well with Ogg Vorbis that
	// have more then two channels.
	if (type == MPEG or type == WAV) {
		if (rw == 0) {
			qWarning() << "ERROR:" << SDL_GetError();
			SDL_ClearError();
			return 0;
		}
		Sound_AudioInfo wantedFormat;
		wantedFormat.channels = 2;
		wantedFormat.rate = 44100;
		wantedFormat.format = MIX_DEFAULT_FORMAT;
		// Note that we use a large buffer size to speed-up decoding of large
		// MP3s on Windows; the decoding will take extremely long with small
		// buffer sizes.
		Sound_Sample* sample = Sound_NewSample(rw, type == WAV ? "WAV" : "MP3", &wantedFormat,
#ifdef Q_WS_WIN
											   6291456
#else
											   131072
#endif
											   );
		if (sample == 0) {
			qWarning() << "ERROR:" << Sound_GetError();
			Sound_ClearError();
			return 0;
		}
		Uint32 bytes = Sound_DecodeAll(sample);
		if (sample->flags & SOUND_SAMPLEFLAG_ERROR) {
			// We don't abort since some of these errors can be non-fatal.
			// Unfortunately, there's no way to tell :-/
			qWarning() << "WARNING:" << Sound_GetError();
			Sound_ClearError();
		}
		Uint8* buf = static_cast<Uint8*>(malloc(sample->buffer_size));
		memcpy(buf, sample->buffer, sample->buffer_size);
		Sound_FreeSample(sample);
		chunk = Mix_QuickLoad_RAW(buf, sample->buffer_size);
		if (chunk == 0) {
			qWarning() << "ERROR:" << Mix_GetError();
			Mix_SetError("");
			free(buf);
			return 0;
		}
	} else {
		Q_ASSERT(type == OGG);
		chunk = Mix_LoadWAV_RW(rw, true);
		if (chunk == 0) {
			qWarning() << "ERROR:" << Mix_GetError();
			Mix_SetError("");
			return 0;
		}
	}

	// Alternative way of decoding MPEG, utilizing SMPEG directly.  It sucks,
	// since SMPEG, for the piece of crap it is, tends to play MP3s at double
	// speed (chipmunks ahoy...)
#if 0
	if (type == MPEG) {
		// The sound is an mp3.  We'll decode it into an SDL_Mixer chunk using
		// SMPEG.
		SMPEG* smpeg = SMPEG_new_data(data.data(), data.size(), 0, 0);

		// Set the format we want the decoded raw data to have.
		SDL_AudioSpec spec;
		SMPEG_wantedSpec(smpeg, &spec);
		spec.channels = 2;
		spec.format = MIX_DEFAULT_FORMAT;
		spec.freq = 44100;
		SMPEG_actualSpec(smpeg, &spec);

		// We decode the mpeg stream in steps of 8192kB each and increase the
		// size of the output buffer after each step.
		size_t bufSize = 8192;
		Uint8* buf = static_cast<Uint8*>(malloc(bufSize));
		// Needs to be set to digital silence because SMPEG is mixing source
		// and destination.
		memset(buf, 0, bufSize);
		// Prepare for decoding and decode the first bunch of data.
		SMPEG_play(smpeg);
		int bytesWritten = SMPEG_playAudio(smpeg, buf, 8192);
		// Decode the rest.  Increase buffer as needed.
		while (bytesWritten > 0 and bytesWritten <= 8192) {
			bufSize += 8192;
			buf = static_cast<Uint8*>(realloc(buf, bufSize));
			memset(buf + (bufSize - 8192), 0, 8192);
			bytesWritten = SMPEG_playAudio(smpeg, buf + (bufSize - 8192), 8192);
		}
		// Done with decoding.
		SMPEG_stop(smpeg);
		if (SMPEG_error(smpeg) != 0) {
			qWarning() << "ERROR: cannot decode sound data:" << SMPEG_error(smpeg);
			free(buf);
			SMPEG_delete(smpeg);
			return 0;
		}
		SMPEG_delete(smpeg);
		// Adjust final buffer size to fit the decoded data exactly.
		buf = static_cast<Uint8*>(realloc(buf, (bufSize - 8192) + bytesWritten));
		chunk = Mix_QuickLoad_RAW(buf, (bufSize - 8192) + bytesWritten);
	}
#endif

	// We have all the data we need; create the sound object.  It is
	// *important* not to pass the CHtmlSysWin object as the parent in the
	// constructor; doing so would result in Qt deleting the sound object when
	// the parent object gets destroyed.  Therefore, we simply pass 0 to make
	// the sound object parentless.
	CHtmlSysSound* sound;
	switch (type) {
	  case WAV:
		//qDebug() << "Sound type: WAV";
		sound = new CHtmlSysSoundWavQt(0, chunk, WAV);
		break;

	  case OGG:
		//qDebug() << "Sound type: OGG";
		sound = new CHtmlSysSoundOggQt(0, chunk, OGG);
		break;

	  case MPEG:
		//qDebug() << "Sound type: MPEG";
		sound = new CHtmlSysSoundMpegQt(0, chunk, MPEG);
		break;
	}
	return sound;
}
