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
#include <phonon/audiooutput.h>

#include "htmlqt.h"

#include "htmlurl.h"


CHtmlSysFrameQt* qFrame = 0;
CHtmlSysWinGroupQt* qWinGroup = 0;

/* --------------------------------------------------------------------
 * QTadsMediaObject
 */
QTadsMediaObject::QTadsMediaObject( QObject* parent, Phonon::MediaObject* mediaObject )
  : QObject(parent), fMediaObject(mediaObject), fDone_func(0), fDone_func_ctx(0), fRepeats(0), fRepeatsWanted(1)
{
	connect(this->fMediaObject, SIGNAL(aboutToFinish()), this, SLOT(fLoop()));
	connect(this->fMediaObject, SIGNAL(finished()), this, SLOT(fFinish()));
	connect(this->fMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
			this, SLOT(fErrorHandler(Phonon::State)));
}


QTadsMediaObject::~QTadsMediaObject()
{
	this->fMediaObject->deleteLater();
	//delete this->fFile;
}


void
QTadsMediaObject::fLoop()
{
	// Schedule the same source for another playback if we're looping or didn't
	// reach the wanted repeat count yet.
	if (this->fRepeatsWanted == 0 or this->fRepeats < this->fRepeatsWanted) {
		this->fMediaObject->enqueue(this->fMediaObject->currentSource());
		++this->fRepeats;
	} else {
		// The Xine Phonon backend will cut off the sound prematurelly if a
		// transitionTime of 0 (the default) is used and this is the last sound
		// in the queue.  We set it to 1ms to work around that bug.
		this->fMediaObject->setTransitionTime(1);
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
QTadsMediaObject::fErrorHandler( Phonon::State newState )
{
	if (newState == Phonon::ErrorState) {
		qWarning().nospace() << "Phonon error (type " << this->fMediaObject->errorType()
				<< "): " << this->fMediaObject->errorString();
	}
}


void
QTadsMediaObject::startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat )
{
	this->fRepeatsWanted = repeat;
	this->fDone_func = done_func;
	this->fDone_func_ctx = done_func_ctx;
	++this->fRepeats;
	this->fMediaObject->play();
}


void
QTadsMediaObject::cancelPlaying( bool sync )
{
	this->fMediaObject->clear();
	this->fFinish();
	//emit finished();
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


// TODO: Check for I/O errors.
CHtmlSysSound*
QTadsMediaObject::createSound( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
							   unsigned long filesize, CHtmlSysWin* win, SoundType type )
{
	qDebug() << Q_FUNC_INFO << "called";
	qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
			<< "url:" << url->get_url();

	QFile file(filename);
	QTemporaryFile* tmpFile = new QTemporaryFile(QDir::tempPath() + "/qtads_XXXXXX",
												 static_cast<CHtmlSysWinQt*>(win));
	tmpFile->setAutoRemove(false);
	file.open(QIODevice::ReadOnly);
	file.seek(seekpos);
	tmpFile->open();
	tmpFile->write(file.read(filesize));
	file.close();
	QString fname = tmpFile->fileName();
	tmpFile->close();
	delete tmpFile;
	Phonon::MediaObject* mObj = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource(fname));
	qDebug() << "source filename:" << mObj->currentSource().fileName();

	CHtmlSysSound* sound;
	QTadsMediaObject* cast;
	switch (type) {
	  case WAV:
		qDebug() << "Sound type: WAV";
		sound = new CHtmlSysSoundWavQt(static_cast<CHtmlSysWinQt*>(win), mObj);
		cast = static_cast<CHtmlSysSoundWavQt*>(sound);
		break;

	  case OGG:
		qDebug() << "Sound type: OGG";
		sound = new CHtmlSysSoundOggQt(static_cast<CHtmlSysWinQt*>(win), mObj);
		cast = static_cast<CHtmlSysSoundOggQt*>(sound);
		break;

	  default:
		// Assume MPEG.  Even if it's not MPEG, Phonon should be able to play
		// it since it autodetects the audio format anyway.
		qDebug() << "Sound type: MPEG";
		sound = new CHtmlSysSoundMpegQt(static_cast<CHtmlSysWinQt*>(win), mObj);
		cast = static_cast<CHtmlSysSoundMpegQt*>(sound);
		break;
	}

	/* Normally we would load the sound data into a buffer instead of creating
	 * files, but the Xine backend of Phonon is buggy as hell and this hangs
	 * the application.
	 */
	/*
	sound->fData = new QBuffer(sound);
	sound->fData->setData(file.read(filesize));
	qDebug() << "bytes read:" << sound->fData->data().length();
	Phonon::MediaSource source(sound->fData);
	qDebug() << "source type:" << source.type();
	sound->setCurrentSource(source);
	sound->fOutput = new Phonon::AudioOutput(Phonon::GameCategory, sound);
	Phonon::createPath(sound, sound->fOutput);
	*/

	return sound;
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
