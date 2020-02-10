// This is copyrighted software. More information is at the end of this file.
#include "syssoundmidi.h"

auto CHtmlSysSoundMidiQt::play_sound(
    CHtmlSysWin* /*const win*/, void (*const done_func)(void*, int repeat_count),
    void* const done_func_ctx, const int repeat, const textchar_t* /*const url*/, const int vol,
    const long fade_in, const long fade_out, const int crossfade) -> int
#ifndef NO_AUDIO
{
    // qDebug() << "play_sound url:" << url << "repeat:" << repeat;
    return startPlaying(done_func, done_func_ctx, repeat, vol, fade_in, fade_out, crossfade);
}
#else
{
    return 1;
}
#endif

void CHtmlSysSoundMidiQt::add_crossfade(CHtmlSysWin* /*const win*/, const long ms)
#ifndef NO_AUDIO
{
    addCrossFade(ms);
}
#else
{ }
#endif

void CHtmlSysSoundMidiQt::cancel_sound(
    CHtmlSysWin* /*const win*/, const int sync, const long fade_out_ms, const int fade_in_bg)
#ifndef NO_AUDIO
{
    // qDebug() << Q_FUNC_INFO;

    cancelPlaying(sync, fade_out_ms, fade_in_bg);
}
#else
{ }
#endif

void CHtmlSysSoundMidiQt::resume()
{
    // qDebug() << Q_FUNC_INFO;
}

auto CHtmlSysSoundMidi::create_midi(
    const CHtmlUrl* const url, const textchar_t* const filename, const unsigned long seekpos,
    const unsigned long filesize, CHtmlSysWin* const win) -> CHtmlSysResource*
#ifndef Q_OS_ANDROID
{
    return QTadsSound::createSound(url, filename, seekpos, filesize, win, QTadsSound::MIDI);
}
#else
{
    return nullptr;
}
#endif

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
