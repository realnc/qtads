/* $Header: d:/cvsroot/tads/html/htmlinp.h,v 1.2 1999/05/17 02:52:21 MJRoberts Exp $ */

/* 
 *   Copyright (c) 1997 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  htmlinp.h - HTML input editor
Function
  This class handles the internals of editing an input buffer.  This
  class does not provide a user interface; instead, it's intended to
  be used by user interface code to simplify the maintenance of an
  input buffer.

  This class also provides a command history function.
Notes
  The caller provides the buffer, and is responsible for managing
  the memory containing the buffer (in particular, we will not delete
  the buffer).
Modified
  09/28/97 MJRoberts  - Creation
*/

#ifndef HTMLINP_H
#define HTMLINP_H

#ifndef TADSHTML_H
#include "tadshtml.h"
#endif
#ifndef HTML_OS_H
#include "html_os.h"
#endif


class CHtmlInputBuf
{
public:
    CHtmlInputBuf(size_t max_hist_cnt)
    {
        undo_buf_ = 0;
        setbuf(0, 0);
        set_hist_size(max_hist_cnt);
        utf8_ = FALSE;
    }
    CHtmlInputBuf(textchar_t *buf, size_t bufsiz, size_t max_hist_cnt)
    {
        undo_buf_ = 0;
        setbuf(buf, bufsiz);
        set_hist_size(max_hist_cnt);
        utf8_ = FALSE;
    }

    ~CHtmlInputBuf();

    /*
     *   Set the input buffer's character set (also known as the "code
     *   page").  This MUST be called IF the system implementation of
     *   os_next_char() and os_prev_char() require the character set
     *   information.
     *   
     *   In general, the system window code will need to call this for
     *   implementations that (a) support multi-byte character sets (MBCS)
     *   and (b) allow the character set to change on the fly.  On
     *   single-byte-only platforms, os_next_char() and os_prev_char() are
     *   trivial in that one character always equals one byte, so the
     *   character set information isn't needed to implement them.  Even if
     *   MBCSs are supported, the character set information isn't needed if
     *   the character set can't change during execution, since the system
     *   code can simply assume everything's in the fixed app-wide character
     *   set.  It's only necessary to have the per-string information on
     *   systems like Windows, where MBCSs are supported and where each
     *   string can be in its own separate character set.
     *   
     *   If the character set information is needed, this must be called
     *   immediately after construction, and again each time the buffer
     *   character set changes (for example, due to a user preference
     *   change).
     *   
     *   Note that only one character set can be selected at a time per input
     *   buffer.  It's not possible for an individual input buffer to use a
     *   mix of different character sets - the whole buffer must be in a
     *   single character set.  
     */
    void set_charset(oshtml_charset_id_t charset) { charset_ = charset; }

    /*
     *   Put the buffer in UTF-8 mode.  We're in single-byte mode by default,
     *   meaning that one character == one byte.  In UTF-8 mode, we'll
     *   observe UTF-8 multi-byte characters when stepping through the
     *   buffer, to ensure that we always move by whole characters.  (Note,
     *   however, that we don't check explicit position settings sent in from
     *   the caller for validity, such as in set_caret() or set_sel_range().
     *   We only check our own changes; we assume that when our caller puts
     *   us in UTF-8 mode, the caller is also UTF-8 aware, so we leave it up
     *   to the caller to get its own position settings right.)  
     */
    void set_utf8_mode(int f) { utf8_ = f; }
    int get_utf8_mode() const { return utf8_; }

    /* increment a buffer offset, observing utf-8 mode as needed */
    size_t nextchar(size_t idx)
    {
        /* move to the next byte */
        idx = os_next_char(charset_, buf_ + idx, len_ - idx) - buf_;

        /* in utf-8 mode, skip continuation characters (up to 2 per char) */
        if (utf8_)
        {
            if ((*(buf_ + idx) & 0xC0) == 0x80)
                ++idx;
            if ((*(buf_ + idx) & 0xC0) == 0x80)
                ++idx;
        }

        /* return the result */
        return idx;
    }
    
    /* decrement a pointer into our buffer, observing utf-8 mode as needed */
    size_t prevchar(size_t idx)
    {
        /* move to the previous byte */
        idx = os_prev_char(charset_, buf_ + idx, buf_) - buf_;

        /* in utf-8 mode, skip continuation characters (up to 2 per char) */
        if (utf8_)
        {
            if (idx > 0 && (*(buf_ + idx) & 0xC0) == 0x80)
                --idx;
            if (idx > 0 && (*(buf_ + idx) & 0xC0) == 0x80)
                --idx;
        }

        /* return the result */
        return idx;
    }

    /* set the buffer and initialize the editing state */
    void setbuf(textchar_t *buf, size_t bufsiz) { setbuf(buf, bufsiz, 0); }
    void setbuf(textchar_t *buf, size_t bufsiz, size_t initlen);

    /* change the buffer, without resetting any editing state */
    void changebuf(textchar_t *buf, size_t bufsiz);

    /* set the selection range */
    void get_sel_range(size_t *start, size_t *end, size_t *caret) const
    {
        /* set the range */
        *start = sel_start_;
        *end = sel_end_;
        *caret = caret_;
    }

    /* show or hide the caret */
    void show_caret() { caret_vis_ = TRUE; }
    void hide_caret() { caret_vis_ = FALSE; }

    /* determine if the caret is showing */
    int is_caret_visible() const { return caret_vis_; }

    /* get/set the current command length */
    size_t getlen() const { return len_; }
    void setlen(size_t len) { len_ = len; }

    /* get the buffer pointer */
    textchar_t *getbuf() const { return buf_; }

    /*
     *   Set the selection range and caret position.  The caret should
     *   always be at one end or the other of the selection range. 
     */
    void set_sel_range(size_t start, size_t end, size_t caret)
    {
        /* make sure everything is in range */
        if (start > len_) start = len_;
        if (end > len_) end = len_;
        if (caret > len_) caret = len_;

        /* save the caret position */
        caret_ = caret;

        /* make sure the range is in canonical order (start <= end) */
        if (start <= end)
        {
            sel_start_ = start;
            sel_end_ = end;
        }
        else
        {
            sel_end_ = start;
            sel_start_ = end;
        }

        /* show the caret */
        show_caret();
    }

    /* set the insertion point and set a zero-length range */
    void set_caret(size_t caret)
    {
        /* make sure the value is in range */
        if (caret > len_)
            caret = len_;

        /* set the insertion point and range to the new point */
        caret_ = sel_start_ = sel_end_ = caret;
    }

    /* get the caret position */
    size_t get_caret() const { return caret_; }

    /*
     *   Delete the selected range.  Returns true if anything changed,
     *   false if not. 
     */
    int del_selection();

    /*
     *   Move the cursor left or right one character.  If 'extend' is
     *   true, we'll extend the selection range, otherwise we'll move the
     *   cursor and set a zero-length selection range.  If 'word' is true,
     *   we'll move to the next word break.  
     */
    void move_left(int extend, int word);
    void move_right(int extend, int word);

    /*
     *   Backspace - delete selection range, or delete character to the
     *   left of the cursor if the selection range length is zero.
     *   Returns true if anything changed, false if not.  
     */
    int backspace();

    /*
     *   Delete right - delete the selection range, or delete the
     *   character to the right of the cursor if the selection range
     *   length is zero.  Returns true if anything changed, false if not.
     */
    int del_right();

    /*
     *   Delete to end of line.  Returns true if anything changed.
     */
    int del_eol();

    /*
     *   Delete entire line.  Returns true if anything changed.
     */
    int del_line();

    /*
     *   Add a character; replaces the selection range, or inserts the
     *   character at the selection point if the range is zero length.
     *   Returns true if anything changed, false if not.  
     */
    int add_char(textchar_t c);

    /*
     *   Add a UTF-8 character.  We accept characters in the basic
     *   multilingual plane (u0000 to uFFFF) only.  We'll encode the
     *   character into multiple bytes as needed to represent it as UTF-8.
     *   Returns true if anything changed, false if not.  If the character is
     *   too large to fit the buffer when expanded into UTF-8, we'll return
     *   false and insert nothing - we'll never insert an ill-formed partial
     *   character.  
     */
    int add_char_utf8(unsigned int c);

    /*
     *   Add a string; replaces the selection range or inserts the string
     *   at the selection point if the range is zero length.  Returns true
     *   if anything changed, false if not.  If 'trunc' is true, we'll
     *   truncate the input if necessary to make it fit in the buffer,
     *   otherwise we'll ignore the entire request if the input won't fit.
     */
    int add_string(const textchar_t *str, size_t len, int trunc);

    /*
     *   Move the selection to the start/end of the command.  If 'extend'
     *   is true, extends range to the start/end of the command.  
     */
    void start_of_line(int extend);
    void end_of_line(int extend);

    /* add a history item */
    void add_hist(const textchar_t *str, size_t len);

    /* add the current buffer contents to the history list */
    void add_hist() { add_hist(buf_, len_); }

    /*
     *   Set the history cursor to the last (most recent) history item.
     *   Calling select_prev_hist() will select the last item into the
     *   buffer.  This should be called at the start of each new command.
     */
    void set_last_hist() { histpos_ = histcnt_; }

    /*
     *   Select the current/next/previous history item into the buffer.
     *   These return true if anything happened.  
     */
    int select_cur_hist();
    int select_next_hist();
    int select_prev_hist();

    /* select a previous history item matching the given prefix string */
    int select_prev_hist_prefix(const textchar_t *pre, size_t len);

    /* select based on the current buffer contents and caret position */
    int select_prev_hist_prefix();

    /* check to see if we can undo anything in this command */
    int can_undo() { return undo_buf_ != 0; }

    /* undo the last change */
    int undo();

private:
    /* initialize the history size */
    void set_hist_size(size_t max_hist_cnt);
    
    /* adjust selection range */
    void adjust_sel(size_t newpos, int extend);

    /*
     *   save the current line under construction for later
     *   reconstruction if we scroll through the history back to the new,
     *   otherwise unsaved, line 
     */
    void save_cur_line();

    /* save the current buffer in the undo */
    void save_undo();

    /* buffer */
    textchar_t *buf_;
    size_t bufsiz_;

    /* the character set */
    oshtml_charset_id_t charset_;

    /* length of the input in the buffer */
    size_t len_;

    /* selection range */
    size_t sel_start_;
    size_t sel_end_;

    /*
     *   caret position - if a range is selected, the caret will always
     *   be at one end or the other of the selected range 
     */
    size_t caret_;

    /* command history list */
    textchar_t **history_;

    /* saved current line under construction */
    textchar_t *saved_cur_line_;

    /* maximum number of entries in history list */
    size_t histsiz_;

    /* current number of entries in history list */
    size_t histcnt_;

    /* current position in history buffer */
    size_t histpos_;

    /* caret visibility */
    int caret_vis_ : 1;

    /* flag: true -> we're in UTF-8 mode; false -> single-byte mode */
    int utf8_ : 1;

    /* undo buffer - contents of buffer prior to last change */
    textchar_t *undo_buf_;
    size_t undo_len_;
    size_t undo_sel_start_;
    size_t undo_sel_end_;
    size_t undo_caret_;
    size_t undo_histpos_;
};

#endif /* HTMLINP_H */
