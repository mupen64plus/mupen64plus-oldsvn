/**
 * Mupen64 - assemble.c
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
#include "../recomph.h"
#include <malloc.h>
#include <stdio.h>
#include "../r4300.h"

typedef struct _jump_table
{
   unsigned int mi_addr;
   unsigned int pc_addr;
} jump_table;

static jump_table *jumps_table = NULL;
static int jumps_number, max_jumps_number;

void init_assembler(void *block_jumps_table, int block_jumps_number, void *block_riprel_table, int block_riprel_number)
{
}

void free_assembler(void **block_jumps_table, int *block_jumps_number, void **block_riprel_table, int *block_riprel_number)
{
}

static void add_jump(unsigned int pc_addr, unsigned int mi_addr)
{
}

void passe2(precomp_instr *dest, int start, int end, precomp_block *block)
{
}

inline void put8(unsigned char octet)
{
}

inline void put32(unsigned int dword)
{
}

inline void put16(unsigned short word)
{
}

void push_reg32(int reg32)
{
}

void pop_reg32(int reg32)
{
}

void mov_eax_memoffs32(unsigned int *memoffs32)
{
}

void mov_memoffs32_eax(unsigned int *memoffs32)
{
}

void mov_ax_memoffs16(unsigned short *memoffs16)
{
}

void mov_memoffs16_ax(unsigned short *memoffs16)
{
}

void mov_al_memoffs8(unsigned char *memoffs8)
{
}

void mov_memoffs8_al(unsigned char *memoffs8)
{
}

void mov_m8_imm8(unsigned char *m8, unsigned char imm8)
{
}

void mov_m8_reg8(unsigned char *m8, int reg8)
{
}

void mov_reg16_m16(int reg16, unsigned short *m16)
{
}

void mov_m16_reg16(unsigned short *m16, int reg16)
{
}

void cmp_reg32_m32(int reg32, unsigned int *m32)
{
}

void cmp_reg32_reg32(int reg1, int reg2)
{
}

void cmp_reg32_imm8(int reg32, unsigned char imm8)
{
}

void cmp_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8)
{
}

void cmp_reg32_imm32(int reg32, unsigned int imm32)
{
}

void test_reg32_imm32(int reg32, unsigned int imm32)
{
}

void test_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void cmp_al_imm8(unsigned char imm8)
{
}

void add_m32_reg32(unsigned int *m32, int reg32)
{
}

void sub_reg32_m32(int reg32, unsigned int *m32)
{
}

void sub_reg32_reg32(int reg1, int reg2)
{
}

void sbb_reg32_reg32(int reg1, int reg2)
{
}

void sub_reg32_imm32(int reg32, unsigned int imm32)
{
}

void sub_eax_imm32(unsigned int imm32)
{
}

void jne_rj(unsigned char saut)
{
}

void je_rj(unsigned char saut)
{
}

void jb_rj(unsigned char saut)
{
}

void jbe_rj(unsigned char saut)
{
}

void ja_rj(unsigned char saut)
{
}

void jae_rj(unsigned char saut)
{
}

void jle_rj(unsigned char saut)
{
}

void jge_rj(unsigned char saut)
{
}

void jg_rj(unsigned char saut)
{
}

void jl_rj(unsigned char saut)
{
}

void jp_rj(unsigned char saut)
{
}

void je_near(unsigned int mi_addr)
{
}

void je_near_rj(unsigned int saut)
{
}

void jl_near(unsigned int mi_addr)
{
}

void jl_near_rj(unsigned int saut)
{
}

void jne_near(unsigned int mi_addr)
{
}

void jne_near_rj(unsigned int saut)
{
}

void jge_near(unsigned int mi_addr)
{
}

void jge_near_rj(unsigned int saut)
{
}

void jg_near(unsigned int mi_addr)
{
}

void jle_near(unsigned int mi_addr)
{
}

void jle_near_rj(unsigned int saut)
{
}

void mov_reg32_imm32(int reg32, unsigned int imm32)
{
}

void jmp_imm(int saut)
{
}

void jmp_imm_short(char saut)
{
}

void dec_reg32(int reg32)
{
}

void inc_reg32(int reg32)
{
}

void or_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void or_m32_reg32(unsigned int *m32, unsigned int reg32)
{
}

void or_reg32_m32(unsigned int reg32, unsigned int *m32)
{
}

void or_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void and_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void and_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void and_reg32_m32(unsigned int reg32, unsigned int *m32)
{
}

void xor_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void xor_reg32_m32(unsigned int reg32, unsigned int *m32)
{
}

void add_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void add_m32_imm8(unsigned int *m32, unsigned char imm8)
{
}

void sub_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void push_imm32(unsigned int imm32)
{
}

void add_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
}

void add_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
}

void inc_m32(unsigned int *m32)
{
}

void cmp_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void cmp_m32_imm8(unsigned int *m32, unsigned char imm8)
{
}

void cmp_m8_imm8(unsigned char *m8, unsigned char imm8)
{
}

void cmp_eax_imm32(unsigned int imm32)
{
}

void mov_m32_imm32(unsigned int *m32, unsigned int imm32)
{
}

void jmp(unsigned int mi_addr)
{
}

void cdq()
{
}

void cwde()
{
}

void cbw()
{
}

void mov_m32_reg32(unsigned int *m32, unsigned int reg32)
{
}

void ret()
{
}

void call_reg32(unsigned int reg32)
{
}

void call_m32(unsigned int *m32)
{
}

void shr_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
}

void shr_reg32_cl(unsigned int reg32)
{
}

void sar_reg32_cl(unsigned int reg32)
{
}

void shl_reg32_cl(unsigned int reg32)
{
}

void shld_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
}

void shld_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
}

void shrd_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
}

void sar_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
}

void shrd_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
}

void mul_m32(unsigned int *m32)
{
}

void imul_m32(unsigned int *m32)
{
}

void imul_reg32(unsigned int reg32)
{
}

void mul_reg32(unsigned int reg32)
{
}

void idiv_reg32(unsigned int reg32)
{
}

void div_reg32(unsigned int reg32)
{
}

void idiv_m32(unsigned int *m32)
{
}

void div_m32(unsigned int *m32)
{
}

void add_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void adc_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void add_reg32_m32(unsigned int reg32, unsigned int *m32)
{
}

void adc_reg32_m32(unsigned int reg32, unsigned int *m32)
{
}

void adc_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
}

void jmp_reg32(unsigned int reg32)
{
}

void jmp_m32(unsigned int *m32)
{
}

void mov_reg32_preg32(unsigned int reg1, unsigned int reg2)
{
}

void mov_preg32_reg32(int reg1, int reg2)
{
}

void mov_reg32_preg32preg32pimm32(int reg1, int reg2, int reg3, unsigned int imm32)
{
}

void mov_reg32_preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
}

void mov_reg32_preg32x4preg32(int reg1, int reg2, int reg3)
{
}

void mov_reg32_preg32x4preg32pimm32(int reg1, int reg2, int reg3, unsigned int imm32)
{
}

void mov_reg32_preg32x4pimm32(int reg1, int reg2, unsigned int imm32)
{
}

void mov_preg32preg32pimm32_reg8(int reg1, int reg2, unsigned int imm32, int reg8)
{
}

void mov_preg32pimm32_reg8(int reg32, unsigned int imm32, int reg8)
{
}

void mov_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8)
{
}

void mov_preg32pimm32_reg16(int reg32, unsigned int imm32, int reg16)
{
}

void mov_preg32pimm32_reg32(int reg1, unsigned int imm32, int reg2)
{
}

void add_eax_imm32(unsigned int imm32)
{
}

void shl_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
}

void mov_reg32_m32(unsigned int reg32, unsigned int* m32)
{
}

void mov_reg8_m8(int reg8, unsigned char *m8)
{
}

void and_eax_imm32(unsigned int imm32)
{
}

void and_reg32_imm32(int reg32, unsigned int imm32)
{
}

void or_reg32_imm32(int reg32, unsigned int imm32)
{
}

void and_reg32_imm8(int reg32, unsigned char imm8)
{
}

void and_ax_imm16(unsigned short imm16)
{
}

void and_al_imm8(unsigned char imm8)
{
}

void or_ax_imm16(unsigned short imm16)
{
}

void or_eax_imm32(unsigned int imm32)
{
}

void xor_ax_imm16(unsigned short imm16)
{
}

void xor_al_imm8(unsigned char imm8)
{
}

void xor_reg32_imm32(int reg32, unsigned int imm32)
{
}

void xor_reg8_imm8(int reg8, unsigned char imm8)
{
}

void nop()
{
}

void mov_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
}

void not_reg32(unsigned int reg32)
{
}

void movsx_reg32_m8(int reg32, unsigned char *m8)
{
}

void movsx_reg32_reg8(int reg32, int reg8)
{
}

void movsx_reg32_8preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
}

void movsx_reg32_16preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
}

void movsx_reg32_reg16(int reg32, int reg16)
{
}

void movsx_reg32_m16(int reg32, unsigned short *m16)
{
}

void fldcw_m16(unsigned short *m16)
{
}

void fld_preg32_dword(int reg32)
{
}

void fdiv_preg32_dword(int reg32)
{
}

void fstp_preg32_dword(int reg32)
{
}

void fchs()
{
}

void fstp_preg32_qword(int reg32)
{
}

void fadd_preg32_dword(int reg32)
{
}

void fsub_preg32_dword(int reg32)
{
}

void fmul_preg32_dword(int reg32)
{
}

void fcomp_preg32_dword(int reg32)
{
}

void fistp_preg32_dword(int reg32)
{
}

void fistp_m32(unsigned int *m32)
{
}

void fistp_preg32_qword(int reg32)
{
}

void fistp_m64(unsigned long long *m64)
{
}

void fld_preg32_qword(int reg32)
{
}

void fild_preg32_qword(int reg32)
{
}

void fild_preg32_dword(int reg32)
{
}

void fadd_preg32_qword(int reg32)
{
}

void fdiv_preg32_qword(int reg32)
{
}

void fsub_preg32_qword(int reg32)
{
}

void fmul_preg32_qword(int reg32)
{
}

void fsqrt()
{
}

void fabs_()
{
}

void fcomip_fpreg(int fpreg)
{
}

void fucomip_fpreg(int fpreg)
{
}

void ffree_fpreg(int fpreg)
{
}

