/**
 * Mupen64 - r4300/x86_64/gtlb.c
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


#include <stdio.h>
#include "../recomph.h"
#include "assemble.h"
#include "../r4300.h"
#include "../ops.h"

void gentlbwi()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[104]);
#endif
   gencallinterp((unsigned long long)TLBWI, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBWI));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void gentlbp()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[105]);
#endif
   gencallinterp((unsigned long long)TLBP, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBP));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void gentlbr()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[106]);
#endif
   gencallinterp((unsigned long long)TLBR, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBR));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void generet()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[108]);
#endif
   gencallinterp((unsigned long long)ERET, 1);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   genupdate_system(0);
   mov_reg32_imm32(EAX, (unsigned int)(ERET));
   call_reg32(EAX);
   mov_reg32_imm32(EAX, (unsigned int)(jump_code));
   jmp_reg32(EAX);*/
}

void gentlbwr()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[107]);
#endif
   gencallinterp((unsigned long long)TLBWR, 0);
}
