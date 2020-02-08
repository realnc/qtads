// This is copyrighted software. More information is at the end of this file.
#include "htmlfmt.h"

#include "dispwidget.h"
#include "syswinaboutbox.h"

CHtmlSysWinAboutBoxQt::CHtmlSysWinAboutBoxQt(class CHtmlFormatter* formatter, QWidget* parent)
    : CHtmlSysWinQt(formatter, parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(200, 140);
}

// Without this, building with LTO enabled on mingw doesn't work. No idea why.
CHtmlSysWinAboutBoxQt::~CHtmlSysWinAboutBoxQt() = default;

void CHtmlSysWinAboutBoxQt::resizeEvent(QResizeEvent* e)
{
    formatter_->start_at_top(false);
    do_formatting(true, false, true);
    QScrollArea::resizeEvent(e);
}

QSize CHtmlSysWinAboutBoxQt::sizeHint() const
{
    // Ensure that we're always large enough to show the whole contents of the
    // "about" content.
    return dispWidget->size();
}

void CHtmlSysWinAboutBoxQt::set_banner_size(long width, HTML_BannerWin_Units_t, int, long height,
                                            HTML_BannerWin_Units_t height_units, int)
{
    bannerSize = height;
    bannerSizeUnits = height_units;
    dispWidget->resize(width + margins.left + margins.right, height + margins.top + margins.bottom);
    QRect rec(geometry());
    calcChildBannerSizes(rec);
    adjustSize();
}

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
