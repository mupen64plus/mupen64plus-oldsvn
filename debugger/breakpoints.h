/**
 * Mupen64Plus - debugger/breakpoints.c
 *
 * Copyright (C) 2008 DarkJezter
 * Copyright (C) 2008 HyperHacker (at gmail dot com)
 *
 * Mupen64 homepage: http://code.google.com/p/mupen64plus/
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence.
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

#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "debugger.h"

#define BREAKPOINTS_MAX_NUMBER  128

#define BPT_FLAG_ENABLED                0x01
#define BPT_FLAG_CONDITIONAL            0x02
#define BPT_FLAG_COUNTER        0x04
#define BPT_FLAG_READ           0x08
#define BPT_FLAG_WRITE          0x10
#define BPT_FLAG_EXEC           0x20
#define BPT_FLAG_LOG            0x40 //Log to the console when this breakpoint hits.

#define BPT_CHECK_FLAG(a, b)  ((a.flags & b) == b)
#define BPT_SET_FLAG(a, b)    a.flags = (a.flags | b);
#define BPT_CLEAR_FLAG(a, b)  a.flags = (a.flags & (~b));
#define BPT_TOGGLE_FLAG(a, b) a.flags = (a.flags ^ b);

typedef struct _breakpoint {
    uint32 address; 
    uint32 endaddr;
    uint32 flags;
    //uint32 condition;  //Placeholder for breakpoint condition
    } breakpoint;

extern int g_NumBreakpoints;
extern breakpoint g_Breakpoints[];


int add_breakpoint( uint32 address );
int add_breakpoint_struct(breakpoint* newbp);
void remove_breakpoint_by_address( uint32 address );
void enable_breakpoint( int breakpoint );
void disable_breakpoint( int breakpoint );
int check_breakpoints( uint32 address );
int check_breakpoints_on_mem_access( uint32 address, uint32 size, uint32 flags );
int lookup_breakpoint( uint32 address, uint32 flags );
int log_breakpoint(uint32 PC, uint32 Flag, uint32 Access);

#endif  // BREAKPOINTS_H
