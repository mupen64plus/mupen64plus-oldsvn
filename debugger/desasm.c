/*
 * debugger/desasm.c
 * 
 * Debugger for Mupen64 - davFr
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64 is copyrighted (C) 2002 Hacktarux
 * Mupen64 homepage: http://mupen64.emulation64.com
 *         email address: hacktarux@yahoo.fr
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

#include <stdio.h>
#include "desasm.h"

//TODO: Lots and lots
// to differanciate between update (need reload) and scroll (doesn't need reload)
// to reorganise whole code.
 

//static uint16 max_row=30;   //i plan to update this value on widget resizing.
//static uint32 previous_focus;

//]=-=-=-=-=-=-=-=-=-=-=[ Mise-a-jour Desassembleur ]=-=-=-=-=-=-=-=-=-=-=[

int get_instruction( uint32 address, uint32 *ptr_instruction )
// Returns 0 if returned value *ptr_instruction is valid.
//    (code based on prefetch() in r4300/pure_interpreter.c)
{
    uint32 addr;

    addr = address;
    if ( (addr>=0x80000000) && (addr<0xc0000000) )
    {
    //=== Read in Physical Memory Space================/
        if ((addr>=0xa4000000) && (addr<0xa4001000)){
            ptr_instruction[0] = SP_DMEM[ (addr&0xFFF)/4 ];
            return 0;
        }
        else if ((addr>=0x80000000) && (addr<0x80800000)){
            ptr_instruction[0] = rdram[ (addr&0xFFFFFF)/4 ];
            return 0;
        }
        else if ((addr>=0xB0000000) && (addr < 0xB0000000+taille_rom)){
            //Why code could not be executed directly from ROM? Look at PALadin...;)
            ptr_instruction[0] = ((unsigned int*)rom)[(addr&0xFFFFFF)/4];
            return 0;
        }else {
                printf("[DASM] error: reading code at 0x%lX.\n", addr );
            return 1;
        }
    } else {
    //=== Read in Virtual Memory Space (via TLB)=======/
        uint32 physical_address;

        printf("[DASM] reading at 0x%lX in virtual memory space.\n", addr );

        if (tlb_LUT_r[addr>>12])
            physical_address = (tlb_LUT_r[addr>>12]&0xFFFFF000)|(addr&0xFFF);
        else {
            printf("[DASM] reading at 0x%lX would perturb TLB emulation.\n", addr );
            return 2;
        }
                
        return get_instruction( physical_address, ptr_instruction );
    }
}
