#!/bin/sh

#
# mupen64plus install script
#

INSTALL=ginstall

usage()
{
	echo "usage: $(basename $0) [prefix]"
	echo -e "\tprefix - install prefix (default: /usr/local)"
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
INSTALLDIR=${PREFIX}/share/mupen64plus

echo "Installing Mupen64Plus to $PREFIX"
$INSTALL -D -s -m 0755 mupen64plus "${BINDIR}/mupen64plus" || exit $?
$INSTALL -d -v "${INSTALLDIR}" || exit $?
$INSTALL -d -v "${INSTALLDIR}/config" || exit $?
$INSTALL -m 0644 config/* "${INSTALLDIR}/config" || exit $?
$INSTALL -d -v "${INSTALLDIR}/doc" || exit $?
$INSTALL -m 0644 doc/* "${INSTALLDIR}/doc" || exit $?
$INSTALL -d -v "${INSTALLDIR}/icons" || exit $?
$INSTALL -m 0644 icons/* "${INSTALLDIR}/icons" || exit $?
$INSTALL -d -v "${INSTALLDIR}/lang" || exit $?
$INSTALL -m 0644 lang/* "${INSTALLDIR}/lang" || exit $?
$INSTALL -d -v "${INSTALLDIR}/plugins" || exit $?
$INSTALL -m 0755 plugins/* "${INSTALLDIR}/plugins" || exit $?
echo "Done."
