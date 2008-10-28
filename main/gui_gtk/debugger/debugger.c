/*
 * Debugger for Mupen64Plus main/gui_gtk/debugger/debugger.c
 * Copyright (C) 2002 davFr - robind@esiee.fr
 * Copyright (C) 2008 HyperHacker (at gmail, dot com)
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

#include <glib.h>

#include "debugger.h"

#include "../main_gtk.h"

GdkColor    color_modif,    // Color of modified register.
                   color_ident;    // Unchanged register.

GtkWidget   *winRegisters;

PangoFontDescription *debugger_font_desc;

void init_debugger_frontend()
{
    color_modif.red = 0x8000;
    color_modif.green = 0xD000;
    color_modif.blue = 0xFFFF;

    color_ident.red = 0xFFFF;
    color_ident.green = 0xFFFF;
    color_ident.blue = 0xFFFF;

    debugger_font_desc = pango_font_description_from_string("Monospace 9");

    Uint32 self = SDL_ThreadID();
    /* If we're calling from a thread other than the main gtk thread, take gdk lock. */

    if (self != g_GuiThreadID)
        gdk_threads_enter();

    init_registers();
    init_desasm();
     /*
    init_breakpoints();
    init_memedit();
    init_varlist();
    init_TLBwindow();
    */

    if (self != g_GuiThreadID)
        gdk_threads_leave();

    if(dynacore!=0)
      error_message("You are trying to use the debugger with the dynamic-recompiler.  This is unfinished, and many features of the debugger WILL NOT WORK PROPERLY.  If you have a bug you'd like to report, try it first using the debugger with either of the interpreted cores.");

}

void update_debugger_frontend()
// Update debugger state and display.
{
    Uint32 self = SDL_ThreadID();
    /* If we're calling from a thread other than the main gtk thread, take gdk lock. */

    if (self != g_GuiThreadID)
        gdk_threads_enter();

    if(registers_opened)
        update_registers();

    if(desasm_opened)
        update_desasm(PC->addr);

    if(regTLB_opened)
        update_TLBwindow();

    if(memedit_opened)
        update_memory_editor();

    if(varlist_opened)
        update_varlist();

    if (self != g_GuiThreadID)
        gdk_threads_leave();
}


//Runs each VI for auto-updating views
void debugger_frontend_vi()
{
    if(memedit_auto_update && memedit_opened) {
        gdk_threads_enter();
        update_memory_editor();
        gdk_threads_leave();
    }
    
    if(varlist_auto_update && varlist_opened) {
        gdk_threads_enter();
        update_varlist();
        gdk_threads_leave();
    }
}

