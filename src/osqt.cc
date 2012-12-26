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
 * This file should only contain Tads OS specific functions.  That doesn't mean
 * that you can't use C++ code inside the functions; you can use any C++
 * feature you want, as long as the function headers are compatible with the
 * prototypes in "osifc.h".  The only exception are static helper functions
 * that are clearly marked as such.
 */

#include <QApplication>
#if QT_VERSION < 0x050000
    #include <QDesktopServices>
#else
    #include <QStandardPaths>
#endif
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryFile>
#include <QDirIterator>
#include <QDebug>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "os.h"
#include "osifcext.h"
#include "globals.h"
#include "settings.h"
#include "sysframe.h"
#include "syswininput.h"


/* --------------------------------------------------------------------
 * Basic file I/O interface.
 */

static osfildef*
createQFile( const char* fname, QFile::OpenMode mode )
{
    Q_ASSERT(fname != 0);

    QFile* file = new QFile(QString::fromUtf8(fname));
    if (not file->open(mode)) {
        delete file;
        return 0;
    }
    return file;
}


/* Open text file for reading.
 */
osfildef*
osfoprt( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::ReadOnly | QFile::Text);
}


/* Open text file for writing.
 */
osfildef*
osfopwt( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::WriteOnly | QFile::Truncate | QFile::Text);
}


/* Open text file for reading and writing, keeping existing contents.
 */
osfildef*
osfoprwt( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::ReadWrite | QFile::Text);
}


/* Open text file for reading and writing, truncating existing contents.
 */
osfildef*
osfoprwtt( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::ReadWrite | QFile::Truncate | QFile::Text);
}


/* Open binary file for writing.
 */
osfildef*
osfopwb( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::WriteOnly | QFile::Truncate);
}


/* Open binary file for reading.
 */
osfildef*
osfoprb( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::ReadOnly);
}


/* Open binary file for reading and writing, keeping any existing contents.
 */
osfildef*
osfoprwb( const char* fname, os_filetype_t filetype )
{
    return createQFile(fname, QFile::ReadWrite);
}


/* Open binary file for reading and writing, truncating existing contents.
 */
osfildef*
osfoprwtb( const char* fname, os_filetype_t )
{
    return createQFile(fname, QFile::ReadWrite | QFile::Truncate);
}


/* Duplicate a file handle.
 */
osfildef*
osfdup( osfildef* orig, const char* mode )
{
    Q_ASSERT(orig != 0);
    Q_ASSERT(mode != 0);
    Q_ASSERT(mode[0] != '\0');

    QFile::OpenMode qMode = 0;
    size_t i = 0;

    if (mode[i] == 'r') {
        ++i;
        if (mode[i] == '+') {
            qMode = QFile::ReadWrite;
            ++i;
        } else {
            qMode = QFile::ReadOnly;
        }
    } else if (mode[0] == 'w') {
        ++i;
        qMode = QFile::WriteOnly;
    } else {
        return 0;
    }

    if (mode[i] == 't' or mode[i] == 's' or mode[i] == '\0') {
        qMode |= QFile::Text;
    } else if (mode[i] == 'b') {
        // Binary mode is the default; there's no explicit flag for it.
    } else {
        return 0;
    }

    QFile* file = new QFile(0);
    if (not file->open(orig->handle(), qMode)) {
        delete file;
        return 0;
    }
    return file;
}


/* Get a line of text from a text file.
 */
char*
osfgets( char* buf, size_t len, osfildef* fp )
{
    Q_ASSERT(buf != 0);
    Q_ASSERT(fp != 0);

    if (fp->readLine(buf, len) != len) {
        return 0;
    }
    return buf;
}


/* Write a line of text to a text file.
 */
int
osfputs( const char* buf, osfildef* fp )
{
    if (fp->write(buf, qstrlen(buf)) < 1)
        return EOF;
    return 1;
}


/* Write a null-terminated string to a text file.
 */
void
os_fprintz( osfildef* fp, const char* str )
{
    Q_ASSERT(fp != 0);
    Q_ASSERT(str != 0);

    fp->write(str, qstrlen(str));
}


/* Print a counted-length string (which might not be null-terminated)
 * to a file.
 */
void
os_fprint( osfildef* fp, const char* str, size_t len )
{
    Q_ASSERT(fp != 0);
    Q_ASSERT(str != 0);

    fp->write(str, len);
}


/* Write bytes to file.
 */
int
osfwb( osfildef* fp, const void* buf, int bufl )
{
    Q_ASSERT(fp != 0);
    Q_ASSERT(buf != 0);

    if (fp->write(static_cast<const char*>(buf), bufl) < 1)
        return 1;
    return 0;
}


/* Flush buffered writes to a file.
 */
int
osfflush( osfildef* fp )
{
    Q_ASSERT(fp != 0);

    if (not fp->flush())
        return EOF;
    return 0;
}


/* Get a character from a file.
 */
int
osfgetc( osfildef* fp )
{
    Q_ASSERT(fp != 0);

    char c;
    if (not fp->getChar(&c))
        return EOF;
    return c;
}


/* Read bytes from file.
 */
int
osfrb( osfildef* fp, void* buf, int bufl )
{
    Q_ASSERT(fp != 0);
    Q_ASSERT(buf != 0);

    return fp->read(static_cast<char*>(buf), bufl) != bufl;
}


/* Read bytes from file and return the number of bytes read.
 */
size_t
osfrbc( osfildef* fp, void* buf, size_t bufl )
{
    Q_ASSERT(fp != 0);
    Q_ASSERT(buf != 0);

    int bytesRead = fp->read(static_cast<char*>(buf), bufl);
    if (bytesRead < 0)
        return 0;
    return bytesRead;
}


/* Get the current seek location in the file.
 */
long
osfpos( osfildef* fp )
{
    Q_ASSERT(fp != 0);

    return fp->pos();
}


/* Seek to a location in the file.
 */
int
osfseek( osfildef* fp, long pos, int mode )
{
    Q_ASSERT(fp != 0);

    if (mode == OSFSK_CUR)
        pos += fp->pos();
    else if (mode == OSFSK_END)
        pos += fp->size();
    return not fp->seek(pos);
}


/* Close a file.
 */
void
osfcls( osfildef* fp )
{
    delete fp;
}


/* Delete a file.
 */
int
osfdel( const char* fname )
{
    return not QFile::remove(QString::fromUtf8(fname));
}


/* Rename a file.
 */
int
os_rename_file( const char* oldname, const char* newname )
{
    return QFile::rename(QString::fromUtf8(oldname),
                         QString::fromUtf8(newname));
}


/* Access a file - determine if the file exists.
 */
int
osfacc( const char* fname )
{
    const QFileInfo info(QString::fromUtf8(fname));
    // Since exists() returns false for dangling symlinks, we need
    // to explicitly check whether it's a symlink or not.
    if (not info.exists() and not info.isSymLink()) {
        return 1;
    }
    return 0;
}


// On Windows, we need to enable NTFS permission lookups.
#ifdef Q_OS_WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

int
os_file_stat( const char* fname, int follow_links, os_file_stat_t* s )
{
#ifdef Q_OS_WIN32
    struct __stat64 info;

    // $$$ we should support Windows symlinks and junction points

    /* get the file information */
    if (_stat64(fname, &info))
        return FALSE;

    /* translate the status fields */
    s->sizelo = (uint32_t)(info.st_size & 0xFFFFFFFF);
    s->sizehi = (uint32_t)(info.st_size >> 32);
    s->cre_time = (os_time_t)info.st_ctime;
    s->mod_time = (os_time_t)info.st_mtime;
    s->acc_time = (os_time_t)info.st_atime;
    s->mode = info.st_mode;
#else
    struct stat buf;
    if ((follow_links ? stat(fname, &buf) : lstat(fname, &buf)) != 0)
        return false;

    s->sizelo = (uint32_t)(buf.st_size & 0xFFFFFFFF);
    s->sizehi = sizeof(buf.st_size) > 4
                ? (uint32_t)((buf.st_size >> 32) & 0xFFFFFFFF)
                : 0;
    s->cre_time = buf.st_ctime;
    s->mod_time = buf.st_mtime;
    s->acc_time = buf.st_atime;
    s->mode = buf.st_mode;
#endif
    s->attrs = 0;

    // QFileInfo::exists() cannot be trusted due to it's weird symlink
    // handling.
    if (osfacc(fname) != 0) {
        return false;
    }
    QFileInfo inf(QFile::decodeName(fname));
    bool isLink = inf.isSymLink();
#ifdef Q_OS_WIN32
    // Don't treat shortcut files as symlinks.
    if (isLink and (QString::compare(inf.suffix(), QLatin1String("lnk"), Qt::CaseInsensitive) == 0)) {
        isLink = false;
    }
#endif

    if (follow_links and isLink) {
        if (inf.symLinkTarget().isEmpty()) {
            return false;
        }
        inf.setFile(inf.symLinkTarget());
    }

    if (inf.isHidden()) {
        s->attrs |= OSFATTR_HIDDEN;
    }

#ifdef Q_OS_WIN32
    // Enable NTFS permissions.
    ++qt_ntfs_permission_lookup;
#endif
    if (inf.isReadable()) {
        s->attrs |= OSFATTR_READ;
    }
    if (inf.isWritable()) {
        s->attrs |= OSFATTR_WRITE;
    }
#ifdef Q_OS_WIN32
    // Disable NTFS permissions.
    --qt_ntfs_permission_lookup;
#endif
    return true;
}


/* Manually resolve a symbolic link.
 */
int
os_resolve_symlink( const char* fname, char* target, size_t target_size )
{
    const QByteArray& str = QFile::encodeName(QFileInfo(QFile::decodeName(fname)).symLinkTarget());
    if (str.isEmpty() or str.size() >= target_size) {
        return false;
    }
    qstrcpy(target, str.constData());
    return true;
}


/* Get a list of root directories.
 */
size_t
os_get_root_dirs( char* buf, size_t buflen )
{
    const QFileInfoList& rootList = QDir::drives();
    // Paranoia.
    if (rootList.size() == 0) {
        return 0;
    }

    QByteArray str;
    for (int i = 0; i < rootList.size(); ++i) {
        str += rootList.at(i).path().toLatin1();
        // Every path needs to be NULL-terminated.
        str += '\0';
    }
    // The whole result must end with two NULL bytes.
    str += '\0';

    if (buf != 0 and buflen >= str.size()) {
        memcpy(buf, str.constData(), str.size());
    }
    return str.size();
}


int
os_open_dir( const char* dirname, osdirhdl_t* handle )
{
    QDirIterator* d = new QDirIterator(QFile::decodeName(dirname),
                                       QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System);
    if (d->next().isEmpty()) {
        // We can't read anything.  Don't know why, don't care.
        return false;
    }
    *handle = d;
    return true;
}


int
os_read_dir( osdirhdl_t handle, char* fname, size_t fname_size )
{
    const QByteArray& str = QFile::encodeName(handle->fileName());
    if (str.isEmpty() or str.size() >= fname_size) {
        return false;
    }
    qstrcpy(fname, str.constData());
    handle->next();
    return true;
}


void
os_close_dir( osdirhdl_t handle )
{
    delete handle;
}


/* Get a file's mode/type.  This returns the same information as
 * the 'mode' member of os_file_stat_t from os_file_stat(), so we
 * simply call that routine and return the value.
 */
int
osfmode( const char* fname, int follow_links, unsigned long* mode,
         unsigned long* attr )
{
    os_file_stat_t s;
    int ok;
    if ((ok = os_file_stat(fname, follow_links, &s)) != false) {
        if (mode != NULL)
            *mode = s.mode;
        if (attr != NULL)
            *attr = s.attrs;
    }
    return ok;
}


/* Determine if the given filename refers to a special file.
 */
os_specfile_t
os_is_special_file( const char* fname )
{
    // We also check for "./" and "../" instead of just "." and
    // "..".  (We use OSPATHCHAR instead of '/' though.)
    const char selfWithSep[3] = {'.', OSPATHCHAR, '\0'};
    const char parentWithSep[4] = {'.', '.', OSPATHCHAR, '\0'};
    if ((strcmp(fname, ".") == 0) or (strcmp(fname, selfWithSep) == 0))
        return OS_SPECFILE_SELF;
    if ((strcmp(fname, "..") == 0) or (strcmp(fname, parentWithSep) == 0))
        return OS_SPECFILE_PARENT;
    return OS_SPECFILE_NONE;
}


// --------------------------------------------------------------------

/* Convert string to all-lowercase.
 */
char*
os_strlwr( char* s )
{
    Q_ASSERT(s != 0);
    Q_ASSERT(std::strlen(s) >= std::strlen(QString::fromUtf8(s).toLower().toUtf8()));

    return std::strcpy(s, QString::fromUtf8(s).toLower().toUtf8());
}


/* --------------------------------------------------------------------
 * Special file and directory locations.
 */

/* Seek to the resource file embedded in the current executable file.
 *
 * We don't support this (and probably never will.)
 */
osfildef*
os_exeseek( const char*, const char* )
{
    return 0;
}


/* Get the full filename (including directory path) to the executable
 * file.
 */
int
os_get_exe_filename( char* buf, size_t buflen, const char* )
{
    QFileInfo inf(QApplication::applicationFilePath());

    if (not inf.exists() or not inf.isReadable()) {
        return false;
    }

    const QByteArray& encodedFilename = QFile::encodeName(inf.filePath());
    if (encodedFilename.length() + 1 > static_cast<int>(buflen)) {
        // The result would not fit in the buffer.
        return false;
    }
    qstrcpy(buf, encodedFilename.constData());
    return true;
}


/* Get a special directory path.
 */
void
os_get_special_path( char* buf, size_t buflen, const char* /*argv0*/, int id )
{
    Q_ASSERT(buf != 0);
    Q_ASSERT(buflen > 0);

    switch (id) {
      case OS_GSP_T3_RES:
      case OS_GSP_T3_INC:
      case OS_GSP_T3_LIB:
      case OS_GSP_T3_USER_LIBS:
        // We can safely ignore those. They're needed only by the compiler.
        // OS_GSP_T3_RES is only needed by the base code implementation of
        // charmap.cc (tads3/charmap.cpp) which we don't use.

        // FIXME: We do use tads3/charmap.cpp, so this needs to be handled.
        return;

      case OS_GSP_T3_APP_DATA:
      case OS_GSP_LOGFILE: {
        const QString& dirStr =
        #if QT_VERSION < 0x050000
                QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        #else
                QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        #endif
        QDir dir(dirStr);
        // Create the directory if it doesn't exist.
        if (not dir.exists() and not dir.mkpath(dirStr)) {
            // TODO: Error dialog.
            qWarning() << "Could not create directory path:" << dirStr;
            Q_ASSERT(QFile::encodeName(QDir::tempPath()).size() < static_cast<int>(buflen));
            strncpy(buf, QFile::encodeName(QDir::tempPath()).constData(), buflen);
            return;
        }
        Q_ASSERT(QFile::encodeName(dirStr).size() < static_cast<int>(buflen));
        strncpy(buf, QFile::encodeName(dirStr).constData(), buflen);
        buf[buflen - 1] = '\0';
        break;
      }

      default:
        // We didn't recognize the specified id. That means the base code
        // added a new value for it that we don't know about.
        // TODO: Error dialog.
        qWarning("Unknown id in os_get_special_path()");
    }
}


/* --------------------------------------------------------------------
 */

/* Look for a file in the standard locations: current directory, program
 * directory, PATH.
 *
 * FIXME: We only look in the current directory, whatever that might be.
 */
#if 0
int
os_locate( const char* fname, int /*flen*/, const char* /*arg0*/, char* buf, size_t bufsiz )
{
    //qDebug() << Q_FUNC_INFO << "\n Looking for:" << fname;
    Q_ASSERT(fname != 0);
    Q_ASSERT(buf != 0);

    const QFileInfo& fileInfo = QFileInfo(QFile::decodeName(fname));
    const QByteArray& result = QFile::encodeName(fileInfo.absoluteFilePath());
    if (bufsiz > result.length() and QFile::exists(fileInfo.absoluteFilePath())) {
        strcpy(buf, result.constData());
        return true;
    }
    // Not found or buffer not big enough.
    return false;
}
#endif


/* --------------------------------------------------------------------
 */

/* Create and open a temporary file.
 */
osfildef*
os_create_tempfile( const char* fname, char* buf )
{
    if (fname != 0 and fname[0] != '\0') {
        // A filename has been specified; use it.
        return createQFile(fname, QFile::ReadWrite | QFile::Truncate);
    }

    Q_ASSERT(buf != 0);

    // No filename was given; create a temporary file.
    buf[0] = '\0';
    QTemporaryFile* file = new QTemporaryFile(QString::fromLatin1("/qtads_XXXXXX"));
    if (not file->open()) {
        delete file;
        return 0;
    }
    return file;
}


/* Delete a temporary file created with os_create_tempfile().
 */
int
osfdel_temp( const char* fname )
{
    Q_ASSERT(fname != 0);

    if (fname[0] == '\0' or QFile::remove(QString::fromUtf8(fname))) {
        // If fname was empty, it has been already deleted automatically by
        // osfcls().  If fname was not empty, QFile::remove has taken care of
        // deleting the file.
        return 0;
    }
    // QFile::remove() failed.
    return 1;
}


/* This isn't actually used by the basecode.
 */
#if 0
void
os_get_tmp_path( char* buf )
{
}
#endif


/* Generate a name for a temporary file.
 */
int
os_gen_temp_filename( char* buf, size_t buflen )
{
    QTemporaryFile tmpfile(QDir::tempPath() + QString::fromLatin1("/qtads_XXXXXX"));
    if (not tmpfile.open()) {
        return false;
    }

    // Don't automatically delete the file from disk. This is safer,
    // since another process could create a file with the same name
    // before our caller gets the chance to re-create the file.
    tmpfile.setAutoRemove(false);

    const QByteArray& data = tmpfile.fileName().toUtf8();
    if (data.length() >= buflen) {
        // 'buf' isn't big enough to hold the result, including the
        // terminating '\0'.
        return false;
    }
    qstrcpy(buf, data.constData());
    return true;
}


/* --------------------------------------------------------------------
 * Basic directory/folder management routines.
 */

/* Create a directory.
 */
int
os_mkdir( const char* dir, int create_parents )
{
    if (create_parents) {
        return QDir().mkpath(QFile::decodeName(dir));
    }
    return QDir().mkdir(QFile::decodeName(dir));
}


/* Remove a directory.
 */
int
os_rmdir( const char* dir )
{
    return QDir().rmdir(QFile::decodeName(dir));
}


/* --------------------------------------------------------------------
 * Filename manipulation routines.
 */

/* Apply a default extension to a filename, if it doesn't already have one.
 */
void
os_defext( char* fn, const char* ext )
{
    Q_ASSERT(fn != 0);
    Q_ASSERT(ext != 0);

    if (QFileInfo(QFile::decodeName(fn)).suffix().isEmpty()) {
        os_addext(fn, ext);
    }
}


/* Unconditionally add an extention to a filename.
 *
 * TODO: Find out if there are systems that don't use the dot as the extension
 * separator.  (Only systems supported by Qt of course.)
 */
void
os_addext( char* fn, const char* ext )
{
    Q_ASSERT(fn != 0);
    Q_ASSERT(ext != 0);

    std::strcat(fn, ".");
    std::strcat(fn, ext);
}


/* Remove the extension from a filename.
 */
void
os_remext( char* fn )
{
    Q_ASSERT(fn != 0);

    QFileInfo file(QFile::decodeName(fn));
    if (file.suffix().isEmpty()) {
        return;
    }
    std::strcpy(fn, QFile::encodeName(file.completeBaseName()));
}


/* Get a pointer to the root name portion of a filename.
 *
 * Note that Qt's native path separator character is '/'.  It doesn't matter on
 * what OS we're running.
 */
char*
os_get_root_name( const char* buf )
{
    Q_ASSERT(buf != 0);

    const char* p = buf;
    for (p += std::strlen(buf) - 1; p > buf and *p != '/'; --p)
        ;
    if (p != buf) {
        ++p;
    }
    return const_cast<char*>(p);
}


/* Build a full path name, given a path and a filename.
 */
void
os_build_full_path( char* fullpathbuf, size_t fullpathbuflen, const char* path, const char* filename )
{
    Q_ASSERT(fullpathbuf != 0);
    Q_ASSERT(path != 0);
    Q_ASSERT(filename != 0);

    qstrncpy(fullpathbuf, QFile::encodeName(QFileInfo(QDir(QFile::decodeName(path)),
                                                      QFile::decodeName(filename)).filePath()),
             fullpathbuflen);
}


void
os_combine_paths( char* fullpathbuf, size_t fullpathbuflen, const char* path, const char* filename )
{
    Q_ASSERT(fullpathbuf != 0);
    Q_ASSERT(path != 0);
    Q_ASSERT(filename != 0);

    bool filenameIsDot = false;
    bool filenameIsDotDot = false;
    if (qstrcmp(filename, "..") == 0 or qstrcmp(filename, "../") == 0)
        filenameIsDotDot = true;
    else if (qstrcmp(filename, ".") == 0 or qstrcmp(filename, "./") == 0) {
        filenameIsDot = true;
    }

    if (filenameIsDot or filenameIsDotDot) {
        os_build_full_path(fullpathbuf, fullpathbuflen, path, "");
        if (qstrlen(fullpathbuf) + 3 <= fullpathbuflen) {
            strcat(fullpathbuf, filenameIsDot ? "." : "..");
        }
    } else {
        os_build_full_path(fullpathbuf, fullpathbuflen, path, filename);
    }
}


/* Extract the path from a filename.
 */
void
os_get_path_name( char* pathbuf, size_t pathbuflen, const char* fname )
{
    Q_ASSERT(pathbuf != 0);
    Q_ASSERT(fname != 0);

    qstrncpy(pathbuf, QFile::encodeName(QFileInfo(QFile::decodeName(fname)).path()).constData(),
             pathbuflen);
}


/* Convert a relative URL into a relative filename path.
 */
void
os_cvt_url_dir( char* result_buf, size_t result_buf_size, const char* src_url )
{
    Q_ASSERT(result_buf != 0);
    Q_ASSERT(src_url != 0);

    QString result(QFile::decodeName(src_url));
    qstrncpy(result_buf, QFile::encodeName(result).constData(), result_buf_size);
}


/* Determine whether a filename specifies an absolute or relative path.
 */
int
os_is_file_absolute( const char* fname )
{
    Q_ASSERT(fname != 0);

    return QFileInfo(QFile::decodeName(fname)).isAbsolute();
}


/* Get the absolute, fully qualified filename for a file.
 */
int
os_get_abs_filename( char* result_buf, size_t result_buf_size, const char* filename )
{
    Q_ASSERT(result_buf != 0);

    const QByteArray& data = QFile::encodeName(QFileInfo(QFile::decodeName(filename)).absoluteFilePath());
    qstrncpy(result_buf, data.constData(), result_buf_size);
    if (data.length() >= result_buf_size) {
        // Result didn't fit in 'result_buf'.
        return false;
    }
    return true;
}


// This is only used by the HTML TADS debugger, which we don't include
// in our build.
#if 0
int
os_get_rel_path( char* result_buf, size_t result_buf_size, const char* basepath, const char* filename)
{
    Q_ASSERT(result_buf != 0);
    Q_ASSERT(basepath != 0);
    Q_ASSERT(filename != 0);

    QDir baseDir(QFile::decodeName(basepath));
    const QString& result = baseDir.relativeFilePath(QFile::decodeName(filename));
    if (result.isEmpty()) {
        qstrncpy(result_buf, filename, result_buf_size);
        return false;
    }
    qstrncpy(result_buf, QFile::encodeName(result), result_buf_size);
    return true;
}
#endif


/* Determine if the given file is in the given directory.
 */
#if 1
/* TODO: We use the version from the DOS/Windows implementation for now.
 * I'll do the Qt-specific implementation later.
 */
static void
canonicalize_path( char* path )
{
    // We canonicalize only the path, in case the file doesn't actually exist.
    // QFileInfo::canonicalFilePath() doesn't work for non-existent files.
    QFileInfo info(QFile::decodeName(path));
    QString cleanPath;
    if (info.isDir()) {
        cleanPath = info.filePath();
    } else {
        cleanPath = info.path();
    }

    QByteArray canonPath(QFile::encodeName(QDir(cleanPath).canonicalPath()));
    // Append the filename if we previously stripped it.
    if (not info.isDir()) {
        QString cleanFilename(QFile::decodeName(path));
        int i = cleanFilename.length();
        while (cleanFilename[i] != QChar::fromLatin1('/') and i > 0) {
            --i;
        }
        canonPath.append(QFile::encodeName(cleanFilename.mid(i)));
    }
    qstrncpy(path, canonPath.constData(), OSFNMAX);
}

int
os_is_file_in_dir( const char* filename, const char* path, int include_subdirs,
                   int match_self )
{
    char filename_buf[OSFNMAX], path_buf[OSFNMAX];
    size_t flen, plen;

    /* absolute-ize the filename, if necessary */
    if (!os_is_file_absolute(filename))
    {
        os_get_abs_filename(filename_buf, sizeof(filename_buf), filename);
        filename = filename_buf;
    }

    /* absolute-ize the path, if necessary */
    if (!os_is_file_absolute(path))
    {
        os_get_abs_filename(path_buf, sizeof(path_buf), path);
        path = path_buf;
    }

    /*
     *   canonicalize the paths, to remove .. and . elements - this will make
     *   it possible to directly compare the path strings
     */
    safe_strcpy(filename_buf, sizeof(filename_buf), filename);
    canonicalize_path(filename_buf);
    filename = filename_buf;

    safe_strcpy(path_buf, sizeof(path_buf), path);
    canonicalize_path(path_buf);
    path = path_buf;

    /* get the length of the filename and the length of the path */
    flen = strlen(filename);
    plen = strlen(path);

    /* if the path ends in a separator character, ignore that */
    if (plen > 0 && (path[plen-1] == '\\' || path[plen-1] == '/'))
        --plen;

    /*
     *   Check that the filename has 'path' as its path prefix.  First, check
     *   that the leading substring of the filename matches 'path', ignoring
     *   case.  Note that we need the filename to be at least two characters
     *   longer than the path: it must have a path separator after the path
     *   name, and at least one character for a filename past that.
     */
    if (flen < plen + 2 || memicmp(filename, path, plen) != 0)
        return FALSE;

    /*
     *   Okay, 'path' is the leading substring of 'filename'; next make sure
     *   that this prefix actually ends at a path separator character in the
     *   filename.  (This is necessary so that we don't confuse "c:\a\b.txt"
     *   as matching "c:\abc\d.txt" - if we only matched the "c:\a" prefix,
     *   we'd miss the fact that the file is actually in directory "c:\abc",
     *   not "c:\a".)
     */
    if (filename[plen] != '\\' && filename[plen] != '/')
        return FALSE;

    /*
     *   We're good on the path prefix - we definitely have a file that's
     *   within the 'path' directory or one of its subdirectories.  If we're
     *   allowed to match on subdirectories, we already have our answer
     *   (true).  If we're not allowed to match subdirectories, we still have
     *   one more check, which is that the rest of the filename is free of
     *   path separator charactres.  If it is, we have a file that's directly
     *   in the 'path' directory; otherwise it's in a subdirectory of 'path'
     *   and thus isn't a match.
     */
    if (include_subdirs)
    {
        /*
         *   filename is in the 'path' directory or one of its
         *   subdirectories, and we're allowed to match on subdirectories, so
         *   we have a match
         */
        return TRUE;
    }
    else
    {
        const char *p;

        /*
         *   We're not allowed to match subdirectories, so scan the rest of
         *   the filename for path separators.  If we find any, the file is
         *   in a subdirectory of 'path' rather than directly in 'path'
         *   itself, so it's not a match.  If we don't find any separators,
         *   we have a file directly in 'path', so it's a match.
         */
        for (p = filename + plen + 1 ;
             *p != '\0' && *p != '/' && *p != '\\' ; ++p) ;

        /*
         *   if we reached the end of the string without finding a path
         *   separator character, it's a match
         */
        return (*p == '\0');
    }
}

#else

int
os_is_file_in_dir( const char* filename, const char* path, int include_subdirs )
{
    Q_ASSERT(filename != 0);
    Q_ASSERT(path != 0);
    Q_ASSERT(filename[0] != '\0');
    Q_ASSERT(filename[qstrlen(filename) - 1] != '/');

    QFileInfo fileInf(QFile::decodeName(filename));
    const QString& pathStr = QFileInfo(QFile::decodeName(path)).canonicalFilePath();

    // If the filename is absolute but the file doesn't exist, we know
    // that we're not going to find it anywhere, so report failure.
    if (fileInf.isAbsolute() and not fileInf.exists()) {
        return false;
    }

    const QString& fnameStr = fileInf.filePath();

    // Look in 'path' first, before recursing its subdirectories.
    bool found;
    if (fnameStr.startsWith(pathStr) and fileInf.exists())
        found = true;
    else
        found = false;

    // If we already found the file in 'path', or we're not searching in
    // subdirectories, report the result now; in both cases, we don't need
    // to recurse subdirectories.
    if (found or not include_subdirs) {
        return found;
    }

    // We didn't find the file and need to recurse all subdirectories.
    // Iterate over every subdirectory and look for the file in each one. We
    // only need to iterate directories, not regular files, and we omit the
    // "." and ".." directory entries. We do follow symbolic links; it's OK
    // to do so, since QDirIterator will detect loops.
    QDirIterator it(pathStr, QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext() and not found) {
        if (fnameStr.startsWith(QDir(it.next()).canonicalPath()) and fileInf.exists()) {
            found = true;
        }
    }
    return found;
}
#endif


// --------------------------------------------------------------------

/* Get a suitable seed for a random number generator.
 */
void
os_rand( long* val )
{
    Q_ASSERT(val != 0);

    static bool initialized = false;
    if (not initialized) {
        qsrand(QDateTime::currentDateTime().toTime_t());
        initialized = true;
    }
    *val = qrand();
}


/* Generate random bytes for use in seeding a PRNG (pseudo-random number
 * generator).
 */
void
os_gen_rand_bytes( unsigned char* buf, size_t len )
{
#ifdef Q_OS_UNIX
    QFile rdev(QString::fromLatin1("/dev/urandom"));
    if (rdev.open(QFile::ReadOnly)) {
        qint64 bytesRead = rdev.read(reinterpret_cast<char*>(buf), len);
        if (bytesRead >= len)
            return;
        if (bytesRead > 0) {
            // For whatever reason, we couldn't read the requested amount of
            // bytes from the device. We'll fill the rest with the generic
            // generator below.
            buf += bytesRead;
            len -= bytesRead;
        }
    }
#endif

    static bool initialized = false;
    if (not initialized) {
        qsrand(QDateTime::currentDateTime().toTime_t());
        initialized = true;
    }
    for (size_t i = 0; i < len; ++i) {
        buf[i] = qrand() % 256;
    }
}


/* --------------------------------------------------------------------
 * Allocating sprintf and vsprintf.
 */
// FIXME: Implement this.
//int
//os_asprintf( char** bufptr, const char* fmt, ... )
//{}

// FIXME: Implement this.
//int
//os_vasprintf( char** bufptr, const char* fmt, va_list ap )
//{}


/* --------------------------------------------------------------------
 */

/* Set busy cursor.
 *
 * This made sense with a 386 back in the day, where loading a T2 game needed
 * some time.  On today's computers this takes milliseconds, so it doesn't make
 * sense to provide a "busy cursor".
 */
void
os_csr_busy( int /*flag*/ )
{
    /*
    if (flag) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }
    */
}


/* --------------------------------------------------------------------
 * User Input Routines.
 */

/* Ask the user for a filename, using a system-dependent dialog or
 * other mechanism.
 */
int
os_askfile( const char* prompt, char* fname_buf, int fname_buf_len, int prompt_type, os_filetype_t file_type )
{
    Q_ASSERT(prompt_type == OS_AFP_SAVE or prompt_type == OS_AFP_OPEN);
    Q_ASSERT(prompt != 0);
    Q_ASSERT(fname_buf != 0);

    QString filter;
    QString ext;

    switch (file_type) {
      case OSFTGAME:
        filter = QObject::tr("TADS 2 Games") + QString::fromLatin1(" (*.gam *.Gam *.GAM)");
        break;
      case OSFTSAVE:
        filter = QObject::tr("TADS 2 Saved Games") + QString::fromLatin1(" (*.sav *.Sav *.SAV)");
        break;
      case OSFTLOG:
        filter = QObject::tr("Game Transcripts") + QString::fromLatin1(" (*.txt *.Txt *.TXT)");
        break;
      case OSFTT3IMG:
        Q_ASSERT(qFrame->tads3());
        filter = QObject::tr("TADS 3 Games") + QString::fromLatin1(" (*.t3 *.T3)");
        break;
      case OSFTT3SAV:
        Q_ASSERT(qFrame->tads3());
        filter = QObject::tr("TADS 3 Saved Games") + QString::fromLatin1(" (*.t3v *.T3v *.T3V)");
        ext = QString::fromLatin1("t3v");
        break;
    }

    // Always provide an "All Files" filter.
    if (not filter.isEmpty()) {
        filter.append(QString::fromLatin1(";;"));
        filter.append(QObject::tr("All Files") + QString::fromLatin1(" (*)"));
    }

    QString promptStr;
    if (qFrame->tads3()) {
        promptStr = QString::fromUtf8(prompt);
    } else {
        // TADS 2 does not use UTF-8; use the encoding from our settings for the
        // prompt message.
        QTextCodec* codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);
        promptStr = codec->toUnicode(prompt);
    }

    QString filename;
    if (prompt_type == OS_AFP_OPEN) {
        filename = QFileDialog::getOpenFileName(qFrame->gameWindow(), promptStr, QDir::currentPath(), filter);
    } else {
        filename = QFileDialog::getSaveFileName(qFrame->gameWindow(), promptStr, QDir::currentPath(), filter);
    }

    if (filename.isEmpty()) {
        // User cancelled.
        return OS_AFE_CANCEL;
    }

    const QByteArray& result = QFile::encodeName(filename);
    if (fname_buf_len <= result.length()) {
        return OS_AFE_FAILURE;
    }
    strcpy(fname_buf, result.constData());
    if (not ext.isEmpty()) {
        // Since `ext' is not empty, an extension should be
        // appended (if none exists already).
        os_defext(fname_buf, QFile::encodeName(ext).constData());
        fname_buf[fname_buf_len - 1] = '\0';
    }
    return OS_AFE_SUCCESS;
}


// --------------------------------------------------------------------

/* Ask for input through a dialog.
 */
int
os_input_dialog( int icon_id, const char* prompt, int standard_button_set, const char** buttons,
                 int button_count, int default_index, int cancel_index )
{
    Q_ASSERT(prompt != 0);
    Q_ASSERT(icon_id == OS_INDLG_ICON_NONE or icon_id == OS_INDLG_ICON_WARNING
             or icon_id == OS_INDLG_ICON_INFO or icon_id == OS_INDLG_ICON_QUESTION
             or icon_id == OS_INDLG_ICON_ERROR);
    Q_ASSERT(standard_button_set == 0 or standard_button_set == OS_INDLG_OK
             or standard_button_set == OS_INDLG_OKCANCEL
             or standard_button_set == OS_INDLG_YESNO
             or standard_button_set == OS_INDLG_YESNOCANCEL);

    QMessageBox dialog(qWinGroup);

    // We'll use that if we're running a T2 game.
    QTextCodec* t2Codec = QTextCodec::codecForName(qFrame->settings()->tads2Encoding);

    dialog.setText(qFrame->tads3() ? QString::fromUtf8(prompt) : t2Codec->toUnicode(prompt));

    switch (icon_id) {
      case OS_INDLG_ICON_NONE:
        dialog.setIcon(QMessageBox::NoIcon);
        break;
      case OS_INDLG_ICON_WARNING:
        dialog.setIcon(QMessageBox::Warning);
        break;
      case OS_INDLG_ICON_INFO:
        dialog.setIcon(QMessageBox::Information);
        break;
      case OS_INDLG_ICON_QUESTION:
        dialog.setIcon(QMessageBox::Question);
        break;
      case OS_INDLG_ICON_ERROR:
        dialog.setIcon(QMessageBox::Critical);
        break;
    }

    QList<QPushButton*> buttonList;
    if (standard_button_set != 0) {
        switch (standard_button_set) {
          case OS_INDLG_OK:
            buttonList.append(dialog.addButton(QMessageBox::Ok));
            break;
          case OS_INDLG_OKCANCEL:
            buttonList.append(dialog.addButton(QMessageBox::Ok));
            buttonList.append(dialog.addButton(QMessageBox::Cancel));
            break;
          case OS_INDLG_YESNO:
            buttonList.append(dialog.addButton(QMessageBox::Yes));
            buttonList.append(dialog.addButton(QMessageBox::No));
            break;
          case OS_INDLG_YESNOCANCEL:
            buttonList.append(dialog.addButton(QMessageBox::Yes));
            buttonList.append(dialog.addButton(QMessageBox::No));
            buttonList.append(dialog.addButton(QMessageBox::Cancel));
            break;
          default:
            qWarning("os_input_dialog: unrecognized button set");
        }
    } else for (int i = 0; i < button_count; ++i) {
        Q_ASSERT(buttons[i] != 0);
        const QString& buttonText = qFrame->tads3() ? QString::fromUtf8(buttons[i]) : t2Codec->toUnicode(buttons[i]);
        buttonList.append(dialog.addButton(buttonText, QMessageBox::AcceptRole));
    }

    if (default_index != 0) {
        dialog.setDefaultButton(buttonList[default_index - 1]);
    }
    if (cancel_index != 0) {
        dialog.setEscapeButton(buttonList[default_index - 1]);
    }
    // We append a space to the window title to avoid the "<2>" that would
    // otherwise be appended automatically by some window managers (like KDE.)
    dialog.setWindowTitle(qWinGroup->windowTitle() + QChar::fromLatin1(' '));
    dialog.exec();
    QAbstractButton* result = dialog.clickedButton();
    if (result == 0) {
        return cancel_index;
    }
    return buttonList.indexOf(static_cast<QPushButton*>(result)) + 1;
}


/* --------------------------------------------------------------------
 * Time-functions.
 */

/* Higher-precision time (nanosecond precision).
 */
void
os_time_ns( os_time_t* seconds, long* nanoseconds )
{
    const QDateTime& curTime = QDateTime::currentDateTime();
    *seconds = curTime.toTime_t();
    *nanoseconds = curTime.time().msec() * 1000000;
}


/* Get the current system high-precision timer.
 */
long
os_get_sys_clock_ms( void )
{
    static QTime zeroPoint(QTime::currentTime());
    static long lastRet = -1;
    static unsigned long wraps = 0;

    long ret = zeroPoint.elapsed();

    if (ret < lastRet) {
        // Timer has wrapped to zero.  This only happens when 24 hours have
        // passed since this function has been called for the first time.  It's
        // unlikely that someone will run the interpreter for 24 hours, but
        // still...
        //
        // Note that the code *will* break if we're running for more than
        // 11.767.033 years, 251 days, 20 hours and 24 minutes.  :P
        ++wraps;
    }

    lastRet = ret;
    return ret + (wraps * 86400000L);
}


/* Sleep for a while.
 */
void
os_sleep_ms( long ms )
{
    if (not qFrame->gameRunning() or ms < 1) {
        return;
    }

    QEventLoop idleLoop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, SIGNAL(timeout()), &idleLoop, SLOT(quit()));
    QObject::connect(qFrame, SIGNAL(gameQuitting()), &idleLoop, SLOT(quit()));
    timer.start(ms);
    idleLoop.exec();
}


/* Set a file's type information.
 *
 * TODO: Find out if this can be empty on all systems Qt supports.
 */
void
os_settype( const char*, os_filetype_t )
{
}


/* --------------------------------------------------------------------
 */

/* Get filename from startup parameter, if possible.
 *
 * TODO: Find out what this is supposed to do.
 */
int
os_paramfile( char* /*buf*/ )
{
    return false;
}


/* Terminate the program and exit with the given exit status.
 */
void
os_term( int /*status*/ )
{
    qDebug() << Q_FUNC_INFO;
}


/* Initialize the time zone.
 *
 * TODO: Find out if this can be empty on all systems Qt supports.
 */
void
os_tzset( void )
{
}


/* Set the default saved-game extension.
 *
 * We don't need to implement this since this routine is intended to be
 * invoked only if the interpreter is running as a stand-alone game,
 * and this isn't possible in QTads.
 */
void
os_set_save_ext( const char* )
{
}


/* --------------------------------------------------------------------
 */

/* Generate the name of the character set mapping table for Unicode
 * characters to and from the given local character set.
 *
 * We use UTF-8 for everything, which should work on all platforms.
 */
void
os_get_charmap( char* mapname, int charmap_id )
{
    //qDebug() << Q_FUNC_INFO;

    Q_ASSERT(qFrame->tads3());

    switch(charmap_id) {
      case OS_CHARMAP_DISPLAY:
      case OS_CHARMAP_FILENAME:
      case OS_CHARMAP_FILECONTENTS:
        strcpy(mapname, "utf-8");
        break;
      default:
        qWarning("os_get_charmap() got an unknown charmap id");
        strcpy(mapname, "utf-8");
    }
}


/* Generate a filename for a character-set mapping file.
 */
void
os_gen_charmap_filename( char* filename, char* internal_id, char* /*argv0*/ )
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(filename != 0);

    strncpy(filename, QFile::encodeName(QString::fromLatin1(internal_id)
                              + QString::fromLatin1(".tcp")).constData(), OSFNMAX);
    filename[OSFNMAX - 1] = '\0';
}


/* Receive notification that a character mapping file has been loaded.
 *
 * We simply switch the codec that QString uses to convert to and from
 * char* and QCString.
 */
void
os_advise_load_charmap( char* /*id*/, char* /*ldesc*/, char* /*sysinfo*/ )
{
    qDebug() << Q_FUNC_INFO;
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName(sysinfo));
}


/* --------------------------------------------------------------------
 */

/* Get system information.
 */
int
os_get_sysinfo( int code, void* /*param*/, long* result )
{
    Q_ASSERT(result != 0);

    switch(code)
    {
      case SYSINFO_HTML:
      case SYSINFO_JPEG:
      case SYSINFO_PNG:
      case SYSINFO_LINKS_HTTP:
      case SYSINFO_LINKS_FTP:
      case SYSINFO_LINKS_NEWS:
      case SYSINFO_LINKS_MAILTO:
      case SYSINFO_LINKS_TELNET:
      case SYSINFO_PNG_TRANS:
      case SYSINFO_PNG_ALPHA:
      case SYSINFO_OGG:
      case SYSINFO_MNG:
      case SYSINFO_MNG_TRANS:
      case SYSINFO_MNG_ALPHA:
      case SYSINFO_TEXT_HILITE:
      case SYSINFO_BANNERS:
        *result = 1;
        break;

      case SYSINFO_WAV:
      case SYSINFO_MIDI:
      case SYSINFO_WAV_MIDI_OVL:
      case SYSINFO_WAV_OVL:
      case SYSINFO_MPEG:
      case SYSINFO_MPEG1:
      case SYSINFO_MPEG2:
      case SYSINFO_MPEG3:
#ifndef Q_OS_ANDROID
        *result = 1;
#else
        *result = 0;
#endif
        break;

      case SYSINFO_AUDIO_FADE:
      case SYSINFO_AUDIO_CROSSFADE:
        // We support fades and crossfades for everything except MIDI.
        *result = SYSINFO_AUDIOFADE_MPEG | SYSINFO_AUDIOFADE_OGG | SYSINFO_AUDIOFADE_WAV;
        break;

      case SYSINFO_PREF_IMAGES:
        if (qFrame->settings()->enableGraphics) {
            *result = 1;
        } else {
            *result = 0;
        }
        break;

      case SYSINFO_PREF_SOUNDS:
        if (qFrame->settings()->enableSoundEffects) {
            *result = 1;
        } else {
            *result = 0;
        }
        break;

      case SYSINFO_PREF_MUSIC:
        if (qFrame->settings()->enableMusic) {
            *result = 1;
        } else {
            *result = 0;
        }
        break;

      case SYSINFO_PREF_LINKS:
        if (qFrame->settings()->enableLinks) {
            *result = 1;
        } else {
            *result = 0;
        }
        break;

      case SYSINFO_TEXT_COLORS:
        *result = SYSINFO_TXC_RGB;
        break;

      case SYSINFO_INTERP_CLASS:
        *result = SYSINFO_ICLASS_HTML;
        break;

      default:
        // We didn't recognize the code, which means that this
        // QTads version is too old.
        qWarning("Game specified an unknown os_get_sysinfo() code.");
        return false;
    }
    // We recognized the code.
    return true;
}


/* --------------------------------------------------------------------
 */

/* Open a popup menu window.
 */
// FIXME: Just a dummy implementation for now.
int
os_show_popup_menu( int default_pos, int x, int y, const char* txt, size_t txtlen, union os_event_info_t* evt )
{
    if (qFrame->gameRunning()) {
        return OSPOP_FAIL;
    }
    return OSPOP_EOF;
}


/* Enable/disable a System Menu Command event in os_get_event().
 */
// FIXME: Just a dummy implementation for now.
void
os_enable_cmd_event( int id, unsigned int status )
{
}


void
os_init_ui_after_load( class CVmBifTable* bif_table, class CVmMetaTable* meta_table)
{
}
