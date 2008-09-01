#!/bin/bash

#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   Mupen64plus - install.sh                                              *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2007-2008 Richard42                                     *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
# Mupen64plus uninstall script.

RM=/bin/rm

usage()
{
	echo "usage: $(basename $0) [PREFIX] [SHAREDIR] [BINDIR] [LIBDIR] [MANDIR]"
	echo -e "\tPREFIX   - installation directories prefix (default: /usr/local)"
	echo -e "\tSHAREDIR - path to Mupen64Plus shared data files (default: \$PREFIX/share/mupen64plus)"
	echo -e "\tBINDIR   - path to Mupen64Plus binary program files (default: \$PREFIX/bin)"
	echo -e "\tLIBDIR   - path to Mupen64Plus plugins (default: \$SHAREDIR/plugins)"
	echo -e "\tMANDIR   - path to manual files (default: \$PREFIX/man/man1)"
}

if [ $# -gt 5 ]; then
	usage
	exit 1
fi

if [ $# -gt 0 ]; then
	PREFIX=$1
else
	PREFIX=/usr/local
fi

if [ $# -gt 1 ]; then
	SHAREDIR=$2
else
	SHAREDIR=${PREFIX}/share/mupen64plus
fi

if [ $# -gt 2 ]; then
	BINDIR=$3
else
	BINDIR=${PREFIX}/bin
fi

if [ $# -gt 3 ]; then
	LIBDIR=$4
else
	LIBDIR=${SHAREDIR}/plugins
fi

if [ $# -gt 4 ]; then
	MANDIR=$5
else
	MANDIR=${PREFIX}/man/man1
fi

echo "Uninstalling Mupen64Plus from $PREFIX"
echo "Removing ${BINDIR}/mupen64plus"
$RM -f "${BINDIR}/mupen64plus" || exit $?
echo "Removing ${LIBDIR}"
$RM -rf "${LIBDIR}" || exit $?
echo "Removing ${SHAREDIR}"
$RM -rf "${SHAREDIR}" || exit $?
echo "Removing man page"
$RM -f "${MANDIR}/mupen64plus.1.gz" || exit $?
echo "Done."

