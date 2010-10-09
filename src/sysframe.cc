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

#include <QLayout>
#include <QLabel>
#include <QStatusBar>
#include <QDir>
#include <QTextCodec>

#include "qtadshostifc.h"
#include "settings.h"
#include "dispwidget.h"
#include "syswinaboutbox.h"
#include "syswininput.h"

#include "htmlprs.h"
#include "htmlinp.h"
#include "htmlfmt.h"
#include "htmlrf.h"
#include "htmltxar.h"
#include "vmmaincn.h"


CHtmlSysFrameQt::CHtmlSysFrameQt( int& argc, char* argv[], const char* appName, const char* appVersion,
								  const char* orgName, const char* orgDomain )
: QApplication(argc, argv), fGameWin(0), fGameRunning(false), fReformatPending(false)
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(qFrame == 0);

	this->setApplicationName(appName);
	this->setApplicationVersion(appVersion);
	this->setOrganizationName(orgName);
	this->setOrganizationDomain(orgDomain);

	// Load our persistent settings.
	this->fSettings = new Settings;
	this->fSettings->loadFromDisk();

	// Initialize the input color with the user-configured one.  The game is
	// free to change the input color later on.
	const QColor& tmpCol = this->fSettings->inputColor;
	this->fInputColor = HTML_make_color(tmpCol.red(), tmpCol.green(), tmpCol.blue());

	this->fParser = 0;
	this->fInputBuffer = 0;
	this->fTadsBuffer = 0;
	this->fFormatter = 0;

	// Clear the TADS appctx; all unused fields must be 0.
	memset(&this->fAppctx, 0, sizeof(this->fAppctx));

	// Create the TADS host and client application interfaces.
	this->fHostifc = new QTadsHostIfc(&this->fAppctx);
	this->fClientifc = new CVmMainClientConsole;

	// Set our global pointer.
	qFrame = this;

	// Create our main application window.
	this->fMainWin = new CHtmlSysWinGroupQt;
	this->fMainWin->setWindowTitle(appName);
	this->fMainWin->updateRecentGames();

	// Automatically quit the application when the last window has closed.
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

	// We're the main HTML TADS frame object.
	CHtmlSysFrame::set_frame_obj(this);
}


CHtmlSysFrameQt::~CHtmlSysFrameQt()
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(qFrame != 0);

	// Delete the "about this game" box.
	this->fMainWin->deleteAboutBox();

	// We're no longer the main frame object.
	CHtmlSysFrame::set_frame_obj(0);

	// Save our persistent settings.
	this->fSettings->saveToDisk();

	// We're being destroyed, so our global pointer is no longer valid.
	qFrame = 0;

	// Delete HTML banners, orphaned banners and main game window.
	while (not this->fBannerList.isEmpty()) {
		delete this->fBannerList.takeLast();
	}
	while (not this->fOrhpanBannerList.isEmpty()) {
		os_banner_delete(this->fOrhpanBannerList.takeLast());
	}
	if (this->fGameWin != 0) {
		delete this->fGameWin;
	}

	// Release the parser and delete our parser, buffers and formatter.
	if (this->fFormatter != 0) {
		this->fFormatter->release_parser();
	}
	if (this->fParser != 0) {
		delete this->fParser;
	}
	if (this->fInputBuffer != 0) {
		delete[] this->fInputBuffer;
	}
	if (this->fTadsBuffer != 0) {
		delete this->fTadsBuffer;
	}
	if (this->fFormatter != 0) {
		delete this->fFormatter;
	}

	// Delete cached fonts.
	while (not this->fFontList.isEmpty()) {
		delete this->fFontList.takeLast();
	}

	// Delete our TADS application interfaces and settings.
	delete this->fClientifc;
	delete this->fHostifc;
	delete this->fSettings;

	delete this->fMainWin;
}


void
CHtmlSysFrameQt::fRunGame()
{
	if (this->fNextGame.isEmpty()) {
		// Nothing to run.
		return;
	}

	while (not this->fNextGame.isEmpty()) {
		QFileInfo finfo(QFileInfo(this->fNextGame).absoluteFilePath());
		this->fNextGame.clear();

		// Change to the game file's directory.
		QDir::setCurrent(finfo.absolutePath());

		// Run the appropriate TADS VM.
		int vmType = vm_get_game_type(finfo.absoluteFilePath().toLocal8Bit().constData(), 0, 0, 0, 0);
		if (vmType == VM_GGT_TADS2 or vmType == VM_GGT_TADS3) {
			// Delete all HTML and orphaned banners.
			while (not this->fOrhpanBannerList.isEmpty()) {
				os_banner_delete(this->fOrhpanBannerList.takeLast());
			}
			while (not this->fBannerList.isEmpty()) {
				delete this->fBannerList.takeLast();
			}

			// Delete the current main HTML game window.
			if (this->fGameWin != 0) {
				delete this->fGameWin;
			}

			// Delete current HTML parser, input buffer and main game window formatter.
			if (this->fFormatter != 0) {
				this->fFormatter->release_parser();
			}
			if (this->fParser != 0) {
				delete this->fParser;
			}
			if (this->fInputBuffer != 0) {
				delete[] this->fInputBuffer;
			}
			if (this->fTadsBuffer != 0) {
				delete this->fTadsBuffer;
			}
			if (this->fFormatter != 0) {
				delete this->fFormatter;
			}

			// Delete cached fonts.
			while (not this->fFontList.isEmpty()) {
				delete this->fFontList.takeLast();
			}

			// Recreate them.
			this->fParser = new CHtmlParser(true);
			this->fInputBuffer = new textchar_t[1024];
			this->fTadsBuffer = new CHtmlInputBuf(fInputBuffer, 1024, 100);
			this->fTadsBuffer->set_utf8_mode(true);
			this->fFormatter = new CHtmlFormatterInput(this->fParser);
			// Tell the resource finder about our appctx.
			this->fFormatter->get_res_finder()->init_appctx(&this->fAppctx);
			this->fGameWin = new CHtmlSysWinInputQt(this->fFormatter, qWinGroup->centralWidget());
			this->fGameWin->resize(qWinGroup->centralWidget()->size());
			this->fGameWin->show();
			this->fGameWin->setFocus();

			// Set the application's window title to contain the filename of the game
			// we're running.  The game is free to change that later on.
			qWinGroup->setWindowTitle(finfo.fileName() + " - " + qFrame->applicationName());

			// Add the game file to our "recent games" list.
			int recentIdx = this->fSettings->recentGamesList.indexOf(finfo.absoluteFilePath());
			if (recentIdx > 0) {
				// It's already in the list and it's not the first item.  Make
				// it the first item so that it becomes the most recent entry.
				this->fSettings->recentGamesList.move(recentIdx, 0);
			} else if (recentIdx < 0) {
				// It's not in the list.  Prepend it as the most recent item
				// and, if the list is full, delete the oldest one.
				if (this->fSettings->recentGamesList.size() >= this->fSettings->recentGamesCapacity) {
					this->fSettings->recentGamesList.removeLast();
				}
				this->fSettings->recentGamesList.prepend(finfo.absoluteFilePath());
			}
			this->fMainWin->updateRecentGames();
			this->fSettings->saveToDisk();

			// Run the appropriate VM.
			this->fGameRunning = true;
			this->fGameFile = finfo.absoluteFilePath().toLocal8Bit();
			emit gameStarting();
			if (vmType == VM_GGT_TADS2) {
				this->fRunT2Game(finfo.absoluteFilePath());
			} else {
				this->fRunT3Game(finfo.absoluteFilePath());
			}
			this->fGameRunning = false;
			this->fGameFile.clear();
			emit gameHasQuit();

			// Flush any pending output and cancel all sounds and animations.
			this->flush_txtbuf(true, false);
			this->fFormatter->cancel_sound(HTML_Attrib_invalid, 0.0, false, false);
			this->fFormatter->cancel_playback();
		} else {
			qWarning() << finfo.fileName() << "is not a TADS game file.";
		}
	}

	// Reset application window title.
	qWinGroup->setWindowTitle(qFrame->applicationName());
}


void
CHtmlSysFrameQt::fRunT2Game( const QString& fname )
{
	// We'll be loading a T2 game.
	this->fTads3 = false;

	// T2 requires argc/argv style arguments.
	char argv0[] = "qtads";
	char* argv1 = new char[fname.toLocal8Bit().size() + 1];
	strcpy(argv1, fname.toLocal8Bit());
	char* argv[2] = {argv0, argv1};

	// We always use .sav as the extension for T2 save files.
	char savExt[] = "sav";

	// Start the T2 VM.
	trdmain(2, argv, &this->fAppctx, savExt);

	delete[] argv1;
}


void
CHtmlSysFrameQt::fRunT3Game( const QString& fname )
{
	this->fTads3 = true;
	vm_run_image(this->fClientifc, fname.toLocal8Bit(), this->fHostifc, 0, 0, 0, false, 0, 0, false,
				 false, 0, 0, 0, 0);
}


void
CHtmlSysFrameQt::main( QString gameFileName )
{
	// Restore the application's size.
	this->fMainWin->resize(this->fSettings->appSize);
	this->fMainWin->show();

	// If a game file was specified, try to run it.
	if (not gameFileName.isEmpty()) {
		this->setNextGame(gameFileName);
	}
}


CHtmlSysFontQt*
CHtmlSysFrameQt::createFont( const CHtmlFontDesc* font_desc )
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(font_desc != 0);

	CHtmlFontDesc newFontDesc = *font_desc;
	CHtmlSysFontQt newFont;
	newFont.setStyleStrategy(QFont::StyleStrategy(QFont::PreferOutline | QFont::PreferQuality));

	// Use the weight they provided (we may change this if a weight modifier is
	// specified).
	if (newFontDesc.weight < 400) {
		newFont.setWeight(QFont::Light);
	} else if (newFontDesc.weight < 700) {
		newFont.setWeight(QFont::Normal);
	} else if (newFontDesc.weight < 900) {
		newFont.setWeight(QFont::Bold);
	} else {
		newFont.setWeight(QFont::Black);
	}

	// Apply the specific font attributes.
	newFont.setItalic(newFontDesc.italic);
	newFont.setUnderline(newFontDesc.underline);
	newFont.setStrikeOut(newFontDesc.strikeout);

	// Assume that we'll use the base point size of the default text font (the
	// main proportional font) as the basis of the size.  If we find a specific
	// named parameterized font name, we'll change to use the size as specified
	// in the player preferences for that parameterized font; but if we have a
	// particular name given, the player has no way to directly specify the
	// base size for that font, so the best we can do is use the default text
	// font size for guidance.
	int base_point_size = this->fSettings->mainFont.pointSize();

	// If a face name is listed, try to find the given face in the system.
	// Note that we wait until after setting all of the basic attributes (in
	// particular, weight, color, and styles) before dealing with the face
	// name; this is to allow parameterized face names to override the basic
	// attributes.  For example, the "TADS-Input" font allows the player to
	// specify color, bold, and italic styles in addition to the font name.
	if (newFontDesc.face[0] != 0) {
		// The face name field can contain multiple face names separated by
		// commas.  We split them into a list and try each one individualy.
		bool matchFound = false;
		const QStringList& strList = QString(newFontDesc.face).split(QChar(','), QString::SkipEmptyParts);
		for (int i = 0; i < strList.size() and not matchFound; ++i) {
			const QString& s = strList.at(i).simplified().toLower();
			if (s == QString(HTMLFONT_TADS_SERIF).toLower()) {
				strcpy(newFontDesc.face, this->fSettings->serifFont.family().toLatin1().constData());
				base_point_size = this->fSettings->serifFont.pointSize();
				matchFound = true;
			} else if (s == QString(HTMLFONT_TADS_SANS).toLower()) {
				strcpy(newFontDesc.face, this->fSettings->sansFont.family().toLatin1().constData());
				base_point_size = this->fSettings->sansFont.pointSize();
				matchFound = true;
			} else if (s == QString(HTMLFONT_TADS_SCRIPT).toLower()) {
				strcpy(newFontDesc.face, this->fSettings->scriptFont.family().toLatin1().constData());
				base_point_size = this->fSettings->scriptFont.pointSize();
				matchFound = true;
			} else if (s == QString(HTMLFONT_TADS_TYPEWRITER).toLower()) {
				strcpy(newFontDesc.face, this->fSettings->writerFont.family().toLatin1().constData());
				base_point_size = this->fSettings->writerFont.pointSize();
				matchFound = true;
			} else if (s == QString(HTMLFONT_TADS_INPUT).toLower()) {
				strcpy(newFontDesc.face, this->fSettings->inputFont.family().toLatin1().constData());
				base_point_size = this->fSettings->inputFont.pointSize();
				newFont.setBold(this->fSettings->inputFont.bold());
				newFont.setItalic(this->fSettings->inputFont.italic());
				if (newFontDesc.default_color) {
					newFontDesc.color = HTML_COLOR_INPUT;
					newFont.color(HTML_COLOR_INPUT);
				}
				matchFound = true;
			} else if (s == "qtads-grid") {
				// "qtads-grid" is an internal face name; it means we should
				// return a font suitable for a text grid banner.
				strcpy(newFontDesc.face, this->fSettings->fixedFont.family().toLatin1().constData());
				base_point_size = this->fSettings->fixedFont.pointSize();
				matchFound = true;
			} else {
				newFont.setFamily(s.toLower());
				if (newFont.exactMatch()) {
					matchFound = true;
					strcpy(newFontDesc.face, s.toLatin1().constData());
				}
			}
		}
		// If we didn't find a match, use the main game font as set by
		// the user.
		if (not matchFound) {
			strcpy(newFontDesc.face, this->fSettings->mainFont.family().toLatin1().constData());
		}
	// Apply characteristics only if the face wasn't specified.
	} else {
		// See if fixed-pitch is desired.
		if (newFontDesc.fixed_pitch) {
			// Use prefered monospaced font.
			strcpy(newFontDesc.face, this->fSettings->fixedFont.family().toLatin1().constData());
			base_point_size = this->fSettings->fixedFont.pointSize();
		} else {
			// Use prefered proportional font.
			strcpy(newFontDesc.face, this->fSettings->mainFont.family().toLatin1().constData());
			base_point_size = this->fSettings->mainFont.pointSize();
		}

		// See if serifs are desired for a variable-pitch font.
		if (not newFontDesc.serif and not newFontDesc.fixed_pitch) {
			strcpy(newFontDesc.face, this->fSettings->serifFont.family().toLatin1().constData());
			base_point_size = this->fSettings->serifFont.pointSize();
		}

		// See if emphasis (EM) is desired - render italic if so.
		if (newFontDesc.pe_em) {
			newFont.setItalic(true);
		}

		// See if strong emphasis (STRONG) is desired - render bold if so.
		if (newFontDesc.pe_strong) {
			newFontDesc.weight = 700;
			newFont.setWeight(QFont::Bold);
		}

		// If we're in an address block, render in italics.
		if (newFontDesc.pe_address) {
			newFont.setItalic(true);
		}

		// See if this is a defining instance (DFN) - render in italics.
		if (newFontDesc.pe_dfn) {
			newFont.setItalic(true);
		}

		// See if this is sample code (SAMP), keyboard code (KBD), or a
		// variable (VAR) - render these in a monospaced roman font if so.
		if (newFontDesc.pe_samp or newFontDesc.pe_kbd or newFontDesc.pe_var) {
			// Render KBD in bold.
			if (newFontDesc.pe_kbd) {
				newFont.setWeight(QFont::Bold);
			}
			strcpy(newFontDesc.face, this->fSettings->fixedFont.family().toLatin1().constData());
			base_point_size = this->fSettings->fixedFont.pointSize();
		}

		// See if this is a citation (CITE) - render in italics if so.
		if (newFontDesc.pe_cite) {
			newFont.setItalic(true);
		}
	}

	newFont.setFamily(QString(newFontDesc.face).toLower());

	// Note the HTML SIZE parameter requested - if this is zero, it indicates
	// that we want to use a specific point size instead.
	int htmlsize = newFontDesc.htmlsize;

	// If a valid HTML size is specified, map it to a point size.
	int pointsize;
	if (htmlsize >= 1 and htmlsize <= 7) {
		static const int size_pct[] = { 60, 80, 100, 120, 150, 200, 300 };

		// An HTML size is specified -- if a BIG or SMALL attribute is present,
		// bump the HTML size by 1 in the appropriate direction, if we're not
		// already at a limit.
		if (font_desc->pe_big and htmlsize < 7) {
			++htmlsize;
		} else if (font_desc->pe_small && htmlsize > 1) {
			--htmlsize;
		}

		// Adjust for the HTML SIZE setting.  There are 7 possible size
		// settings, numbered 1 through 7.  Adjust the font size by applying a
		// scaling factor for the font sizes.  Our size factor table is in
		// percentages, so multiply by the size factor, add in 50 so that we
		// round properly, and divide by 100 to get the new size.
		pointsize = ((base_point_size * size_pct[htmlsize - 1]) + 50) / 100;
	} else {
		// There's no HTML size - use the explicit point size provided.
		pointsize = font_desc->pointsize;
	}

	newFont.setPointSize(pointsize > 0 ? pointsize : base_point_size);

	if (not newFontDesc.default_color) {
		newFont.color(newFontDesc.color);
	}
	if (not newFontDesc.default_bgcolor) {
		newFont.bgColor(newFontDesc.bgcolor);
	}

	// Check whether a matching font is already in our cache.
	for (int i = 0; i < this->fFontList.size(); ++i) {
		if (*this->fFontList.at(i) == newFont) {
			return this->fFontList[i];
		}
	}

	//qDebug() << "Font not found in cache; creating new font:" << newFont
	//		<< "\nFonts so far:" << this->fFontList.size() + 1;

	// There was no match in our cache. Create a new font and store it in our
	// cache.
	CHtmlSysFontQt* font = new CHtmlSysFontQt(newFont);
	font->set_font_desc(&newFontDesc);
	this->fFontList.append(font);
	return font;
}


void
CHtmlSysFrameQt::adjustBannerSizes()
{
	if (this->fGameWin == 0) {
		return;
	}

	// Start with the main game window.  Its area can never exceed the
	// application's central frame.
	QRect siz(qWinGroup->centralWidget()->rect());
	this->fGameWin->calcChildBannerSizes(siz);
}


void
CHtmlSysFrameQt::reformatBanners( bool showStatus, bool freezeDisplay, bool resetSounds )
{
	if (this->fGameWin == 0) {
		return;
	}

	// Recalculate the banner layout, in case any of the underlying units (such
	// as the default font size) changed.
	this->adjustBannerSizes();

	// Always reformat the main panel window.
	this->fGameWin->doReformat(showStatus, freezeDisplay, resetSounds);

	// Reformat each banner window.
	for (int i = 0; i < this->fBannerList.size(); ++i) {
		this->fBannerList.at(i)->doReformat(showStatus, freezeDisplay, false);
	}
}


void CHtmlSysFrameQt::pruneParseTree()
{
	static int checkCount = 0;

	// If there's a reformat pending, perform it.
	if (this->fReformatPending) {
		this->fReformatPending = false;
		this->reformatBanners(true, true, false);
	}

	// Skip this entirely most of the time; only check every so often, so that
	// we don't waste a lot of time doing this too frequently.
	++checkCount;
	if (checkCount < 10) {
		return;
	}
	checkCount = 0;

	// Check to see if we're consuming too much memory - if not, there's
	// nothing we need to do here.
	if (this->fParser->get_text_array()->get_mem_in_use() < 65536) {
		return;
	}

	// Perform the pruning and reformat all banners.
	this->fParser->prune_tree(65536 / 2);
	this->reformatBanners(false, true, false);
}


void
CHtmlSysFrameQt::notifyPreferencesChange( const Settings* sett )
{
	// Bail out if we currently don't have an active formatter.
	if (this->fFormatter == 0) {
		return;
	}

	// If digital sounds are now turned off, cancel sound playback in the
	// effects layers
	if (not sett->enableDigitalSound) {
		this->fFormatter->cancel_sound(HTML_Attrib_ambient, 0.0, false, false);
		this->fFormatter->cancel_sound(HTML_Attrib_bgambient, 0.0, false, false);
		this->fFormatter->cancel_sound(HTML_Attrib_foreground, 0.0, false, false);
	}

	// If MIDI is now turned off, cancel playback in the music layer.
	if (not sett->enableMidiSound) {
		this->fFormatter->cancel_sound(HTML_Attrib_background, 0.0, false, false);
	}

	// Change the text cursor's height according to the new input font's height.
	qFrame->gameWindow()->setCursorHeight(QFontMetrics(sett->inputFont).height());

	qFrame->reformatBanners(true, true, false);
}


void
CHtmlSysFrameQt::flush_txtbuf( int fmt, int immediate_redraw )
{
	// Flush and clear the buffer.
	this->fParser->parse(&this->fBuffer, qWinGroup);
	this->fBuffer.clear();

	// If desired, run the parsed source through the formatter and display it.
	if (fmt) {
#if QT_VERSION < 0x040500
		// For some reason, setting update_win to false hangs with an infinite
		// loop on Qt 4.4.  No idea what causes it.
		this->fGameWin->do_formatting(false, true, false);
#else
		this->fGameWin->do_formatting(false, false, false);
#endif
	}

	// Also flush all banner windows.
	for (int i = 0; i < this->fBannerList.size(); ++i) {
		this->fBannerList.at(i)->get_formatter()->flush_txtbuf(fmt);
	}

	// If desired, immediately update the display.
	if (immediate_redraw) {
		this->fMainWin->centralWidget()->update();
	}
}


void
CHtmlSysFrameQt::start_new_page()
{
	//qDebug() << Q_FUNC_INFO;

	// Don't bother if the game is quitting.
	if (not this->fGameRunning) {
		return;
	}

	// Flush any pending output.
	this->flush_txtbuf(true, false);

	// Cancel all animations.
	this->fFormatter->cancel_playback();

	// Tell the parser to clear the page.
	this->fParser->clear_page();

	// Remove all banners.  The formatter will do the right thing and only
	// remove banners that have not been created programmatically (like those
	// created with <BANNER> tags.)
	this->fFormatter->remove_all_banners(false);

	// Notify the main game window that we're clearing the page.
	this->fGameWin->notify_clear_contents();

	// Reformat the window for the new blank page.
	this->reformatBanners(false, false, true);
}


void
CHtmlSysFrameQt::set_nonstop_mode( int flag )
{
	//qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysFrameQt::display_output( const textchar_t *buf, size_t len )
{
	//qDebug() << Q_FUNC_INFO;

	// Just add the new text to our buffer.  Append it as-is if we're running
	// a TADS 3 game, since it's already UTF-8 encoded.
	if (this->fTads3) {
		this->fBuffer.append(buf, len);
	} else {
		// TADS 2 does not use UTF-8; use the encoding from our settings.
		QTextCodec* codec = QTextCodec::codecForName(this->fSettings->tads2Encoding);
		this->fBuffer.append(codec->toUnicode(buf, len).toUtf8().constData());
	}
}


int
CHtmlSysFrameQt::check_break_key()
{
	//qDebug() << Q_FUNC_INFO;

	// TODO: We don't check for any such shortcut yet.
	return false;
}


int
CHtmlSysFrameQt::get_input( textchar_t* buf, size_t bufsiz )
{
	//qDebug() << Q_FUNC_INFO;

	// Flush and prune before input.
	this->flush_txtbuf(true, false);
	this->pruneParseTree();

	int ret = this->fGameWin->getInput(this->fTadsBuffer);

	// If input exceeds the buffer size, make sure we don't overflow.
	int len = this->fTadsBuffer->getlen() > bufsiz ? bufsiz : this->fTadsBuffer->getlen();

	// For TADS 3, we use the result as-is; it's already in UTF-8.  For TADS 2,
	// we will need to use the prefered encoding.
	if (this->fTads3) {
		strncpy(buf, this->fTadsBuffer->getbuf(), len);
	} else {
		QTextCodec* codec = QTextCodec::codecForName(this->fSettings->tads2Encoding);
		strncpy(buf, codec->fromUnicode(QString::fromUtf8(this->fTadsBuffer->getbuf(),
														  this->fTadsBuffer->getlen())).constData(), len);
	}
	buf[len] = '\0';
	return ret;
}


int
CHtmlSysFrameQt::get_input_timeout( textchar_t* buf, size_t buflen, unsigned long timeout, int use_timeout )
{
	//qDebug() << Q_FUNC_INFO;

	if (use_timeout) {
		// Flush and prune before input.
		this->flush_txtbuf(true, false);
		this->pruneParseTree();

		bool timedOut = false;
		this->fGameWin->getInput(this->fTadsBuffer, timeout, true, &timedOut);

		// If input exceeds the buffer size, make sure we don't overflow.
		int len = this->fTadsBuffer->getlen() > buflen ? buflen : this->fTadsBuffer->getlen();

		// For TADS 3, we use the result as-is; it's already in UTF-8.  For TADS 2,
		// we will need to use the prefered encoding.
		if (this->fTads3) {
			strncpy(buf, this->fTadsBuffer->getbuf(), len);
		} else {
			QTextCodec* codec = QTextCodec::codecForName(this->fSettings->tads2Encoding);
			strncpy(buf, codec->fromUnicode(QString::fromUtf8(this->fTadsBuffer->getbuf(),
															  this->fTadsBuffer->getlen())).constData(), len);
		}
		buf[len] = '\0';

		if (timedOut) {
			return OS_EVT_TIMEOUT;
		}
	} else {
		this->get_input(buf, buflen);
	}

	// Return EOF if we're quitting the game.
	if (not this->fGameRunning) {
		return OS_EVT_EOF;
	}
	return OS_EVT_LINE;
}


void
CHtmlSysFrameQt::get_input_cancel( int reset )
{
	//qDebug() << Q_FUNC_INFO;
	this->fGameWin->cancelInput(reset);
}


int
CHtmlSysFrameQt::get_input_event( unsigned long timeout, int use_timeout, os_event_info_t* info )
{
	//qDebug() << Q_FUNC_INFO << "use_timeout:" << use_timeout;

	// Flush and prune before input.
	this->flush_txtbuf(true, false);
	this->pruneParseTree();

	// Get the input.
	bool timedOut = false;
	int res = this->fGameWin->getKeypress(timeout, use_timeout, &timedOut);

	// Return EOF if we're quitting the game.
	if (not this->fGameRunning) {
		return OS_EVT_EOF;
	}

	// If the timeout expired, tell the caller.
	if (use_timeout and timedOut) {
		return OS_EVT_TIMEOUT;
	}

	if (res == -2) {
		// It was an HREF event (user clicked a hyperlink).  Get the last
		// pending HREF event.
		// For TADS 3, we use the result as-is; it's already in UTF-8.  For TADS 2,
		// we will need to use the prefered encoding.
		if (this->fTads3) {
			strncpy(info->href, this->fGameWin->pendingHrefEvent().toUtf8().constData(),
					sizeof(info->href) - 1);
		} else {
			QTextCodec* codec = QTextCodec::codecForName(this->fSettings->tads2Encoding);
			strncpy(info->href, codec->fromUnicode(this->fGameWin->pendingHrefEvent()).constData(),
					sizeof(info->href) - 1);
		}
		info->href[sizeof(info->href) - 1] = '\0';
		return OS_EVT_HREF;
	} else if (res == 0) {
		// It was an extended character; call again to get the
		// extended code.
		info->key[0] = 0;
		info->key[1] = this->fGameWin->getKeypress();
	} else {
		// A normal character.  Return it as is.
		info->key[0] = res;
		info->key[1] = 0;
	}
	// Tell the caller it was a key-event.
	return OS_EVT_KEY;
}


// FIXME: OS_EVT_HREF isn't handled and shouldn't be allowed here.
textchar_t
CHtmlSysFrameQt::wait_for_keystroke( int pause_only )
{
	//qDebug() << Q_FUNC_INFO;

	static int pendingCmd = 0;
	int ret;

	// If we have a pending keystroke command from out last call, return it
	// now.
	if (pendingCmd != 0) {
		ret = pendingCmd;
		pendingCmd = 0;
		//qDebug() << ret;
		return ret;
	}

	QLabel moreText(pause_only ? tr("*** MORE ***  [press a key to continue]") : tr("Please press a key"));
	// Display a permanent QLabel instead of a temporary message.  This allows
	// other status bar messages (like when hovering over hyperlinks) to
	// temporary remove the MORE text instead of replacing it.
	moreText.setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	moreText.setLineWidth(0);
	moreText.setContentsMargins(0, 0, 0, 0);
	qWinGroup->statusBar()->addWidget(&moreText);

	// Get the input.
	os_event_info_t info;
	ret = this->get_input_event(0, false, &info);

	// Remove the status message.
	qWinGroup->statusBar()->removeWidget(&moreText);

	if (ret == OS_EVT_EOF) {
		pendingCmd = CMD_EOF;
		return 0;
	}

	if (ret == OS_EVT_KEY and info.key[0] == 0) {
		// It was an extended character.  Prepare to return it on our next
		// call.
		pendingCmd = info.key[1];
		return 0;
	}
	return info.key[0];
}


void
CHtmlSysFrameQt::pause_for_exit()
{
	qDebug() << Q_FUNC_INFO;

	// Just wait for a keystroke and discard it.
	this->wait_for_keystroke(true);
}


void
CHtmlSysFrameQt::pause_for_more()
{
	//qDebug() << Q_FUNC_INFO;

	this->wait_for_keystroke(true);
}


void
CHtmlSysFrameQt::dbg_print( const char *msg )
{
	//qDebug() << "HTML TADS Debug message:" << msg;
}


CHtmlSysWin*
CHtmlSysFrameQt::create_banner_window( CHtmlSysWin* parent, HTML_BannerWin_Type_t window_type,
									   CHtmlFormatter* formatter, int where, CHtmlSysWin* other,
									   HTML_BannerWin_Pos_t pos, unsigned long style )
{
	//qDebug() << Q_FUNC_INFO;
	//return 0;

	//qDebug() << "Creating new banner. parent:" << parent << "type:" << window_type << "where:" << where
	//		<< "other:" << other << "pos:" << pos << "style:" << style;

	// Create the banner window.
	CHtmlSysWinQt* banner = new CHtmlSysWinQt(formatter, 0, qWinGroup->centralWidget());
	CHtmlSysWinQt* castParent = static_cast<CHtmlSysWinQt*>(parent);
	CHtmlSysWinQt* castOther = static_cast<CHtmlSysWinQt*>(other);

	// Don't allow MORE mode in text grids.
	if (window_type == HTML_BANNERWIN_TEXTGRID) {
		style &= ~OS_BANNER_STYLE_MOREMODE;
	}

	// MORE mode implies auto vscroll.
	if (style & OS_BANNER_STYLE_MOREMODE) {
		style |= OS_BANNER_STYLE_AUTO_VSCROLL;
	}

	// If no parent was specified, it means that it's a child of the main
	// game window.
	if (parent == 0) {
		parent = castParent = this->fGameWin;
	}

	// If BEFORE or AFTER is requested but 'other' isn't a child of the
	// parent, we must behave as if OS_BANNER_LAST were specified.
	if (where == OS_BANNER_BEFORE or where == OS_BANNER_AFTER) {
		Q_ASSERT(other != 0);
		if (castOther->parentBanner() != parent) {
			where = OS_BANNER_LAST;
		}
	}

	// Add the banner and store it in our list.
	castParent->addBanner(banner, window_type, where, castOther, pos, style);
	this->fBannerList.append(banner);
	return banner;
}


void
CHtmlSysFrameQt::orphan_banner_window( CHtmlFormatterBannerExt* banner )
{
	//qDebug() << Q_FUNC_INFO;

	this->fOrhpanBannerList.append(banner);
}


CHtmlSysWin*
CHtmlSysFrameQt::create_aboutbox_window( CHtmlFormatter* formatter )
{
	//qDebug() << Q_FUNC_INFO;

	return this->fMainWin->createAboutBox(formatter);
}


void
CHtmlSysFrameQt::remove_banner_window( CHtmlSysWin* win )
{
	//qDebug() << Q_FUNC_INFO;

	// If this is the "about this game" box, ask the main window to delete it.
	if (win == this->fMainWin->aboutBox()) {
		this->fMainWin->deleteAboutBox();
		return;
	}

	CHtmlSysWinQt* castWin = static_cast<CHtmlSysWinQt*>(win);

	// Before deleting it, remove it from our list and give keyboard focus to
	// its parent, if it has one.
	this->fBannerList.removeAll(castWin);
	if (castWin->parentBanner() != 0) {
		castWin->parentBanner()->setFocus();
	}

	// Delete it and recalculate the banner layout.
	delete win;
	this->adjustBannerSizes();
}


int
CHtmlSysFrameQt::get_exe_resource( const textchar_t* resname, size_t resnamelen, textchar_t* fname_buf,
								   size_t fname_buf_len, unsigned long* seek_pos, unsigned long* siz )
{
	//qDebug() << Q_FUNC_INFO;
	//qDebug() << "resname:" << resname << "fname_buf:" << fname_buf << "seek_pos:" << seek_pos;

	return false;
}
