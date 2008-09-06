/*
 * Mupen64Plus - debugger/debugger.c
 *
 * Copyright (C) 2002 davFr - robind@esiee.fr
 * Copyright (C) 2008 DarkJezter
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

//#include <glib.h>
#include "debugger.h"

// State of the Emulation Thread:
// 0 -> pause, 2 -> run.

int g_DebuggerEnabled = 0;    // wether the debugger is enabled or not
int debugger_mode;
int run;

SDL_cond  *debugger_done_cond;
SDL_mutex *mutex;

uint32 previousPC;

//]=-=-=-=-=-=-=-=-=-=-=[ Initialisation du Debugger ]=-=-=-=-=-=-=-=-=-=-=-=[

void init_debugger()
{
    debugger_mode = 1;
    run = 0;

    init_debugger_frontend();

    init_host_disassembler();

    mutex = SDL_CreateMutex();
    debugger_done_cond = SDL_CreateCond();
}

void uninit_debugger()
{
    SDL_DestroyMutex(mutex);
    mutex = NULL;
    SDL_DestroyCond(debugger_done_cond);
    debugger_done_cond = NULL;
}

//]=-=-=-=-=-=-=-=-=-=-=-=-=[ Mise-a-Jour Debugger ]=-=-=-=-=-=-=-=-=-=-=-=-=[

void update_debugger()
// Update debugger state and display.
// Should be called after each R4300 instruction.
{
    int bpt;
    
    if(run==2) {
        bpt = check_breakpoints(PC->addr);
        if( bpt==-1 ) {
            previousPC = PC->addr;
            return;
        }
        else {
            run = 0;
            switch_button_to_run();
            
            if(BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_LOG))
                log_breakpoint(PC->addr, BPT_FLAG_EXEC, 0);
        }
    }
    else if ( previousPC == PC->addr )
    {
        return;
    }
    update_debugger_frontend();

    previousPC = PC->addr;
    // Emulation thread is blocked until a button is clicked.
    SDL_CondWait(debugger_done_cond, mutex);
}

