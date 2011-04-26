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
#ifndef Q_WS_ANDROID
    struct Mix_Chunk* fChunk;
    int fChannel;
    SoundType fType;
    bool fPlaying;
    int fFadeOut;
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
    unsigned fLength;

    // All QTadsMediaObjects that currently exist.  We need this in order to
    // implement the SDL_Mixer callback (which in turn needs to call the TADS
    // callback) that is invoked after a channel has stopped playing.  That
    // callback has to be a static member (C++ methods can't be C callbacks),
    // and since there's no 'this' pointer in static member functions, it needs
    // to invoke the TADS callback based on the channel number.
    static QList<QTadsSound*> fObjList;

    // We can't call SDL_mixer functions from inside an SDL_mixer callback, so
    // we use the following method: when the sound stops and the callback gets
    // called, we don't play it again (if it's looped) from inside the callback
    // but emit a signal which connects to a slot which plays the sound one
    // more time.
    void
    emitReadyToLoop()
    { emit readyToLoop(); }

    void
    emitReadyToFadeOut()
    { emit readyToFadeOut(); }

  private slots:
    void
    fDoFadeOut();

    void
    fDoLoop();

    void
    fPrepareFadeOut();

    void
    fDeleteTimer();

  signals:
    void readyToLoop();
    void readyToFadeOut();

  public:
    QTadsSound( QObject* parent, struct Mix_Chunk* chunk, SoundType type );

    virtual
    ~QTadsSound();
#endif

    // The SDL_Mixer callback for when a sound finished playing.
    static void callback( int channel );

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
