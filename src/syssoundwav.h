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
#ifndef SYSSOUNDWAV_H
#define SYSSOUNDWAV_H

#include "qtadssound.h"
#include "htmlsys.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysSoundWavQt: public QTadsSound, public CHtmlSysSoundWav {
    Q_OBJECT

  public:
#ifndef NO_AUDIO
    CHtmlSysSoundWavQt( QObject* parent, Mix_Chunk* chunk, SoundType type )
    : QTadsSound(parent, chunk, type)
    { }
#endif

    //
    // CHtmlSysSoundWav interface implementation.
    //
    int
    play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
                const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade ) override;

    void
    add_crossfade( CHtmlSysWin* win, long ms ) override;

    void
    cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg ) override;

    int
    maybe_suspend( CHtmlSysSound* ) override
    // We always return false since we have no limitation regarding the amount
    // of sounds we can play simultaneously.
    { return false; }

    void
    resume() override;
};


#endif
