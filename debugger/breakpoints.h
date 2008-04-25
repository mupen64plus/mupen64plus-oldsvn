/**
 * Mupen64 - breakpoints.h
 * Copyright (C) 2002 DavFr - robind@esiee.fr
 *
 * If you want to contribute to this part of the project please
 * contact me (or Hacktarux) first.
 * 
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
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

#define BPT_FLAG_ENABLED        0x01
#define BPT_FLAG_CONDITIONAL	0x02
#define BPT_FLAG_COUNTER	0x04

#define BPT_CHECK_FLAG(a, b)  ((a.flags & b) == b)
#define BPT_SET_FLAG(a, b)    a.flags = (a.flags | b);
#define BPT_CLEAR_FLAG(a, b)  a.flags = (a.flags & (~b));
#define BPT_TOGGLE_FLAG(a, b) a.flags = (a.flags ^ b);

typedef struct _breakpoint {
    uint32 address;
    uint32 flags;
    //uint32 condition;  //Placeholder for breakpoint condition
    } breakpoint;

extern int g_NumBreakpoints;
extern breakpoint g_Breakpoints[];


int add_breakpoint( uint32 address );
void remove_breakpoint_by_address( uint32 address );
void enable_breakpoint( int breakpoint );
void disable_breakpoint( int breakpoint );
int check_breakpoints( uint32 address );
int lookup_breakpoint( uint32 address );

#endif  // BREAKPOINTS_H
