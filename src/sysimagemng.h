// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QDebug>
#include <QMovie>

#include "config.h"
#include "qtadsimage.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysImageMngQt: public QMovie, public CHtmlSysImageMng
{
    Q_OBJECT

private:
    CHtmlSysImageDisplaySite* fDispSite;

private slots:
    void updateDisplay(const QRect& rect)
    {
        if (fDispSite != nullptr) {
            fDispSite->dispsite_inval(rect.x(), rect.y(), rect.width(), rect.height());
        }
    }

public:
    CHtmlSysImageMngQt()
        : fDispSite(nullptr)
    {
        connect(this, &QMovie::updated, this, &CHtmlSysImageMngQt::updateDisplay);
    }

    //
    // CHtmlSysImageMng interface implementation.
    //
    void set_display_site(CHtmlSysImageDisplaySite* dispSite) override
    {
        fDispSite = dispSite;
    }

    void cancel_playback() override
    {
        stop();
    }

    void pause_playback() override
    {
        setPaused(true);
    }

    void resume_playback() override
    {
        setPaused(false);
    }

    void draw_image(CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode) override
    {
        QTadsImage(currentImage()).drawFromPaintEvent(win, pos, mode);
    }

    auto get_width() const -> unsigned long override
    {
        return frameRect().width();
    }

    auto get_height() const -> unsigned long override
    {
        return frameRect().height();
    }

    auto map_palette(CHtmlSysWin*, int) -> int override
    {
        return false;
    }

    void notify_timer() override
    {
        qDebug() << Q_FUNC_INFO;
    }

    void notify_image_change(int, int, int, int) override
    {
        qDebug() << Q_FUNC_INFO;
    }
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
