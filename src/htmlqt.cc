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

#include <QDir>
#include <QBuffer>
#include <QPainter>
#include <smpeg.h>

#include "htmlqt.h"

#include "htmlurl.h"


CHtmlSysFrameQt* qFrame = 0;
CHtmlSysWinGroupQt* qWinGroup = 0;


/* --------------------------------------------------------------------
 * CHtmlSysSoundMidiQt
 */
CHtmlSysSoundMidiQt* CHtmlSysSoundMidiQt::fActiveMidi = 0;

CHtmlSysSoundMidiQt::CHtmlSysSoundMidiQt( SDL_RWops* music )
: fRWops(music), fPlaying(false), fDone_func(0), fDone_func_ctx(0), fRepeats(0), fRepeatsWanted(1)
{
	this->fMusic = Mix_LoadMUS_RW(this->fRWops);
}


CHtmlSysSoundMidiQt::~CHtmlSysSoundMidiQt()
{
	this->fRepeatsWanted = -1;
	if (this->fPlaying) {
		Mix_HaltMusic();
		CHtmlSysSoundMidiQt::callback();
		CHtmlSysSoundMidiQt::fActiveMidi = 0;
	}
	Mix_FreeMusic(this->fMusic);
	SDL_FreeRW(this->fRWops);
}


void
CHtmlSysSoundMidiQt::callback()
{
	Q_ASSERT(fActiveMidi != 0);

	// If it's an infinite loop sound, or it has not reached the wanted repeat
	// count yet, play again.
	if ((fActiveMidi->fRepeatsWanted == 0) or (fActiveMidi->fRepeats < fActiveMidi->fRepeatsWanted)) {
		Mix_PlayMusic(fActiveMidi->fMusic, 0);
		++fActiveMidi->fRepeats;
		return;
	}

	// Sound has repeated enough times, or it has been halted.  In either case,
	// we need to invoke the TADS callback, if there is one.
	fActiveMidi->fPlaying = false;
	if (fActiveMidi->fDone_func) {
		//qDebug() << "Invoking callback - repeats:" << fActiveMidi->fRepeats;
		fActiveMidi->fDone_func(fActiveMidi->fDone_func_ctx, fActiveMidi->fRepeats);
	}
	fActiveMidi->fPlaying = false;
	fActiveMidi = 0;
}


int
CHtmlSysSoundMidiQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count),
								 void* done_func_ctx, int repeat, const textchar_t* url, int vol,
								 long fade_in, long fade_out, int crossfade )
{
	//qDebug() << "play_sound url:" << url << "repeat:" << repeat;

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
	Mix_VolumeMusic(vol);

	this->fRepeatsWanted = repeat;
	if (CHtmlSysSoundMidiQt::fActiveMidi != 0 and CHtmlSysSoundMidiQt::fActiveMidi->fPlaying) {
		// Another MIDI sound is playing.  Halt it before we play a new one.
		Mix_HaltMusic();
		CHtmlSysSoundMidiQt::callback();
	}
	if (Mix_PlayMusic(this->fMusic, 0) == -1) {
		qWarning() << "ERROR: Can't play MIDI:" << Mix_GetError();
		return 1;
	}
	this->fPlaying = true;
	this->fRepeats = 1;
	CHtmlSysSoundMidiQt::fActiveMidi = this;
	this->fDone_func = done_func;
	this->fDone_func_ctx = done_func_ctx;
	return 0;
}


void
CHtmlSysSoundMidiQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	qDebug() << Q_FUNC_INFO;

	if (this->fPlaying) {
		this->fRepeatsWanted = -1;
		Mix_HaltMusic();
		CHtmlSysSoundMidiQt::callback();
		CHtmlSysSoundMidiQt::fActiveMidi = 0;
		this->fPlaying = false;
	}
}


CHtmlSysResource*
CHtmlSysSoundMidi::create_midi( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
								unsigned long filesize, CHtmlSysWin* win )
{
	//qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
	//		<< "url:" << url->get_url();

	// Check if the file exists and is readable.
	QFileInfo inf(filename);
	if (not inf.exists() or not inf.isReadable()) {
		qWarning() << "ERROR:" << filename << "doesn't exist or is unreadable";
		return 0;
	}

	// Open the file and seek to the specified position.
	QFile file(filename);
	if (not file.open(QIODevice::ReadOnly)) {
		qWarning() << "ERROR: Can't open file" << filename;
		return 0;
	}
	if (not file.seek(seekpos)) {
		qWarning() << "ERROR: Can't seek in file" << filename;
		file.close();
		return 0;
	}
	QByteArray data(file.read(filesize));
	file.close();
	if (data.isEmpty() or data.size() < filesize) {
		qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << filename;
		return 0;
	}
	return new CHtmlSysSoundMidiQt(SDL_RWFromConstMem(data.constData(), data.size()));
}


/* --------------------------------------------------------------------
 * QTadsSound
 */
QList<QTadsSound*> QTadsSound::fObjList;

QTadsSound::QTadsSound( QObject* parent, Mix_Chunk* chunk, SoundType type )
: QObject(parent), fChunk(chunk), fChannel(-1), fType(type), fPlaying(false), fDone_func(0),
  fDone_func_ctx(0), fRepeats(0), fRepeatsWanted(1)
{
	this->fLength = (this->fChunk->alen * 8) / (2 * 16 * 44.1);
	//qDebug() << "Sound length:" << this->fLength;
}


QTadsSound::~QTadsSound()
{
	//qDebug() << Q_FUNC_INFO;
	this->fRepeatsWanted = -1;
	if (this->fPlaying) {
		Mix_HaltChannel(this->fChannel);
	}
	Mix_FreeChunk(this->fChunk);
}


void
QTadsSound::callback( int channel )
{
	QTadsSound* mObj = 0;
	// Find the object that uses the specified channel.
	for (int i = 0; i < fObjList.size() and mObj == 0; ++i) {
		if (fObjList.at(i)->fChannel == channel) {
			mObj = fObjList.at(i);
		}
	}

	// If it's an infinite loop sound, or it has not reached the wanted repeat
	// count yet, play again on the same channel.
	if ((mObj->fRepeatsWanted == 0) or (mObj->fRepeats < mObj->fRepeatsWanted)) {
		Mix_PlayChannel(channel, mObj->fChunk, 0);
		++mObj->fRepeats;
		return;
	}

	// Sound has repeated enough times, or it has been halted.  In either case,
	// we need to invoke the TADS callback, if there is one.
	mObj->fPlaying = false;
	if (mObj->fDone_func) {
		//qDebug() << "Invoking callback - repeats:" << mObj->fRepeats;
		mObj->fDone_func(mObj->fDone_func_ctx, mObj->fRepeats);
	}
	fObjList.removeAll(mObj);
}


void
QTadsSound::effectCallback( int chan, void* stream, int len, void* udata )
{
}


void
QTadsSound::startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
								int vol )
{
	Q_ASSERT(not this->fPlaying);
	Q_ASSERT(not QTadsSound::fObjList.contains(this));

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
	this->fChannel = Mix_PlayChannel(-1, this->fChunk, 0);
	if (this->fChannel == -1) {
		qWarning() << "Error: Can't play sound:" << Mix_GetError();
	} else {
		this->fTime.start();
		//Mix_UnregisterAllEffects(this->fChannel);
		//Mix_RegisterEffect(this->fChannel, effectCallback, 0, this);
		this->fPlaying = true;
		this->fRepeats = 1;
		QTadsSound::fObjList.append(this);
		this->fDone_func = done_func;
		this->fDone_func_ctx = done_func_ctx;
	}
}


void
QTadsSound::cancelPlaying( bool sync )
{
	if (this->fPlaying) {
		this->fRepeatsWanted = -1;
		Mix_HaltChannel(this->fChannel);
	}
}


// TODO: Check for I/O errors.
CHtmlSysSound*
QTadsSound::createSound( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							   unsigned long filesize, CHtmlSysWin* win, SoundType type )
{
	//qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
	//		<< "url:" << url->get_url();

	// Check if the file exists and is readable.
	QFileInfo inf(filename);
	if (not inf.exists() or not inf.isReadable()) {
		qWarning() << "ERROR:" << filename << "doesn't exist or is unreadable";
		return 0;
	}

	// Open the file and seek to the specified position.
	QFile file(filename);
	if (not file.open(QIODevice::ReadOnly)) {
		qWarning() << "ERROR: Can't open file" << filename;
		return 0;
	}
	if (not file.seek(seekpos)) {
		qWarning() << "ERROR: Can't seek in file" << filename;
		file.close();
		return 0;
	}
	QByteArray data(file.read(filesize));
	file.close();
	if (data.isEmpty() or data.size() < filesize) {
		qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << filename;
		return 0;
	}

	Mix_Chunk* chunk;
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
	} else {
		Q_ASSERT(type == WAV or type == OGG);
		// The sound is either WAV or Ogg Vorbis data.  We'll just let
		// SDL_Mixer handle it.
		SDL_RWops* rw = SDL_RWFromConstMem(data.constData(), data.size());
		chunk = Mix_LoadWAV_RW(rw, 1);
	}

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
		sound = new CHtmlSysSoundWavQt(0, chunk, OGG);
		break;

	  case MPEG:
		//qDebug() << "Sound type: MPEG";
		sound = new CHtmlSysSoundWavQt(0, chunk, MPEG);
		break;
	}
	return sound;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundWavQt
 */
int
CHtmlSysSoundWavQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								int repeat, const textchar_t* url, int vol, long fade_in, long fade_out,
								int crossfade )
{
	//qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat, vol);
	return 0;
}


void
CHtmlSysSoundWavQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO << "\n Crossfades not implemented yet.";
}


void
CHtmlSysSoundWavQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	//qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundWavQt::resume()
{
	//qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundOggQt
 */
int
CHtmlSysSoundOggQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								int repeat, const textchar_t* url, int vol, long fade_in, long fade_out,
								int crossfade )
{
	//qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat, vol);
	return 0;
}


void
CHtmlSysSoundOggQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO << "\n Crossfades not implemented yet.";
}


void
CHtmlSysSoundOggQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	//qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundOggQt::resume()
{
	//qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundMpegQt
 */
int
CHtmlSysSoundMpegQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								 int repeat, const textchar_t* url, int vol, long fade_in, long fade_out,
								 int crossfade )
{
	//qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat, vol);
	return 0;
}


void
CHtmlSysSoundMpegQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO << "\n Crossfades not implemented yet.";
}


void
CHtmlSysSoundMpegQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	//qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundMpegQt::resume()
{
	//qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysImageMngQt
 */
void
CHtmlSysImageMngQt::draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
{
	QPainter painter(static_cast<CHtmlSysWinQt*>(win)->widget());
	const QImage& img = this->currentImage();
	const QRect& pixRect = this->frameRect();
	if (mode == HTMLIMG_DRAW_CLIP) {
		// Clip mode.  Only draw the part of the image that would fit.  If the
		// image is smaller than pos, adjust the drawing area to avoid scaling.
		int targetWidth;
		int targetHeight;
		if (pixRect.width() > pos->right - pos->left) {
			targetWidth = pos->right - pos->left;
		} else {
			targetWidth = pixRect.width();
		}
		if (pixRect.height() > pos->bottom - pos->top) {
			targetHeight = pos->bottom - pos->top;
		} else {
			targetHeight = pixRect.height();
		}
		painter.drawImage(pos->left, pos->top, img, 0, 0, targetWidth, targetHeight);
	} else {
		// QPainter will scale it by default.
		painter.drawImage(QRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top), img);
	}
}


/* --------------------------------------------------------------------
 * Non-portable static methods
 */

/* Helper routine.  Loads any type of image from the specified offset inside
 * the given file and returns it.  Has the same semantics as the various
 * CHtmlSysImage*::create_*() routines.  The image type is specified in
 * 'imageType'.  It has the same format as the list returned by
 * QImageReader::supportedImageFormats() (like "JPG", "PNG", etc.)
 */
static CHtmlSysResource*
createImageFromFile( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
					 unsigned long filesize, CHtmlSysWin* win, const QString& imageType )
{
	//qDebug() << "Loading" << imageType << "image from" << filename << "at offset" << seekpos
	//		<< "with size" << filesize << "url:" << url->get_url();

	// Check if the file exists and is readable.
	QFileInfo inf(filename);
	if (not inf.exists() or not inf.isReadable()) {
		qWarning() << "ERROR:" << filename << "doesn't exist or is unreadable";
		return 0;
	}

	// Open the file and seek to the specified position.
	QFile file(filename);
	if (not file.open(QIODevice::ReadOnly)) {
		qWarning() << "ERROR: Can't open file" << filename;
		return 0;
	}
	if (not file.seek(seekpos)) {
		qWarning() << "ERROR: Can't seek in file" << filename;
		file.close();
		return 0;
	}

	CHtmlSysResource* image;
	// Better get an error at compile-time using static_cast rather than an
	// abort at runtime using dynamic_cast.
	QTadsImage* cast;
	CHtmlSysImageMngQt* mngCast;

	// Create an object of the appropriate class for the specified image type.
	// Also cast the object to a QTadsImage so we can loadFromData() later on.
	if (imageType == "JPG" or imageType == "JPEG") {
		image = new CHtmlSysImageJpegQt;
		cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImageJpegQt*>(image));
	} else if (imageType == "PNG") {
		image = new CHtmlSysImagePngQt;
		cast = static_cast<QTadsImage*>(static_cast<CHtmlSysImagePngQt*>(image));
	} else if (imageType == "MNG") {
		image = new CHtmlSysImageMngQt;
		mngCast = static_cast<CHtmlSysImageMngQt*>(image);
	} else {
		// TODO: Don't just abort but be graceful.
		qFatal(Q_FUNC_INFO, "unknown image type.");
	}

	// Load the image data.
	const QByteArray& data(file.read(filesize));
	file.close();
	if (data.isEmpty() or data.size() < filesize) {
		qWarning() << "ERROR: Could not read" << filesize << "bytes from file" << filename;
		delete image;
		return 0;
	}

	if (imageType == "MNG") {
		QBuffer* buf = new QBuffer(mngCast);
		buf->setData(data);
		buf->open(QBuffer::ReadOnly);
		mngCast->setFormat("MNG");
		mngCast->setDevice(buf);
		mngCast->start();
	} else if (not cast->loadFromData(data, imageType.toAscii())) {
		qWarning() << "ERROR: Could not parse image data";
		delete image;
		return 0;
	}
	return image;
}


CHtmlSysResource*
CHtmlSysImageJpeg::create_jpeg( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
								unsigned long filesize, CHtmlSysWin* win )
{
	return ::createImageFromFile(url, filename, seekpos, filesize, win, "JPG");
}


CHtmlSysResource*
CHtmlSysImagePng::create_png( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							  unsigned long filesize, CHtmlSysWin* win )
{
	return ::createImageFromFile(url, filename, seekpos, filesize, win, "PNG");
}


CHtmlSysResource*
CHtmlSysImageMng::create_mng( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							  unsigned long filesize, CHtmlSysWin* win )
{
	return ::createImageFromFile(url, filename, seekpos, filesize, win, "MNG");
}


CHtmlSysResource*
CHtmlSysSoundWav::create_wav( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							  unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::WAV);
}


CHtmlSysResource*
CHtmlSysSoundMpeg::create_mpeg( const CHtmlUrl* url, const textchar_t* filename,
								unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::MPEG);
}


CHtmlSysResource*
CHtmlSysSoundOgg::create_ogg( const CHtmlUrl* url, const textchar_t* filename,
							  unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::OGG);
}
