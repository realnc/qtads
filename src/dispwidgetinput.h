// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "dispwidget.h"

#include <QTimer>

class CHtmlFormatter;
class CHtmlInputBuf;
class CHtmlSysWinQt;
class CHtmlTagTextInput;

class DisplayWidgetInput final: public DisplayWidget
{
    Q_OBJECT

private:
    // Position of the text cursor.
    QPoint fCursorPos;

    // Height of the text cursor in pixels.
    unsigned fHeight;

    // Last position of the text cursor.
    QPoint fLastCursorPos;

    // Is the text cursor visible?
    bool fCursorVisible = false;

    // Text cursor blink visibility.  Changed by a timer to show/hide the
    // cursor at specific intervals if fCursorVisible is true.
    bool fBlinkVisible = false;

    // Text cursor blink timer.
    QTimer fBlinkTimer;

    // Current input tag, if there is one (null if there isn't.)
    const CHtmlTagTextInput* fInpTag = nullptr;

    // The TADS input buffer we're working with.
    CHtmlInputBuf& fTadsBuffer;

private slots:
    // Called by the timer to blink the text cursor.
    void fBlinkCursor();

    // We need to know when the application loses focus entirely so that we
    // can disable keyboard cursor blinking when we lose focus.
    void fHandleFocusChange(const QWidget* old, const QWidget* now);

protected:
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

public:
    DisplayWidgetInput(
        CHtmlSysWinQt& parent, CHtmlFormatter& formatter_, CHtmlInputBuf& tadsBuffer);

    // Set the height of the text cursor in pixels.
    void setCursorHeight(const unsigned height)
    {
        fHeight = height - 1;
    }

    // Show/hide the text cursor.
    void setCursorVisible(const bool visible)
    {
        fCursorVisible = visible;
    }

    auto isCursorVisible() const -> bool
    {
        return fCursorVisible;
    }

    void
    updateCursorPos(CHtmlFormatter& formatter_, bool keepSelection, bool updateFormatterSelection);

    // Reset cursor blink timer.  This will read the blinking rate from the
    // desktop environment and ajust the blink timer as needed.
    void resetCursorBlinking();

    // Set the current input tag.
    void setInputTag(const CHtmlTagTextInput* const inputTag)
    {
        fInpTag = inputTag;
    }

    void clearSelection() override;
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
