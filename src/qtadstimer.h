// This is copyrighted software. More information is at the end of this file.
#ifndef QTADSTIMER_H
#define QTADSTIMER_H

#include <QTimer>

#include "htmlsys.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class QTadsTimer: public QTimer, public CHtmlSysTimer
{
    Q_OBJECT

public slots:
    // We connect the timeout() signal to this slot.
    void trigger()
    {
        // If we have a callback, call it.
        if (func_ != 0) {
            invoke_callback();
        }
    }

public:
    QTadsTimer(void (*func)(void*), void* ctx, QObject* parent = 0)
        : QTimer(parent)
        , CHtmlSysTimer(func, ctx)
    {
        connect(this, SIGNAL(timeout()), this, SLOT(trigger()));
    }

    // We bring this into public scope since we need to evaluate the callback
    // pointer in order to unregister the timer.
    using CHtmlSysTimer::func_;
};

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
