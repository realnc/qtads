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

#include "globals.h"
#include "sysframe.h"
#include "settings.h"
#include "syssoundmidi.h"
#include "syssoundwav.h"
#include "syssoundogg.h"
#include "syssoundmpeg.h"


/* --------------------------------------------------------------------
 * CHtmlSysSoundMidiQt
 */
int
CHtmlSysSoundMidiQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                 void* done_func_ctx, int repeat, const textchar_t* /*url*/, int vol,
                                 long fade_in, long fade_out, int crossfade )
#ifndef NO_AUDIO
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;
    return startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
}
#else
{
    return 1;
}
#endif


void
CHtmlSysSoundMidiQt::add_crossfade( CHtmlSysWin*, long ms )
#ifndef NO_AUDIO
{
    addCrossFade(ms);
}
#else
{ }
#endif


void
CHtmlSysSoundMidiQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
#ifndef NO_AUDIO
{
    //qDebug() << Q_FUNC_INFO;

    cancelPlaying(sync, fade_out_ms, fade_in_bg);
}
#else
{ }
#endif


void
CHtmlSysSoundMidiQt::resume()
{
    //qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * CHtmlSysSoundWavQt
 */
int
CHtmlSysSoundWavQt::play_sound( CHtmlSysWin*, void (*done_func)(void*, int repeat_count),
                                void* done_func_ctx, int repeat, const textchar_t* /*url*/, int vol,
                                long fade_in, long fade_out, int crossfade )
#ifndef NO_AUDIO
{
    //qDebug() << "play_sound url:" << url << "repeat:" << repeat;
    return startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
}
#else
{
    return 1;
}
#endif


void
CHtmlSysSoundWavQt::add_crossfade( CHtmlSysWin*, long ms )
#ifndef NO_AUDIO
{
    addCrossFade(ms);
}
#else
{
}
#endif


void
CHtmlSysSoundWavQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
#ifndef NO_AUDIO
{
    //qDebug() << Q_FUNC_INFO;

    cancelPlaying(sync, fade_out_ms, fade_in_bg);
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
#ifndef NO_AUDIO
    return startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
#else
    return 1;
#endif
}


void
CHtmlSysSoundOggQt::add_crossfade( CHtmlSysWin*, long ms )
{
#ifndef NO_AUDIO
    addCrossFade(ms);
#endif
}


void
CHtmlSysSoundOggQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
{
    //qDebug() << Q_FUNC_INFO;

#ifndef NO_AUDIO
    cancelPlaying(sync, fade_out_ms, fade_in_bg);
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
#ifndef NO_AUDIO
    return startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
#else
    return 1;
#endif
}


void
CHtmlSysSoundMpegQt::add_crossfade( CHtmlSysWin*, long ms )
{
#ifndef NO_AUDIO
    addCrossFade(ms);
#endif
}


void
CHtmlSysSoundMpegQt::cancel_sound( CHtmlSysWin*, int sync, long fade_out_ms, int fade_in_bg )
{
    //qDebug() << Q_FUNC_INFO;

#ifndef NO_AUDIO
    cancelPlaying(sync, fade_out_ms, fade_in_bg);
#endif
}


void
CHtmlSysSoundMpegQt::resume()
{
    //qDebug() << Q_FUNC_INFO;
}


/* --------------------------------------------------------------------
 * Base code static function implementations.
 */
CHtmlSysResource*
CHtmlSysSoundMidi::create_midi( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                                unsigned long filesize, CHtmlSysWin* win )
#ifndef Q_OS_ANDROID
{
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::MIDI);
}
#else
{
    return 0;
}
#endif


CHtmlSysResource*
CHtmlSysSoundWav::create_wav( const CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
                              unsigned long filesize, CHtmlSysWin* win )
{
#ifndef NO_AUDIO
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::WAV);
#else
    return 0;
#endif
}


CHtmlSysResource*
CHtmlSysSoundMpeg::create_mpeg( const CHtmlUrl* url, const textchar_t* filename,
                                unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
#ifndef NO_AUDIO
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::MPEG);
#else
    return 0;
#endif
}


CHtmlSysResource*
CHtmlSysSoundOgg::create_ogg( const CHtmlUrl* url, const textchar_t* filename,
                              unsigned long seekpos, unsigned long filesize, CHtmlSysWin* win )
{
#ifndef NO_AUDIO
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::OGG);
#else
    return 0;
#endif
}
