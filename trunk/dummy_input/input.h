/**
 * Mupen64Plus - input.h
 * Copyright (C) 2008 Scott Gorman (okaygo)
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA
 *
**/

#ifndef INPUT_H
#define INPUT_H

#ifndef __LINUX__
#include <windows.h>
#else
# include "../main/winlnxdefs.h"
#endif

extern char pluginName[];
extern char configdir[PATH_MAX];

#endif

