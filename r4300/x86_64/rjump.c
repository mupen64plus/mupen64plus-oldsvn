/**
 * Mupen64 - r4300/x86_64/rjump.c
 * Copyright (C) 2007 Richard Goedeken, Hacktarux
 * Based on code written by Hacktarux, Copyright (C) 2002
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * Forum homepage: http://www.emutalk.net/forumdisplay.php?f=50
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


#include <stdlib.h>

#include "../recomp.h"
#include "../r4300.h"
#include "../macros.h"
#include "../ops.h"
#include "../recomph.h"

void dyna_jump()
{
}

static long save_rbp = 0;
static long save_rsp = 0;
static long save_rip = 0;

void dyna_start(void (*code)())
{
}

void dyna_stop()
{
}

