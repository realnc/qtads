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
#ifndef SYSFONT_H
#define SYSFONT_H

#include <QFontMetrics>
#include <QFontInfo>
#include <QColor>

#include "htmlsys.h"


/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysFontQt: public QFont, public CHtmlSysFont {
  private:
    QColor fColor;
    QColor fBgColor;

  public:
    // When color() is a valid color (QColor::isValid()) it should be used as
    // the foreground color when drawing text in this font.
    const QColor&
    color() const
    { return this->fColor; }

    HTML_color_t
    htmlColor() const
    { return HTML_make_color(this->fColor.red(), this->fColor.green(), this->fColor.blue()); }

    void
    color( HTML_color_t color )
    { this->fColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)); }

    // When bgColor() is a valid color (QColor::isValid()) it should be used as
    // the background color when drawing text in this font.
    const QColor&
    bgColor() const
    { return this->fBgColor; }

    HTML_color_t
    htmlBgColor() const
    { return HTML_make_color(this->fBgColor.red(), this->fBgColor.green(), this->fBgColor.blue()); }

    void
    bgColor( HTML_color_t color )
    { this->fBgColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)); }

    bool
    operator ==( const CHtmlSysFontQt& f ) const
    { return QFont::operator ==(f) and this->fColor == f.fColor and this->fBgColor == f.fBgColor; }

    CHtmlSysFontQt&
    operator =( const QFont& f )
    {
        QFont::operator =(f);
        return *this;
    }

    // Set the font descriptor.
    void
    set_font_desc( const CHtmlFontDesc* src )
    {
        desc_.copy_from(src);
        // Clear the explicit-face-name flag, since this is important only
        // when looking up a font.
        desc_.face_set_explicitly = false;
    }

    //
    // CHtmlSysFont interface implementation.
    //
    virtual void
    get_font_metrics( CHtmlFontMetrics* m )
    {
        //qDebug() << Q_FUNC_INFO << "called";

        QFontMetrics tmp(*this);

        m->ascender_height = tmp.ascent();
        m->descender_height = tmp.descent();
        m->total_height = tmp.height();
    }

    virtual int
    is_fixed_pitch()
    { return QFontInfo(*this).fixedPitch(); }

    virtual int
    get_em_size()
    { return QFontInfo(*this).pixelSize(); }
};


#endif
