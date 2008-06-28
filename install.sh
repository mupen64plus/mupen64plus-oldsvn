#!/bin/bash

#
# mupen64plus install script
#

if [ -x /usr/bin/ginstall ]; then
	INSTALL=/usr/bin/ginstall
elif [ -x /bin/ginstall ]; then
	INSTALL=/bin/ginstall
elif [ -x /usr/bin/install ]; then
	INSTALL=/usr/bin/install
elif [ -x /bin/install ]; then
	INSTALL=/bin/install
else
	INSTALL=install
fi

usage()
{
	echo "usage: $(basename $0) [prefix]"
	echo -e "\tprefix - install prefix (default: /usr/local)"
	echo "To install KDE4 version, use: GUI=KDE4 $(basename $0)"
}

if [ $# -gt 1 ]; then
	usage
	exit 1
fi

if [ $# -eq 1 ]; then
	PREFIX=$1
else
	PREFIX=/usr/local
fi

BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/man/man1
INSTALLDIR=${PREFIX}/share/mupen64plus

echo "Installing Mupen64Plus to $PREFIX"
$INSTALL -D -m 0755 mupen64plus "${BINDIR}/mupen64plus" || exit $?
$INSTALL -d -v "${INSTALLDIR}" || exit $?
$INSTALL -d -v "${INSTALLDIR}/config" || exit $?
$INSTALL -m 0644 config/* "${INSTALLDIR}/config" || exit $?
$INSTALL -m 0644 mupen64plus.ini "${INSTALLDIR}/" || exit $?
$INSTALL -d -v "${INSTALLDIR}/doc" || exit $?
$INSTALL -m 0644 doc/HiRezTexture.txt "${INSTALLDIR}/doc" || exit $?
$INSTALL -d -v "${MANDIR}" || exit $?
$INSTALL -m 0644 doc/mupen64plus.1.gz "${MANDIR}" || exit $?
$INSTALL -d -v "${INSTALLDIR}/fonts" || exit $?
$INSTALL -m 0644 fonts/* "${INSTALLDIR}/fonts" || exit $?
$INSTALL -d -v "${INSTALLDIR}/icons" || exit $?
$INSTALL -m 0644 icons/*.png "${INSTALLDIR}/icons" || exit $?
$INSTALL -m 0644 icons/*.xpm "${INSTALLDIR}/icons" || exit $?

$INSTALL -d -v "${INSTALLDIR}/icons/32x32" || exit $?
$INSTALL -m 0644 icons/32x32/* "${INSTALLDIR}/icons/32x32" || exit $?

$INSTALL -d -v "${INSTALLDIR}/icons/22x22" || exit $?
$INSTALL -m 0644 icons/22x22/* "${INSTALLDIR}/icons/22x22" || exit $?

$INSTALL -d -v "${INSTALLDIR}/icons/16x16" || exit $?
$INSTALL -m 0644 icons/16x16/* "${INSTALLDIR}/icons/16x16" || exit $?

$INSTALL -d -v "${INSTALLDIR}/lang" || exit $?
$INSTALL -m 0644 lang/* "${INSTALLDIR}/lang" || exit $?
$INSTALL -d -v "${INSTALLDIR}/plugins" || exit $?
$INSTALL -m 0755 plugins/* "${INSTALLDIR}/plugins" || exit $?
if [ "${GUI}" == "KDE4" ]; then
    $INSTALL -m 0644 main/gui_kde4/mupen64plusui.rc "${INSTALLDIR}" || exit $?
fi
echo "Done."

