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
   if (stop == 1)
   { return; }
   if (PC->reg_cache_infos.need_map)
     *return_address = (unsigned long) (PC->reg_cache_infos.jump_wrapper);
   else
     *return_address = (unsigned long) (actual->code + PC->local_addr);
}

static long save_rbp = 0;
static long save_rsp = 0;
static long save_rip = 0;

void dyna_start(void (*code)())
{
  /* save the base and stack pointers */
  /* make a call and a pop to retrieve the instruction pointer and save it too */
  /* then call the code(), which should theoretically never return.  */
  /* When dyna_stop() sets the *return_address to the saved RIP, the emulator thread will come back here. */
  /* It will jump to label 2, restore the base and stack pointers, and exit this function */
  printf("R4300 core: starting 64-bit dynamic recompiler at: 0x%lx.\n", (unsigned long) code);
#if defined(__GNUC__) && defined(__x86_64__)
   asm volatile 
      (" mov  %%rbp, save_rbp \n"
       " mov  %%rsp, save_rsp \n"
       " call 1f              \n"
       " jmp 2f               \n"
       "1:                    \n"
       " pop  %%rax           \n"
       " mov  %%rax, save_rip \n"
       " call *%%rbx          \n"
       "2:                    \n"
       " mov  save_rbp, %%rbp \n"
       " mov  save_rsp, %%rsp \n"
       :
       : "b" (code)
       : "%rax", "memory"
       );
#endif

   /* clear the registers so we don't return here a second time; that would be a bug */
   save_rbp=0;
   save_rsp=0;
   save_rip=0;
}

void dyna_stop()
{
  if (save_rip == 0)
    printf("Warning: instruction pointer is 0 at dyna_stop()\n");
  else
  {
    *return_address = (unsigned long) save_rip;
  }
}

