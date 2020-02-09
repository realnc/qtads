// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <cstddef>

#include "appctx.h"
#include "config.h"
#include "resload.h"
#include "vmhost.h"

/* Host application interface.  This provides a bridge between the T3 VM host
 * interface (class CVmHostIfc) and the TADS 2 application context (struct
 * appctxdef) mechanism.
 */
class QTadsHostIfc: public CVmHostIfc
{
private:
    appctxdef* fAppctx;
    int fIoSafetyRead;
    int fIoSafetyWrite;
    CResLoader* fCmapResLoader;

public:
    QTadsHostIfc(struct appctxdef* appctx)
        : fAppctx(appctx)
        , fIoSafetyRead(VM_IO_SAFETY_READWRITE_CUR)
        , fIoSafetyWrite(VM_IO_SAFETY_READWRITE_CUR)
    {
        // TODO: Use the directory where charmap files are stored.
        fCmapResLoader = new CResLoader("./");
    }

    ~QTadsHostIfc() override
    {
        delete fCmapResLoader;
    }

    //
    // CVmHostIfc interface implementation.
    //

    auto get_io_safety_read() -> int override
    {
        if (fAppctx != nullptr and fAppctx->get_io_safety_level != nullptr) {
            // Ask the app context to handle it.
            int readLvl;
            (*fAppctx->get_io_safety_level)(fAppctx->io_safety_level_ctx, &readLvl, nullptr);
            return readLvl;
        } else {
            // The app context doesn't care - use our own level memory */
            return fIoSafetyRead;
        }
    }

    auto get_io_safety_write() -> int override
    {
        if (fAppctx != nullptr and fAppctx->get_io_safety_level != nullptr) {
            // Ask the app context to handle it.
            int writeLvl;
            (*fAppctx->get_io_safety_level)(fAppctx->io_safety_level_ctx, nullptr, &writeLvl);
            return writeLvl;
        } else {
            // The app context doesn't care - use our own level memory */
            return fIoSafetyWrite;
        }
    }

    void set_io_safety(int readLvl, int writeLvl) override
    {
        if (fAppctx != nullptr and fAppctx->set_io_safety_level != nullptr) {
            // Let the app context handle it.
            (*fAppctx->set_io_safety_level)(fAppctx->io_safety_level_ctx, readLvl, writeLvl);
        } else {
            // The app doesn't care - set our own level memory.
            fIoSafetyRead = readLvl;
            fIoSafetyWrite = writeLvl;
        }
    }

    // FIXME: Implement
    void get_net_safety(int* client_level, int* server_level) override
    {
        if (client_level != nullptr)
            *client_level = 0;
        if (server_level != nullptr)
            *server_level = 0;
    }

    // FIXME: Implement
    void set_net_safety(int /*client_level*/, int /*server_level*/) override
    {}

    auto get_sys_res_loader() -> class CResLoader* override
    {
        return fCmapResLoader;
    }

    void set_image_name(const char* fname) override
    {
        // Pass it through the app context if possible.
        if (fAppctx != nullptr and fAppctx->set_game_name != nullptr) {
            (*fAppctx->set_game_name)(fAppctx->set_game_name_ctx, fname);
        }
    }

    void set_res_dir(const char* fname) override
    {
        // Pass it through the app context if possible.
        if (fAppctx != nullptr and fAppctx->set_res_dir != nullptr) {
            (*fAppctx->set_res_dir)(fAppctx->set_res_dir_ctx, fname);
        }
    }

    auto add_resfile(const char* fname) -> int override
    {
        // Pass it through the app context if possible.
        if (fAppctx != nullptr and fAppctx->add_resfile != nullptr) {
            return (*fAppctx->add_resfile)(fAppctx->add_resfile_ctx, fname);
        } else {
            return 0;
        }
    }

    // We suport add_resfile() if the application context does.
    auto can_add_resfiles() -> int override
    {
        // If the add_resfile function is defined in the application context,
        // we support adding resource files.
        return (fAppctx != nullptr and fAppctx->add_resfile != nullptr);
    }

    void add_resource(unsigned long ofs, unsigned long siz, const char* res_name,
                      size_t res_name_len, int fileno) override
    {
        // Pass it through the app context if possible.
        if (fAppctx != nullptr and fAppctx->add_resource != nullptr) {
            (*fAppctx->add_resource)(fAppctx->add_resource_ctx, ofs, siz, res_name, res_name_len,
                                     fileno);
        }
    }

    void add_resource(const char* fname, size_t fname_len, const char* res_name,
                      size_t res_name_len) override
    {
        // Pass it through the app context if possible.
        if (fAppctx != nullptr and fAppctx->add_resource_link != nullptr) {
            (*fAppctx->add_resource_link)(fAppctx->add_resource_link_ctx, fname, fname_len,
                                          res_name, res_name_len);
        }
    }

    auto get_res_path() -> const char* override
    {
        // Get the path from the app context if possible.
        return (fAppctx != nullptr ? fAppctx->ext_res_path : nullptr);
    }

    // Determine if a resource exists.
    auto resfile_exists(const char* res_name, size_t res_name_len) -> int override
    {
        // Let the application context handle it if possible; if not, just
        // return false, since we can't otherwise provide resource operations.
        if (fAppctx != nullptr and fAppctx->resfile_exists != nullptr) {
            return (*fAppctx->resfile_exists)(fAppctx->resfile_exists_ctx, res_name, res_name_len);
        } else {
            return false;
        }
    }

    auto find_resource(const char* res_name, size_t res_name_len,
                            unsigned long* res_size) -> osfildef* override
    {
        // Let the application context handle it; if we don't have an
        // application context, we don't provide resource operation, so simply
        // return failure.
        if (fAppctx != nullptr and fAppctx->find_resource != nullptr) {
            return (*fAppctx->find_resource)(fAppctx->find_resource_ctx, res_name, res_name_len,
                                             res_size);
        } else {
            return nullptr;
        }
    }

    auto get_image_name(char* buf, size_t buflen) -> vmhost_gin_t override
    {
        // Let the application context handle it if possible; otherwise, return
        // false, since we can't otherwise ask for an image name.
        if (fAppctx != nullptr and fAppctx->get_game_name != nullptr) {
            // Ask the host system to get a name.
            int ret = (*fAppctx->get_game_name)(fAppctx->get_game_name_ctx, buf, buflen);

            // If that failed, the user must have chosen to cancel; otherwise,
            // we were successful.
            return (ret ? VMHOST_GIN_SUCCESS : VMHOST_GIN_CANCEL);
        } else {
            // We can't ask for a name.
            return VMHOST_GIN_IGNORED;
        }
    }

    // Get a special file system path.
    void get_special_file_path(char* buf, size_t buflen, int id) override
    {
        return os_get_special_path(buf, buflen, nullptr, id);
    }
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
