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

/* Qt-specific Tads OS functions.
 *
 * This file *must not* contain C++ code, since it gets included from
 * the portable Tads C code.
 */

#ifndef OSQT_H
#define OSQT_H

#include "config.h"

/* Most systems have typedefs for ushort, uint and ulong.  If not, the
 * qtads.pro project file should be modified to define OS_NO_TYPES_DEFINED. */
#ifndef OS_NO_TYPES_DEFINED
#define OS_USHORT_DEFINED
#define OS_UINT_DEFINED
#define OS_ULONG_DEFINED
#endif

/* Standard C headers should never be included from inside an extern "C" block.
 * However, we are included from tads2/os.h from inside such a block ourselves,
 * so everything we include will be extern "C" too.  We need to reverse this or
 * some compilers will bark (Sun C++ 5.9 on Linux, for example.) */
#ifdef __cplusplus
extern "C++" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif

/* We don't support the Atari 2600. */
#include "osbigmem.h"

/* Provide some non-standard functions (memicmp(), etc). */
#include "missing.h"

/* Our Tads OEM identifier. */
#define TADS_OEM_NAME "Nikos Chantziaras <realnc@gmail.com>"

/* We assume that the C-compiler is mostly ANSI compatible. */
#define OSANSI

/* Special function qualifier needed for certain types of callback functions.
 * This is for old 16-bit systems; we don't need it and define it to nothing.
 */
#define OS_LOADDS

/* Unices don't suffer the near/far pointers brain damage (thank God) so we
 * make this a do-nothing macro. */
#define osfar_t

/* This is used to explicitly discard computed values (some compilers would
 * otherwise give a warning like "computed value not used" in some cases).
 * Casting to void should work on every ANSI-Compiler. */
#define DISCARD (void)

/* Copies a struct into another.  ANSI C allows the assignment operator to be
 * used with structs. */
#define OSCPYSTRUCT(x,y) ((x)=(y))

/* Link error messages into the application. */
#define ERR_LINK_MESSAGES

/* System identifier and system descriptive name.  We also state "Windows"
 * since we compile and run just fine under MS Windows. */
#define OS_SYSTEM_NAME "Qt"
#define OS_SYSTEM_LDESC "Qt (Unix/MacOSX/MS-Windows)"

/* Program Exit Codes. */
#define OSEXSUCC 0 /* Successful completion. */
#define OSEXFAIL 1 /* Failure. */

/* Theoretical maximum osmalloc() size.  Unix systems have at least a 32-bit
 * memory space.  Even on 64-bit systems, 2^32 is a good value, so we don't
 * bother trying to find out an exact value. */
#define OSMALMAX 0xffffffffL

/* Maximum length of a filename. */
#define OSFNMAX 255

#ifndef OSPATHALT
/* Other path separator characters. */
#define OSPATHALT ""
#endif

#ifndef OSPATHURL
/* Path separator characters for URL conversions. */
#define OSPATHURL "/"
#endif

#ifndef OSPATHCHAR
/* Normal path separator character. */
#define OSPATHCHAR '/'
#endif

#ifndef OS_NEWLINE_SEQ
/* ASCII string giving the local newline sequence to write on output. */
#define OS_NEWLINE_SEQ  "\n"
#endif

/* Directory separator for PATH-style environment variables. */
#define OSPATHSEP ':'

/* File handle structure for osfxxx functions. */
typedef FILE osfildef;

/* The maximum width of a line of text.  We ignore this, but the base code
 * needs it defined.  We use a very large value to avoid drawing the text in
 * "steps" in Tads 3. */
#define OS_MAXWIDTH 65535

/* Disable the Tads swap file; computers have plenty of RAM these days. */
#define OS_DEFAULT_SWAP_ENABLED 0

/* TADS 2 macro/function configuration.  Modern configurations always use the
 * no-macro versions, so these definitions should always be set as shown
 * below. */
#define OS_MCM_NO_MACRO
#define ERR_NO_MACRO

/* Not really needed; just a dummy. */
#define OS_TR_USAGE "usage: frob [options] file"

/* These values are used for the "mode" parameter of osfseek() to
 * indicate where to seek in the file. */
#define OSFSK_SET SEEK_SET /* Set position relative to the start of the file. */
#define OSFSK_CUR SEEK_CUR /* Set position relative to the current file position. */
#define OSFSK_END SEEK_END /* Set position relative to the end of the file. */


/* ============= Functions follow ================ */

/* Allocate a block of memory of the given size in bytes. */
#define osmalloc malloc

/* Free memory previously allocated with osmalloc(). */
#define osfree free

/* Reallocate memory previously allocated with osmalloc() or osrealloc(),
 * changing the block's size to the given number of bytes. */
#define osrealloc realloc

/* Open text file for reading. */
#define osfoprt(fname,typ) (fopen((fname),"r"))

/* Open text file for writing. */
#define osfopwt(fname,typ) (fopen((fname),"w"))

/* Open text file for reading and writing, keeping the file's existing
 * contents if the file already exists or creating a new file if no
 * such file exists. */
osfildef*
osfoprwt( const char* fname, os_filetype_t typ );

/* Open text file for reading/writing.  If the file already exists,
 * truncate the existing contents.  Create a new file if it doesn't
 * already exist. */
#define osfoprwtt(fname,typ) (fopen((fname),"w+"))

/* Open binary file for writing. */
#define osfopwb(fname,typ) (fopen((fname),"wb"))

/* Open source file for reading - use the appropriate text or binary
 * mode. */
#define osfoprs osfoprt

/* Open binary file for reading. */
#define osfoprb(fname,typ) (fopen((fname),"rb"))

/* Open binary file for reading/writing.  If the file already exists,
 * keep the existing contents.  Create a new file if it doesn't already
 * exist. */
osfildef*
osfoprwb( const char* fname, os_filetype_t typ );

/* Open binary file for reading/writing.  If the file already exists,
 * truncate the existing contents.  Create a new file if it doesn't
 * already exist. */
#define osfoprwtb(fname,typ) (fopen((fname),"w+b"))

/* Get a line of text from a text file. */
#define osfgets fgets

/* Write a line of text to a text file. */
#define osfputs fputs

/* Write bytes to file. */
#define osfwb(fp,buf,bufl) (fwrite((buf),(bufl),1,(fp))!=1)

/* Flush buffered writes to a file. */
#define osfflush fflush

/* Read bytes from file. */
#define osfrb(fp,buf,bufl) (fread((buf),(bufl),1,(fp))!=1)

/* Read bytes from file and return the number of bytes read. */
#define osfrbc(fp,buf,bufl) (fread((buf),1,(bufl),(fp)))

/* Get the current seek location in the file. */
#define osfpos ftell

/* Seek to a location in the file. */
#define osfseek fseek

/* Close a file. */
#define osfcls fclose

/* Delete a file. */
#define osfdel remove

/* Access a file - determine if the file exists.
 *
 * We map this to the access() function.  It should be available in
 * virtually every system out there, as it appears in many standards
 * (SVID, AT&T, POSIX, X/OPEN, BSD 4.3, DOS, MS Windows, maybe more). */
#define osfacc(fname) (access((fname), F_OK))

/* Get a character from a file. */
#define osfgetc fgetc


#endif /* OSQT_H */
