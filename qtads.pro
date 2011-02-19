TEMPLATE = app
CONFIG += qt silent warn_off
QT += network
VERSION = 2.1.1.99
macx {
	QMAKE_INFO_PLIST = Info.plist
	QMAKE_LFLAGS += -F./Frameworks
	LIBS += -framework SDL_mixer -framework SDL_sound -framework SDL
	INCLUDEPATH += \
		./Frameworks/SDL.framework/Headers \
		./Frameworks/SDL_mixer.framework/Headers \
		./Frameworks/smpeg.framework/Headers \
		./Frameworks/SDL_sound.framework/Headers
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
	QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
} else {
	CONFIG += link_pkgconfig
	PKGCONFIG += sdl
	# Normally we would use pkg-config for SDL_mixer too, but it has to appear
	# in the linker flags before SDL_sound, which lacks pkg-config support, or
	# else we crash.
	LIBS += -lSDL_mixer -lSDL_sound
}
win32 {
	LIBS += -lvorbisfile -lvorbis -logg
}

RESOURCES += resources.qrc

#CONFIG += debug
#CONFIG += release

# We use warn_off to allow only default warnings, not to supress them all.
QMAKE_CXXFLAGS_WARN_OFF =
QMAKE_CFLAGS_WARN_OFF =

*-g++* {
	# Tads 3 has problems with strict aliasing rules in GCC
	QMAKE_CXXFLAGS_RELEASE += -fno-strict-aliasing
	QMAKE_CXXFLAGS_DEBUG += -fno-strict-aliasing

	# Avoid a flood of "unused paramater" warnings.
	QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
	QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
}

!macx {
	QMAKE_CFLAGS += $$system(smpeg-config --cflags)
	QMAKE_CXXFLAGS += $$system(smpeg-config --cflags)
}

# This is just a hack to make code completion work OK in Qt Creator.
INCLUDEPATH += /usr/include/SDL /usr/include/smpeg
INCLUDEPATH -= /usr/include/SDL /usr/include/smpeg

# Where to find the portable Tads sources.
T2DIR = tads2
T3DIR = tads3
HTDIR = htmltads

DEFINES += \
	QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII \
	TROLLTECH_QT \
	_M_QT \
	T3_COMPILING_FOR_HTML \
	VM_FLAT_POOL \
	USE_HTML

CONFIG(release, debug|release) {
	DEFINES += VMGLOB_VARS
} else {
	DEFINES += VMGLOB_PARAM TADSHTML_DEBUG T3_DEBUG
}

macx|win32 {
	DEFINES += OS_NO_TYPES_DEFINED
	TARGET = QTads
} else {
	TARGET = qtads
}

# These macros override some default buffer-sizes the T3VM uses for UNDO
# operations.  QTads runs on systems with a lot of RAM (compared to DOS), so we
# could increase these buffers.  Normally, the player would only be able to
# UNDO turns in a T3 game only 10 times or so.  With larger UNDO-buffers, more
# turns are possible.  See tads3/vmparam.h for an explanation.  Note that
# VM_UNDO_MAX_SAVEPTS can't be larger than 255.
#
# This, however, incurs quite a bit of overhead, slowing game response down
# over time, which is why we disable this for now.
#DEFINES += \
#	VM_STACK_SIZE=65536 \
#	VM_STACK_RESERVE=1024 \
#	VM_UNDO_MAX_RECORDS=65536 \
#	VM_UNDO_MAX_SAVEPTS=255

INCLUDEPATH += src $$T2DIR $$T3DIR $$HTDIR
DEPENDPATH += src $$T2DIR $$T3DIR $$HTDIR
OBJECTS_DIR = obj
MOC_DIR = tmp
UI_DIR = tmp

# DISTFILES += AUTHORS COPYING ChangeLog INSTALL NEWS README SOURCE_README TODO
# Extra files to include in the distribution tar.gz archive (by doing a
# "make dist").
# DISTFILES += \
# $$T2DIR/LICENSE.TXT \
# $$T2DIR/tadsver.htm \
# $$T2DIR/portnote.txt \
# $$T3DIR/portnote.htm \
# $$HTDIR/LICENSE.TXT \
# $$HTDIR/notes/porting.htm

FORMS += \
	src/confdialog.ui \
	src/gameinfodialog.ui \
	src/aboutqtadsdialog.ui

# QTads headers
HEADERS += \
	src/osqt.h \
	src/hos_qt.h \
	src/h_qt.h \
	src/oswin.h \
	src/hos_w32.h \
	src/missing.h \
	src/globals.h \
	src/sysfont.h \
	src/sysframe.h \
	src/syswinaboutbox.h \
	src/syswingroup.h \
	src/syswin.h \
	src/syswininput.h \
	src/sysimagejpeg.h \
	src/sysimagepng.h \
	src/sysimagemng.h \
	src/syssoundogg.h \
	src/syssoundwav.h \
	src/syssoundmpeg.h \
	src/syssoundmidi.h \
	src/qtadscharmap.h \
	src/qtadsimage.h \
	src/qtadssound.h \
	src/dispwidget.h \
	src/dispwidgetinput.h \
	src/qtadshostifc.h \
	src/qtadstimer.h \
	src/vmuni.h \
	src/confdialog.h \
	src/settings.h \
	src/gameinfodialog.h \
	src/kcolorbutton.h \
	src/aboutqtadsdialog.h

# QTads sources.
SOURCES += \
	src/oemqt.c \
	src/osqt.cc \
	src/hos_qt.cc \
	src/globals.cc \
	src/sysframe.cc \
	src/syswingroup.cc \
	src/syswin.cc \
	src/syswinaboutbox.cc \
	src/syswininput.cc \
	src/sysimage.cc \
	src/syssound.cc \
	src/missing.cc \
	src/charmap.cc \
	src/qtadscharmap.cc \
	src/qtadsimage.cc \
	src/qtadssound.cc \
	src/main.cc \
	src/dispwidget.cc \
	src/dispwidgetinput.cc \
	src/confdialog.cc \
	src/settings.cc \
	src/gameinfodialog.cc \
	src/kcolorbutton.cc \
	src/aboutqtadsdialog.cc

# Portable Tads headers.  We simply include every header from the Tads
# directories.  It's sub-optimal, but the safest solution.
# Tads 2 headers.
HEADERS += $$T2DIR/*.h

# Tads 3 headers.
HEADERS += $$T3DIR/*.h

# HTML TADS headers.
HEADERS += $$HTDIR/*.h

# Portable Tads sources.  We always know which are needed.
# HTML Tads sources.
SOURCES += \
	$$HTDIR/tadshtml.cpp \
	$$HTDIR/tadsrtyp.cpp \
	$$HTDIR/htmlprs.cpp \
	$$HTDIR/htmltags.cpp \
	$$HTDIR/htmlfmt.cpp \
	$$HTDIR/htmldisp.cpp \
	$$HTDIR/htmlsys.cpp \
	$$HTDIR/htmltxar.cpp \
	$$HTDIR/htmlinp.cpp \
	$$HTDIR/htmlrc.cpp \
	$$HTDIR/htmlrf.cpp \
	$$HTDIR/htmlhash.cpp \
	$$HTDIR/oshtml.cpp \
	$$HTDIR/htmlsnd.cpp

# Tads2 sources.
SOURCES += \
	$$T2DIR/argize.c \
	$$T2DIR/ler.c \
	$$T2DIR/mcm.c \
	$$T2DIR/mcs.c \
	$$T2DIR/mch.c \
	$$T2DIR/obj.c \
	$$T2DIR/cmd.c \
	$$T2DIR/errmsg.c \
	$$T2DIR/fioxor.c \
	$$T2DIR/oserr.c \
	$$T2DIR/runstat.c \
	$$T2DIR/fio.c \
	$$T2DIR/getstr.c \
	$$T2DIR/cmap.c \
	$$T2DIR/askf_os.c \
	$$T2DIR/indlg_os.c \
	$$T2DIR/osifc.c \
	$$T2DIR/dat.c \
	$$T2DIR/lst.c \
	$$T2DIR/run.c \
	$$T2DIR/out.c \
	$$T2DIR/voc.c \
	$$T2DIR/bif.c \
	$$T2DIR/suprun.c \
	$$T2DIR/regex.c \
	$$T2DIR/vocab.c \
	$$T2DIR/execmd.c \
	$$T2DIR/ply.c \
	$$T2DIR/qas.c \
	$$T2DIR/trd.c \
	$$T2DIR/dbgtr.c \
	$$T2DIR/linfdum.c \
	$$T2DIR/osrestad.c \
	$$T2DIR/bifgdum.c \
	$$T2DIR/output.c

# Tads3 sources.
SOURCES += \
	$$T3DIR/askf_os3.cpp \
	$$T3DIR/gameinfo.cpp \
	$$T3DIR/indlg_os3.cpp \
	$$T3DIR/resfind.cpp \
	$$T3DIR/resnoexe.cpp \
	$$T3DIR/resload.cpp \
	$$T3DIR/std.cpp \
	$$T3DIR/utf8.cpp \
	$$T3DIR/vmanonfn.cpp \
	$$T3DIR/vmbif.cpp \
	$$T3DIR/vmbifl.cpp \
	$$T3DIR/vmbifregx.cpp \
	$$T3DIR/vmbift3.cpp \
	$$T3DIR/vmbiftad.cpp \
	$$T3DIR/vmbiftio.cpp \
	$$T3DIR/vmbiftix.cpp \
	$$T3DIR/vmbignum.cpp \
	$$T3DIR/vmbt3_nd.cpp \
	$$T3DIR/vmbytarr.cpp \
	$$T3DIR/vmcfgfl.cpp \
	$$T3DIR/vmcoll.cpp \
	$$T3DIR/vmconhmp.cpp \
	$$T3DIR/vmconhtm.cpp \
	$$T3DIR/vmconsol.cpp \
	$$T3DIR/vmcrc.cpp \
	$$T3DIR/vmcset.cpp \
	$$T3DIR/vmdict.cpp \
	$$T3DIR/vmerr.cpp \
	$$T3DIR/vmerrmsg.cpp \
	$$T3DIR/vmfile.cpp \
	$$T3DIR/vmfilobj.cpp \
	$$T3DIR/vmfunc.cpp \
	$$T3DIR/vmglob.cpp \
	$$T3DIR/vmgram.cpp \
	$$T3DIR/vmhash.cpp \
	$$T3DIR/vmimage.cpp \
	$$T3DIR/vmimg_nd.cpp \
	$$T3DIR/vminit.cpp \
	$$T3DIR/vminitfl.cpp \
	$$T3DIR/vmini_nd.cpp \
	$$T3DIR/vmintcls.cpp \
	$$T3DIR/vmiter.cpp \
	$$T3DIR/vmlookup.cpp \
	$$T3DIR/vmlst.cpp \
	$$T3DIR/vmmain.cpp \
	$$T3DIR/vmmcreg.cpp \
	$$T3DIR/vmmeta.cpp \
	$$T3DIR/vmobj.cpp \
	$$T3DIR/vmpat.cpp \
	$$T3DIR/vmpool.cpp \
	$$T3DIR/vmpoolfl.cpp \
	$$T3DIR/vmregex.cpp \
	$$T3DIR/vmrun.cpp \
	$$T3DIR/vmrunsym.cpp \
	$$T3DIR/vmsa.cpp \
	$$T3DIR/vmsave.cpp \
	$$T3DIR/vmsort.cpp \
	$$T3DIR/vmsortv.cpp \
	$$T3DIR/vmsrcf.cpp \
	$$T3DIR/vmstack.cpp \
	$$T3DIR/vmstr.cpp \
	$$T3DIR/vmstrcmp.cpp \
	$$T3DIR/vmtobj.cpp \
	$$T3DIR/vmtype.cpp \
	$$T3DIR/vmtypedh.cpp \
	$$T3DIR/vmundo.cpp \
	$$T3DIR/vmvec.cpp
