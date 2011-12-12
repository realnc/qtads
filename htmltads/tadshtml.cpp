#ifdef RCSID
static char RCSid[] =
"$Header: d:/cvsroot/tads/html/tadshtml.cpp,v 1.3 1999/07/11 00:46:41 MJRoberts Exp $";
#endif

/* 
 *   Copyright (c) 1997 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  tadshtml.cpp - utility functions for TADS HTML engine
Function
  
Notes
  
Modified
  08/26/97 MJRoberts  - Creation
*/

#include <stdio.h>
#include <memory.h>

#ifndef TADSHTML_H
#include "tadshtml.h"
#endif
#ifndef HTMLSYS_H
#include "htmlsys.h"
#endif
#ifndef HTML_OS_H
#include "html_os.h"
#endif

/* ------------------------------------------------------------------------ */
/*
 *   Text stream buffer.  This buffer accumulates output for eventual
 *   submission to the parser.  
 */

/*
 *   Append text to the end of the buffer 
 */
void CHtmlTextBuffer::append(const textchar_t *txt, size_t len)
{
    /* make sure we have enough space allocated */
    if (buf_ == 0)
    {
        /* nothing allocated yet - allocate the initial buffer space */
        bufalo_ = init_alloc_unit;
        buf_ = (textchar_t *)th_malloc(bufalo_ * sizeof(textchar_t));

        /* nothing in the buffer yet - the end is the beginning */
        bufend_ = buf_;
    }
    else if (getlen() + len > bufalo_)
    {
        size_t curlen;

        /* remember the current length, so we can reposition bufend_ later */
        curlen = getlen();

        /* allocate the additional space */
        bufalo_ += extra_alloc_unit;
        buf_ = (textchar_t *)th_realloc(buf_, bufalo_ * sizeof(textchar_t));

        /* update the end pointer, in case the buffer moved */
        bufend_ = buf_ + curlen;
    }

    /* copy the text into the buffer */
    memcpy(bufend_, txt, len * sizeof(*txt));

    /* remember the new end pointer */
    bufend_ += len;
}

/* ------------------------------------------------------------------------ */
/*
 *   Counted-length string pointer 
 */

textchar_t CCntlenStrPtr::nextchar() const
{
    /*
     *   if we have another character after the current character, return
     *   it, otherwise return a null character 
     */
    return (textchar_t)(len_ > 1 ? *(ptr_ + 1) : 0);
}

/* ------------------------------------------------------------------------ */
/*
 *   Safe strcpy 
 */
void safe_strcpy(textchar_t *dst, size_t dstsize,
                 const textchar_t *src, size_t srclen)
{
    size_t copylen;

    /* if the destination buffer is zero-size, we can't do anything */
    if (dstsize == 0)
        return;

    /* assume we'll copy the whole source */
    copylen = srclen;

    /* limit to the available destination space less null termination */
    if (copylen > dstsize - 1)
        copylen = dstsize - 1;

    /* copy the characters */
    memcpy(dst, src, copylen * sizeof(src[0]));

    /* null-terminate */
    dst[copylen] = '\0';
}


/* ------------------------------------------------------------------------ */
/*
 *   Get the rightmost instance of a substring in a string 
 */
textchar_t *get_strrstr(const textchar_t *str, const textchar_t *sub)
{
    const char *p;
    size_t slen = get_strlen(str);
    size_t sublen = get_strlen(sub);

    /* 
     *   if the substring is longer than the string, we're obviously not
     *   going to find it 
     */
    if (sublen > slen)
        return 0;
    
    /* scan from the end of the string backwards */
    for (p = str + slen - sublen + 1 ; p != str ; )
    {
        /* back one character */
        --p;

        /* check for a match */
        if (memcmp(p, sub, sublen) == 0)
            return (char *)p;
    }

    /* not found */
    return 0;
}



