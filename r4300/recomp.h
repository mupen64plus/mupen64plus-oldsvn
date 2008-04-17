/**
 * Mupen64 - recomp.h
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

#ifndef RECOMP_H
#define RECOMP_H

#if defined(__x86_64__)
  #include "x86_64/assemble.h"
#else
  #include "x86/assemble.h"
#endif

typedef struct _precomp_instr
{
   void (*ops)();
   union
     {
    struct
      {
         long long int *rs;
         long long int *rt;
         short immediate;
      } i;
    struct
      {
         unsigned int inst_index;
      } j;
    struct
      {
         long long int *rs;
         long long int *rt;
         long long int *rd;
         unsigned char sa;
         unsigned char nrd;
      } r;
    struct
      {
         unsigned char base;
         unsigned char ft;
         short offset;
      } lf;
    struct
      {
         unsigned char ft;
         unsigned char fs;
         unsigned char fd;
      } cf;
     } f;
   unsigned int addr; /* word-aligned instruction address in r4300 address space */
   unsigned int local_addr; /* byte offset to start of corresponding x86_64 instructions, from start of code block */
   reg_cache_struct reg_cache_infos;
} precomp_instr;

typedef struct _precomp_block
{
   precomp_instr *block;
   unsigned int start;
   unsigned int end;
   unsigned char *code;
   unsigned int code_length;
   unsigned int max_code_length;
   void *jumps_table;
   int jumps_number;
   void *riprel_table;
   int riprel_number;
   //unsigned char md5[16];
   unsigned int adler32;
} precomp_block;

void recompile_block(int *source, precomp_block *block, unsigned int func);
void init_block(int *source, precomp_block *block);
void recompile_opcode();
void prefetch_opcode(unsigned int op);
void dyna_jump();
void dyna_start();
void dyna_stop();

extern precomp_instr *dst; /* precomp_instr structure for instruction being recompiled */

#if defined(__x86_64__)
  #include "x86_64/regcache.h"
#else
  #include "x86/regcache.h"
#endif

#endif

