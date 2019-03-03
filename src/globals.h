// This is copyrighted software. More information is at the end of this file.
#pragma once

#ifdef __cplusplus
/* Works like qApp, but contains the global CHtmlSysFrameQt object instead.  If
 * this variable is 0, it means that no such object has been created yet.
 *
 * qApp and qFrame actually both point to the same object (the global
 * QApplication instance), but qFrame is provided simply to avoid casting the
 * global qApp object into a CHtmlSysFrameQt when we need to use it as such.
 */
extern class CHtmlSysFrameQt* qFrame;

/* The global CHtmlSysWinGroupQt object.  0 if none exists.  Like qApp/qFrame,
 * this is a singleton object and it's handy to have a global pointer to it.
 */
extern class CHtmlSysWinGroupQt* qWinGroup;
#endif

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
