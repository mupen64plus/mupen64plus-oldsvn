#!/bin/sh
#
# mupen64plus install script
#
# Copyright 2007, 2008 The Mupen64Plus Development Team
# Modifications Copyright 2008 Guido Berhoerster
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

set -e

export PATH=/bin:/usr/bin

if command -v ginstall >/dev/null 2>&1; then
    INSTALL=ginstall
elif install --help >/dev/null 2>&1; then
    INSTALL=install
else
    printf "error: GNU install not found\n" >&2
    exit 1
fi

usage()
{
printf "usage: $(basename $0) [PREFIX] [SHAREDIR] [BINDIR] [LIBDIR] [MANDIR] [APPLICATIONSDIR]
\tPREFIX   - installation directories prefix (default: /usr/local)
\tSHAREDIR - path to Mupen64Plus shared data files (default: \$PREFIX/share/mupen64plus)
\tBINDIR   - path to Mupen64Plus binary program files (default: \$PREFIX/bin)
\tLIBDIR   - path to Mupen64Plus plugins (default: \$SHAREDIR/plugins)
\tMANDIR   - path to manual files (default: \$PREFIX/man/man1)
\tAPPLICATIONSDIR - path to place .desktop file in (default: \$PREFIX/share/applications)
"
}

if [ $# -gt 6 ]; then
	usage
	exit 1
fi

PREFIX="${1:-/usr/local}"
SHAREDIR="${2:-${PREFIX}/share/mupen64plus}"
BINDIR="${3:-${PREFIX}/bin}"
LIBDIR="${4:-${SHAREDIR}/plugins}"
MANDIR="${5:-${PREFIX}/man/man1}"
APPLICATIONSDIR="${6:-${PREFIX}/share/applications}"

printf "Installing Mupen64Plus to ${PREFIX}\n"
$INSTALL -d -v "${SHAREDIR}"
$INSTALL -d -v "${SHAREDIR}/config"
$INSTALL -m 0644 config/* "${SHAREDIR}/config"
$INSTALL -m 0644 mupen64plus.ini "${SHAREDIR}/"
$INSTALL -d -v "${SHAREDIR}/doc"
$INSTALL -m 0644 doc/HiRezTexture.txt "${SHAREDIR}/doc"
$INSTALL -d -v "${SHAREDIR}/fonts"
$INSTALL -m 0644 fonts/* "${SHAREDIR}/fonts"
$INSTALL -d -v "${SHAREDIR}/icons"
$INSTALL -m 0644 icons/*.png "${SHAREDIR}/icons"
$INSTALL -d -v "${SHAREDIR}/icons/32x32"
$INSTALL -m 0644 icons/32x32/* "${SHAREDIR}/icons/32x32"
$INSTALL -d -v "${SHAREDIR}/icons/22x22"
$INSTALL -m 0644 icons/22x22/* "${SHAREDIR}/icons/22x22"
$INSTALL -d -v "${SHAREDIR}/icons/16x16"
$INSTALL -m 0644 icons/16x16/* "${SHAREDIR}/icons/16x16"
$INSTALL -d -v "${SHAREDIR}/lang"
$INSTALL -m 0644 lang/* "${SHAREDIR}/lang"
$INSTALL -D -m 0755 mupen64plus "${BINDIR}/mupen64plus"
$INSTALL -d -v "${MANDIR}"
$INSTALL -m 0644 doc/mupen64plus.1.gz "${MANDIR}"
$INSTALL -d -v "${LIBDIR}"
$INSTALL -m 0644 plugins/* "${LIBDIR}"
$INSTALL -d -v "${APPLICATIONSDIR}"
$INSTALL -m 0644 mupen64plus.desktop "${APPLICATIONSDIR}"


printf "Done.\n"

