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
#include <QBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>

#include "dispwidget.h"
#include "qtadstimer.h"
#include "settings.h"
#include "syswininput.h"
#include "sysimagejpeg.h"
#include "sysimagepng.h"

#include "htmlfmt.h"
#include "htmlrc.h"


CHtmlSysWinQt::CHtmlSysWinQt( CHtmlFormatter* formatter, DisplayWidget* dispWidget, QWidget* parent )
: QScrollArea(parent), CHtmlSysWin(formatter), fBannerStyleModeMode(true), fBannerStyleAutoVScroll(true),
  fBannerStyleGrid(false), fBannerStyleBorder(0), fDontReformat(0), fInPagePauseMode(false),
  fParentBanner(0), fBgImage(0), fBorderLine(qWinGroup->centralWidget()), lastInputHeight(0),
  margins(8, 2, 8, 2), bannerSize(0), bannerSizeUnits(HTML_BANNERWIN_UNITS_PIX)
{
    if (dispWidget == 0) {
        this->dispWidget = new DisplayWidget(this, formatter);
        this->setWidget(this->dispWidget);
    }
    this->formatter_->set_win(this, &margins);
    this->viewport()->setForegroundRole(QPalette::Text);
    this->viewport()->setBackgroundRole(QPalette::Base);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    this->setLineWidth(0);
    this->setContentsMargins(0, 0, 0, 0);
    this->fBorderLine.setFrameStyle(QFrame::Box | QFrame::Plain);
    QPalette p(this->fBorderLine.palette());
    p.setColor(QPalette::WindowText, Qt::darkGray);
    this->fBorderLine.setPalette(p);

    p = this->palette();
    p.setColor(QPalette::Base, qFrame->settings()->bannerBgColor);
    p.setColor(QPalette::Text, qFrame->settings()->bannerTextColor);
    this->setPalette(p);

    this->setAttribute(Qt::WA_InputMethodEnabled);

    // TEMP: Just to make the area of this widget visibly more apparent.
    //this->viewport()->setBackgroundRole(QPalette::ToolTipBase);
}


CHtmlSysWinQt::~CHtmlSysWinQt()
{
    //qDebug() << Q_FUNC_INFO;

    // Remove my reference on the background image if I have one.
    if (this->fBgImage != 0) {
        this->fBgImage->remove_ref();
    }

    // Don't allow the formatter to reference us anymore, since
    // we're about to be deleted.
    this->formatter_->unset_win();

    // Remove ourselves from our parent banner's child list.
    if (this->fParentBanner != 0) {
        this->fParentBanner->fChildBanners.removeAll(this);
    }

    // Remove all references to us from our child banners.
    for (int i = 0; i < this->fChildBanners.size(); ++i) {
        Q_ASSERT(this->fChildBanners.at(i)->fParentBanner == this);
        this->fChildBanners.at(i)->fParentBanner = 0;
    }
}


void
CHtmlSysWinQt::fSetupPainterForFont( QPainter& painter, bool hilite, CHtmlSysFont* font )
{
    const CHtmlSysFontQt& fontCast = *static_cast<CHtmlSysFontQt*>(font);
    painter.setFont(fontCast);

    if (fontCast.use_font_color()) {
        // The font has its own color; use it.
        HTML_color_t color = fontCast.get_font_color();
        painter.setPen(QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
    } else if (font->get_font_color() == static_cast<HTML_color_t>(HTML_COLOR_INPUT)) {
        painter.setPen(qFrame->inputColor());
    }

    if (hilite) {
        painter.setBackgroundMode(Qt::OpaqueMode);
        painter.setBackground(QApplication::palette().highlight());
        painter.setPen(QApplication::palette().color(QPalette::HighlightedText));
    }

    if (fontCast.use_font_bgcolor()) {
        painter.setBackgroundMode(Qt::OpaqueMode);
        HTML_color_t color = fontCast.get_font_bgcolor();
        painter.setBackground(QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
    }
}


void
CHtmlSysWinQt::keyPressEvent( QKeyEvent* event )
{
    qFrame->gameWindow()->keyPressEvent(event);
}


void
CHtmlSysWinQt::inputMethodEvent( QInputMethodEvent* e )
{
    qFrame->gameWindow()->inputMethodEvent(e);
}


void
CHtmlSysWinQt::scrollContentsBy(int dx, int dy)
{
    QScrollArea::scrollContentsBy(dx, dy);

    this->dispWidget->updateLinkTracking(QPoint());

    // If we reached the bottom, clear page-pause mode.
    if (this->fInPagePauseMode and this->verticalScrollBar()->value() == this->verticalScrollBar()->maximum()) {
        this->fInPagePauseMode = false;
        qFrame->gameWindow()->removeFromPagePauseQueue(this);
    }
}


void
CHtmlSysWinQt::wheelEvent( QWheelEvent* e )
{
    // Only allow the event if the banner has a vertical scrollbar.  Banners
    // without one are not supposed to be scrollable.
    if (this == qFrame->gameWindow() or this->fBannerStyleVScroll) {
        QScrollArea::wheelEvent(e);
    } else {
        e->ignore();
    }
}


void
CHtmlSysWinQt::resizeEvent( QResizeEvent* e )
{
    QScrollArea::resizeEvent(e);

    // If we reached the bottom, clear page-pause mode.
    if (this->fInPagePauseMode and this->verticalScrollBar()->value() == this->verticalScrollBar()->maximum()) {
        this->fInPagePauseMode = false;
        qFrame->gameWindow()->removeFromPagePauseQueue(this);
    }
}


void
CHtmlSysWinQt::mousePressEvent( QMouseEvent* e )
{
    // This is a work-around for KDE's Oxygen style window grabbing. Oxygen
    // allows window grabbing by clicking on any "empty" area of an
    // application. An area is considered empty, if mouse press events reach
    // QMainWindow. We don't want our game window to have draggable areas
    // just because we don't handle mouse press events. So we accept the event
    // here so that KDE knows not to allow window dragging.
    e->accept();
}


void
CHtmlSysWinQt::calcChildBannerSizes( QRect& parentSize )
{
    //qDebug() << Q_FUNC_INFO;
    ++this->fDontReformat;

    QRect newSize;
    QRect oldSize;

    // Get our current size.
    oldSize = this->geometry();

    // Start off assuming we'll take the entire parent area.  If we're a
    // top-level window, we'll leave it exactly like that.  If we're a banner,
    // we'll be aligned on three sides with the parent area, since we always
    // carve out a chunk by splitting the parent area; we'll adjust the fourth
    // side according to our banner size and alignment specifications.
    newSize = parentSize;

    // If we're a banner window, carve out our own area from the parent window;
    // otherwise, we must be a top-level window, so take the entire available
    // space for ourselves.
    if (this != qFrame->gameWindow()) {
        QFrame& borderLine = this->fBorderLine;
        CHtmlRect margins = this->formatter_->get_phys_margins();

        // Should be draw a border?
        bool drawBorder = this->fBannerStyleBorder;

        if (this->bannerSize == 0) {
            // This is a zero-size banner.  Don't use a border or margins.
            // A zero-size banner really should be a zero-size banner.
            drawBorder = false;
            margins.set(0, 0, 0, 0);
        }

        // Calculate our current on-screen size.
        int wid = oldSize.width();
        int ht = oldSize.height();

        // Convert the width units to pixels.
        switch (this->bannerSizeUnits) {
          case HTML_BANNERWIN_UNITS_PIX:
            // Pixels - use the stored width directly.
            wid = this->bannerSize;
            ht = this->bannerSize;
            // Add space for the border if needed.
            if (drawBorder) {
                ++wid;
                ++ht;
            }
            break;

          case HTML_BANNERWIN_UNITS_CHARS:
            // Character cells - calculate the size in terms of the width of a
            // "0" character in the window's default font.
            wid = this->bannerSize * measure_text(get_default_font(), "0", 1, 0).x;
            ht = this->bannerSize * measure_text(get_default_font(), "0", 1, 0).y;
            // Add space for the border if needed.
            if (drawBorder) {
                ++wid;
                ++ht;
            }
            // Add the margins.
            wid += margins.left + margins.right;
            ht += margins.top + margins.bottom;
            break;

          case HTML_BANNERWIN_UNITS_PCT:
            // Percentage - calculate the width as a percentage of the parent
            // size.
            wid = (this->bannerSize * parentSize.width()) / 100;
            ht = (this->bannerSize * parentSize.height()) / 100;
            break;
        }

        // Make sure that the banner doesn't exceed the available area.
        if (wid > parentSize.width()) {
            wid = parentSize.width();
        }
        if (ht > parentSize.height()) {
            ht = parentSize.height();
        }

        // Position the banner according to our alignment type.
        switch(this->fBannerPos) {
          case HTML_BANNERWIN_POS_TOP:
            if (drawBorder) {
                // Adjust for the border by taking the space for it out of
                // the banner.
                newSize.setBottom(newSize.top() + ht - 2);

                // Position the border window at the bottom of our area.
                borderLine.setGeometry(newSize.left(), newSize.top() + ht - 1, newSize.left() + newSize.width(), 1);
            } else {
                // Align the banner at the top of the window.
                newSize.setBottom(newSize.top() + ht - 1);
            }

            // Take the space out of the top of the parent window.
            parentSize.setTop(parentSize.top() + ht);
            break;

          case HTML_BANNERWIN_POS_BOTTOM:
            if (drawBorder) {
                // Adjust for the border by taking the space for it out of
                // the banner.
                newSize.setTop(newSize.top() + newSize.height() - ht + 2);

                // Position the border window at the top of our area.
                borderLine.setGeometry(newSize.left(), newSize.top() - 1, newSize.left() + newSize.width(), 1);
            } else {
                // Align the banner at the bottom of the window.
                newSize.setTop(newSize.top() + newSize.height() - ht + 1);
            }

            // Take the space out of the bottom of the parent area.
            parentSize.setBottom((parentSize.top() + parentSize.height()) - ht);
            break;

          case HTML_BANNERWIN_POS_LEFT:
            if (drawBorder) {
                // Adjust for the border by taking the space for it out of
                // the banner.
                newSize.setRight(newSize.left() + wid - 2);

                // Position the border window at the right of our area.
                borderLine.setGeometry(newSize.left() + width(), newSize.top(), 1, newSize.height());
            } else {
                // Align the banner at the left of the window.
                newSize.setRight(newSize.left() + wid - 1);
            }

            // Take the space from the left of the parent window.
            parentSize.setLeft(parentSize.left() + wid);
            break;

          case HTML_BANNERWIN_POS_RIGHT:
            if (drawBorder) {
                // Adjust for the border by taking the space for it out of
                // the banner.
                newSize.setLeft(newSize.left() + newSize.width() - wid + 2);

                // Position the border window at the left of our area.
                borderLine.setGeometry(newSize.left() - 1, newSize.top(), 1, newSize.height());
            } else {
                // Align the banner at the right of the window.
                newSize.setLeft(newSize.left() + newSize.width() - wid + 1);
            }

            // Take the space from the right of the parent window.
            parentSize.setRight(parentSize.left() + parentSize.width() - wid);
            break;
        }

        if (drawBorder) {
            borderLine.show();
        } else {
            borderLine.hide();
        }
    }

    // Now that we know our own full area, lay out our banner children.  This
    // will update newSize to reflect our actual size after deducting space for
    // our children.
    for (int i = 0; i < this->fChildBanners.size(); ++i) {
        this->fChildBanners.at(i)->calcChildBannerSizes(newSize);
    }

    // Set our new window position if it differs from the old one.
    if (newSize != oldSize) {
        this->setGeometry(newSize);

        // Resizing the display widget by the height we gained or lost during
        // the resize avoids cutting-off the bottom of the output.  I've no
        // idea why; the problem probably lies elsewhere, but for now, this
        // seems to fix it.
        this->dispWidget->resize(this->dispWidget->width(),
                                 this->dispWidget->height() + (newSize.height() - oldSize.height()));

        // Since we changed size, we will need a reformat.
        //if (this != qFrame->gameWindow())
        //qDebug() << newSize.width();
        //if (newSize.width() != oldSize.width())
        qFrame->scheduleReformat();
    }

    //qFrame->gameWindow()->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    --this->fDontReformat;
}


void
CHtmlSysWinQt::doReformat( int showStatus, int freezeDisplay, int resetSounds )
{
    // Forget any tracking links.
    this->dispWidget->notifyClearContents();

    // Restart the formatter.
    this->formatter_->start_at_top(resetSounds);

    // Format the window contents.
    this->do_formatting(showStatus, not freezeDisplay, freezeDisplay);

    // Reset last seen position.  Substract the top margin when doing this,
    // since for scrolling purposes we need to take the whole area into
    // account, not only the position where actual content starts.
    this->lastInputHeight = this->formatter_->get_max_y_pos() - this->formatter_->get_phys_margins().top;
}


void
CHtmlSysWinQt::addBanner( CHtmlSysWinQt* banner, HTML_BannerWin_Type_t type, int where,
                          CHtmlSysWinQt* other, HTML_BannerWin_Pos_t pos, unsigned long style )
{
    Q_ASSERT(banner->fParentBanner == 0);

    // Enable/disable the scrollbars, according to what was requested.
    if (style & OS_BANNER_STYLE_VSCROLL) {
        banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        banner->fBannerStyleVScroll = true;
    } else {
        banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        banner->fBannerStyleVScroll = false;
    }
    if (style & OS_BANNER_STYLE_HSCROLL) {
        banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        banner->fBannerStyleHScroll = true;
    } else {
        banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        banner->fBannerStyleHScroll = true;
    }

    banner->fBannerStyleGrid = type & HTML_BANNERWIN_TEXTGRID;
    banner->fParentBanner = this;
    banner->fBannerPos = pos;
    banner->fBannerWhere = where;
    banner->fBannerStyleModeMode = style & OS_BANNER_STYLE_MOREMODE;
    banner->fBannerStyleAutoVScroll = style & OS_BANNER_STYLE_AUTO_VSCROLL;
    banner->fBannerStyleBorder = style & OS_BANNER_STYLE_BORDER;

    banner->setGeometry(this->geometry());
    // Qt will not update the viewport() geometry unless the widget is shown
    // first.
    banner->show();
    banner->hide();

    // Add the banner to our child list.
    Q_ASSERT(not this->fChildBanners.contains(banner));
    switch (where) {
      case OS_BANNER_FIRST:
        this->fChildBanners.prepend(banner);
        break;
      case OS_BANNER_LAST:
        this->fChildBanners.append(banner);
        break;
      case OS_BANNER_BEFORE:
        Q_ASSERT(this->fChildBanners.contains(other));
        this->fChildBanners.insert(this->fChildBanners.indexOf(other), banner);
        break;
      default:
        Q_ASSERT(where == OS_BANNER_AFTER);
        Q_ASSERT(this->fChildBanners.contains(other));
        this->fChildBanners.insert(this->fChildBanners.indexOf(other) + 1, banner);
    }
}

extern void
os_sleep_ms( long ms );

void
CHtmlSysWinQt::scrollDown( bool force, bool justOneLine )
{
    QScrollBar* bar = this->verticalScrollBar();

    // If we're very small, or shouldn't scroll, or there's nothing to scroll,
    // ignore the call.
    if (this->width() < 10 or this->height() < 10 or not this->fBannerStyleAutoVScroll
        or bar->maximum() == bar->minimum())
    {
        return;
    }

    // Simply scroll directly to the bottom if more-mode is disabled for this
    // banner or the game is in non-stop mode.
    if (not this->fBannerStyleModeMode or qFrame->nonStopMode()) {
        if (qFrame->settings()->softScrolling) {
            while (bar->value() < bar->maximum()) {
                bar->setValue(bar->value() + bar->singleStep());
                os_sleep_ms(5);
            }
        } else {
            bar->triggerAction(QAbstractSlider::SliderToMaximum);
        }
        return;
    }

    if (this->fInPagePauseMode and not force) {
        return;
    }

    if (this->fInPagePauseMode) {
        if (justOneLine) {
            bar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        } else if (qFrame->settings()->softScrolling) {
            int target = bar->value() + bar->pageStep() - bar->singleStep() * 2;
            while (bar->value() < target and bar->value() < bar->maximum()) {
                bar->setValue(bar->value() + bar->singleStep());
                os_sleep_ms(5);
            }
        } else {
            bar->setValue(bar->value() + bar->pageStep() - bar->singleStep() * 2);
        }
    } else if (qFrame->settings()->softScrolling) {
        while (bar->value() < this->lastInputHeight and bar->value() < bar->maximum()) {
            bar->setValue(bar->value() + bar->singleStep());
            os_sleep_ms(5);
        }
    } else {
        bar->setValue(this->lastInputHeight);
    }

    // If we didn't arrive at the bottom, enable page-pause.
    if (bar->value() < bar->maximum()) {
        qFrame->gameWindow()->addToPagePauseQueue(this);
        this->fInPagePauseMode = true;
        // Note our new position.  Make sure that next time, we scroll down by
        // *almost* a page; we leave two lines of context.
        this->lastInputHeight += bar->pageStep() - bar->singleStep() * 2;
        return;
    }
    this->fInPagePauseMode = false;
    return;
}


void
CHtmlSysWinQt::notify_clear_contents()
{
    //qDebug() << Q_FUNC_INFO;

    // Tell our display widget about it.
    this->dispWidget->notifyClearContents();

    // Reset our last-seen position (for page-pause handling) back to the top
    // and clear page-pause mode.
    this->lastInputHeight = 0;
    this->fInPagePauseMode = false;
    qFrame->gameWindow()->removeFromPagePauseQueue(this);
}


int
CHtmlSysWinQt::close_window( int force )
{
    qDebug() << Q_FUNC_INFO;

    return force ? true : false;
}


CHtmlPoint
CHtmlSysWinQt::measure_text( CHtmlSysFont* font, const textchar_t* str, size_t len, int* ascent )
{
    //qDebug() << Q_FUNC_INFO;

    const QFontMetrics& tmpMetr = QFontMetrics(*static_cast<CHtmlSysFontQt*>(font));
    if (ascent != 0) {
        *ascent = tmpMetr.ascent();
    }
    // We don't return the actual width of the text, but the distance to where
    // subsequent text should be drawn.  This is really what our caller needs
    // to know, otherwise letters will start jumping left and right when
    // selecting text or moving the text cursor.
    return CHtmlPoint(tmpMetr.width(QString::fromUtf8(str, len)), tmpMetr.height());
}


size_t
CHtmlSysWinQt::get_max_chars_in_width( CHtmlSysFont* font, const textchar_t* str, size_t len, long wid )
{
    // We do a binary search on the results of measure_text() until we find an
    // amount of characters that fit.
    int first = 1;
    int last = len;
    long mid;
    int skippedBytes;
    while (first <= last) {
        skippedBytes = 1;
        // New mid point.
        mid = (first + last) / 2;
        // We might have landed inside a UTF-8 continuation byte. In that case,
        // adjust mid until we have a complete UTF character.
        while ((str[mid-1] & 0xC0) == 0x80 and mid < len) {
            ++mid;
            ++skippedBytes;
        }
        long bestFit = this->measure_text(font, str, mid, 0).x;
        if (bestFit < wid) {
            first = mid + skippedBytes;
        } else if (bestFit > wid) {
            last = mid - skippedBytes;
        } else {
            // Exact match.
            return mid;
        }
    }
    // We didn't find an exact match, which means one less than the result we
    // got will fit.
    return mid - 1;
}


void
CHtmlSysWinQt::draw_text( int hilite, long x, long y, CHtmlSysFont* font, const textchar_t* str, size_t len )
{
    if (len == 0) {
        // Huh?
        return;
    }

    QPainter painter(this->dispWidget);
    this->fSetupPainterForFont(painter, hilite, font);
    painter.drawText(x, y + painter.fontMetrics().ascent(), QString::fromUtf8(str, len));
}


void
CHtmlSysWinQt::draw_text_space( int hilite, long x, long y, CHtmlSysFont* font, long wid )
{
    QPainter painter(this->dispWidget);
    this->fSetupPainterForFont(painter, hilite, font);

    // Construct a string of spaces that's at least 'width' pixels wide.
    QString str(QChar::fromAscii(' '));
    const QFontMetrics& metr = painter.fontMetrics();
    while (metr.width(str) < wid) {
        str.append(QChar::fromAscii(' '));
    }

    painter.drawText(x, y, wid, metr.height(), Qt::AlignLeft, str);
}


void
CHtmlSysWinQt::draw_bullet( int hilite, long x, long y, CHtmlSysFont* font, HTML_SysWin_Bullet_t style )
{
    int unicode;
    switch (style) {
      case HTML_SYSWIN_BULLET_SQUARE:
        unicode = 0x25AA;
        break;
      case HTML_SYSWIN_BULLET_CIRCLE:
        unicode = 0x25CB;
        break;
      case HTML_SYSWIN_BULLET_DISC:
        unicode = 0x2022;
        break;
      default:
        Q_ASSERT(style == HTML_SYSWIN_BULLET_PLAIN);
        // Nothing to draw.
        return;
    }
    const QByteArray& utf8 = QString(QChar(unicode)).toUtf8();
    this->draw_text(hilite, x, y, font, utf8.constData(), utf8.size());
}


void
CHtmlSysWinQt::draw_hrule( const CHtmlRect* pos, int shade )
{
    //qDebug() << Q_FUNC_INFO;
    Q_ASSERT(pos != 0);

    QPainter painter(this->dispWidget);
    if (shade) {
        if (pos->bottom - pos->top > 2) {
            qDrawShadePanel(&painter, pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
                            this->palette(), true, 1, 0);
        } else {
            qDrawShadeLine(&painter, pos->left, pos->top, pos->right, pos->top, this->palette(), true, 1, 0);
        }
    } else {
        painter.fillRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
                         QApplication::palette().color(QPalette::Dark));
    }
}


void
CHtmlSysWinQt::draw_table_border( const CHtmlRect* pos, int width, int cell )
{
    //qDebug() << Q_FUNC_INFO;

    QPainter pnt(this->dispWidget);
    QPalette pal;
    // Use Midlight for Light to closer match Windows HTML TADS appearance.
    pal.setColor(QPalette::Light, QApplication::palette().color(QPalette::Midlight));
    qDrawShadePanel(&pnt, pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top, pal, cell, width);
}


void
CHtmlSysWinQt::draw_table_bkg( const CHtmlRect* pos, HTML_color_t bgcolor )
{
    //qDebug() << Q_FUNC_INFO;

    QPainter painter(this->dispWidget);
    int red = HTML_color_red(bgcolor);
    int green = HTML_color_green(bgcolor);
    int blue = HTML_color_blue(bgcolor);
    painter.fillRect(pos->left, pos->top, pos->right - pos->left, pos->bottom - pos->top,
                     QColor(red, green, blue));
}


void
CHtmlSysWinQt::draw_dbgsrc_icon( const CHtmlRect* pos, unsigned int stat )
{
    qDebug() << Q_FUNC_INFO;
}


int
CHtmlSysWinQt::do_formatting( int /*show_status*/, int update_win, int freeze_display )
{
    //qDebug() << Q_FUNC_INFO;

    if (this->fDontReformat != 0) {
        qWarning() << "Reentrancy detected in" << Q_FUNC_INFO << "\nThis should not happen.";
        return false;
    }

    // If desired, freeze display updating while we're working.
    if (freeze_display) {
        this->formatter_->freeze_display(true);
    }

    // Redraw the window before we start, so that something happens immediately
    // and the window doesn't look frozen.
    if (update_win) {
        qFrame->advanceEventLoop();
    }

    // Get the window area in document coordinates, so we'll know when we've
    // formatted past the bottom of the current display area.  Note that we'll
    // ignore the presence or absence of a horizontal scrollbar, since it could
    // come or go while we're formatting; by assuming that it won't be there,
    // we'll be maximally conservative about redrawing the whole area.
    unsigned long winBottom = qMax(static_cast<unsigned long>(this->height()), this->formatter_->get_max_y_pos());

    // We don't have enough formatting done yet to draw the window.
    bool drawn = false;

    // Reformat everything, drawing as soon as possible, if desired.
    /*
    if (update_win) {
        while (formatter_->more_to_do() and not drawn) {
            // Format another line.
            this->formatter_->do_formatting();

            // If we have enough content to do so, redraw the window; we're not
            // really done with the formatting yet, but at least we'll update
            // the window as soon as we can, so the user isn't staring at a
            // blank window longer than necessary.
            if (this->formatter_->get_max_y_pos() >= winBottom) {
                // If the formatter's updating is frozen, invalidate the window;
                // the formatter won't have been invalidating it as it goes.
                if (freeze_display) {
                    this->viewport()->update();
                }

                // Let the event loop redraw the window.
                qFrame->advanceEventLoop();

                // We've now drawn it.
                drawn = true;
            }
        }
    }
    */

    // After drawing, keep going until we've formatted everything.
    while (this->formatter_->more_to_do()) {
        this->formatter_->do_formatting();
    }

    // Unfreeze the display if we froze it.
    if (freeze_display) {
        this->formatter_->freeze_display(false);
    }

    if (this->fBannerStyleGrid) {
        this->dispWidget->resize(this->viewport()->size());
    } else {
        long newWidth;
        if (this->formatter_->get_outer_max_line_width() > this->viewport()->width()) {
            newWidth = this->formatter_->get_outer_max_line_width();
        } else {
            newWidth = this->viewport()->width();
        }
        this->dispWidget->resize(newWidth, this->formatter_->get_max_y_pos());
    }

    // If we didn't do any drawing, and we updated the window coming in,
    // invalidate the window - the initial redraw will have validated
    // everything, but we've just made it all invalid again and didn't get
    // around to drawing it.  Do the same thing if we froze the display and we
    // didn't do any updating.
    if ((update_win and not drawn) or freeze_display) {
        this->dispWidget->update();
    }

    // Make sure we don't lose any link we were previously tracking.
    this->dispWidget->updateLinkTracking(QPoint());

    // Return an indication of whether we did any updating.
    return drawn;
}


CHtmlSysFont*
CHtmlSysWinQt::get_default_font()
{
    //qDebug() << Q_FUNC_INFO;

    CHtmlFontDesc desc;
    qFrame->settings()->mainFont.get_font_desc(&desc);
    desc.htmlsize = 3;
    return get_font(&desc);
}


void
CHtmlSysWinQt::register_timer_func( void (*timer_func)(void*), void* func_ctx )
{
    QTadsTimer* timer = new QTadsTimer(timer_func, func_ctx, this);
    timer->setSingleShot(false);
    timer->set_repeating(true);
    timer->set_active(true);
    timer->start(1000);
    this->fTimerList.append(timer);
}


void
CHtmlSysWinQt::unregister_timer_func( void (*timer_func)( void * ), void* func_ctx )
{
    for (int i = 0; i < this->fTimerList.size(); ++i) {
        if (this->fTimerList.at(i)->func_ == timer_func) {
            delete this->fTimerList.takeAt(i);
            return;
        }
    }
}


CHtmlSysTimer*
CHtmlSysWinQt::create_timer( void (*timer_func)( void* ), void *func_ctx )
{
    qDebug() << Q_FUNC_INFO;

    return new QTadsTimer(timer_func, func_ctx, this);
}


void
CHtmlSysWinQt::set_timer( CHtmlSysTimer* timer, long interval_ms, int repeat )
{
    qDebug() << Q_FUNC_INFO;

    static_cast<QTadsTimer*>(timer)->setSingleShot(not repeat);
    static_cast<QTadsTimer*>(timer)->set_repeating(repeat);
    static_cast<QTadsTimer*>(timer)->start(interval_ms);
}


void
CHtmlSysWinQt::cancel_timer( CHtmlSysTimer* timer )
{
    qDebug() << Q_FUNC_INFO;

    static_cast<QTadsTimer*>(timer)->stop();
}


void
CHtmlSysWinQt::delete_timer( CHtmlSysTimer* timer )
{
    qDebug() << Q_FUNC_INFO;

    delete timer;
}


void
CHtmlSysWinQt::fmt_adjust_hscroll()
{
    //qDebug() << Q_FUNC_INFO;

    if (this->fBannerStyleGrid) {
        this->dispWidget->resize(this->viewport()->width(), this->dispWidget->height());
        return;
    }

    if (this->formatter_->get_outer_max_line_width() != this->dispWidget->width()) {
        long newWidth;
        if (this->formatter_->get_outer_max_line_width() > this->viewport()->width()) {
            newWidth = this->formatter_->get_outer_max_line_width();
        } else {
            newWidth = this->viewport()->width();
        }
        this->dispWidget->resize(newWidth, this->dispWidget->height());
    }
}


void
CHtmlSysWinQt::fmt_adjust_vscroll()
{
    //qDebug() << Q_FUNC_INFO;

    // Get the target height.
    int targetHt = static_cast<int>(this->formatter_->get_max_y_pos());

    if (targetHt == this->dispWidget->height()) {
        // No change in height, so nothing to do.
        return;
    }

    if (this->fBannerStyleGrid) {
        // Text grid banners never scroll in any direction, so they only need
        // to be the same size as the viewport.
        this->dispWidget->resize(this->dispWidget->width(), this->viewport()->height());
    } else {
        this->dispWidget->resize(this->dispWidget->width(), targetHt);
        this->scrollDown(false, false);
    }
}


void
CHtmlSysWinQt::inval_doc_coords( const CHtmlRect* area )
{
    //qDebug() << Q_FUNC_INFO;

    //qDebug() << "Invalidating area" << area->left << area->top << area->right << area->bottom;

    long width = area->right == HTMLSYSWIN_MAX_RIGHT ?
                 this->dispWidget->width() - area->left
                 : area->right - area->left;
    long height = area->bottom == HTMLSYSWIN_MAX_BOTTOM ?
                  this->dispWidget->height() - area->top
                  : area->bottom - area->top;

    this->dispWidget->update(QRect(area->left, area->top, width, height));
}


void
CHtmlSysWinQt::advise_clearing_disp_list()
{
    //qDebug() << Q_FUNC_INFO;
    this->dispWidget->notifyClearContents();
}


void
CHtmlSysWinQt::scroll_to_doc_coords( const CHtmlRect* pos )
{
    qDebug() << Q_FUNC_INFO;

    //qFrame->advanceEventLoop();
}


void
CHtmlSysWinQt::get_scroll_doc_coords( CHtmlRect* pos )
{
    qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::set_window_title( const textchar_t* title )
{
    //qDebug() << Q_FUNC_INFO;

    qWinGroup->setWindowTitle(QString::fromUtf8(title));
}


void
CHtmlSysWinQt::set_html_bg_color( HTML_color_t color, int use_default )
{
    //qDebug() << Q_FUNC_INFO;

    // If we have an active background image, ignore the call.
    if (this->fBgImage != 0) {
        return;
    }

    if (use_default) {
        QPalette p(this->palette());
        if (this->fBannerStyleGrid) {
            p.setColor(QPalette::Base, Qt::black);
        } else {
            p.setColor(QPalette::Base, qFrame->settings()->mainBgColor);
        }
        this->setPalette(p);
        return;
    }

    color = this->map_system_color(color);
    QPalette p(this->palette());
    p.setColor(QPalette::Base, QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
    this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_text_color( HTML_color_t color, int use_default )
{
    //qDebug() << Q_FUNC_INFO;

    if (use_default) {
        QPalette p(this->palette());
        if (this->fBannerStyleGrid) {
            p.setColor(QPalette::Text, Qt::lightGray);
        } else {
            p.setColor(QPalette::Text, qFrame->settings()->mainTextColor);
        }
        this->setPalette(p);
        return;
    }

    color = this->map_system_color(color);
    QPalette p(this->palette());
    p.setColor(QPalette::Text, QColor(HTML_color_red(color), HTML_color_green(color), HTML_color_blue(color)));
    this->setPalette(p);
}


void
CHtmlSysWinQt::set_html_link_colors( HTML_color_t link_color, int link_use_default,
                     HTML_color_t vlink_color, int vlink_use_default,
                     HTML_color_t alink_color, int alink_use_default,
                     HTML_color_t hlink_color, int hlink_use_default )
{
    //qDebug() << Q_FUNC_INFO;
}


HTML_color_t
CHtmlSysWinQt::map_system_color( HTML_color_t color )
{
    //qDebug() << Q_FUNC_INFO;

    // If it's just an RGB value, return it unchanged.
    if (not html_is_color_special(color)) {
        return color;
    }

    QColor col;

    switch (color) {
      case HTML_COLOR_STATUSBG:
        col = qFrame->settings()->bannerBgColor;
        break;

      case HTML_COLOR_STATUSTEXT:
        col = qFrame->settings()->bannerTextColor;
        break;

      case HTML_COLOR_LINK:
        col = qFrame->settings()->unvisitedLinkColor;
        break;

      case HTML_COLOR_ALINK:
        col = qFrame->settings()->clickedLinkColor;
        break;

      case HTML_COLOR_TEXT:
        col = qFrame->settings()->mainTextColor;
        break;

      case HTML_COLOR_BGCOLOR:
        col = qFrame->settings()->mainBgColor;
        break;

      case HTML_COLOR_INPUT:
        col = qFrame->settings()->inputColor;
        break;

      case HTML_COLOR_HLINK:
        col = qFrame->settings()->hoveringLinkColor;
        break;

      default:
        // Return black for everything else.
        return 0;
    }

    return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_link_color() const
{
    //qDebug() << Q_FUNC_INFO;

    const QColor& col = qFrame->settings()->unvisitedLinkColor;
    return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_alink_color() const
{
    //qDebug() << Q_FUNC_INFO;

    const QColor& col = qFrame->settings()->clickedLinkColor;
    return HTML_make_color(col.red(), col.green(), col.blue());
}


HTML_color_t
CHtmlSysWinQt::get_html_vlink_color() const
{
    //qDebug() << Q_FUNC_INFO;

    // Visited links aren't really implemented by the HTML base code.
    return this->get_html_link_color();
}


HTML_color_t
CHtmlSysWinQt::get_html_hlink_color() const
{
    //qDebug() << Q_FUNC_INFO;

    const QColor& col = qFrame->settings()->hoveringLinkColor;
    return HTML_make_color(col.red(), col.green(), col.blue());
}


int
CHtmlSysWinQt::get_html_link_underline() const
{
    //qDebug() << Q_FUNC_INFO;

    return qFrame->settings()->underlineLinks;
}


int
CHtmlSysWinQt::get_html_show_links() const
{
    //qDebug() << Q_FUNC_INFO;

    return qFrame->settings()->enableLinks;
}


int
CHtmlSysWinQt::get_html_show_graphics() const
{
    //qDebug() << Q_FUNC_INFO;

    return qFrame->settings()->enableGraphics;
}


void
CHtmlSysWinQt::set_html_bg_image( CHtmlResCacheObject* image )
{
    //qDebug() << Q_FUNC_INFO;

    if (image == 0 or image->get_image() == 0) {
        // No image specified; forget the current image if we have one and
        // restore the default background color.
        if (this->fBgImage != 0) {
            this->fBgImage->remove_ref();
            this->fBgImage = 0;
        }
        this->set_html_bg_color(0, true);
        return;
    }

    QTadsImage* castImg;
    switch (image->get_image()->get_res_type()) {
      case HTML_res_type_JPEG:
        castImg = static_cast<CHtmlSysImageJpegQt*>(image->get_image());
        break;
      case HTML_res_type_PNG:
        castImg = static_cast<CHtmlSysImagePngQt*>(image->get_image());
        break;
      case HTML_res_type_MNG:
        // FIXME: Handle MNG backgrounds.
        //castImg = static_cast<CHtmlSysImageMngQt*>(image->get_image());
        return;
      default:
        qWarning() << Q_FUNC_INFO << "Unknown resource type";
        castImg = reinterpret_cast<QTadsImage*>(image->get_image());
    }

    QPalette p(this->palette());
    p.setBrush(QPalette::Base, *castImg);
    this->setPalette(p);
    image->add_ref();
    this->fBgImage = image;
}


void
CHtmlSysWinQt::inval_html_bg_image( unsigned int x, unsigned int y, unsigned int wid, unsigned int ht )
{
    //qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysWinQt::set_banner_size( long width, HTML_BannerWin_Units_t width_units, int use_width,
                                long height, HTML_BannerWin_Units_t height_units, int use_height )
{
    //qDebug() << Q_FUNC_INFO;

    if (this == qFrame->gameWindow()) {
        // We're the main game window.  Ignore the call.
        return;
    }

    if (this->fBannerPos == HTML_BANNERWIN_POS_TOP or this->fBannerPos == HTML_BANNERWIN_POS_BOTTOM) {
        if (not use_height) {
            return;
        }
        this->bannerSize = height;
        this->bannerSizeUnits = height_units;
    } else {
        Q_ASSERT(this->fBannerPos == HTML_BANNERWIN_POS_LEFT or this->fBannerPos == HTML_BANNERWIN_POS_RIGHT);
        if (not use_width) {
            return;
        }
        this->bannerSize = width;
        this->bannerSizeUnits = width_units;
    }
    qFrame->adjustBannerSizes();
    if (not this->isVisible()) {
        this->show();
    }
    return;
}


void
CHtmlSysWinQt::set_banner_info( HTML_BannerWin_Pos_t pos, unsigned long style )
{
    //qDebug() << Q_FUNC_INFO;

    this->fBannerStyleModeMode = style & OS_BANNER_STYLE_MOREMODE;
    this->fBannerStyleAutoVScroll = (style & OS_BANNER_STYLE_AUTO_VSCROLL) or this->fBannerStyleModeMode;
    this->fBannerStyleBorder = style & OS_BANNER_STYLE_BORDER;
    this->fBannerPos = pos;

    // FIXME: ignore the scrollbar changes (for now, anyway)
}


void
CHtmlSysWinQt::get_banner_info( HTML_BannerWin_Pos_t* pos, unsigned long* style )
{
    //qDebug() << Q_FUNC_INFO;
    Q_ASSERT(pos != 0);
    Q_ASSERT(style != 0);

    *pos = this->fBannerPos;

    // Set the style flags.
    *style = 0;
    if (this->fBannerStyleModeMode) {
        *style |= OS_BANNER_STYLE_MOREMODE;
    }
    if (this->fBannerStyleAutoVScroll or this->fBannerStyleModeMode) {
        *style |= OS_BANNER_STYLE_AUTO_VSCROLL;
    }
    if (this->fBannerStyleBorder) {
        *style |= OS_BANNER_STYLE_BORDER;
    }

    // We provide full HTML interpretation, so we support <TAB>.
    *style |= OS_BANNER_STYLE_TAB_ALIGN;
}
