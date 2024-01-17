First of all, if you're on Gentoo Linux or some other portage-based system, then
there's no need to continue; QTads is in portage and you can install it normally
with:

    emerge games-engines/qtads

For all other Linux distributions, a self-contained AppImage binary is provided
on the download page. If you still want to build QTads yourself from source,
read on.

# Dependencies

For QTads to build correctly, you will need to have the Qt5 libraries along with
their development headers/tools installed. You need at least Qt 5.5. (As of
January 2024, QTads is not compatible with Qt6.) 
https://github.com/realnc/qtads/issues/23

If audio support is enabled, you'll also need:

    SDL 2
    libsndfile
    libfluidsynth (version 2.x)
    libvorbisfile
    libmpg123

Most Linux distributions provide the development versions in packages that have
"-dev" appended to the package name.

On macOS, you can use Homebrew to install dependencies. https://brew.sh/

    brew install qt@5

Homebrew installs QT 5 "keg-only," and it will include a line like this,
explaining how to add it to your $PATH.

    export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"

# Building

QTads uses qmake as its build system.  To build it:

    cd <directory where you unpacked the QTads sources>
    qmake PREFIX=/usr/local
    make -j4

Replace "-j4" with the amount of CPUs on your machine. "-j2" for a dual-core
CPU, for example. Specifying PREFIX is optional. If you omit it, the default
is /usr/local.

This will create a "qtads" binary which you can then copy and run from anywhere.
There are no files that need to be installed, although can do a full
installation as well with:

    make install

This will install all files into /usr/local, unless you specified a different
PREFIX. This will also install a desktop file and MIME information data for
better integration with your desktop (if your deskop is monitoring /usr/local
for such files, that is.) You might need to run the install command as root, if
you don't have write permission for the specified PREFIX.

You can disable audio support when building by running qmake like this instead:

    qmake -config disable-audio

This will produce a version of QTads that does not support audio.
