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
#include <QFileInfo>
#ifndef Q_OS_ANDROID
#include <SDL_mixer.h>
#endif

#include "globals.h"
#include "sysframe.h"
#include "settings.h"
#include "syssoundmidi.h"
#include "syssoundwav.h"
#include "syssoundogg.h"
#include "syssoundmpeg.h"


#ifndef Q_OS_ANDROID
/* --------------------------------------------------------------------
 * CHtmlSysSoundMidiQt
 */
CHtmlSysSoundMidiQt* CHtmlSysSoundMidiQt::fActiveMidi = 0;

CHtmlSysSoundMidiQt::CHtmlSysSoundMidiQt( SDL_RWops* music )
    : fRWops(music),
      fPlaying(false),
      fDone_func(0),
      fDone_func_ctx(0),
      fRepeats(0),
      fRepeatsWanted(1)
{
    this->fMusic = Mix_LoadMUS_RW(this->fRWops);
}


CHtmlSysSoundMidiQt::~CHtmlSysSoundMidiQt()
{
    this->fRepeatsWanted = -1;
    if (CHtmlSysSoundMidiQt::fActiveMidi == this) {
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
    //qDebug() << Q_FUNC_INFO;

    if (CHtmlSysSoundMidiQt::fActiveMidi == 0) {
        // No music is playing.  Nothing to do.
        return;
    }

    CHtmlSysSoundMidiQt* curMidi = CHtmlSysSoundMidiQt::fActiveMidi;

    // If it's an infinite loop sound, or it has not reached the wanted repeat
    // count yet, play again.
    if ((curMidi->fRepeatsWanted == 0) or (curMidi->fRepeats < curMidi->fRepeatsWanted)) {
        Mix_PlayMusic(curMidi->fMusic, 0);
        ++curMidi->fRepeats;
        return;
    }

    CHtmlSysSoundMidiQt::fActiveMidi = 0;
    // Sound has repeated enough times, or it has been halted.  In either case,
    // we need to invoke the TADS callback, if there is one.
    if (curMidi->fDone_func) {
        //qDebug() << "Invoking callback - repeats:" << curMidi->fRepeats;
        curMidi->fDone_func(curMidi->fDone_func_ctx, curMidi->fRepeats);
    }
    curMidi->fPlaying = false;
}
#endif


int
CHtmlSysSoundMidiQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                 void* done_func_ctx, int repeat, const textchar_t* /*url*/, int vol,
                                 long /*fade_in*/, long /*fade_out*/, int /*crossfade*/ )
#ifndef Q_OS_ANDROID
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;

    // Check if the user disabled background music.
    if (not qFrame->settings()->enableMusic) {
        return 1;
    }

    if (CHtmlSysSoundMidiQt::fActiveMidi != 0) {
        // Only one MIDI sound can be active at a time.
        return 1;
    }

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
    if (Mix_PlayMusic(this->fMusic, 0) == -1) {
        qWarning() << "ERROR: Can't play MIDI:" << Mix_GetError();
        return 1;
    }
    this->fPlaying = true;
    this->fRepeats = 1;
    this->fDone_func = done_func;
    this->fDone_func_ctx = done_func_ctx;
    CHtmlSysSoundMidiQt::fActiveMidi = this;
    return 0;
}
#else
{
    return 1;
}
#endif


void
CHtmlSysSoundMidiQt::cancel_sound( CHtmlSysWin*, int, long, int )
#ifndef Q_OS_ANDROID
{
    //qDebug() << Q_FUNC_INFO;

    if (fActiveMidi == this) {
        this->fRepeatsWanted = -1;
        Mix_HaltMusic();
        CHtmlSysSoundMidiQt::callback();
        CHtmlSysSoundMidiQt::fActiveMidi = 0;
        this->fPlaying = false;
    }
}
#else
{
}
#endif


CHtmlSysResource*
CHtmlSysSoundMidi::create_midi( const CHtmlUrl* /*url*/, const textchar_t* filename, unsigned long seekpos,
                                unsigned long filesize, CHtmlSysWin* )
#ifndef Q_OS_ANDROID
{
    //qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
    //      << "url:" << url->get_url();

    // Check if the file exists and is readable.
    QFileInfo inf(fnameToQStr(filename));
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
    return new CHtmlSysSoundMidiQt(SDL_RWFromConstMem(data.constData(), data.size()));
}
#else
{
    return 0;
}
#endif


/* --------------------------------------------------------------------
 * CHtmlSysSoundWavQt
 */
int
CHtmlSysSoundWavQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                void* done_func_ctx, int repeat, const textchar_t* /*url*/, int vol,
                                long fade_in, long fade_out, int crossfade )
#ifndef Q_OS_ANDROID
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;
    return this->startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
}
#else
{
    return 1;
}
#endif


void
CHtmlSysSoundWavQt::add_crossfade( CHtmlSysWin*, long ms )
#ifndef Q_OS_ANDROID
{
    this->addCrossFade(ms);
}
#else
{
}
#endif


void
CHtmlSysSoundWavQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
#ifndef Q_OS_ANDROID
{
    //qDebug() << Q_FUNC_INFO;

    this->cancelPlaying(sync, fade_out_ms, fade_in_bg);
}
#else
{
}
#endif


void
CHtmlSysSoundWavQt::resume()
{
    //qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundOggQt
 */
int
CHtmlSysSoundOggQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                void* done_func_ctx, int repeat, const textchar_t*, int vol,
                                long fade_in, long fade_out, int crossfade )
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;
#ifndef Q_OS_ANDROID
    return this->startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
#else
    return 1;
#endif
}


void
CHtmlSysSoundOggQt::add_crossfade( CHtmlSysWin*, long ms )
{
#ifndef Q_OS_ANDROID
    this->addCrossFade(ms);
#endif
}


void
CHtmlSysSoundOggQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
{
    //qDebug() << Q_FUNC_INFO;

#ifndef Q_OS_ANDROID
    this->cancelPlaying(sync, fade_out_ms, fade_in_bg);
#endif
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
CHtmlSysSoundMpegQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                 void* done_func_ctx, int repeat, const textchar_t* /*url*/, int vol,
                                 long fade_in, long fade_out, int crossfade )
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;
#ifndef Q_OS_ANDROID
    return this->startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
#else
    return 1;
#endif
}


void
CHtmlSysSoundMpegQt::add_crossfade( CHtmlSysWin*, long ms )
{
#ifndef Q_OS_ANDROID
    this->addCrossFade(ms);
#endif
}


void
CHtmlSysSoundMpegQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
{
    //qDebug() << Q_FUNC_INFO;

#ifndef Q_OS_ANDROID
    this->cancelPlaying(sync, fade_out_ms, fade_in_bg);
#endif
}


void
CHtmlSysSoundMpegQt::resume()
{
    //qDebug() << Q_FUNC_INFO;
}


CHtmlSysResource*
CHtmlSysSoundWav::create_wav( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                              unsigned long filesize, CHtmlSysWin* win )
{
#ifndef Q_OS_ANDROID
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::WAV);
#else
    return 0;
#endif
}


CHtmlSysResource*
CHtmlSysSoundMpeg::create_mpeg( const CHtmlUrl* url, const textchar_t* filename,
                                unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
#ifndef Q_OS_ANDROID
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::MPEG);
#else
    return 0;
#endif
}


CHtmlSysResource*
CHtmlSysSoundOgg::create_ogg( const CHtmlUrl* url, const textchar_t* filename,
                              unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
#ifndef Q_OS_ANDROID
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::OGG);
#else
    return 0;
#endif
}
