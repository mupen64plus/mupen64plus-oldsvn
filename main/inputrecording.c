/**
 * Mupen64Plus - inputrecording.c
 * Copyright (C) 2008 okaygo
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/


// Rename to input_handler.c

#include "plugin.h"
#include "rom.h"
#include "savestates.h"
#include "main.h"
#include "inputrecording.h"
#include "../memory/memory.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int l_CurrentSample = 0;
int l_CurrentVI = 0;

void _NewVI()
{
	l_CurrentVI++;
	// What other functions do we need to hook?
}

void _GetKeys( int Control, BUTTONS *Keys )
{
	// Since we handle input here now, we always want to start with getKeys.
    getKeys(Control, &Keys);
    
    if (g_EmulatorPlayback)
    {
        // read keys from file.
    }
    
    if (g_EmulatorRecording)
    {
        // record keys
    }
}  