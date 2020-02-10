// This is copyrighted software. More information is at the end of this file.

// TADS heeaders are finicky. This needs to be included first.
#include "os.h"

#include "qtadshostifc.h"

QTadsHostIfc::QTadsHostIfc(appctxdef* const appctx)
    : fAppctx(appctx)
    , fIoSafetyRead(VM_IO_SAFETY_READWRITE_CUR)
    , fIoSafetyWrite(VM_IO_SAFETY_READWRITE_CUR)
{
    // TODO: Use the directory where charmap files are stored.
    fCmapResLoader = new CResLoader("./");
}

QTadsHostIfc::~QTadsHostIfc()
{
    delete fCmapResLoader;
}

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
