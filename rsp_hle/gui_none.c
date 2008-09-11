/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - no_gui.c                                                *
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

#include <stdio.h>
#include "gui.h"

BOOL pseudospecifichle;
BOOL pseudoaudiohle;
BOOL pseudographicshle;
char pseudoaudioname[100];

void configDialog(HWND handle)
{
    handle = 0; /* Keeps the compiler quiet about unused variables. */
    printf("[RSP] RSP Configuration:\nPlease manually edit the config file, the GUI is disabled.\n");
}

void MessageBox(HWND handle, char* message, char* title, int flags)
{
    handle = 0; /* Keeps the compiler quiet about unused variables */
    flags = 0;  /* while still maintaining compatibility with GUI functions. */
    printf("[RSP] %s:\n%s\n", title, message);
}