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
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef SYSIMAGEMNG_H
#define SYSIMAGEMNG_H

#include <QDebug>
#include <QMovie>

#include "qtadsimage.h"
#include "config.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysImageMngQt: public QMovie, public CHtmlSysImageMng {
    Q_OBJECT

  private:
    CHtmlSysImageDisplaySite* fDispSite;

  private slots:
    void
    updateDisplay( const QRect& rect )
    {
        if (this->fDispSite != 0) {
            this->fDispSite->dispsite_inval(rect.x(), rect.y(), rect.width(), rect.height());
        }
    }

  public:
    CHtmlSysImageMngQt()
    : fDispSite(0)
    { connect(this, SIGNAL(updated(QRect)), this, SLOT(updateDisplay(QRect))); }

    //
    // CHtmlSysImageMng interface implementation.
    //
    void
    set_display_site ( CHtmlSysImageDisplaySite* dispSite ) override
    { this->fDispSite = dispSite; }

    void
    cancel_playback() override
    { this->stop(); }

    void
    pause_playback() override
    { this->setPaused(true); }

    void
    resume_playback() override
    { this->setPaused(false); }

    void
    draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode ) override
    { QTadsImage(this->currentImage()).drawFromPaintEvent(win, pos, mode); }

    unsigned long
    get_width() const override
    { return this->frameRect().width(); }

    unsigned long
    get_height() const override
    { return this->frameRect().height(); }

    int
    map_palette( CHtmlSysWin*, int ) override
    { return false; }

    void
    notify_timer() override
    { qDebug() << Q_FUNC_INFO; }

    void
    notify_image_change( int, int, int, int ) override
    { qDebug() << Q_FUNC_INFO; }
};


#endif
