// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "syswin.h"

/* We need special handling for the "About this game" box.
 */
class CHtmlSysWinAboutBoxQt: public CHtmlSysWinQt
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent* e) override
    // It shouldn't be possible to do game input from the about box, so we
    // bypass the inherited input handling and revert to the default.
    {
        QScrollArea::keyPressEvent(e);
    }

    void resizeEvent(QResizeEvent* e) override;

    auto sizeHint() const -> QSize override;

public:
    CHtmlSysWinAboutBoxQt(class CHtmlFormatter* formatter, QWidget* parent);
    ~CHtmlSysWinAboutBoxQt() override;

    // We have scrollbars always disabled, so we can report our own
    // width/height rather than our viewport's.  We need to do that because
    // the formatter needs to know our size before we become visible, and our
    // viewport only reports a valid size after show() is called.
    auto get_disp_width() -> long override
    {
        return width();
    }

    auto get_disp_height() -> long override
    {
        return height();
    }

    void set_banner_size(
        long width, HTML_BannerWin_Units_t width_units, int use_width, long height,
        HTML_BannerWin_Units_t height_units, int use_height) override;
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
