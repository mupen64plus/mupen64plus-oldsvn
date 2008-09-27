/**
 * Mupen64 - r4300/x86_64/gcop1_w.c
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
#include "interpret.h"

void gencvt_s_w()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_S_W
   gencallinterp((unsigned long long)CVT_S_W, 0);
#else
   gencheck_cop1_unusable();
   mov_reg64_m64abs(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fild_preg64_dword(RAX);
   mov_reg64_m64abs(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void gencvt_d_w()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_D_W
   gencallinterp((unsigned long long)CVT_D_W, 0);
#else
   gencheck_cop1_unusable();
   mov_reg64_m64abs(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fild_preg64_dword(RAX);
   mov_reg64_m64abs(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

