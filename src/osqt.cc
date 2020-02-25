// This is copyrighted software. More information is at the end of this file.

/* Qt-specific Tads OS functions.
 *
 * This file should only contain Tads OS specific functions.  That doesn't mean
 * that you can't use C++ code inside the functions; you can use any C++
 * feature you want, as long as the function headers are compatible with the
 * prototypes in "osifc.h".  The only exception are static helper functions
 * that are clearly marked as such.
 */

// Make sure we get vasprintf() from cstdio, which in mingw is a GNU extension.
#if defined(__MINGW32__) and not defined(_GNU_SOURCE)
    #define _GNU_SOURCE
    #define GNU_SOURCE_DEFINED
#endif
#include <cstdio>
#ifdef GNU_SOURCE_DEFINED
    #undef _GNU_SOURCE
    #undef GNU_SOURCE_DEFINED
#endif

#include "globals.h"
#include "os.h"
#include "osifcext.h"
#include "settings.h"
#include "sysframe.h"
#include "syswininput.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <random>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #include <QRandomGenerator>
#endif

/* Translates a local filename to a unicode QString.
 */
auto fnameToQStr(const char* const fname) -> QString
{
    Q_ASSERT(qFrame != nullptr);

    return qFrame->tads3() ? QString::fromUtf8(fname) : QFile::decodeName(fname);
}

/* Translates a unicode QString filename to a local filename.
 */
auto qStrToFname(const QString& fnameStr) -> QByteArray
{
    return qFrame->tads3() ? fnameStr.toUtf8() : QFile::encodeName(fnameStr);
}

/* --------------------------------------------------------------------
 * Basic file I/O interface.
 */

static auto createQFile(const char* const fname, const QFile::OpenMode mode) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    auto* const file = new QFile(fnameToQStr(fname));
    if (not file->open(mode)) {
        delete file;
        return nullptr;
    }
    return file;
}

/* Open text file for reading.
 */
auto osfoprt(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadOnly | QFile::Text);
}

/* Open text file for writing.
 */
auto osfopwt(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::WriteOnly | QFile::Truncate | QFile::Text);
}

/* Open text file for reading and writing, keeping existing contents.
 */
auto osfoprwt(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadWrite | QFile::Text);
}

/* Open text file for reading and writing, truncating existing contents.
 */
auto osfoprwtt(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadWrite | QFile::Truncate | QFile::Text);
}

/* Open binary file for writing.
 */
auto osfopwb(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::WriteOnly | QFile::Truncate);
}

/* Open binary file for reading.
 */
auto osfoprb(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadOnly);
}

/* Open binary file for reading and writing, keeping any existing contents.
 */
auto osfoprwb(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadWrite);
}

/* Open binary file for reading and writing, truncating existing contents.
 */
auto osfoprwtb(const char* const fname, const os_filetype_t /*type*/) -> osfildef*
{
    Q_ASSERT(fname != nullptr);

    return createQFile(fname, QFile::ReadWrite | QFile::Truncate);
}

/* Duplicate a file handle.
 */
auto osfdup(osfildef* const orig, const char* const mode) -> osfildef*
{
    Q_ASSERT(orig != nullptr);
    Q_ASSERT(mode != nullptr);
    Q_ASSERT(mode[0] != '\0');

    QFile::OpenMode qMode = {};
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
        return nullptr;
    }

    if (mode[i] == 't' or mode[i] == 's' or mode[i] == '\0') {
        qMode |= QFile::Text;
    } else if (mode[i] == 'b') {
        // Binary mode is the default; there's no explicit flag for it.
    } else {
        return nullptr;
    }

    auto* const file = new QFile;
    if (not file->open(orig->handle(), qMode)) {
        delete file;
        return nullptr;
    }
    return file;
}

/* Get a line of text from a text file.
 */
auto osfgets(char* const buf, const size_t len, osfildef* const fp) -> char*
{
    Q_ASSERT(buf != nullptr);
    Q_ASSERT(fp != nullptr);

    return fp->readLine(buf, len) == static_cast<qint64>(len) ? buf : nullptr;
}

/* Write a line of text to a text file.
 */
auto osfputs(const char* const buf, osfildef* const fp) -> int
{
    Q_ASSERT(buf != nullptr);
    Q_ASSERT(fp != nullptr);

    return fp->write(buf, qstrlen(buf)) > 0 ? 1 : EOF;
}

/* Write a null-terminated string to a text file.
 */
void os_fprintz(osfildef* const fp, const char* const str)
{
    Q_ASSERT(fp != nullptr);
    Q_ASSERT(str != nullptr);

    fp->write(str, qstrlen(str));
}

/* Print a counted-length string (which might not be null-terminated)
 * to a file.
 */
void os_fprint(osfildef* const fp, const char* const str, const size_t len)
{
    Q_ASSERT(fp != nullptr);
    Q_ASSERT(str != nullptr);

    fp->write(str, len);
}

/* Write bytes to file.
 */
auto osfwb(osfildef* const fp, const void* const buf, const int bufl) -> int
{
    Q_ASSERT(fp != nullptr);
    Q_ASSERT(buf != nullptr);

    return fp->write(static_cast<const char*>(buf), bufl) != bufl;
}

/* Flush buffered writes to a file.
 */
auto osfflush(osfildef* const fp) -> int
{
    Q_ASSERT(fp != nullptr);

    return fp->flush() ? 0 : EOF;
}

/* Get a character from a file.
 */
auto osfgetc(osfildef* const fp) -> int
{
    Q_ASSERT(fp != nullptr);

    char c;
    return fp->getChar(&c) ? c : EOF;
}

/* Read bytes from file.
 */
auto osfrb(osfildef* const fp, void* const buf, const int bufl) -> int
{
    Q_ASSERT(fp != nullptr);
    Q_ASSERT(buf != nullptr);

    return fp->read(static_cast<char*>(buf), bufl) != bufl;
}

/* Read bytes from file and return the number of bytes read.
 */
auto osfrbc(osfildef* const fp, void* const buf, const size_t bufl) -> size_t
{
    Q_ASSERT(fp != nullptr);
    Q_ASSERT(buf != nullptr);

    const auto bytesRead = fp->read(static_cast<char*>(buf), bufl);
    return bytesRead > 0 ? bytesRead : 0;
}

/* Get the current seek location in the file.
 */
auto osfpos(osfildef* const fp) -> long
{
    Q_ASSERT(fp != nullptr);

    return fp->pos();
}

/* Seek to a location in the file.
 */
auto osfseek(osfildef* const fp, const long pos, const int mode) -> int
{
    Q_ASSERT(fp != nullptr);

    switch (mode) {
    default:
        qWarning() << Q_FUNC_INFO << "unknown seek mode, assuming OSFSK_SET";
        Q_FALLTHROUGH();
    case OSFSK_SET:
        return not fp->seek(pos);
    case OSFSK_CUR:
        return not fp->seek(fp->pos() + pos);
    case OSFSK_END:
        return not fp->seek(fp->size() + pos);
    }
}

/* Close a file.
 */
void osfcls(osfildef* const fp)
{
    Q_ASSERT(fp != nullptr);

    delete fp;
}

/* Delete a file.
 */
auto osfdel(const char* const fname) -> int
{
    Q_ASSERT(fname != nullptr);

    return not QFile::remove(fnameToQStr(fname));
}

/* Rename a file.
 */
auto os_rename_file(const char* const oldname, const char* const newname) -> int
{
    Q_ASSERT(oldname != nullptr);
    Q_ASSERT(newname != nullptr);

    return QFile::rename(fnameToQStr(oldname), fnameToQStr(newname));
}

/* Access a file - determine if the file exists.
 */
auto osfacc(const char* const fname) -> int
{
    Q_ASSERT(fname != nullptr);

    const QFileInfo info(fnameToQStr(fname));
    // Since exists() returns false for dangling symlinks, we need
    // to explicitly check whether it's a symlink or not.
    return not info.exists() and not info.isSymLink();
}

// On Windows, we need to enable NTFS permission lookups.
#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

auto os_file_stat(const char* const fname, const int follow_links, os_file_stat_t* const s) -> int
{
    Q_ASSERT(fname != nullptr);
    Q_ASSERT(s != nullptr);

    const auto fnameStr = fnameToQStr(fname);

#ifdef Q_OS_WIN
    struct __stat64 info;

    // Get a UTF-16 version of the filename and NULL-terminate it.
    wchar_t* wFname = new wchar_t[fnameStr.length() + 1];
    wFname[fnameStr.toWCharArray(wFname)] = L'\0';

    // Get the file information.
    const int statRet = _wstat64(wFname, &info);
    delete[] wFname;
    if (statRet != 0)
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
    const auto statRet = follow_links ? stat(QFile::encodeName(fnameStr).constData(), &buf)
                                      : lstat(QFile::encodeName(fnameStr).constData(), &buf);
    if (statRet != 0) {
        return false;
    }

    s->sizelo = (uint32_t)(buf.st_size & 0xFFFFFFFF);
    s->sizehi = sizeof(buf.st_size) > 4 ? (uint32_t)((buf.st_size >> 32) & 0xFFFFFFFF) : 0;
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
    QFileInfo inf(fnameStr);
    auto isLink = inf.isSymLink();
#ifdef Q_OS_WIN
    // Don't treat shortcut files as symlinks.
    if (isLink and (QString::compare(inf.suffix(), "lnk", Qt::CaseInsensitive) == 0)) {
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

#ifdef Q_OS_WIN
    // Enable NTFS permissions.
    ++qt_ntfs_permission_lookup;
#endif
    if (inf.isReadable()) {
        s->attrs |= OSFATTR_READ;
    }
    if (inf.isWritable()) {
        s->attrs |= OSFATTR_WRITE;
    }
#ifdef Q_OS_WIN
    // Disable NTFS permissions.
    --qt_ntfs_permission_lookup;
#endif
    return true;
}

/* Manually resolve a symbolic link.
 */
auto os_resolve_symlink(const char* const fname, char* const target, const size_t target_size)
    -> int
{
    Q_ASSERT(fname != nullptr);
    Q_ASSERT(target != nullptr);

    const auto resolved = qStrToFname(QFileInfo(fnameToQStr(fname)).symLinkTarget());
    if (resolved.isEmpty() or resolved.size() >= static_cast<int>(target_size)) {
        return false;
    }
    std::memcpy(target, resolved.constData(), resolved.size() + 1);
    return true;
}

/* Get a list of root directories.
 */
auto os_get_root_dirs(char* const buf, const size_t buflen) -> size_t
{
    const auto rootList = QDir::drives();
    // Paranoia.
    if (rootList.size() == 0) {
        return 0;
    }

    QByteArray str;
    for (const auto& i : rootList) {
        str += i.path().toLatin1();
        // Every path needs to be NULL-terminated.
        str += '\0';
    }

    // The whole result must end with two NULL bytes.
    str += '\0';

    if (buf != nullptr and static_cast<int>(buflen) >= str.size()) {
        std::memcpy(buf, str.constData(), str.size());
    }
    return str.size();
}

auto os_open_dir(const char* const dirname, osdirhdl_t* const handle) -> int
{
    Q_ASSERT(dirname != nullptr);
    Q_ASSERT(handle != nullptr);

    auto* const it = new QDirIterator(
        fnameToQStr(dirname), QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System);
    if (it->next().isEmpty()) {
        // We can't read anything.  Don't know why, don't care.
        delete it;
        return false;
    }
    *handle = it;
    return true;
}

auto os_read_dir(const osdirhdl_t handle, char* const fname, const size_t fname_size) -> int
{
    Q_ASSERT(handle != nullptr);
    Q_ASSERT(fname != nullptr);

    const auto str = qStrToFname(handle->fileName());
    if (str.isEmpty() or str.size() >= static_cast<int>(fname_size)) {
        return false;
    }
    std::memcpy(fname, str.constData(), str.size() + 1);
    handle->next();
    return true;
}

void os_close_dir(const osdirhdl_t handle)
{
    Q_ASSERT(handle != nullptr);

    delete handle;
}

/* Get a file's mode/type.  This returns the same information as
 * the 'mode' member of os_file_stat_t from os_file_stat(), so we
 * simply call that routine and return the value.
 */
auto osfmode(
    const char* const fname, const int follow_links, unsigned long* const mode,
    unsigned long* const attr) -> int
{
    Q_ASSERT(fname != nullptr);

    os_file_stat_t s;
    const auto ok = os_file_stat(fname, follow_links, &s);
    if (ok) {
        if (mode != nullptr) {
            *mode = s.mode;
        }
        if (attr != nullptr) {
            *attr = s.attrs;
        }
    }
    return ok;
}

/* Determine if the given filename refers to a special file.
 */
auto os_is_special_file(const char* const fname) -> os_specfile_t
{
    Q_ASSERT(fname != nullptr);

    // We also check for "./" and "../" instead of just "." and
    // "..".  (We use OSPATHCHAR instead of '/' though.)
    constexpr char selfWithSep[] = {'.', OSPATHCHAR, '\0'};
    constexpr char parentWithSep[] = {'.', '.', OSPATHCHAR, '\0'};
    if ((strcmp(fname, ".") == 0) or (strcmp(fname, selfWithSep) == 0))
        return OS_SPECFILE_SELF;
    if ((strcmp(fname, "..") == 0) or (strcmp(fname, parentWithSep) == 0))
        return OS_SPECFILE_PARENT;
    return OS_SPECFILE_NONE;
}

// --------------------------------------------------------------------

/* Convert string to all-lowercase.
 */
auto os_strlwr(char* const s) -> char*
{
    Q_ASSERT(s != nullptr);

    // There's not much we can do here about the possible buffer overflow. We just hope the caller
    // has made the 's' buffer large enough to hold a string that's possibly longer after converting
    // to lower case (can happen in some languages.) There's no way for us to check.
    QByteArray lower;
    if (qFrame->tads3()) {
        lower = QString::fromUtf8(s).toLower().toUtf8();
    } else {
        const auto* const codec = QTextCodec::codecForName(qFrame->settings().tads2Encoding);
        lower = codec->fromUnicode(codec->toUnicode(s).toLower());
    }
    std::memcpy(s, lower.constData(), lower.size() + 1);
    return s;
}

/* --------------------------------------------------------------------
 * Special file and directory locations.
 */

/* Seek to the resource file embedded in the current executable file.
 *
 * We don't support this (and probably never will.)
 */
auto os_exeseek(const char* const /*argv0*/, const char* const /*type*/) -> osfildef*
{
    return nullptr;
}

/* Get the full filename (including directory path) to the executable
 * file.
 */
auto os_get_exe_filename(char* const buf, const size_t buflen, const char* const /*argv0*/) -> int
{
    Q_ASSERT(buf != nullptr);

    QFileInfo inf(QApplication::applicationFilePath());

    if (not inf.exists() or not inf.isReadable()) {
        return false;
    }

    const auto encodedFilename = qStrToFname(inf.filePath());
    if (encodedFilename.size() >= static_cast<int>(buflen)) {
        return false;
    }
    std::memcpy(buf, encodedFilename.constData(), encodedFilename.size() + 1);
    return true;
}

/* Get a special directory path.
 */
void os_get_special_path(
    char* const buf, const size_t buflen, const char* const /*argv0*/, const int id)
{
    Q_ASSERT(buf != nullptr);
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
        const auto dirStr = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QDir dir(dirStr);
        QByteArray result;
        // Create the directory if it doesn't exist.
        if (not dir.exists() and not dir.mkpath(dirStr)) {
            // TODO: Error dialog.
            qWarning().nospace() << "Could not create directory path " << dirStr << ", will use "
                                 << QDir::tempPath() << " instead";
            result = qStrToFname(QDir::tempPath());
        } else {
            result = qStrToFname(dirStr);
        }
        if (result.size() < static_cast<int>(buflen)) {
            std::memcpy(buf, result.constData(), result.size() + 1);
        } else {
            qWarning() << Q_FUNC_INFO
                       << "Result would overflow output buffer, returning empty path instead";
            buf[0] = '\0';
        }
        return;
    }

    default:
        // We didn't recognize the specified id. That means the base code
        // added a new value for it that we don't know about.
        // TODO: Error dialog.
        qWarning("Unknown id in os_get_special_path(), will use empty path");
        buf[0] = '\0';
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
auto os_locate(
    const char* const fname, const int /*flen*/, const char* const /*arg0*/, char* const buf,
    const size_t bufsiz) -> int
{
    // qDebug() << Q_FUNC_INFO << "\n Looking for:" << fname;
    Q_ASSERT(fname != nullptr);
    Q_ASSERT(buf != nullptr);

    const QFileInfo& fileInfo = QFileInfo(fnameToQStr(fname));
    const QByteArray& result = qStrToFname(fileInfo.absoluteFilePath());
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
auto os_create_tempfile(const char* const fname, char* const buf) -> osfildef*
{
    if (fname != nullptr and fname[0] != '\0') {
        // A filename has been specified; use it.
        return createQFile(fname, QFile::ReadWrite | QFile::Truncate);
    }

    Q_ASSERT(buf != nullptr);

    // No filename was given; create a temporary file.
    buf[0] = '\0';
    auto* const file = new QTemporaryFile(QDir::tempPath() + "/qtads_XXXXXX");
    if (not file->open()) {
        delete file;
        return nullptr;
    }
    return file;
}

/* Delete a temporary file created with os_create_tempfile().
 */
auto osfdel_temp(const char* fname) -> int
{
    Q_ASSERT(fname != nullptr);

    if (fname[0] != '\0') {
        return not QFile::remove(fnameToQStr(fname));
    }
    return 0;
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
auto os_gen_temp_filename(char* const buf, const size_t buflen) -> int
{
    Q_ASSERT(buf != nullptr);

    QTemporaryFile tmpfile(QDir::tempPath() + "/qtads_XXXXXX");
    if (not tmpfile.open()) {
        return false;
    }

    // Don't automatically delete the file from disk. This is safer,
    // since another process could create a file with the same name
    // before our caller gets the chance to re-create the file.
    tmpfile.setAutoRemove(false);

    const auto filename = qStrToFname(tmpfile.fileName());
    if (filename.size() >= static_cast<int>(buflen)) {
        return false;
    }
    std::memcpy(buf, filename.constData(), filename.size() + 1);
    return true;
}

/* --------------------------------------------------------------------
 * Basic directory/folder management routines.
 */

/* Create a directory.
 */
auto os_mkdir(const char* const dir, const int create_parents) -> int
{
    Q_ASSERT(dir != nullptr);

    return create_parents ? QDir().mkpath(fnameToQStr(dir)) : QDir().mkdir(fnameToQStr(dir));
}

/* Remove a directory.
 */
auto os_rmdir(const char* const dir) -> int
{
    Q_ASSERT(dir != nullptr);

    return QDir().rmdir(fnameToQStr(dir));
}

/* --------------------------------------------------------------------
 * Filename manipulation routines.
 */

/* Apply a default extension to a filename, if it doesn't already have one.
 */
void os_defext(char* const fn, const char* const ext)
{
    Q_ASSERT(fn != nullptr);
    Q_ASSERT(ext != nullptr);

    if (QFileInfo(fnameToQStr(fn)).suffix().isEmpty()) {
        os_addext(fn, ext);
    }
}

/* Unconditionally add an extention to a filename.
 */
void os_addext(char* const fn, const char* const ext)
{
    Q_ASSERT(fn != nullptr);
    Q_ASSERT(ext != nullptr);

    std::strcat(fn, ".");
    std::strcat(fn, ext);
}

/* Remove the extension from a filename.
 */
void os_remext(char* const fn)
{
    Q_ASSERT(fn != nullptr);

    QFileInfo info(fnameToQStr(fn));
    if (not info.suffix().isEmpty()) {
        fn[qstrlen(fn) - qStrToFname(info.suffix()).size() - 1] = '\0';
    }
}

/* Get a pointer to the root name portion of a filename.
 *
 * Note that Qt's native path separator character is '/'.  It doesn't matter on
 * what OS we're running.
 *
 * FIXME: This only works for UTF-8 (T3 only).
 */
auto os_get_root_name(const char* const buf) -> char*
{
    Q_ASSERT(buf != nullptr);

    const auto* p = buf;
    for (p += std::strlen(buf) - 1; p > buf and *p != '/'; --p)
        ;
    if (p != buf) {
        ++p;
    }
    return const_cast<char*>(p);
}

/* Build a full path name, given a path and a filename.
 */
void os_build_full_path(
    char* const fullpathbuf, const size_t fullpathbuflen, const char* const path,
    const char* const filename)
{
    Q_ASSERT(fullpathbuf != nullptr);
    Q_ASSERT(path != nullptr);
    Q_ASSERT(filename != nullptr);

    qstrncpy(
        fullpathbuf,
        qStrToFname(QFileInfo(QDir(fnameToQStr(path)), fnameToQStr(filename)).filePath()),
        fullpathbuflen);
}

void os_combine_paths(
    char* const fullpathbuf, const size_t fullpathbuflen, const char* const path,
    const char* const filename)
{
    Q_ASSERT(fullpathbuf != nullptr);
    Q_ASSERT(path != nullptr);
    Q_ASSERT(filename != nullptr);

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
void os_get_path_name(char* const pathbuf, const size_t pathbuflen, const char* const fname)
{
    Q_ASSERT(pathbuf != nullptr);
    Q_ASSERT(fname != nullptr);

    qstrncpy(pathbuf, qStrToFname(QFileInfo(fnameToQStr(fname)).path()).constData(), pathbuflen);
}

/* Convert a relative URL into a relative filename path.
 */
void os_cvt_url_dir(char* const result_buf, const size_t result_buf_size, const char* const src_url)
{
    Q_ASSERT(result_buf != nullptr);
    Q_ASSERT(src_url != nullptr);

    const auto result = fnameToQStr(src_url);
    qstrncpy(result_buf, qStrToFname(result).constData(), result_buf_size);
}

/* Determine whether a filename specifies an absolute or relative path.
 */
auto os_is_file_absolute(const char* const fname) -> int
{
    Q_ASSERT(fname != nullptr);

    return QFileInfo(fnameToQStr(fname)).isAbsolute();
}

/* Get the absolute, fully qualified filename for a file.
 */
auto os_get_abs_filename(
    char* const result_buf, const size_t result_buf_size, const char* const filename) -> int
{
    Q_ASSERT(result_buf != nullptr);
    Q_ASSERT(filename != nullptr);

    const auto data = qStrToFname(QFileInfo(fnameToQStr(filename)).absoluteFilePath());
    qstrncpy(result_buf, data.constData(), result_buf_size);
    if (data.length() >= static_cast<int>(result_buf_size)) {
        // Result didn't fit in 'result_buf'.
        return false;
    }
    return true;
}

// This is only used by the HTML TADS debugger, which we don't include
// in our build.
#if 0
int os_get_rel_path(
    char* const result_buf, const size_t result_buf_size, const char* const basepath,
    const char* const filename)
{
    Q_ASSERT(result_buf != nullptr);
    Q_ASSERT(basepath != nullptr);
    Q_ASSERT(filename != nullptr);

    QDir baseDir(fnameToQStr(basepath));
    const auto result = baseDir.relativeFilePath(fnameToQStr(filename));
    if (result.isEmpty()) {
        qstrncpy(result_buf, filename, result_buf_size);
        return false;
    }
    qstrncpy(result_buf, qStrToFname(result), result_buf_size);
    return true;
}
#endif

/* Determine if the given file is in the given directory.
 */
#if 1
/* TODO: We use the version from the DOS/Windows implementation for now.
 * I'll do the Qt-specific implementation later.
 */
static void canonicalize_path(char* const path)
{
    Q_ASSERT(path != nullptr);

    // We canonicalize only the path, in case the file doesn't actually exist.
    // QFileInfo::canonicalFilePath() doesn't work for non-existent files.
    QFileInfo info(fnameToQStr(path));
    QString cleanPath;
    if (info.isDir()) {
        cleanPath = info.filePath();
    } else {
        cleanPath = info.path();
    }

    auto canonPath = qStrToFname(QDir(cleanPath).canonicalPath());
    // Append the filename if we previously stripped it.
    if (not info.isDir()) {
        const auto cleanFilename = fnameToQStr(path);
        int i = cleanFilename.length();
        do {
            --i;
        } while (i > 0 and cleanFilename[i] != '/');
        canonPath += qStrToFname(cleanFilename.mid(i));
    }
    qstrncpy(path, canonPath.constData(), OSFNMAX);
}

auto os_is_file_in_dir(
    const char* filename, const char* path, const int include_subdirs, const int match_self) -> int
{
    Q_ASSERT(filename != nullptr);
    Q_ASSERT(path != nullptr);

    char filename_buf[OSFNMAX], path_buf[OSFNMAX];
    size_t flen, plen;

    /* absolute-ize the filename, if necessary */
    if (!os_is_file_absolute(filename)) {
        os_get_abs_filename(filename_buf, sizeof(filename_buf), filename);
        filename = filename_buf;
    }

    /* absolute-ize the path, if necessary */
    if (!os_is_file_absolute(path)) {
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
    if (plen > 0 && (path[plen - 1] == '\\' || path[plen - 1] == '/'))
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
    if (include_subdirs) {
        /*
         *   filename is in the 'path' directory or one of its
         *   subdirectories, and we're allowed to match on subdirectories, so
         *   we have a match
         */
        return TRUE;
    } else {
        const char* p;

        /*
         *   We're not allowed to match subdirectories, so scan the rest of
         *   the filename for path separators.  If we find any, the file is
         *   in a subdirectory of 'path' rather than directly in 'path'
         *   itself, so it's not a match.  If we don't find any separators,
         *   we have a file directly in 'path', so it's a match.
         */
        for (p = filename + plen + 1; *p != '\0' && *p != '/' && *p != '\\'; ++p)
            ;

        /*
         *   if we reached the end of the string without finding a path
         *   separator character, it's a match
         */
        return (*p == '\0');
    }
}

#else

auto os_is_file_in_dir(
    const char* const filename, const char* const path, const int include_subdirs) -> int
{
    Q_ASSERT(filename != nullptr);
    Q_ASSERT(path != nullptr);
    Q_ASSERT(filename[0] != '\0');
    Q_ASSERT(filename[qstrlen(filename) - 1] != '/');

    QFileInfo fileInf(fnameToQStr(filename));
    const auto pathStr = QFileInfo(fnameToQStr(path)).canonicalFilePath();

    // If the filename is absolute but the file doesn't exist, we know
    // that we're not going to find it anywhere, so report failure.
    if (fileInf.isAbsolute() and not fileInf.exists()) {
        return false;
    }

    const auto fnameStr = fileInf.filePath();

    // If we already found the file in 'path', or we're not searching in
    // subdirectories, report the result now; in both cases, we don't need
    // to recurse subdirectories.
    if ((fnameStr.startsWith(pathStr) and fileInf.exists()) or not include_subdirs) {
        return true;
    }

    // We didn't find the file and need to recurse all subdirectories.
    // Iterate over every subdirectory and look for the file in each one. We
    // only need to iterate directories, not regular files, and we omit the
    // "." and ".." directory entries. We do follow symbolic links; it's OK
    // to do so, since QDirIterator will detect loops.
    QDirIterator it(
        pathStr, QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
        if (fnameStr.startsWith(QDir(it.next()).canonicalPath()) and fileInf.exists()) {
            return true;
        }
    }
    return false;
}
#endif

// --------------------------------------------------------------------

/*
 * We prefer QRandomGenerator if available, since it's guaranteed never to block. Also, it works on
 * mingw without issues. std::random_device is used as a fallback, because it might block (it
 * shouldn't, but might, according to the standard.) We disallow std::random_device with mingw,
 * because it's broken altogether there (https://sourceforge.net/p/mingw-w64/bugs/338).
 */
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0) and defined(__MINGW32__)
    #error MinGW (and MinGW-w64) has a broken std::random_device implementation.
#endif

/* Get a suitable seed for a random number generator.
 */
void os_rand(long* const val)
{
    Q_ASSERT(val != nullptr);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    if (sizeof(long) < 8) {
        *val = static_cast<long>(QRandomGenerator::system()->generate());
    } else {
        *val = static_cast<long>(QRandomGenerator::system()->generate64());
    }
#else
    std::random_device rd;
    std::uniform_int_distribution<long> dis;
    *val = dis(rd);
#endif
}

/* Generate random bytes for use in seeding a PRNG (pseudo-random number generator).
 */
void os_gen_rand_bytes(unsigned char* const buf, const size_t len)
{
    Q_ASSERT(buf != nullptr);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    Q_ASSERT(len % 4 == 0);
    std::vector<quint32> tmp(len / 4);
    QRandomGenerator::system()->fillRange(tmp.data(), tmp.size());
    std::memcpy(buf, tmp.data(), tmp.size() * sizeof(*tmp.data()));
#else
    std::random_device dev;
    std::uniform_int_distribution<unsigned char> dist;
    std::generate_n(buf, len, [&dev, &dist] { return dist(dev); });
#endif
}

/* --------------------------------------------------------------------
 * Allocating sprintf and vsprintf.
 */
// Currently, this is not used anywhere.
// auto os_asprintf(char** bufptr, const char* fmt, ...) -> int
// { }

auto os_vasprintf(char** const bufptr, const char* const fmt, va_list ap) -> int
{
    Q_ASSERT(bufptr != nullptr);
    Q_ASSERT(fmt != nullptr);

    return vasprintf(bufptr, fmt, ap);
}

/* --------------------------------------------------------------------
 */

/* Set busy cursor.
 *
 * This made sense with a 386 back in the day, where loading a T2 game needed
 * some time.  On today's computers this takes milliseconds, so it doesn't make
 * sense to provide a "busy cursor".
 */
void os_csr_busy(const int /*flag*/)
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
auto os_askfile(
    const char* const prompt, char* const fname_buf, const int fname_buf_len, const int prompt_type,
    const os_filetype_t file_type) -> int
{
    Q_ASSERT(prompt != nullptr);
    Q_ASSERT(fname_buf != nullptr);
    Q_ASSERT(prompt_type == OS_AFP_SAVE or prompt_type == OS_AFP_OPEN);

    QString filter;
    QString ext;

    switch (file_type) {
    case OSFTGAME:
        filter = QObject::tr("TADS 2 Games") + " (*.gam *.Gam *.GAM)";
        break;
    case OSFTSAVE:
        filter = QObject::tr("TADS 2 Saved Games") + " (*.sav *.Sav *.SAV)";
        break;
    case OSFTLOG:
        filter = QObject::tr("Game Transcripts") + " (*.txt *.Txt *.TXT)";
        break;
    case OSFTT3IMG:
        Q_ASSERT(qFrame->tads3());
        filter = QObject::tr("TADS 3 Games") + " (*.t3 *.T3)";
        break;
    case OSFTT3SAV:
        Q_ASSERT(qFrame->tads3());
        filter = QObject::tr("TADS 3 Saved Games") + " (*.t3v *.T3v *.T3V)";
        ext = "t3v";
        break;
    }

    // Always provide an "All Files" filter.
    if (not filter.isEmpty()) {
        filter += ";;" + QObject::tr("All Files") + " (*)";
    }

    const auto promptStr = qFrame->tads3()
        ? QString::fromUtf8(prompt)
        : QTextCodec::codecForName(qFrame->settings().tads2Encoding)->toUnicode(prompt);
    const auto filename = prompt_type == OS_AFP_OPEN
        ? QFileDialog::getOpenFileName(qFrame->gameWindow(), promptStr, QDir::currentPath(), filter)
        : QFileDialog::getSaveFileName(
            qFrame->gameWindow(), promptStr, QDir::currentPath(), filter);

    if (filename.isEmpty()) {
        // User cancelled.
        return OS_AFE_CANCEL;
    }

    const auto result = qStrToFname(filename);
    if (result.size() >= fname_buf_len) {
        return OS_AFE_FAILURE;
    }
    std::memcpy(fname_buf, result.constData(), result.size() + 1);
    if (not ext.isEmpty()) {
        // Since `ext' is not empty, an extension should be
        // appended (if none exists already).
        const auto encoded_ext = qStrToFname(ext);
        // +1 to account for the "."
        if (result.size() + encoded_ext.size() + 1 >= fname_buf_len) {
            return OS_AFE_FAILURE;
        }
        os_defext(fname_buf, encoded_ext.constData());
    }
    return OS_AFE_SUCCESS;
}

// --------------------------------------------------------------------

/* Ask for input through a dialog.
 */
auto os_input_dialog(
    const int icon_id, const char* const prompt, const int standard_button_set,
    const char** const buttons, const int button_count, const int default_index,
    const int cancel_index) -> int
{
    Q_ASSERT(prompt != nullptr);
    Q_ASSERT(buttons != nullptr);
    Q_ASSERT(
        icon_id == OS_INDLG_ICON_NONE or icon_id == OS_INDLG_ICON_WARNING
        or icon_id == OS_INDLG_ICON_INFO or icon_id == OS_INDLG_ICON_QUESTION
        or icon_id == OS_INDLG_ICON_ERROR);
    Q_ASSERT(
        standard_button_set == 0 or standard_button_set == OS_INDLG_OK
        or standard_button_set == OS_INDLG_OKCANCEL or standard_button_set == OS_INDLG_YESNO
        or standard_button_set == OS_INDLG_YESNOCANCEL);

    QMessageBox dialog(qWinGroup);

    // We'll use that if we're running a T2 game.
    const auto* const t2Codec = QTextCodec::codecForName(qFrame->settings().tads2Encoding);

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
            buttonList += dialog.addButton(QMessageBox::Ok);
            break;
        case OS_INDLG_OKCANCEL:
            buttonList += dialog.addButton(QMessageBox::Ok);
            buttonList += dialog.addButton(QMessageBox::Cancel);
            break;
        case OS_INDLG_YESNO:
            buttonList += dialog.addButton(QMessageBox::Yes);
            buttonList += dialog.addButton(QMessageBox::No);
            break;
        case OS_INDLG_YESNOCANCEL:
            buttonList += dialog.addButton(QMessageBox::Yes);
            buttonList += dialog.addButton(QMessageBox::No);
            buttonList += dialog.addButton(QMessageBox::Cancel);
            break;
        default:
            qWarning("os_input_dialog: unrecognized button set");
        }
    } else {
        for (int i = 0; i < button_count; ++i) {
            Q_ASSERT(buttons[i] != nullptr);
            const auto buttonText =
                qFrame->tads3() ? QString::fromUtf8(buttons[i]) : t2Codec->toUnicode(buttons[i]);
            buttonList += dialog.addButton(buttonText, QMessageBox::AcceptRole);
        }
    }

    if (default_index != 0) {
        dialog.setDefaultButton(buttonList[default_index - 1]);
    }
    if (cancel_index != 0) {
        dialog.setEscapeButton(buttonList[default_index - 1]);
    }
    // We append a space to the window title to avoid the "<2>" that would
    // otherwise be appended automatically by some window managers (like KDE.)
    dialog.setWindowTitle(qWinGroup->windowTitle() + ' ');
    dialog.exec();
    auto* const result = dialog.clickedButton();
    if (result == nullptr) {
        return cancel_index;
    }
    return buttonList.indexOf(static_cast<QPushButton*>(result)) + 1;
}

/* --------------------------------------------------------------------
 * Time-functions.
 */

/* Higher-precision time (nanosecond precision).
 */
void os_time_ns(os_time_t* const seconds, long* const nanoseconds)
{
    Q_ASSERT(seconds != nullptr);
    Q_ASSERT(nanoseconds != nullptr);

    namespace cr = std::chrono;

    // C++ does not guarantee that the system clock's epoch is the same as
    // the Unix Epoch, but it's the de-facto standard in all compilers.
    const auto now = cr::system_clock::now().time_since_epoch();
    *seconds = cr::duration_cast<cr::seconds>(now).count();
    *nanoseconds = cr::duration_cast<cr::nanoseconds>(now - cr::seconds(*seconds)).count();
}

/* Get the current system high-precision timer.
 */
auto os_get_sys_clock_ms() -> long
{
    return os_get_time();
}

/* Sleep for a while.
 */
void os_sleep_ms(const long ms)
{
    if (not qFrame->gameRunning() or ms < 1) {
        return;
    }

    QEventLoop idleLoop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &idleLoop, &QEventLoop::quit);
    QObject::connect(qFrame, &CHtmlSysFrameQt::gameQuitting, &idleLoop, &QEventLoop::quit);
    timer.start(ms);
    idleLoop.exec();
}

/* Set a file's type information.
 *
 * TODO: Find out if this can be empty on all systems Qt supports.
 */
void os_settype(const char* const /*file*/, const os_filetype_t /*type*/)
{ }

/* --------------------------------------------------------------------
 */

/* Get filename from startup parameter, if possible.
 *
 * TODO: Find out what this is supposed to do.
 */
auto os_paramfile(char* const /*buf*/) -> int
{
    return false;
}

/* Terminate the program and exit with the given exit status.
 */
void os_term(const int /*status*/)
{
    qDebug() << Q_FUNC_INFO;
}

/* Initialize the time zone.
 *
 * TODO: Find out if this can be empty on all systems Qt supports.
 */
void os_tzset()
{ }

/* Set the default saved-game extension.
 *
 * We don't need to implement this since this routine is intended to be
 * invoked only if the interpreter is running as a stand-alone game,
 * and this isn't possible in QTads.
 */
void os_set_save_ext(const char* const)
{ }

/* --------------------------------------------------------------------
 */

/* Generate the name of the character set mapping table for Unicode
 * characters to and from the given local character set.
 *
 * We use UTF-8 for everything, which should work on all platforms.
 */
void os_get_charmap(char* const mapname, const int charmap_id)
{
    // qDebug() << Q_FUNC_INFO;

    Q_ASSERT(qFrame->tads3());
    Q_ASSERT(mapname != nullptr);

    switch (charmap_id) {
    default:
        qWarning("os_get_charmap() got an unknown charmap id");
        Q_FALLTHROUGH();
    case OS_CHARMAP_DISPLAY:
    case OS_CHARMAP_FILENAME:
    case OS_CHARMAP_FILECONTENTS:
        strcpy(mapname, "utf-8");
        break;
    }
}

/* Generate a filename for a character-set mapping file.
 */
void os_gen_charmap_filename(char* const filename, char* const internal_id, char* const /*argv0*/)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(filename != nullptr);
    Q_ASSERT(internal_id != nullptr);

    qstrncpy(filename, qStrToFname(QString::fromLatin1(internal_id) + ".tcp").constData(), OSFNMAX);
}

/* Receive notification that a character mapping file has been loaded.
 *
 * We simply switch the codec that QString uses to convert to and from
 * char* and QCString.
 */
void os_advise_load_charmap(char* const /*id*/, char* const /*ldesc*/, char* const /*sysinfo*/)
{
    qDebug() << Q_FUNC_INFO;
    // QTextCodec::setCodecForCStrings(QTextCodec::codecForName(sysinfo));
}

/* --------------------------------------------------------------------
 */

/* Get system information.
 */
auto os_get_sysinfo(const int code, void* const /*param*/, long* const result) -> int
{
    Q_ASSERT(result != nullptr);

    switch (code) {
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
#ifndef NO_AUDIO
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
        *result = qFrame->settings().enableGraphics;
        break;

    case SYSINFO_PREF_SOUNDS:
        *result = qFrame->settings().enableSoundEffects;
        break;

    case SYSINFO_PREF_MUSIC:
        *result = qFrame->settings().enableMusic;
        break;

    case SYSINFO_PREF_LINKS:
        *result = qFrame->settings().enableLinks;
        break;

    case SYSINFO_TEXT_COLORS:
        *result = SYSINFO_TXC_RGB;
        break;

    case SYSINFO_INTERP_CLASS:
        *result = SYSINFO_ICLASS_HTML;
        break;

    default:
        qWarning("Game specified an unknown os_get_sysinfo() code.");
        return false;
    }
    return true;
}

/* --------------------------------------------------------------------
 */

/* Open a popup menu window.
 */
// FIXME: Just a dummy implementation for now.
auto os_show_popup_menu(
    const int /*default_pos*/, const int /*x*/, const int /*y*/, const char* const /*txt*/,
    const size_t /*txtlen*/, os_event_info_t* const /*evt*/) -> int
{
    if (qFrame->gameRunning()) {
        return OSPOP_FAIL;
    }
    return OSPOP_EOF;
}

/* Enable/disable a System Menu Command event in os_get_event().
 */
// FIXME: Just a dummy implementation for now.
void os_enable_cmd_event(const int /*id*/, const unsigned int /*status*/)
{ }

void os_init_ui_after_load(
    class CVmBifTable* const /*bif_table*/, class CVmMetaTable* const /*meta_table*/)
{ }

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
