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

#include "htmlqt.h"
#include "qtadshostifc.h"
#include "qtadssettings.h"

#include "htmlprs.h"
#include "htmlinp.h"
#include "htmlfmt.h"
#include "htmlrf.h"
#include "vmmaincn.h"


CHtmlSysFrameQt::CHtmlSysFrameQt( int& argc, char* argv[], const char* appName, const char* appVersion,
								  const char* orgName, const char* orgDomain )
: QApplication(argc, argv), fGameRunning(false)
{

	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(qFrame == 0);

	this->setApplicationName(appName);
	this->setApplicationVersion(appVersion);
	this->setOrganizationName(orgName);
	this->setOrganizationDomain(orgDomain);

	this->fSettings = new QTadsSettings;
	this->fSettings->loadFromDisk();

	this->fParser = new CHtmlParser(true);
	this->fInputBuffer = new textchar_t[1024];
	this->fTadsBuffer =new CHtmlInputBuf(fInputBuffer, 1024, 100);
	this->fFormatter = new CHtmlFormatterInput(this->fParser);

	memset(&this->fAppctx, 0, sizeof(this->fAppctx));
	this->fFormatter->get_res_finder()->init_appctx(&this->fAppctx);

	this->fHostifc = new QTadsHostIfc(&this->fAppctx);
	this->fClientifc = new CVmMainClientConsole;

	this->fMainWin = new CHtmlSysWinGroupQt;
	this->fMainWin->setWindowTitle("QTads");
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

	this->fMainWin->resize(800, 600);
	this->fMainWin->show();

	qFrame = this;
}


CHtmlSysFrameQt::~CHtmlSysFrameQt()
{
	//qDebug() << Q_FUNC_INFO;
	Q_ASSERT(qFrame != 0);

	qFrame = 0;
	this->fFormatter->release_parser();
	delete this->fParser;
	delete[] this->fInputBuffer;
	delete this->fTadsBuffer;

	// Normally we would delete the formatter too, but for some reason this
	// will crash the application.  So for now, don't delete it.
	//delete this->fFormatter;

	// Delete cached fonts.
	while (not this->fFontList.isEmpty()) {
		delete this->fFontList.takeLast();
	}

	//delete this->fGameWin;
	//delete this->fMainWin;
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
		QString facename(QString(newFontDesc.face).toLower());
		if (facename == QString(HTMLFONT_TADS_SERIF).toLower()) {
			strcpy(newFontDesc.face, this->fSettings->serifFont.family().toLatin1().constData());
			base_point_size = this->fSettings->serifFont.pointSize();
		} else if (facename == QString(HTMLFONT_TADS_SANS).toLower()) {
			strcpy(newFontDesc.face, this->fSettings->sansFont.family().toLatin1().constData());
			base_point_size = this->fSettings->sansFont.pointSize();
		} else if (facename == QString(HTMLFONT_TADS_SCRIPT).toLower()) {
			strcpy(newFontDesc.face, this->fSettings->scriptFont.family().toLatin1().constData());
			base_point_size = this->fSettings->scriptFont.pointSize();
		} else if (facename == QString(HTMLFONT_TADS_TYPEWRITER).toLower()) {
			strcpy(newFontDesc.face, this->fSettings->writerFont.family().toLatin1().constData());
			base_point_size = this->fSettings->writerFont.pointSize();
		} else if (facename == QString(HTMLFONT_TADS_INPUT).toLower()) {
			strcpy(newFontDesc.face, this->fSettings->inputFont.family().toLatin1().constData());
			base_point_size = this->fSettings->inputFont.pointSize();
			newFont.setBold(this->fSettings->inputFont.bold());
			newFont.setItalic(this->fSettings->inputFont.italic());
		} else {
			// TODO: Parse the specified face name.
			strcpy(newFontDesc.face, font_desc->face);
		}
	} else { // Apply characteristics only if the face wasn't specified.
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

	newFont.setFamily(newFontDesc.face);

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
		newFont.bgColor(newFontDesc.color);
	}

	// Check whether a matching font is already in our cache.
	for (int i = 0; i < this->fFontList.size(); ++i) {
		if (*this->fFontList.at(i) == newFont) {
			return this->fFontList[i];
		}
	}

	//qDebug() << "Font not found in cache; creating new font. Fonts so far:" << this->fFontList.size() + 1;

	CHtmlSysFontQt* font = new CHtmlSysFontQt(newFont);
	font->set_font_desc(&newFontDesc);
	this->fFontList.append(font);
	return font;
}


int
CHtmlSysFrameQt::runT2Game( const QString& fname )
{
	this->fGameWin = new CHtmlSysWinInputQt(this->fFormatter, 0);
	this->fMainWin->setCentralWidget(this->fGameWin);
	this->fGameWin->setFocus();
	this->fFormatter->set_t3_mode(false);
	this->fTads3 = false;

	char argv0[] = "qtads";
	char* argv1 = new char[fname.toLocal8Bit().size() + 1];
	strcpy(argv1, fname.toLocal8Bit());
	char* argv[2] = {argv0, argv1};
	char savExt[] = "sav";

	this->fGameRunning = true;
	int ret = trdmain(2, argv, &this->fAppctx, savExt);
	this->fGameRunning = false;

	delete[] argv1;
	return ret;
}

int
CHtmlSysFrameQt::runT3Game( const QString& fname )
{
	this->fGameWin = new CHtmlSysWinInputQt(this->fFormatter, 0);
	this->fMainWin->setCentralWidget(this->fGameWin);
	this->fGameWin->setFocus();
	this->fFormatter->set_t3_mode(true);
	this->fTads3 = true;

	this->fGameRunning = true;
	return vm_run_image(this->fClientifc, fname.toLocal8Bit(), this->fHostifc, 0, 0,
						0, false, 0, 0, false, false, 0, 0, 0, 0);
	this->fGameRunning = false;
}


void
CHtmlSysFrameQt::flush_txtbuf( int fmt, int immediate_redraw )
{
	this->fParser->parse(&this->fBuffer, qWinGroup);
	this->fBuffer.clear();
	if (fmt) {
		this->fGameWin->do_formatting(false, true, not immediate_redraw);
	}
	// Also flush all banner windows.
	for (int i = 0; i < this->fBannerList.size(); ++i) {
		this->fBannerList.at(i)->get_formatter()->flush_txtbuf(fmt);
	}
}


CHtmlParser*
CHtmlSysFrameQt::get_parser()
{
	return this->fParser;
}


void
CHtmlSysFrameQt::start_new_page()
{
	qDebug() << Q_FUNC_INFO;

	this->fFormatter->remove_all_banners(false);
}


void
CHtmlSysFrameQt::set_nonstop_mode( int flag )
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysFrameQt::display_output( const textchar_t *buf, size_t len )
{
	//qDebug() << Q_FUNC_INFO;

	this->fBuffer.append(buf, len);
}


int
CHtmlSysFrameQt::check_break_key()
{
	qDebug() << Q_FUNC_INFO;

	return false;
}


int
CHtmlSysFrameQt::get_input( textchar_t* buf, size_t bufsiz )
{
	//qDebug() << Q_FUNC_INFO;

	this->flush_txtbuf(true, false);
	int ret = this->fGameWin->getInput(this->fTadsBuffer);
	int len = this->fTadsBuffer->getlen() > bufsiz ? bufsiz : this->fTadsBuffer->getlen();
	strncpy(buf, this->fTadsBuffer->getbuf(), len);
	buf[len] = '\0';
	return ret;
}


int
CHtmlSysFrameQt::get_input_timeout( textchar_t* buf, size_t buflen, unsigned long timeout, int use_timeout )
{
	//qDebug() << Q_FUNC_INFO;

	if (use_timeout) {
		return OS_EVT_NOTIMEOUT;
	}

	this->get_input(buf, buflen);

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
}


int
CHtmlSysFrameQt::get_input_event( unsigned long ms, int use_timeout, os_event_info_t* info )
{
	qDebug() << Q_FUNC_INFO << "use_timeout:" << use_timeout;

	if (use_timeout) {
		qWarning() << Q_FUNC_INFO << "timeout (" << ms << ") was requested by we don't support this yet";
		//return OS_EVT_TIMEOUT;
		//return OS_EVT_NOTIMEOUT;
	}

	bool timedOut = false;

	int res = this->fGameWin->getKeypress(use_timeout ? ms : 0, &timedOut);

	// Return EOF if we're quitting the game.
	if (not this->fGameRunning) {
		return OS_EVT_EOF;
	}

	// If the timeout expired, tell TADS about it.
	if (use_timeout and timedOut) return OS_EVT_TIMEOUT;

	if (res == 0) {
		// It was an extended character; call again to get the
		// extended code.
		info->key[0] = 0;
		info->key[1] = this->fGameWin->getKeypress();
	} else {
		// A normal character.  Return it as is.
		info->key[0] = res;
		info->key[1] = 0;
	}
	// Tell the caller it was an key-event.
	return OS_EVT_KEY;
}


textchar_t
CHtmlSysFrameQt::wait_for_keystroke( int pause_only )
{
	qDebug() << Q_FUNC_INFO;

	return '\n';
}


void
CHtmlSysFrameQt::pause_for_exit()
{
	qDebug() << Q_FUNC_INFO;
}


void
CHtmlSysFrameQt::pause_for_more()
{
	qDebug() << Q_FUNC_INFO;
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

	qDebug() << "Creating new banner. parent:" << parent << "type:" << window_type << "where:" << where
			<< "other:" << other << "pos:" << pos << "style:" << style;

	CHtmlSysWinQt* banner = new CHtmlSysWinQt(formatter, 0);

	if (style & OS_BANNER_STYLE_VSCROLL) {
		banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	} else {
		banner->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	if (style & OS_BANNER_STYLE_HSCROLL) {
		banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	} else {
		banner->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	// If BEFORE or AFTER is requested but 'other' isn't a child of the
	// parent, we must behave as if OS_BANNER_LAST were specified.
	if (where == OS_BANNER_BEFORE or where == OS_BANNER_AFTER) {
		Q_ASSERT(other != 0);
		Q_ASSERT(parent != 0);
		if (static_cast<CHtmlSysWinQt*>(parent)->parentWidget()->layout()
			->indexOf(static_cast<CHtmlSysWinQt*>(other)) == -1) {
			// 'other' is not a child banner of 'parent'.
			where = OS_BANNER_LAST;
		}
	}

	if (parent == 0) {
		parent = this->fGameWin;
	}

	static_cast<CHtmlSysWinQt*>(parent)->addBanner(banner, where, static_cast<CHtmlSysWinQt*>(other), pos, style);
	this->fBannerList.append(banner);
	return banner;
}


void
CHtmlSysFrameQt::orphan_banner_window( CHtmlFormatterBannerExt* banner )
{
	qDebug() << Q_FUNC_INFO;

	os_banner_delete(banner);
}


CHtmlSysWin*
CHtmlSysFrameQt::create_aboutbox_window( CHtmlFormatter* formatter )
{
	//qDebug() << Q_FUNC_INFO;
	return 0;
}


void
CHtmlSysFrameQt::remove_banner_window( CHtmlSysWin* win )
{
	qDebug() << Q_FUNC_INFO;
	Q_ASSERT(static_cast<CHtmlSysWinQt*>(win)->parentBanner() != 0);
	this->fBannerList.removeAll(static_cast<CHtmlSysWinQt*>(win));
	static_cast<CHtmlSysWinQt*>(win)->parentBanner()->setFocus();
	delete win;
	// Recalculate the banner layout.
	QSize siz(qWinGroup->centralWidget()->size());
	qFrame->gameWindow()->calcChildBannerSizes(siz);
}


int
CHtmlSysFrameQt::get_exe_resource( const textchar_t* resname, size_t resnamelen, textchar_t* fname_buf,
								   size_t fname_buf_len, unsigned long* seek_pos, unsigned long* siz )
{
	qDebug() << Q_FUNC_INFO;
	qDebug() << "resname:" << resname << "fname_buf:" << fname_buf << "seek_pos:" << seek_pos;

	return false;
}
