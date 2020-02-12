// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QColor>
#include <QFontInfo>
#include <QFontMetrics>

#include "config.h"
#include "htmlsys.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysFontQt: public QFont, public CHtmlSysFont
{
private:
    QColor fColor;
    QColor fBgColor;

public:
    using QFont::QFont;

    bool needs_fake_bold = false;

    // When color() is a valid color (QColor::isValid()) it should be used as
    // the foreground color when drawing text in this font.
    auto color() const -> const QColor&
    {
        return fColor;
    }

    auto htmlColor() const -> HTML_color_t
    {
        return HTML_make_color(fColor.red(), fColor.green(), fColor.blue());
    }

    void color(HTML_color_t color)
    {
        fColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color));
    }

    // When bgColor() is a valid color (QColor::isValid()) it should be used as
    // the background color when drawing text in this font.
    auto bgColor() const -> const QColor&
    {
        return fBgColor;
    }

    auto htmlBgColor() const -> HTML_color_t
    {
        return HTML_make_color(fBgColor.red(), fBgColor.green(), fBgColor.blue());
    }

    void bgColor(HTML_color_t color)
    {
        fBgColor = QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color));
    }

    auto operator==(const CHtmlSysFontQt& f) const -> bool
    {
        return QFont::operator==(f) and fColor == f.fColor and fBgColor == f.fBgColor
            and needs_fake_bold == f.needs_fake_bold;
    }

    auto operator=(const QFont& f) -> CHtmlSysFontQt&
    {
        QFont::operator=(f);
        return *this;
    }

    // Set the font descriptor.
    void set_font_desc(const CHtmlFontDesc* src)
    {
        desc_.copy_from(src);
        // Clear the explicit-face-name flag, since this is important only
        // when looking up a font.
        desc_.face_set_explicitly = false;
    }

    //
    // CHtmlSysFont interface implementation.
    //
    void get_font_metrics(CHtmlFontMetrics* m) override
    {
        // qDebug() << Q_FUNC_INFO << "called";

        QFontMetrics tmp(*this);

        m->ascender_height = tmp.ascent();
        m->descender_height = tmp.descent();
        m->total_height = tmp.height();
    }

    auto is_fixed_pitch() -> int override
    {
        return QFontInfo(*this).fixedPitch();
    }

    auto get_em_size() -> int override
    {
        return QFontInfo(*this).pixelSize();
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
