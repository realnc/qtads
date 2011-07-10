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
#ifndef QTADSHOSTIFC_H
#define QTADSHOSTIFC_H

#include <cstddef>

#include "vmhost.h"
#include "resload.h"
#include "appctx.h"


/* Host application interface.  This provides a bridge between the T3 VM host
 * interface (class CVmHostIfc) and the TADS 2 application context (struct
 * appctxdef) mechanism.
 */
class QTadsHostIfc: public CVmHostIfc {
  private:
    appctxdef* fAppctx;
    int fIoSafety;
    CResLoader* fCmapResLoader;

  public:
    QTadsHostIfc( struct appctxdef* appctx )
    : fAppctx(appctx), fIoSafety(VM_IO_SAFETY_READWRITE_CUR)
    {
        // TODO: Use the directory where charmap files are stored.
        this->fCmapResLoader = new CResLoader("./");
    }

    ~QTadsHostIfc()
    {
        delete this->fCmapResLoader;
    }

    //
    // CVmHostIfc interface implementation.
    //
    // FIXME: Split read/write.
    virtual int
    get_io_safety_read()
    {
        if (this->fAppctx != 0 and this->fAppctx->get_io_safety_level != 0) {
            // Ask the app context to handle it.
            int read;
            (*this->fAppctx->get_io_safety_level)(this->fAppctx->io_safety_level_ctx, &read, 0);
            return read;
        } else {
            // The app context doesn't care - use our own level memory */
            return this->fIoSafety;
        }
    }

    // FIXME: Split read/write.
    virtual int
    get_io_safety_write()
    {
        if (this->fAppctx != 0 and this->fAppctx->get_io_safety_level != 0) {
            // Ask the app context to handle it.
            int write;
            (*this->fAppctx->get_io_safety_level)(this->fAppctx->io_safety_level_ctx, 0, &write);
            return write;
        } else {
            // The app context doesn't care - use our own level memory */
            return this->fIoSafety;
        }
    }

    // FIXME: Split read/write.
    virtual void
    set_io_safety( int read, int write )
    {
        if (this->fAppctx != 0 and this->fAppctx->set_io_safety_level != 0) {
            // Let the app context handle it.
            (*this->fAppctx->set_io_safety_level)(this->fAppctx->io_safety_level_ctx, read, write);
        } else {
            // The app doesn't care - set our own level memory.
            this->fIoSafety = read;
        }
    }

    // FIXME: Implement
    virtual void
    get_net_safety( int* client_level, int* server_level )
    {
        if (client_level != 0)
            *client_level = 0;
        if (server_level != 0)
            *server_level = 0;
    }

    // FIXME: Implement
    virtual void
    set_net_safety( int client_level, int server_level )
    {
    }

    virtual class CResLoader*
    get_cmap_res_loader()
    { return this->fCmapResLoader; }

    void
    set_image_name( const char* fname )
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->set_game_name != 0) {
            (*this->fAppctx->set_game_name)(this->fAppctx->set_game_name_ctx, fname);
        }
    }

    virtual void
    set_res_dir( const char* fname )
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->set_res_dir != 0) {
            (*this->fAppctx->set_res_dir)(this->fAppctx->set_res_dir_ctx, fname);
        }
    }

    virtual int
    add_resfile( const char* fname )
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resfile != 0) {
            return (*this->fAppctx->add_resfile)(this->fAppctx->add_resfile_ctx, fname);
        } else {
            return 0;
        }
    }

    // We suport add_resfile() if the application context does.
    virtual int
    can_add_resfiles()
    {
        // If the add_resfile function is defined in the application context,
        // we support adding resource files.
        return (this->fAppctx != 0 and this->fAppctx->add_resfile != 0);
    }

    virtual void
    add_resource( unsigned long ofs, unsigned long siz, const char* res_name, size_t res_name_len, int fileno )
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resource != 0) {
            (*this->fAppctx->add_resource)(this->fAppctx->add_resource_ctx, ofs, siz, res_name, res_name_len, fileno);
        }
    }

    virtual void
    add_resource( const char* fname, size_t fname_len, const char* res_name, size_t res_name_len)
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resource_link != 0) {
            (*this->fAppctx->add_resource_link)
                    (this->fAppctx->add_resource_link_ctx, fname, fname_len, res_name, res_name_len);
        }
    }

    virtual const char*
    get_res_path()
    {
        // Get the path from the app context if possible.
        return (this->fAppctx != 0 ? this->fAppctx->ext_res_path : 0);
    }

    // Determine if a resource exists.
    virtual int
    resfile_exists( const char* res_name, size_t res_name_len )
    {
        // Let the application context handle it if possible; if not, just
        // return false, since we can't otherwise provide resource operations.
        if (this->fAppctx != 0 and this->fAppctx->resfile_exists != 0) {
            return (*this->fAppctx->resfile_exists)(this->fAppctx->resfile_exists_ctx, res_name, res_name_len);
        } else {
            return false;
        }
    }

    virtual osfildef*
    find_resource( const char* res_name, size_t res_name_len, unsigned long* res_size )
    {
        // Let the application context handle it; if we don't have an
        // application context, we don't provide resource operation, so simply
        // return failure.
        if (this->fAppctx != 0 and this->fAppctx->find_resource != 0) {
            return (*this->fAppctx->find_resource)(this->fAppctx->find_resource_ctx, res_name, res_name_len, res_size);
        } else {
            return 0;
        }
    }

    virtual vmhost_gin_t
    get_image_name( char* buf, size_t buflen )
    {
        // Let the application context handle it if possible; otherwise, return
        // false, since we can't otherwise ask for an image name.
        if (this->fAppctx != 0 and this->fAppctx->get_game_name != 0) {
            // Ask the host system to get a name.
            int ret = (*this->fAppctx->get_game_name)(this->fAppctx->get_game_name_ctx, buf, buflen);

            // If that failed, the user must have chosen to cancel; otherwise,
            // we were successful.
            return (ret ? VMHOST_GIN_SUCCESS : VMHOST_GIN_CANCEL);
        } else {
            // We can't ask for a name.
            return VMHOST_GIN_IGNORED;
        }
    }

    // Get a special file system path.
    virtual void
    get_special_file_path( char* buf, size_t buflen, int id )
    {
        return os_get_special_path(buf, buflen, 0, id);
    }
};


#endif
