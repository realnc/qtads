// This is copyrighted software. More information is at the end of this file.
#ifndef SYSWIN_H
#define SYSWIN_H

#include <QApplication>
#include <QDesktopWidget>
#include <QScrollArea>

#include "config.h"
#include "globals.h"
#include "sysfont.h"
#include "sysframe.h"
#include "syswingroup.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class CHtmlSysWinQt: public QScrollArea, public CHtmlSysWin
{
    Q_OBJECT

private:
    // If we're a banner, what is our HTML_BannerWin_Pos_t, placement and
    // style flags.
    //
    // Note: To determine whether we're a banner, we don't use a flag but
    // simply check whether "this == qFrame->gameWindow()"; if we're the game
    // window, we can't be a banner.
    HTML_BannerWin_Pos_t fBannerPos;
    int fBannerWhere;
    bool fBannerStyleModeMode;
    bool fBannerStyleVScroll;
    bool fBannerStyleAutoVScroll;
    bool fBannerStyleHScroll;
    bool fBannerStyleGrid;
    bool fBannerStyleBorder;

    // Our border, if we have one.
    QFrame fBorderLine;

    // Guard against re-entrancy for do_formatting().
    int fDontReformat;

    // Are we currently in page-pause mode?
    bool fInPagePauseMode;

    // Our parent banner, if any.  This is 0 if we don't have one.
    CHtmlSysWinQt* fParentBanner;

    // Our child banners, if any.
    QList<CHtmlSysWinQt*> fChildBanners;

    // List of timers with registered callbacks.
    QList<class QTadsTimer*> fTimerList;

    // Our banner background image.
    class CHtmlResCacheObject* fBgImage;

    // Our link colors.
    HTML_color_t fLinkColor = 0;
    HTML_color_t fALinkColor = 0;
    HTML_color_t fHLinkColor = 0;

    void fSetupPainterForFont(QPainter& painter, bool hilite, CHtmlSysFont* font);

protected:
    // The content height at the time of the last user input.  When the
    // formatter is producing a long run of output, we pause between screens to
    // make sure the user has had a chance to see all of the text before we
    // scroll it away.  This member records the position of the last input, so
    // we can be sure not to add more than another screenfull without getting
    // more input.
    int lastInputHeight;

    // Margins.
    CHtmlRect margins;

    // Banner size information, if we are a banner.  This is either our width
    // or height, according to our alignment type (vertical or horizontal).
    long bannerSize;
    HTML_BannerWin_Units_t bannerSizeUnits;

    // Our display widget.
    class DisplayWidget* dispWidget;

    void keyPressEvent(QKeyEvent* event) override;

    void inputMethodEvent(QInputMethodEvent* e) override;

    void scrollContentsBy(int dx, int dy) override;

    void wheelEvent(QWheelEvent* e) override;

    void resizeEvent(QResizeEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;

public:
    CHtmlSysWinQt(class CHtmlFormatter* formatter, QWidget* parent);
    ~CHtmlSysWinQt() override;

    // Returns our display widget.
    class DisplayWidget* displayWidget() const
    {
        return dispWidget;
    }

    // Calculate and adjust the sizes of our child banners.  On entry,
    // 'parentSize' contains the size of the full parent window area; on
    // return, it is updated to indicate the final size of the parent banner's
    // area after deducting the space carved out for children.
    void calcChildBannerSizes(QRect& parentSize);

    // Do a complete reformat.
    void doReformat(int showStatus, int freezeDisplay, int resetSounds);

    void addBanner(CHtmlSysWinQt* banner, HTML_BannerWin_Type_t type, int where,
                   CHtmlSysWinQt* other, HTML_BannerWin_Pos_t pos, unsigned long style);

    // Our parent banner, if there is one.
    CHtmlSysWinQt* parentBanner() const
    {
        return fParentBanner;
    }

    // Scroll down by a page while keeping track of when to page-pause.  The
    // 'force' flag controls whether we should scroll even while waiting for
    // input in page-pause mode.
    void scrollDown(bool force, bool justOneLine, bool instant);

    //
    // CHtmlSysWin interface implementation.
    //
    CHtmlSysWinGroup* get_win_group() override
    {
        return qWinGroup;
    }

    void notify_clear_contents() override;

    int close_window(int force) override;

    long get_disp_width() override
    {
        return viewport()->width() - 3;
    }

    long get_disp_height() override
    {
        return viewport()->height();
    }

    long get_pix_per_inch() override
    {
        return QApplication::desktop()->logicalDpiX();
    }

    CHtmlPoint measure_text(class CHtmlSysFont* font, const textchar_t* str, size_t len,
                            int* ascent) override;

    CHtmlPoint measure_dbgsrc_icon() override
    {
        return CHtmlPoint();
    }

    size_t get_max_chars_in_width(CHtmlSysFont* font, const textchar_t* str, size_t len,
                                  long wid) override;

    void draw_text(int hilite, long x, long y, CHtmlSysFont* font, const textchar_t* str,
                   size_t len) override;

    void draw_text_space(int hilite, long x, long y, CHtmlSysFont* font, long wid) override;

    void draw_bullet(int hilite, long x, long y, CHtmlSysFont* font,
                     HTML_SysWin_Bullet_t style) override;

    void draw_hrule(const CHtmlRect* pos, int shade) override;

    void draw_table_border(const CHtmlRect* pos, int width, int cell) override;

    void draw_table_bkg(const CHtmlRect* pos, HTML_color_t bgcolor) override;

    void draw_dbgsrc_icon(const CHtmlRect* pos, unsigned int stat) override;

    int do_formatting(int show_status, int update_win, int freeze_display) override;

    // We don't deal with palettes, so this is a no-op.
    void recalc_palette() override
    {}

    // We don't deal with palettes, so always return false.
    int get_use_palette() override
    {
        return false;
    }

    CHtmlSysFont* get_default_font() override;

    CHtmlSysFont* get_font(const CHtmlFontDesc* font_desc) override
    {
        if (fBannerStyleGrid) {
            // We're a text grid banner; use our internal font face,
            // "qtads-grid", which will result in createFont() using the fixed
            // width font configured in the user settings.
            CHtmlFontDesc newDesc(*font_desc);
            newDesc.copy_from(font_desc);
            strcpy(newDesc.face, "qtads-grid");
            return qFrame->createFont(&newDesc);
        }
        return qFrame->createFont(font_desc);
    }

    CHtmlSysFont* get_bullet_font(CHtmlSysFont* current_font) override
    {
        return current_font;
    }

    void register_timer_func(void (*timer_func)(void*), void* func_ctx) override;

    void unregister_timer_func(void (*timer_func)(void*), void* func_ctx) override;

    CHtmlSysTimer* create_timer(void (*timer_func)(void*), void* func_ctx) override;

    void set_timer(CHtmlSysTimer* timer, long interval_ms, int repeat) override;

    void cancel_timer(CHtmlSysTimer* timer) override;

    void delete_timer(CHtmlSysTimer* timer) override;

    void fmt_adjust_hscroll() override;

    void fmt_adjust_vscroll() override;

    void inval_doc_coords(const CHtmlRect* area) override;

    void advise_clearing_disp_list() override;

    void scroll_to_doc_coords(const CHtmlRect* pos) override;

    void get_scroll_doc_coords(CHtmlRect* pos) override;

    void set_window_title(const textchar_t* title) override;

    void set_html_bg_color(HTML_color_t color, int use_default) override;

    void set_html_text_color(HTML_color_t color, int use_default) override;

    void set_html_input_color(HTML_color_t, int) override
    // We don't provide input, so we don't deal with this.
    {}

    void set_html_link_colors(HTML_color_t link_color, int link_use_default,
                              HTML_color_t vlink_color, int vlink_use_default,
                              HTML_color_t alink_color, int alink_use_default,
                              HTML_color_t hlink_color, int hlink_use_default) override;

    HTML_color_t map_system_color(HTML_color_t color) override;

    HTML_color_t get_html_link_color() const override;

    HTML_color_t get_html_alink_color() const override;

    HTML_color_t get_html_vlink_color() const override;

    HTML_color_t get_html_hlink_color() const override;

    int get_html_link_underline() const override;

    int get_html_show_links() const override;

    int get_html_show_graphics() const override;

    void set_html_bg_image(class CHtmlResCacheObject* image) override;

    void inval_html_bg_image(unsigned int x, unsigned int y, unsigned int wid,
                             unsigned int ht) override;

    void set_banner_size(long width, HTML_BannerWin_Units_t width_units, int use_width, long height,
                         HTML_BannerWin_Units_t height_units, int use_height) override;

    void set_banner_info(HTML_BannerWin_Pos_t pos, unsigned long style) override;

    void get_banner_info(HTML_BannerWin_Pos_t* pos, unsigned long* style) override;
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
