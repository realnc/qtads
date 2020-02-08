// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QTimer>
#include <chrono>

/* QTimer with std::chrono support for building with Qt 5.7 and older.
 */
class QTimerChrono final: public QTimer
{
    Q_OBJECT

public:
    explicit QTimerChrono(QObject* parent = nullptr)
        : QTimer(parent)
    {}

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
    using QTimer::start;

    void start(std::chrono::milliseconds value)
    {
        start(int(value.count()));
    }
#endif
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
