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

#include "htmlqt.h"

#include "htmlurl.h"


CHtmlSysFrameQt* qFrame = 0;
CHtmlSysWinGroupQt* qWinGroup = 0;


/* --------------------------------------------------------------------
 * QTadsMediaObject
 */
QTadsMediaObject::QTadsMediaObject( QObject* parent, const QString& filename, SoundType type )
  : QObject(parent), fFileName(filename), fChunk(0), fMusic(0), fChannel(-1), fType(type),
	fDone_func(0), fDone_func_ctx(0), fRepeats(0), fRepeatsWanted(1)
{
	this->fFileName = filename;
	if (type == MPEG) {
		this->fMusic = Mix_LoadMUS(filename.toLocal8Bit().constData());
		if (this->fMusic == 0) {
			qWarning() << "Error: Unable to load sound from" << filename;
		}
	} else {
		this->fChunk = Mix_LoadWAV(filename.toLocal8Bit().constData());
		if (this->fChunk == 0) {
			qWarning() << "Error: Unable to load sound from" << filename;
		}
	}
	QFile::remove(this->fFileName);
}


QTadsMediaObject::~QTadsMediaObject()
{
	if (this->fType == MPEG) {
		Mix_FreeMusic(this->fMusic);
	} else {
		Mix_FreeChunk(this->fChunk);
	}
}


void
QTadsMediaObject::fFinish()
{
	// Invoke the callback, if there is one.
	if (this->fDone_func != 0) {
		qDebug() << "INVOKING CALLBACK, REPEAT:" << this->fRepeats;
		this->fDone_func(this->fDone_func_ctx, this->fRepeats);
		this->fDone_func = 0;
	}
}


void
QTadsMediaObject::startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat )
{
	this->fRepeatsWanted = repeat;
	this->fDone_func = done_func;
	this->fDone_func_ctx = done_func_ctx;
	++this->fRepeats;
	if (this->fType == MPEG) {
		if (Mix_PlayMusic(this->fMusic, repeat - 1) == -1) {
			qWarning() << "Error: Can't play sound:" << Mix_GetError();
		}
	} else {
		this->fChannel = Mix_PlayChannel(-1, this->fChunk, repeat - 1);
		if (this->fChannel == -1) {
			qWarning() << "Error: Can't play sound:" << Mix_GetError();
		}
	}
}


void
QTadsMediaObject::cancelPlaying( bool sync )
{
	if (this->fType == MPEG) {
		Mix_HaltMusic();
	} else {
		Mix_HaltChannel(this->fChannel);
	}
	this->fFinish();
}


// TODO: Check for I/O errors.
CHtmlSysSound*
QTadsMediaObject::createSound( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							   unsigned long filesize, CHtmlSysWin* win, SoundType type )
{
	qDebug() << Q_FUNC_INFO << "called";
	qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
			<< "url:" << url->get_url();

	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	file.seek(seekpos);
	QTemporaryFile* tmpFile = new QTemporaryFile(QDir::tempPath() + "/qtads_XXXXXX",
												 static_cast<CHtmlSysWinQt*>(win));
	tmpFile->setAutoRemove(false);
	tmpFile->open();
	tmpFile->write(file.read(filesize));
	file.close();
	QString fname = tmpFile->fileName();
	tmpFile->close();
	delete tmpFile;
	qDebug() << "source filename:" << fname;

	CHtmlSysSound* sound;
	QTadsMediaObject* cast;
	switch (type) {
	  case WAV:
		qDebug() << "Sound type: WAV";
		sound = new CHtmlSysSoundWavQt(static_cast<CHtmlSysWinQt*>(win), fname, WAV);
		cast = static_cast<CHtmlSysSoundWavQt*>(sound);
		break;

	  case OGG:
		qDebug() << "Sound type: OGG";
		sound = new CHtmlSysSoundOggQt(static_cast<CHtmlSysWinQt*>(win), fname, OGG);
		cast = static_cast<CHtmlSysSoundOggQt*>(sound);
		break;

	  case MPEG:
		qDebug() << "Sound type: MPEG";
		sound = new CHtmlSysSoundMpegQt(static_cast<CHtmlSysWinQt*>(win), fname, MPEG);
		cast = static_cast<CHtmlSysSoundMpegQt*>(sound);
		break;
	}
	cast->fFileName = fname;
	return sound;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundWavQt
 */
int
CHtmlSysSoundWavQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								int repeat, const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade )
{
	qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat);
	return 0;
}


void
CHtmlSysSoundWavQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysSoundWavQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundWavQt::resume()
{
	qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundOggQt
 */
int
CHtmlSysSoundOggQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								int repeat, const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade )
{
	qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat);
	return 0;
}


void
CHtmlSysSoundOggQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysSoundOggQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundOggQt::resume()
{
	qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundMpegQt
 */
int
CHtmlSysSoundMpegQt::play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx,
								 int repeat, const textchar_t* url, int vol, long fade_in, long fade_out,
								 int crossfade )
{
	qDebug() << "play_sound url:" << url << "repeat:" << repeat;
	this->startPlaying(done_func, done_func_ctx, repeat);
	return 0;
}


void
CHtmlSysSoundMpegQt::add_crossfade( CHtmlSysWin* win, long ms )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysSoundMpegQt::cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg )
{
	qDebug() << Q_FUNC_INFO;

	this->cancelPlaying(sync);
}


void
CHtmlSysSoundMpegQt::resume()
{
	qDebug() << Q_FUNC_INFO;
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
	qDebug() << "Loading" << imageType << "image from" << filename << "at offset" << seekpos
			<< "with size" << filesize << "url:" << url->get_url();

	CHtmlSysResource* image;

	// Open the file and seek to the specified position.
	QFile file(filename);
	// TODO: Check for errors.
	file.open(QIODevice::ReadOnly);
	file.seek(seekpos);

	// Better get an error at compile-time using static_cast rather than an
	// abort at runtime using dynamic_cast.
	QTadsPixmap* cast;

	// Create an object of the appropriate class for the specified image type.
	// Also cast the object to a QTadsPixmap so we can loadFromData() later on.
	if (imageType == "JPG" or imageType == "JPEG") {
		image = new CHtmlSysImageJpegQt;
		cast = static_cast<QTadsPixmap*>(static_cast<CHtmlSysImageJpegQt*>(image));
	} else if (imageType == "PNG") {
		image = new CHtmlSysImagePngQt;
		cast = static_cast<QTadsPixmap*>(static_cast<CHtmlSysImagePngQt*>(image));
	} else if (imageType == "MNG") {
		image = new CHtmlSysImageMngQt;
		cast = static_cast<QTadsPixmap*>(static_cast<CHtmlSysImageMngQt*>(image));
	} else {
		// TODO: Don't just abort but be graceful.
		qFatal(Q_FUNC_INFO, "unknown image type.");
	}

	// Load the image data.
	// TODO: Check for errors.
	cast->loadFromData(file.read(filesize), imageType.toAscii());
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
CHtmlSysSoundMidi::create_midi( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
								unsigned long filesize, CHtmlSysWin* win )
{
	qDebug() << Q_FUNC_INFO << "called";
	return 0;
}


CHtmlSysResource*
CHtmlSysSoundWav::create_wav( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							  unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsMediaObject::createSound(url, filename, seekpos, filesize, win, QTadsMediaObject::WAV);
}


CHtmlSysResource*
CHtmlSysSoundMpeg::create_mpeg( const CHtmlUrl* url, const textchar_t* filename,
								unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsMediaObject::createSound(url, filename, seekpos, filesize, win, QTadsMediaObject::MPEG);
}


CHtmlSysResource*
CHtmlSysSoundOgg::create_ogg( const CHtmlUrl* url, const textchar_t* filename,
							  unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
	return QTadsMediaObject::createSound(url, filename, seekpos, filesize, win, QTadsMediaObject::OGG);
}
