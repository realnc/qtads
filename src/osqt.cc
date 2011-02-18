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
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
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
 *
 * There's no need to implement this in a Qt-specific way, since we use only
 * portable functions from the standard C-library.  Qt is only used when it
 * makes things simpler or no Standard C alternative exists.
 *
 * Note that the code doesn't care if the system distinguishes between text and
 * binary files, since (as far as I know) the standard functions always do the
 * right thing; a "b" in the mode string is ignored on systems that treat text
 * and binary files the same (like most/all POSIX-systems).
 */

/* Open text file for reading and writing.
 */
osfildef*
osfoprwt( const char* fname, os_filetype_t /*filetype*/ )
{
	Q_ASSERT(fname != 0);

	// Try opening the file in read/write mode.
	osfildef* fp = std::fopen(fname, "r+");

	// If opening the file failed, it probably means that it doesn't exist.  In
	// that case, create a new file in read/write mode.
	if (fp == 0) fp = std::fopen(fname, "w+");
	return fp;
}


/* Open binary file for reading/writing.
 */
osfildef*
osfoprwb( const char* fname, os_filetype_t filetype )
{
	Q_ASSERT(fname != 0);
	Q_ASSERT(filetype != OSFTLOG);

	osfildef* fp = std::fopen(fname, "r+b");
	if (fp == 0) fp = std::fopen(fname, "w+b");
	return fp;
}


/* Print a counted-length string (which might not be null-terminated)
 * to a file.
 */
void
os_fprint( osfildef* fp, const char* str, size_t len )
{
	Q_ASSERT(fp != 0);
	Q_ASSERT(str != 0);

	std::fprintf(fp, "%.*s", static_cast<unsigned>(len), str);
}


/* Write a null-terminated string to a text file.
 */
void
os_fprintz( osfildef* fp, const char* str )
{
	Q_ASSERT(fp != 0);
	Q_ASSERT(str != 0);

	std::fprintf(fp, "%s", str);
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
 *
 * The implementation provided here can handle links.  (Both Unix-links
 * as well as Windows-shortcuts, since Qt supports then both; I don't
 * know how a link looks like on a Mac, but Qt should support that
 * too.)
 *
 * TODO: Search through the PATH env. variable.  Find out how this is
 * supposed to work on OS X.
 */
int
os_get_exe_filename( char* buf, size_t buflen, const char* argv0 )
{
	QFileInfo file(QString::fromLocal8Bit(argv0));
	file.makeAbsolute();
	if (not file.exists() or not file.isReadable()) {
		return false;
	}

	// If the file is some form of link, find out where it points to.
	if (file.isSymLink()) {
		while (not file.readLink().isEmpty()) {
			file.setFile(file.readLink());
		}
		file.makeAbsolute();
		if (not file.exists() or not file.isReadable()) {
			return false;
		}
	}

	const QByteArray& result = file.filePath().toLocal8Bit();
	if (result.length() + 1 > static_cast<int>(buflen)) {
		// The result would not fit in the buffer.
		return false;
	}
	strcpy(buf, result.constData());
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
		return;

	  case OS_GSP_T3_APP_DATA: {
		const QString& dirStr = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
		QDir dir(dirStr);
		// Create the directory if it doesn't exist.
		if (not dir.exists() and not dir.mkpath(dirStr)) {
			// TODO: Error dialog.
			qWarning() << "Could not create directory path:" << dirStr;
			Q_ASSERT(QDir::tempPath().toLocal8Bit().size() < static_cast<int>(buflen));
			strncpy(buf, QDir::tempPath().toLocal8Bit().constData(), buflen);
			return;
		}
		Q_ASSERT(dirStr.toLocal8Bit().size() < static_cast<int>(buflen));
		strncpy(buf, dirStr.toLocal8Bit().constData(), buflen);
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
int
os_locate( const char* fname, int /*flen*/, const char* /*arg0*/, char* buf, size_t bufsiz )
{
	//qDebug() << Q_FUNC_INFO << "\n Looking for:" << fname;
	Q_ASSERT(fname != 0);
	Q_ASSERT(buf != 0);

	const QFileInfo& fileInfo = QFileInfo(QString::fromLocal8Bit(fname));
	const QByteArray& result = fileInfo.absoluteFilePath().toLocal8Bit();
	if (bufsiz > result.length() and QFile::exists(fileInfo.absoluteFilePath())) {
		strcpy(buf, result.constData());
		return true;
	}
	// Not found or buffer not big enough.
	return false;
}


/* --------------------------------------------------------------------
 */

/* Create and open a temporary file.
 */
osfildef*
os_create_tempfile( const char* fname, char* buf )
{
	if (fname != 0 and fname[0] != '\0') {
		// A filename has been specified; use it.
		return std::fopen(fname, "w+b");
	}

	Q_ASSERT(buf != 0);

	// No filename needed; create a nameless temp-file.
	buf[0] = '\0';
	return std::tmpfile();
}


/* Delete a temporary file created with os_create_tempfile().
 */
int
osfdel_temp( const char* fname )
{
	Q_ASSERT(fname != 0);

	if (fname[0] == '\0' or QFile::remove(QString::fromLocal8Bit(fname))) {
		// If fname was empty, it has been already deleted automatically by
		// fclose().  If fname was not empty, QFile::remove has taken care of
		// deleting the file.
		return 0;
	}
	// QFile::remove() failed.
	return 1;
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

	if (QFileInfo(QString::fromLocal8Bit(fn)).suffix().isEmpty()) {
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

	QFileInfo file(QString::fromLocal8Bit(fn));
	if (file.suffix().isEmpty()) {
		return;
	}
	std::strcpy(fn, file.completeBaseName().toLocal8Bit());
}


/* Get a pointer to the root name portion of a filename.
 *
 * Note that Qt's native path separator character is '/'.  It doesn't matter on
 * what OS we're running.
 */
char*
os_get_root_name( char* buf )
{
	Q_ASSERT(buf != 0);

	char* p = buf;
	for (p += std::strlen(buf) - 1; p > buf and *p != '/'; --p)
		;
	if (p != buf) {
		++p;
	}
	return p;
}


/* Build a full path name, given a path and a filename.
 */
void
os_build_full_path( char* fullpathbuf, size_t fullpathbuflen, const char* path, const char* filename )
{
	Q_ASSERT(fullpathbuf != 0);
	Q_ASSERT(path != 0);
	Q_ASSERT(filename != 0);

	std::strncpy(fullpathbuf, QFileInfo(QDir(QString::fromLocal8Bit(path)),
										QString::fromLocal8Bit(filename)).filePath().toLocal8Bit(),
				 fullpathbuflen);
	fullpathbuf[fullpathbuflen - 1] = '\0';
}


/* Extract the path from a filename.
 */
void
os_get_path_name( char* pathbuf, size_t pathbuflen, const char* fname )
{
	strncpy(pathbuf, QFileInfo(QString::fromLocal8Bit(fname)).path().toLocal8Bit().constData(), pathbuflen);
	pathbuf[pathbuflen - 1] = '\0';
}


/* Convert a relative URL into a relative filename path.
 */
void
os_cvt_url_dir( char* result_buf, size_t result_buf_size, const char* src_url, int end_sep )
{
	QString result(QString::fromLocal8Bit(src_url));
	if (end_sep == true and not result.endsWith(QChar::fromAscii('/'))) {
		result.append(QChar::fromAscii('/'));
	}
	strncpy(result_buf, result.toLocal8Bit().constData(), result_buf_size);
	result_buf[result_buf_size - 1] = '\0';
}


/* Determine whether a filename specifies an absolute or relative path.
 */
int
os_is_file_absolute( const char* fname )
{
	return QFileInfo(QString::fromLocal8Bit(fname)).isAbsolute();
}


// --------------------------------------------------------------------

/* Get a suitable seed for a random number generator.
 */
void
os_rand( long* val )
{
	Q_ASSERT(val != 0);

	// Is this the first call to os_rand()?
	static bool initial = true;

	if (initial) {
		// It's the first time we're called.  Initialize the random number
		// generator.
		initial = false;
		time_t t = time(0);
		if (t == static_cast<time_t>(-1)) {
			std::srand(std::rand());
		} else {
			std::srand(static_cast<unsigned int>(t));
		}
	}

	// Generate a random number by using high-order bits, because on some
	// systems the low-order bits aren't very random.
	*val = 1 + static_cast<long>(static_cast<long double>(65535) * std::rand() / (RAND_MAX + 1.0));
}


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
		filter = QObject::tr("TADS 2 Games") + QString::fromAscii(" (*.gam *.Gam *.GAM)");
		break;
	  case OSFTSAVE:
		filter = QObject::tr("TADS 2 Saved Games") + QString::fromAscii(" (*.sav *.Sav *.SAV)");
		break;
	  case OSFTLOG:
		filter = QObject::tr("Game Transcripts") + QString::fromAscii(" (*.txt *.Txt *.TXT)");
		break;
	  case OSFTT3IMG:
		Q_ASSERT(qFrame->tads3());
		filter = QObject::tr("TADS 3 Games") + QString::fromAscii(" (*.t3 *.T3)");
		break;
	  case OSFTT3SAV:
		Q_ASSERT(qFrame->tads3());
		filter = QObject::tr("TADS 3 Saved Games") + QString::fromAscii(" (*.t3v *.T3v *.T3V)");
		ext = QString::fromAscii("t3v");
		break;
	}

	// Always provide an "All Files" filter.
	if (not filter.isEmpty()) {
		filter.append(QString::fromAscii(";;"));
		filter.append(QObject::tr("All Files") + QString::fromAscii(" (*)"));
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

	const QByteArray& result = filename.toLocal8Bit();
	if (fname_buf_len <= result.length()) {
		return OS_AFE_FAILURE;
	}
	strcpy(fname_buf, result.constData());
	if (not ext.isEmpty()) {
		// Since `ext' is not empty, an extension should be
		// appended (if none exists already).
		os_defext(fname_buf, ext.toLocal8Bit().constData());
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
		buttonList.append(dialog.addButton(buttonText, QMessageBox::InvalidRole));
	}

	if (default_index != 0) {
		dialog.setDefaultButton(buttonList[default_index - 1]);
	}
	if (cancel_index != 0) {
		dialog.setEscapeButton(buttonList[default_index - 1]);
	}
	// We append a space to the window title to avoid the "<2>" that would
	// otherwise be appended automatically by some window managers (like KDE.)
	dialog.setWindowTitle(qWinGroup->windowTitle() + QChar::fromAscii(' '));
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

/* Get the current system high-precision timer.
 */
long
os_get_sys_clock_ms( void )
{
	static QTime zeroPoint(QTime::currentTime());
	static int lastRet = -1;
	static unsigned int wraps = 0;

	int ret = zeroPoint.elapsed();

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
	return ret + (wraps * 86400000);
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
 */
void
os_get_charmap( char* mapname, int charmap_id )
{
	//qDebug() << Q_FUNC_INFO;

	Q_ASSERT(qFrame->tads3());

	switch(charmap_id) {
	  case OS_CHARMAP_DISPLAY:
		// Always use UTF-8 for the display, regardless of the
		// local charset; Qt uses Unicode for the display on
		// every system.
		strcpy(mapname, "utf8");
		break;
	  case OS_CHARMAP_FILENAME:
	  case OS_CHARMAP_FILECONTENTS:
		strcpy(mapname, QTextCodec::codecForLocale()->name());
		break;
	  default:
		qWarning("os_get_charmap() got an unknown charmap id");
		strcpy(mapname, QTextCodec::codecForLocale()->name());
		break;
	}
}


/* Generate a filename for a character-set mapping file.
 */
void
os_gen_charmap_filename( char* filename, char* internal_id, char* /*argv0*/ )
{
	qDebug() << Q_FUNC_INFO;
	Q_ASSERT(filename != 0);

	strncpy(filename, QString(QString::fromAscii(internal_id)
							  + QString::fromAscii(".tcp")).toLocal8Bit().constData(), OSFNMAX);
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
#ifndef Q_WS_ANDROID
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
