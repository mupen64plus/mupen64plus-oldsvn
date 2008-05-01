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

#if !defined(NO_ASM) && (defined(__i386__) || defined(__x86_64__))

int  lines_recompiled;
uint32 addr_recompiled;

char opcode_recompiled[564][MAX_DISASSEMBLY];
char args_recompiled[564][MAX_DISASSEMBLY];
int  opaddr_recompiled[564];

char* get_recompiled_opcode(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opcode_recompiled[index];
    else
        return NULL;
}

char* get_recompiled_args(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return args_recompiled[index];
    else
        return NULL;
}

int get_recompiled_addr(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opaddr_recompiled[index];
    else
        return 0;
}

int get_num_recompiled(uint32 addr)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    return lines_recompiled;
}

void decode_recompiled(uint32 addr)
{
    unsigned char *assemb, *end_addr;

    lines_recompiled=0;

    if(blocks[addr>>12] == NULL)
        return;
    
    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) == 0xFFF)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;
    
    while(assemb < end_addr)
    {
        opaddr_recompiled[lines_recompiled] = assemb;
        assemb += host_decode_op(assemb, opcode_recompiled[lines_recompiled], 
                                 args_recompiled[lines_recompiled]);
        lines_recompiled++;
    }
    addr_recompiled=addr;
}

#else

char* get_recompiled(uint32 addr, int index)
{
    return NULL;
}

int get_num_recompiled(uint32 addr)
{
    return 0;
}

#endif
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


void get_dynacomp()
{
/*    precomp_block *blk;
    else
        {
        gtk_clist_get_selection_info( GTK_CLIST(clist), event->x, event->y, &clicked_row, NULL);
        clicked_address =(uint32) gtk_clist_get_row_data( GTK_CLIST(clist), clicked_row);
        blk = blocks[clicked_address >> 12];
        int numblks = blk->block[(clicked_address+1) & 0xFFF].local_addr - blk->block[clicked_address & 0xFFF].local_addr;
        unsigned char *assemb = (blk->code) + blk->block[clicked_address & 0xFFF].local_addr;
        printf("%x %x: ", clicked_address, blk->code);
int i;
for(i=0; i<numblks; i++)
printf("%02x ", assemb[i]);
printf("\n");
        }
*/
}
