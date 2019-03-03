// This is copyrighted software. More information is at the end of this file.
#pragma once
#include "config.h"
#include "dispwidget.h"

class DisplayWidgetInput: public DisplayWidget
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
    bool fCursorVisible;

    // Text cursor blink visibility.  Changed by a timer to show/hide the
    // cursor at specific intervals if fCursorVisible is true.
    bool fBlinkVisible;

    // Text cursor blink timer.
    class QTimer* fBlinkTimer;

    // Current input tag, if there is one (null if there isn't.)
    class CHtmlTagTextInput* fInpTag;

    // The TADS input buffer we're working with.
    class CHtmlInputBuf* fTadsBuffer;

private slots:
    // Called by the timer to blink the text cursor.
    void fBlinkCursor();

    // We need to know when the application loses focus entirely so that we
    // can disable keyboard cursor blinking when we lose focus.
    void fHandleFocusChange(QWidget* old, QWidget* now);

protected:
    void paintEvent(QPaintEvent* e) override;

    void resizeEvent(QResizeEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;

    void mouseMoveEvent(QMouseEvent* e) override;

public:
    DisplayWidgetInput(class CHtmlSysWinQt* parent, class CHtmlFormatter* formatter,
                       CHtmlInputBuf* tadsBuffer);

    // Set the height of the text cursor in pixels.
    void setCursorHeight(unsigned height)
    {
        fHeight = height - 1;
    }

    // Show/hide the text cursor.
    void setCursorVisible(bool visible)
    {
        fCursorVisible = visible;
    }

    bool isCursorVisible()
    {
        return fCursorVisible;
    }

    void updateCursorPos(class CHtmlFormatter* formatter, bool keepSelection,
                         bool updateFormatterSelection);

    // Reset cursor blink timer.  This will read the blinking rate from the
    // desktop environment and ajust the blink timer as needed.
    void resetCursorBlinking();

    // Set the current input tag.
    void setInputTag(CHtmlTagTextInput* inputTag)
    {
        fInpTag = inputTag;
    }

    void clearSelection() override;
};

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
