// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "htmlsys.h"
#include "qtadssound.h"
#ifndef Q_OS_ANDROID
#include "Aulib/AudioStream.h"
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
    CHtmlSysSoundWavQt(QObject* parent, Aulib::AudioStream* stream, SoundType type)
        : QTadsSound(parent, stream, type)
    {}
#endif

    //
    // CHtmlSysSoundWav interface implementation.
    //
    int play_sound(CHtmlSysWin* win, void (*done_func)(void*, int repeat_count),
                   void* done_func_ctx, int repeat, const textchar_t* url, int vol, long fade_in,
                   long fade_out, int crossfade) override;

    void add_crossfade(CHtmlSysWin* win, long ms) override;

    void cancel_sound(CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg) override;

    int maybe_suspend(CHtmlSysSound*) override
    // We always return false since we have no limitation regarding the amount
    // of sounds we can play simultaneously.
    {
        return false;
    }

    void resume() override;
};

/*
    Copyright 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2018, 2019 Nikos
    Chantziaras.

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
