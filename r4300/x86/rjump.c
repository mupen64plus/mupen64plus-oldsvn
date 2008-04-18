/**
 * Mupen64 - rjump.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
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
   if (PC->reg_cache_infos.need_map)
     *return_address = (unsigned long) (PC->reg_cache_infos.jump_wrapper);
   else
     *return_address = (unsigned long) (actual->code + PC->local_addr);
}

static long save_ebp = 0;
static long save_esp = 0;
static long save_eip = 0;

void dyna_start(void (*code)())
{
  /* save the base and stack pointers */
  /* make a call and a pop to retrieve the instruction pointer and save it too */
  /* then call the code(), which should theoretically never return.  */
  /* When dyna_stop() sets the *return_address to the saved EIP, the emulator thread will come back here. */
  /* It will jump to label 2, restore the base and stack pointers, and exit this function */
#ifdef _MSVC_VER
   __asm
   {
     mov _save_ebp, ebp
     mov _save_esp, esp
     call point1
     jmp point2
   point1:
     pop eax
     mov _save_eip, eax
     mov eax, code
     call eax
   point2:
     mov ebp, _save_ebp
     mov esp, _save_esp
   }
#elif defined(__GNUC__) && defined(__i386__)
#ifdef USEWIN32
   asm volatile 
      (" movl %%ebp, _save_ebp \n"
       " movl %%esp, _save_esp \n"
       " call 1f              \n"
       " jmp 2f               \n"
       "1:                    \n"
       " popl %%eax           \n"
       " movl %%eax, _save_eip \n"
       " call *%%ebx          \n"
       "2:                    \n"
       " movl _save_ebp, %%ebp \n"
       " movl _save_esp, %%esp \n"
       :
       : "b" (code)
       : "%eax", "memory"
       );
#else
   asm volatile 
      (" movl %%ebp, save_ebp \n"
       " movl %%esp, save_esp \n"
       " call 1f              \n"
       " jmp 2f               \n"
       "1:                    \n"
       " popl %%eax           \n"
       " movl %%eax, save_eip \n"
       " call *%%ebx          \n"
       "2:                    \n"
       " movl save_ebp, %%ebp \n"
       " movl save_esp, %%esp \n"
       :
       : "b" (code)
       : "%eax", "memory"
       );
#endif
#endif

   /* clear the registers so we don't return here a second time; that would be a bug */
   save_ebp=0;
   save_esp=0;
   save_eip=0;
}

void dyna_stop()
{
  if (save_eip == 0)
    printf("Warning: instruction pointer is 0 at dyna_stop()\n");
  else
  {
    *return_address = (unsigned long) save_eip;
  }
}

