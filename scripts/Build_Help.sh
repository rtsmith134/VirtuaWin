#
# Script for automating the build and release process as much as possible.
#
# Preconditions:
# - shell already configured to run gcc with MinGW and CVS (i.e. CVSROOT and CVS_RSH set)
# - HTML Help compiler (set HELPCOMPILER)
# - Inno Setup (set SETUPCOMPILER if not in standard install location)
# - emacs (or set EDITOR to another good text editor)
# - zip (set ZIP) or winzip with commandline extension
# - ncftp (if uploading to SF)
#
# Note: The CVS program must be windows so line termination will be Windows format (i.e. \r\n) otherwise
#       the source package will be wrong and the README files will not be very readable in notepad.
#       This means that Cygwin's cvs cannot be used!!
#
# Variables
# MSYS need windows style /? arguments to be double slashes, i.e. //? use a varable to handle this.
# sed is used to set the version, but it must not change the line termination to unix style, use option -b
# for cygwin's sed and -c for msys.
SLASH="/"
CDRIVE="/cygdrive/c"
VWPROGRAMFILES="${CDRIVE}/Program Files (x86)"
# if the uname contains MINGW then we are using msys so change CDRIVE
if [ `uname | sed -e "s/^MINGW.*/MINGW/"` == 'MINGW' ] ; then
    SLASH="//"
    CDRIVE="/c"
fi

if [ -z "$EDITOR" ] ; then
    EDITOR=emacs
fi
if [ -z "$HELPCOMPILER" ] ; then
    HELPCOMPILER="${VWPROGRAMFILES}/HTML Help Workshop/hhc"
fi
if [ -z "$SETUPCOMPILER" ] ; then
    SETUPCOMPILER="${VWPROGRAMFILES}/Inno Setup 5/Compil32"
fi
if [ -z "$WINZIP" ] ; then
    WINZIP="${VWPROGRAMFILES}/WinZip/wzzip"
fi

read -p "Compile source? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo compiling helpfile
    cd Help
    "$HELPCOMPILER" virtuawin.hhp
fi

