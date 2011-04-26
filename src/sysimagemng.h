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
#ifndef SYSIMAGEMNG_H
#define SYSIMAGEMNG_H

#include <QDebug>
#include <QMovie>

#include "qtadsimage.h"


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
    virtual void
    set_display_site ( CHtmlSysImageDisplaySite* dispSite )
    { this->fDispSite = dispSite; }

    virtual void
    cancel_playback()
    { this->stop(); }

    virtual void
    pause_playback()
    { this->setPaused(true); }

    virtual void
    resume_playback()
    { this->setPaused(false); }

    virtual void
    draw_image( CHtmlSysWin* win, CHtmlRect* pos, htmlimg_draw_mode_t mode )
    { QTadsImage(this->currentImage()).drawFromPaintEvent(win, pos, mode); }

    virtual unsigned long
    get_width() const
    { return this->frameRect().width(); }

    virtual unsigned long
    get_height() const
    { return this->frameRect().height(); }

    virtual int
    map_palette( CHtmlSysWin* win, int foreground )
    { return false; }

    virtual void
    notify_timer()
    { qDebug() << Q_FUNC_INFO; }

    virtual void
    notify_image_change( int x, int y, int wid, int ht )
    { qDebug() << Q_FUNC_INFO; }
};


#endif
