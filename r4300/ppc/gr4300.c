/**
 * Mupen64 - gr4300.c
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

#include "assemble.h"
#include "../r4300.h"
#include "../macros.h"
#include "../../memory/memory.h"
#include "../interupt.h"
#include "../ops.h"
#include "../recomph.h"
#include "regcache.h"
#include "../exception.h"
#include "interpret.h"

extern unsigned int op;

precomp_instr fake_instr;
static int eax, ebx, ecx, edx, esp, ebp, esi, edi;

int branch_taken;

void gennotcompiled()
{
}

void genlink_subblock()
{
}

void gendebug()
{
}

void gencallinterp(unsigned long addr, int jump)
{
}

void genupdate_count(unsigned int addr)
{
}

void gendelayslot()
{
}

void genni()
{
}

void genreserved()
{
}

void genfin_block()
{
}

void gencheck_interupt(unsigned int instr_structure)
{
}

void gencheck_interupt_out(unsigned int addr)
{
}

void gencheck_interupt_reg() // addr is in EAX
{
}

void gennop()
{
}

void genj()
{
}

void genj_out()
{
}

void genj_idle()
{
}

void genjal()
{
}

void genjal_out()
{
}

void genjal_idle()
{
}

void genbeq_test()
{
}

void gentest()
{
}

void gentest_idle()
{
}

void genbne_test()
{
}

void genbne()
{
}

void genbne_out()
{
}

void genbne_idle()
{
}

void genblez_test()
{
}

void genblez()
{
}

void genblez_idle()
{
}

void genbgtz_test()
{
}

void genbgtz()
{
}

void genbgtz_out()
{
}

void genbgtz_idle()
{
}

void genaddi()
{
}

void genaddiu()
{
}

void genslti()
{
}

void gensltiu()
{
}

void genandi()
{
}

void genori()
{
}

void genxori()
{
}

void genlui()
{
}

void gentestl()
{
}

void genbeql()
{
}

void gentestl_out()
{
}

void genbeql_out()
{
}

void genbeql_idle()
{
}

void genbeq()
{
}

void gentest_out()
{
}

void genbeq_out()
{
}

void genbeq_idle()
{
}

void genbnel()
{
}

void genbnel_out()
{
}

void genbnel_idle()
{
}

void genblezl()
{
}

void genblezl_out()
{
}

void genblezl_idle()
{
}

void genbgtzl()
{
}

void genbgtzl_out()
{
}

void genbgtzl_idle()
{
}

void gendaddi()
{
}

void gendaddiu()
{
}

void genldl()
{
}

void genldr()
{
}

void genlb()
{
}

void genlh()
{
}

void genlwl()
{
}

void genlw()
{
}

void genlbu()
{
}

void genlhu()
{
}

void genlwr()
{
}

void genlwu()
{
}

void gensb()
{
}

void gensh()
{
}

void genswl()
{
}

void gensw()
{
}

void gensdl()
{
}

void gensdr()
{
}

void genswr()
{
}

void gencheck_cop1_unusable()
{
}

void genlwc1()
{
}

void genldc1()
{
}

void gencache()
{
}

void genld()
{
}

void genswc1()
{
}

void gensdc1()
{
}

void gensd()
{
}

void genll()
{
}

void gensc()
{
}

void genblez_out()
{
}

