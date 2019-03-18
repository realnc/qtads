// This is copyrighted software. More information is at the end of this file.
#include "rwopsbundle.h"

#include <SDL_error.h>
#include <SDL_rwops.h>
#include <SDL_version.h>
#include <errno.h>
#include <string.h>

/* Our custom RWops type id. Not strictly needed, but it helps catching bugs if somehow we end up
 * trying to delete a different type of RWops. */
#define CUSTOM_RWOPS_TYPE 3819859

/* Media resource information for our custom RWops implementation. Media resources are embedded
 * inside media bundle files. They begin at 'startPos' and end at 'endPos' inside the 'file' bundle.
 */
typedef struct
{
    FILE* file;
    long startPos;
    long endPos;
#if SDL_VERSION_ATLEAST(1, 3, 0)
    Sint64 size;
#endif
} BundleFileInfo;

/* Helper routine.  Checks whether the 'rwops' is of our own type.
 */
static SDL_bool RWOpsCheck(SDL_RWops* rwops)
{
    if (rwops->type != CUSTOM_RWOPS_TYPE) {
        SDL_SetError("Unrecognized RWops type %u", rwops->type);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

/* RWops size callback. Only exists in SDL2 and reports the size of our data in bytes.
 */
#if SDL_VERSION_ATLEAST(1, 3, 0)
static Sint64 RWOpsSizeFunc(SDL_RWops* rwops)
{
    if (!RWOpsCheck(rwops)) {
        return -1;
    }
    return ((BundleFileInfo*)rwops->hidden.unknown.data1)->size;
}
#endif

/* RWops seek callback. We apply offsets to make all seek operations relative to the start/end of
 * the media resource embedded inside the media bundle file.
 *
 * Must return the new current SEET_SET position.
 */
#if SDL_VERSION_ATLEAST(1, 3, 0)
static Sint64 RWOpsSeekFunc(SDL_RWops* rwops, Sint64 offset, int whence)
#else
static int RWOpsSeekFunc(SDL_RWops* rwops, int offset, int whence)
#endif
{
    BundleFileInfo* info;
    int seekRet;
    if (!RWOpsCheck(rwops)) {
        return -1;
    }
    info = rwops->hidden.unknown.data1;
    errno = 0;
    if (whence == RW_SEEK_CUR) {
        seekRet = fseek(info->file, offset, RW_SEEK_CUR);
    } else if (whence == RW_SEEK_SET) {
        seekRet = fseek(info->file, info->startPos + offset, RW_SEEK_SET);
    } else {
        seekRet = fseek(info->file, info->endPos + offset, RW_SEEK_SET);
    }
    if (seekRet != 0) {
        SDL_SetError("Could not fseek() in media bundle (%s)",
                     errno != 0 ? strerror(errno) : "unknown error");
        return -1;
    }
    return ftell(info->file) - info->startPos;
}

/* RWops read callback. We don't allow reading past the end of the media resource embedded inside
 * the media bundle file.
 *
 * Must return the number of elements (not bytes) that have been read.
 */
#if SDL_VERSION_ATLEAST(1, 3, 0)
static size_t RWOpsReadFunc(SDL_RWops* rwops, void* ptr, size_t size, size_t maxnum)
#else
static int RWOpsReadFunc(SDL_RWops* rwops, void* ptr, int size, int maxnum)
#endif
{
    BundleFileInfo* info;
    long bytesToRead = size * maxnum;
    long curPos;
    size_t itemsRead;
    if (!RWOpsCheck(rwops)) {
        return -1;
    }
    info = rwops->hidden.unknown.data1;
    curPos = ftell(info->file);
    /* Make sure we don't read past the end of the embedded media resource. */
    if (curPos + bytesToRead > info->endPos) {
        bytesToRead = info->endPos - curPos;
        maxnum = bytesToRead / size;
    }
    itemsRead = fread(ptr, size, maxnum, info->file);
    if (maxnum != 0 && itemsRead == 0) {
        SDL_SetError("Could not read from file stream with fread()");
        return -1;
    }
    return itemsRead;
}

/* RWops write callback. This always fails, since we never write to media bundle files.
 */
#if SDL_VERSION_ATLEAST(1, 3, 0)
static size_t RWOpsWriteFunc(SDL_RWops* rwops, const void* ptr, size_t size, size_t num)
#else
static int RWOpsWriteFunc(SDL_RWops* rwops, const void* ptr, int size, int num)
#endif
{
    (void)ptr;
    (void)size;
    (void)num;
    if (!RWOpsCheck(rwops)) {
        return -1;
    }
    SDL_SetError("Media bundle files are not supposed to be written to");
    return -1;
}

/* RWops close callback. Frees the RWops as well as our custom data.
 */
static int RWOpsCloseFunc(SDL_RWops* rwops)
{
    BundleFileInfo* info;
    if (!RWOpsCheck(rwops)) {
        return -1;
    }
    info = rwops->hidden.unknown.data1;
    fclose(info->file);
    SDL_free(info);
    SDL_FreeRW(rwops);
    return 0;
}

SDL_RWops* RWFromMediaBundle(FILE* mediaBundle, long resLength)
{
    BundleFileInfo* info;
    errno = 0;
    SDL_RWops* rwops = SDL_AllocRW();
    if (rwops == NULL) {
        SDL_SetError("%s", errno != 0 ? strerror(errno) : "Cannot allocate memory");
        return NULL;
    }

    errno = 0;
    info = SDL_malloc(sizeof *info);
    if (info == NULL) {
        SDL_SetError("%s", errno != 0 ? strerror(errno) : "Cannot allocate memory");
        SDL_FreeRW(rwops);
        return NULL;
    }
    info->file = mediaBundle;
    errno = 0;
    info->startPos = ftell(mediaBundle);
    if (info->startPos == -1) {
        SDL_SetError("Could not obtain current file stream position (%s)",
                     errno != 0 ? strerror(errno) : "unknown error");
        SDL_free(info);
        SDL_FreeRW(rwops);
        return NULL;
    }
    info->endPos = info->startPos + resLength;

    rwops->hidden.unknown.data1 = info;
#if SDL_VERSION_ATLEAST(1, 3, 0)
    rwops->size = RWOpsSizeFunc;
    info->size = resLength;
#endif
    rwops->seek = RWOpsSeekFunc;
    rwops->read = RWOpsReadFunc;
    rwops->write = RWOpsWriteFunc;
    rwops->close = RWOpsCloseFunc;
    rwops->type = CUSTOM_RWOPS_TYPE;
    return rwops;
}

/* Copyright (C) 2011-2019 Nikos Chantziaras
 *
 * This file is part of Hugor.
 *
 * Hugor is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Hugor is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Hugor.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Additional permission under GNU GPL version 3 section 7
 *
 * If you modify this Program, or any covered work, by linking or combining it
 * with the Hugo Engine (or a modified version of the Hugo Engine), containing
 * parts covered by the terms of the Hugo License, the licensors of this
 * Program grant you additional permission to convey the resulting work.
 * Corresponding Source for a non-source form of such a combination shall
 * include the source code for the parts of the Hugo Engine used as well as
 * that of the covered work.
 */
