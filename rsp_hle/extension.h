/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rsp plugin - extension.h                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Dylan Wagstaff (Pyromanik)                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <dirent.h>
#include "wintypes.h"

char audioname[100];
extern BOOL AudioHle;    /* TRUE = audio lists aren't processed in the rsp plugin            */
extern BOOL GraphicsHle; /* TRUE = gfx lists aren't processed in the rsp pluign              */
extern BOOL SpecificHle; /* TRUE = audio lists are processed by a specific audio plugin      */
                  /* otherwise it's processed by the audio plugin choosen in the emu  */
void startup();
BOOL loadPlugin();
char* next_plugin();

extern char LibPath[PATH_MAX];
