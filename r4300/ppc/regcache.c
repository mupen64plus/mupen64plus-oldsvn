/**
 * Mupen64 - r4300/x86_64/regcache.c
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

#include "regcache.h"
#include "../recomp.h"
#include "../r4300.h"
#include "../recomph.h"

static unsigned long long * reg_content[8];
static precomp_instr* last_access[8];
static precomp_instr* free_since[8];
static int dirty[8];
static int is64bits[8];
static unsigned long long *r0;

void init_cache(precomp_instr* start)
{
}

void free_all_registers()
{
}

void free_registers_move_start()
{
}

// this function frees a specific X86 GPR
void free_register(int reg)
{
}

void flush_registers()
{
}

void reload_registers()
{
}

void stack_save_registers()
{
}

void stack_load_registers()
{
}

int lru_register()
{
}

int lru_base_register() /* EBP cannot be used as a base register for SIB addressing byte */
{
}

void set_register_state(int reg, unsigned int *addr, int _dirty, int _is64bits)
{
}

int lock_register(int reg)
{
}

void unlock_register(int reg)
{
}

// this function finds a register to put the data contained in addr,
// if there was another value before it's cleanly removed of the
// register cache. After that, the register number is returned.
// If data are already cached, the function only returns the register number
int allocate_register_32(unsigned int *addr)
{
}

// this function is similar to allocate_register except it loads
// a 64 bits value, and return the register number of the LSB part
int allocate_register_64(unsigned long long *addr)
{
}

// this function checks if the data located at addr are cached in a register
// and then, it returns 1  if it's a 64 bit value
//                      0  if it's a 32 bit value
//                      -1 if it's not cached
int is64(unsigned int *addr)
{
}

int allocate_register_32_w(unsigned int *addr)
{
}

int allocate_register_64_w(unsigned long long *addr)
{
}

void allocate_register_32_manually(int reg, unsigned int *addr)
{
}

void allocate_register_32_manually_w(int reg, unsigned int *addr)
{
}


// 0x48 0x83 0xEC 0x8                     sub rsp, byte 8
// 0x48 0xA1           0xXXXXXXXXXXXXXXXX mov rax, qword (&code start)
// 0x48 0x05                   0xXXXXXXXX add rax, dword (local_addr)
// 0x48 0x89 0x04 0x24                    mov [rsp], rax
// 0x48 0xB8           0xXXXXXXXXXXXXXXXX mov rax, &reg[0]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rdi, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rsi, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rbp, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rdx, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rcx, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rbx, [rax + XXXXXXXX]
// 0x48 0x8B (reg<<3)|0x80     0xXXXXXXXX mov rax, [rax + XXXXXXXX]
// 0xC3 ret
// total : 84 bytes

void build_wrapper(precomp_instr *instr, unsigned char* pCode, precomp_block* block)
{
}

void build_wrappers(precomp_instr *instr, int start, int end, precomp_block* block)
{
}

void simplify_access()
{
}

