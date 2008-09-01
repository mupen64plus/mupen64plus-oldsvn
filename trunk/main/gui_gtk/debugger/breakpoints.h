/**
 * Mupen64Plus - main/gui_gtk/debugger/breakpoints.h
 *
 * Copyright (C) 2002 DavFr - robind@esiee.fr
 * 
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

#ifndef GUIGTK_BREAKPOINTS_H
#define GUIGTK_BREAKPOINTS_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "debugger.h"

#include "ui_clist_edit.h"


int breakpoints_opened;
int breakpoints_editing; //1 when editing a breakpoint, 0 when adding


GtkWidget *winBreakpoints;

void init_breakpoints();
void get_breakpoint_display_string(char* buf, breakpoint* bpt);
int add_breakpoint( uint32 address );
void update_breakpoints();

#endif  // BREAKPOINTS_H
