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
#ifndef SYSSOUNDMIDI_H
#define SYSSOUNDMIDI_H

#include <QDebug>

#include "htmlsys.h"
#include "config.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysSoundMidiQt: public CHtmlSysSoundMidi {
#ifndef Q_OS_ANDROID
  private:
    struct SDL_RWops* fRWops;
    Mix_Music* fMusic;
    bool fPlaying;

    // TADS callback to invoke on stop.
    void (*fDone_func)(void*, int repeat_count);

    // CTX to pass to the TADS callback.
    void* fDone_func_ctx;

    // How many times we repeated the sound.
    int fRepeats;

    // How many times should we repeat the sound.
    // 0 means repeat forever.
    int fRepeatsWanted;

    // Currently playing MIDI object.  The callback needs this.
    static CHtmlSysSoundMidiQt* fActiveMidi;
#endif

  public:
#ifndef Q_OS_ANDROID
    CHtmlSysSoundMidiQt( struct SDL_RWops* music );
    ~CHtmlSysSoundMidiQt() override;

    // SDL_Mixer callback.
    static void callback();
#endif

    //
    // CHtmlSysSoundMidi interface implementation.
    //
    int
    play_sound( CHtmlSysWin* win, void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat,
                const textchar_t* url, int vol, long fade_in, long fade_out, int crossfade ) override;

    void
    add_crossfade( CHtmlSysWin* win, long ms ) override
    { qDebug() << Q_FUNC_INFO; }

    void
    cancel_sound( CHtmlSysWin* win, int sync, long fade_out_ms, int fade_in_bg ) override;

    int
    maybe_suspend( CHtmlSysSound* ) override
    // MIDI is exclusive - we can only play one MIDI sound at a time.  However,
    // the current HTML TADS model only allows MIDI to play in one layer (the
    // background layer) anyway, so we should never find ourselves wanting to
    // play a MIDI sound while another MIDI sound is already active.  So, we'll
    // just ignore this request entirely; the result will be that the new
    // foreground sound will be unable to play.
    //
    // If at some point in the future the HTML TADS model changes to allow
    // multiple layers of MIDI sounds, this part of this routine will have to
    // have a real implementation.
    { return false; }

    void
    resume() override
    // We never suspend MIDI, so there's nothing to do.  (See the notes in
    // maybe_suspend() - if the model changes to require that MIDI suspension
    // be implemented, we would have to provide a real implementation here.)
    { }
};


#endif
