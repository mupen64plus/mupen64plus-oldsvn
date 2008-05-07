/**
 * Mupen64 - breakpoints.c
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


#include "breakpoints.h"


int g_NumBreakpoints=0;
breakpoint g_Breakpoints[BREAKPOINTS_MAX_NUMBER];


int add_breakpoint( uint32 address )
{
    if( g_NumBreakpoints == BREAKPOINTS_MAX_NUMBER ) {
        printf("BREAKPOINTS_MAX_NUMBER have been reached.\n");//REMOVE ME
        return -1;
    }
    g_Breakpoints[g_NumBreakpoints].address=address;
    g_Breakpoints[g_NumBreakpoints].endaddr=address;
    BPT_SET_FLAG(g_Breakpoints[g_NumBreakpoints], BPT_FLAG_EXEC);

    enable_breakpoint(g_NumBreakpoints);

    return g_NumBreakpoints++;

}

int add_breakpoint_struct(breakpoint* newbp)
{
	 if( g_NumBreakpoints == BREAKPOINTS_MAX_NUMBER ) {
        printf("BREAKPOINTS_MAX_NUMBER have been reached.\n");//REMOVE ME
        return -1;
    }
	memcpy(&g_Breakpoints[g_NumBreakpoints], newbp, sizeof(breakpoint));
	printf("newbp %08X - %08X\n", g_Breakpoints[g_NumBreakpoints].address, g_Breakpoints[g_NumBreakpoints].endaddr);
    return g_NumBreakpoints++;
}

void enable_breakpoint( int breakpoint )
{
    BPT_SET_FLAG(g_Breakpoints[breakpoint], BPT_FLAG_ENABLED);
	//TODO: Must setup breakpoint checks in r4300
}

void disable_breakpoint( int breakpoint )
{
    BPT_CLEAR_FLAG(g_Breakpoints[breakpoint], BPT_FLAG_ENABLED);
	//TODO: Must setup breakpoint checks in r4300
}

void remove_breakpoint_by_num( int bpt )
{
    g_Breakpoints[bpt]=g_Breakpoints[--g_NumBreakpoints];
}

void remove_breakpoint_by_address( uint32 address )
{
    int bpt = lookup_breakpoint( address, 0 );
    if(bpt==-1)
        {
        printf("Tried to remove Nonexistant breakpoint %x!", address);
        }
    else
        remove_breakpoint_by_num( bpt );
}

int lookup_breakpoint( uint32 address, uint32 flags)
{
    int i=0;
    while( i != g_NumBreakpoints )
    {
        if((address >= g_Breakpoints[i].address) && (address <= g_Breakpoints[i].endaddr) && (!flags || ((g_Breakpoints[i].flags & flags) == flags)))
        {
        	printf("Bpt %d (0x%08X - 0x%08X) matches 0x%08X\n", i, g_Breakpoints[i].address,
        		g_Breakpoints[i].endaddr, address);
            return i;
		}
        else
            i++;
    }
    return -1;
}

int check_breakpoints( uint32 address )
{
    int bpt=lookup_breakpoint( address, BPT_FLAG_ENABLED | BPT_FLAG_EXEC );
    if( (bpt != -1) && BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED))
        return bpt;
    return -1;
}


int check_breakpoints_on_mem_access( uint32 address, uint32 size, uint32 flags )
{
	//This function handles memory read/write breakpoints. size specifies the address
	//range to check, flags specifies the flags that all need to be set.
	//It automatically stops and updates the debugger on hit, so the memory access
	//functions only need to call it and can discard the result.
	
	int i, bpt;
	for(i=address; i<=(address + size); i++)
	{
    	bpt=lookup_breakpoint( address, flags );
    	if(bpt != -1)
    	{
    		run = 0;
            switch_button_to_run();
            update_debugger_frontend();
            
             previousPC = PC->addr;
    		// Emulation thread is blocked until a button is clicked.
    		pthread_cond_wait(&debugger_done_cond, &mutex);
            
    		return bpt;
		}
	}
    return -1;
}
