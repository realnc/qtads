// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "htmlsys.h"
#include "qtadssound.h"
#ifndef NO_AUDIO
#include "Aulib/Stream.h"
#endif

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysSoundWavQt: public QTadsSound, public CHtmlSysSoundWav
{
    Q_OBJECT

public:
#ifndef NO_AUDIO
    CHtmlSysSoundWavQt(QObject* parent, Aulib::Stream* stream, SoundType type)
        : QTadsSound(parent, stream, type)
    {}
#endif

    //
    // CHtmlSysSoundWav interface implementation.
    //
    auto play_sound(CHtmlSysWin* win, void (*done_func)(void*, int repeat_count),
                   void* done_func_ctx, int repeat, const textchar_t* url, int vol, long fade_in,
                   long fade_out, int crossfade) -> int override;

    void add_crossfade(CHtmlSysWin* win, long ms) override;

    void cancel_sound(CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg) override;

    auto maybe_suspend(CHtmlSysSound*) -> int override
    // We always return false since we have no limitation regarding the amount
    // of sounds we can play simultaneously.
    {
        return false;
    }

    void resume() override;
};

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
