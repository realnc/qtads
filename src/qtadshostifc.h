/* Copyright (C) 2013 Nikos Chantziaras.
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
#include "config.h"


/* Host application interface.  This provides a bridge between the T3 VM host
 * interface (class CVmHostIfc) and the TADS 2 application context (struct
 * appctxdef) mechanism.
 */
class QTadsHostIfc: public CVmHostIfc {
  private:
    appctxdef* fAppctx;
    int fIoSafetyRead;
    int fIoSafetyWrite;
    CResLoader* fCmapResLoader;

  public:
    QTadsHostIfc( struct appctxdef* appctx )
    : fAppctx(appctx),
      fIoSafetyRead(VM_IO_SAFETY_READWRITE_CUR),
      fIoSafetyWrite(VM_IO_SAFETY_READWRITE_CUR)
    {
        // TODO: Use the directory where charmap files are stored.
        this->fCmapResLoader = new CResLoader("./");
    }

    ~QTadsHostIfc() override
    {
        delete this->fCmapResLoader;
    }

    //
    // CVmHostIfc interface implementation.
    //

    int
    get_io_safety_read() override
    {
        if (this->fAppctx != 0 and this->fAppctx->get_io_safety_level != 0) {
            // Ask the app context to handle it.
            int readLvl;
            (*this->fAppctx->get_io_safety_level)(this->fAppctx->io_safety_level_ctx, &readLvl, 0);
            return readLvl;
        } else {
            // The app context doesn't care - use our own level memory */
            return this->fIoSafetyRead;
        }
    }

    int
    get_io_safety_write() override
    {
        if (this->fAppctx != 0 and this->fAppctx->get_io_safety_level != 0) {
            // Ask the app context to handle it.
            int writeLvl;
            (*this->fAppctx->get_io_safety_level)(this->fAppctx->io_safety_level_ctx, 0, &writeLvl);
            return writeLvl;
        } else {
            // The app context doesn't care - use our own level memory */
            return this->fIoSafetyWrite;
        }
    }

    void
    set_io_safety( int readLvl, int writeLvl ) override
    {
        if (this->fAppctx != 0 and this->fAppctx->set_io_safety_level != 0) {
            // Let the app context handle it.
            (*this->fAppctx->set_io_safety_level)(this->fAppctx->io_safety_level_ctx, readLvl, writeLvl);
        } else {
            // The app doesn't care - set our own level memory.
            this->fIoSafetyRead = readLvl;
            this->fIoSafetyWrite = writeLvl;
        }
    }

    // FIXME: Implement
    void
    get_net_safety( int* client_level, int* server_level ) override
    {
        if (client_level != 0)
            *client_level = 0;
        if (server_level != 0)
            *server_level = 0;
    }

    // FIXME: Implement
    void
    set_net_safety( int /*client_level*/, int /*server_level*/ ) override
    {
    }

    class CResLoader*
    get_sys_res_loader() override
    { return this->fCmapResLoader; }

    void
    set_image_name( const char* fname ) override
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->set_game_name != 0) {
            (*this->fAppctx->set_game_name)(this->fAppctx->set_game_name_ctx, fname);
        }
    }

    void
    set_res_dir( const char* fname ) override
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->set_res_dir != 0) {
            (*this->fAppctx->set_res_dir)(this->fAppctx->set_res_dir_ctx, fname);
        }
    }

    int
    add_resfile( const char* fname ) override
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resfile != 0) {
            return (*this->fAppctx->add_resfile)(this->fAppctx->add_resfile_ctx, fname);
        } else {
            return 0;
        }
    }

    // We suport add_resfile() if the application context does.
    int
    can_add_resfiles() override
    {
        // If the add_resfile function is defined in the application context,
        // we support adding resource files.
        return (this->fAppctx != 0 and this->fAppctx->add_resfile != 0);
    }

    void
    add_resource( unsigned long ofs, unsigned long siz, const char* res_name, size_t res_name_len,
                  int fileno ) override
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resource != 0) {
            (*this->fAppctx->add_resource)(this->fAppctx->add_resource_ctx, ofs, siz, res_name,
                                           res_name_len, fileno);
        }
    }

    void
    add_resource( const char* fname, size_t fname_len, const char* res_name, size_t res_name_len) override
    {
        // Pass it through the app context if possible.
        if (this->fAppctx != 0 and this->fAppctx->add_resource_link != 0) {
            (*this->fAppctx->add_resource_link)
                    (this->fAppctx->add_resource_link_ctx, fname, fname_len, res_name, res_name_len);
        }
    }

    const char*
    get_res_path() override
    {
        // Get the path from the app context if possible.
        return (this->fAppctx != 0 ? this->fAppctx->ext_res_path : 0);
    }

    // Determine if a resource exists.
    int
    resfile_exists( const char* res_name, size_t res_name_len ) override
    {
        // Let the application context handle it if possible; if not, just
        // return false, since we can't otherwise provide resource operations.
        if (this->fAppctx != 0 and this->fAppctx->resfile_exists != 0) {
            return (*this->fAppctx->resfile_exists)(this->fAppctx->resfile_exists_ctx, res_name, res_name_len);
        } else {
            return false;
        }
    }

    osfildef*
    find_resource( const char* res_name, size_t res_name_len, unsigned long* res_size ) override
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

    vmhost_gin_t
    get_image_name( char* buf, size_t buflen ) override
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
    void
    get_special_file_path( char* buf, size_t buflen, int id ) override
    {
        return os_get_special_path(buf, buflen, 0, id);
    }
};


#endif
