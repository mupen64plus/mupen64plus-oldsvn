#!/bin/sh

#
# mupen64plus uninstall script
#

RM=/bin/rm

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
MANDIR=${PREFIX}/man/man1
INSTALLDIR=${PREFIX}/share/mupen64plus

echo "Uninstalling Mupen64Plus from $PREFIX"
echo "Removing ${BINDIR}/mupen64plus"
$RM -f "${BINDIR}/mupen64plus" || exit $?
echo "Removing ${INSTALLDIR}"
$RM -rf "${INSTALLDIR}" || exit $?
echo "Removing man page"
$RM -f "${MANDIR}/mupen64plus.1.gz" || exit $?
echo "Done."
