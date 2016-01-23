/* Copyright (C) 2013 Nikos Chantziaras.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */
#ifndef QTADSSOUND_H
#define QTADSSOUND_H

#include <QObject>
#include <QTime>

#include "tadshtml.h"
#include "config.h"
#ifndef NO_AUDIO
#include "Aulib/AudioStream.h"
#endif


bool initSound();

void quitSound();


/* Provides the common code for all three types of digitized sound (WAV,
 * Ogg Vorbis and MP3).
 */
class QTadsSound: public QObject {
    Q_OBJECT

  public:
    enum SoundType { WAV, OGG, MPEG, MIDI };

#ifndef NO_AUDIO
  private:
    Aulib::AudioStream* fAudStream;
    SoundType fType;
    bool fPlaying;
    std::chrono::milliseconds fFadeOut{};
    bool fCrossFade;
    class QTimer* fFadeOutTimer;
    QTime fTimePos;

    // TADS callback to invoke on stop.
    void (*fDone_func)(void*, int repeat_count);

    // CTX to pass to the TADS callback.
    void* fDone_func_ctx;

    // How many times we repeated the sound.
    int fRepeats;

    // How many times should we repeat the sound.
    // 0 means repeat forever.
    int fRepeatsWanted;

    // Total length of the sound in milliseconds.
    std::chrono::milliseconds fLength{};

    // Aulib finish callback.
    void
    fFinishCallback( Aulib::Stream& strm );

    // Aulib loop callback.
    void
    fLoopCallback( Aulib::Stream& strm );

    void
    emitReadyToFadeOut()
    { emit readyToFadeOut(); }

  private slots:
    void
    fDoFadeOut();

    void
    fPrepareFadeOut();

    void
    fDeleteTimer();

  signals:
    void readyToFadeOut();

  public:
    QTadsSound( QObject* parent, Aulib::AudioStream* stream, SoundType type );
    ~QTadsSound() override;
#endif

  public:
    int
    startPlaying( void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat, int vol,
                  int fadeIn, int fadeOut, bool crossFade );

    void
    cancelPlaying( bool sync, int fadeOut, bool fadeOutInBg );

    void
    addCrossFade( int ms );

    static class CHtmlSysSound*
    createSound( const class CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                 unsigned long filesize, class CHtmlSysWin* win, SoundType type );
};


#endif
