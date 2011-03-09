/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - linkage_arm.s                                           *
 *   Copyright (C) 2009 Ari64                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	.cpu arm9tdmi
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 18, 4
	.file	"dynarec_arm.s"
	.global	rdram
rdram = 0x80000000
	.global	dynarec_local
	.global	reg
	.global	hi
	.global	lo
	.global	reg_cop1_simple
	.global	reg_cop1_double
	.global reg_cop0
	.global	FCR0
	.global	FCR31
	.global	next_interupt
	.global	cycle_count
	.global	last_count
	.global	pending_exception
	.global	pcaddr
	.global	stop
	.global	invc_ptr
	.global	address
	.global	readmem_dword
	.global	dword
	.global	word
	.global	hword
	.global	byte
	.global	PC
	.global	fake_pc
	.global	fake_pc_float
	.bss
	.align	4
	.type	dynarec_local, %object
	.size	dynarec_local, 64
dynarec_local:
	.space	64+16+16+8+8+8+8+256+8+8+128+128+128+8+132+132
next_interupt = dynarec_local + 64
	.type	next_interupt, %object
	.size	next_interupt, 4
cycle_count = next_interupt + 4
	.type	cycle_count, %object
	.size	cycle_count, 4
last_count = cycle_count + 4
	.type	last_count, %object
	.size	last_count, 4
pending_exception = last_count + 4
	.type	pending_exception, %object
	.size	pending_exception, 4
pcaddr = pending_exception + 4
	.type	pcaddr, %object
	.size	pcaddr, 4
stop = pcaddr + 4
	.type	stop, %object
	.size	stop, 4
invc_ptr = stop + 4
	.type	invc_ptr, %object
	.size	invc_ptr, 4
address = invc_ptr + 4
	.type	address, %object
	.size	address, 4
readmem_dword = address + 4
	.type	readmem_dword, %object
	.size	readmem_dword, 8
dword = readmem_dword + 8
	.type	dword, %object
	.size	dword, 8
word = dword + 8
	.type	word, %object
	.size	word, 4
hword = word + 4
	.type	hword, %object
	.size	hword, 2
byte = hword + 2
	.type	byte, %object
	.size	byte, 1 /* 1 byte free */
FCR0 = hword + 4
	.type	FCR0, %object
	.size	FCR0, 4
FCR31 = FCR0 + 4
	.type	FCR31, %object
	.size	FCR31, 4
reg = FCR31 + 4
	.type	reg, %object
	.size	reg, 256
hi = reg + 256
	.type	hi, %object
	.size	hi, 8
lo = hi + 8
	.type	lo, %object
	.size	lo, 8
reg_cop0 = lo + 8
	.type	reg_cop0, %object
	.size	reg_cop0, 128
reg_cop1_simple = reg_cop0 + 128
	.type	reg_cop1_simple, %object
	.size	reg_cop1_simple, 128
reg_cop1_double = reg_cop1_simple + 128
	.type	reg_cop1_double, %object
	.size	reg_cop1_double, 128
PC = reg_cop1_double + 128
	.type	PC, %object
	.size	PC, 4
	/* 4 bytes free */
fake_pc = PC + 8
	.type	fake_pc, %object
	.size	fake_pc, 132
fake_pc_float = fake_pc + 132
	.type	fake_pc_float, %object
	.size	fake_pc_float, 132

	.text
	.align	2
	.global	dyna_linker
	.type	dyna_linker, %function
dyna_linker:
	mov	r2, #0x200000
	ldr	r3, .jiptr
	eor	r2, r2, r0, lsr #10
	ldr	r7, [r1]
	cmp	r2, #8192
	bic	r2, r2, #3
	movcs	r2, #8192
	add	r12, r7, #2
	ldr	r5, [r3, r2]
	lsl	r12, r12, #8
.L1:
	movs	r4, r5
	beq	.L3
	ldr	r3, [r5]
	ldr	r5, [r4, #12]
	teq	r3, r0
	bne	.L1
.L2:
	ldr	r3, [r4, #4]
	ldr	r4, [r4, #8]
	tst	r3, r3
	bne	.L1
	mov	r5, r1
	add	r1, r1, r12, asr #6
	teq	r1, r4
	moveq	pc, r4 /* Stale i-cache */
	bl	add_link
	sub	r2, r4, r5
	and	r1, r7, #0xff000000
	lsl	r2, r2, #6
	sub	r1, r1, #2
	add	r1, r1, r2, lsr #8
	str	r1, [r5]
	mov	pc, r4
.L3:
	ldr	r3, .jdptr
	ldr	r5, [r3, r2]
.L4:
	movs	r4, r5
	beq	.L6
	ldr	r3, [r5]
	ldr	r5, [r4, #12]
	teq	r3, r0
	bne	.L4
.L5:
	ldr	pc, [r4, #8]
.L6:
	mov	r4, r0
	mov	r5, r1
	bl	new_recompile_block
	mov	r0, r4
	mov	r1, r5
	b	dyna_linker
.jiptr:
	.word	jump_in
.jdptr:
	.word	jump_dirty
	.size	dyna_linker, .-dyna_linker
	.align	2
	.global	verify_code
	.type	verify_code, %function
verify_code:
	tst	r3, #4
	mov	r4, #0
	add	r3, r1, r3
	mov	r5, #0
	ldrne	r4, [r1], #4
	mov	r12, #0
	ldrne	r5, [r2], #4
.L7:
	ldr	r7, [r1], #4
	eor	r9, r4, r5
	ldr	r8, [r2], #4
	orrs	r9, r9, r12
	bne	.L8
	ldr	r4, [r1], #4
	eor	r12, r7, r8
	ldr	r5, [r2], #4
	cmp	r1, r3
	bcc	.L7
	teq	r4, r5
	bne	.L8
	mov	pc, lr
.L8:
	mov	r4, r0
	bl	remove_hash
	mov	r0, r4
	bl	new_recompile_block
	mov	r0, r4
	bl	get_addr
	mov	pc, r0
	.size	verify_code, .-verify_code
	.align	2
	.global	cc_interrupt
	.type	cc_interrupt, %function
cc_interrupt:
	ldr	r0, [fp, #last_count-dynarec_local]
	add	r10, r0, r10
	str	r10, [fp, #reg_cop0+36-dynarec_local] /* Count */
	mov	r10, lr
	bl	gen_interupt
	mov	lr, r10
	ldr	r10, [fp, #reg_cop0+36-dynarec_local] /* Count */
	ldr	r0, [fp, #next_interupt-dynarec_local]
	ldr	r1, [fp, #pending_exception-dynarec_local]
	ldr	r2, [fp, #stop-dynarec_local]
	str	r0, [fp, #last_count-dynarec_local]
	sub	r10, r10, r0
	tst	r2, r2
	bne	.L10
	tst	r1, r1
	bne	.L9
	mov	pc, lr
.L9:
	ldr	r0, [fp, #pcaddr-dynarec_local]
	bl	get_addr_ht
	mov	pc, r0
.L10:
	add	r12, fp, #28
	ldmia	r12, {r4, r5, r6, r7, r8, r9, sl, fp, pc}
	.size	cc_interrupt, .-cc_interrupt
	.align	2
	.global	jump_vaddr
	.type	jump_vaddr, %function
jump_vaddr:
	eor	r2, r0, r0, lsl #16
	ldr	r1, .htptr
	lsr	r2, r2, #12
	bic	r2, r2, #15
	ldr	r2, [r1, r2]!
	teq	r2, r0
	bne	.L12
.L11:
	ldr	pc, [r1, #4]
.L12:
	ldr	r2, [r1, #8]!
	teq	r2, r0
	beq	.L11
	bl	get_addr
	mov	pc, r0
.htptr:
	.word	hash_table
	.size	jump_vaddr, .-jump_vaddr
	.align	2
	.global	fp_exception
	.type	fp_exception, %function
fp_exception:
	mov	r2, #0x10000000
.fpe:
	ldr	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	mov	r3, #0x80000000
	str	r0, [fp, #reg_cop0+56-dynarec_local] /* EPC */
	orr	r1, #2
	add	r2, r2, #0x2c
	str	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	str	r2, [fp, #reg_cop0+52-dynarec_local] /* Cause */
	add	r0, r3, #0x180
	bl	get_addr_ht
	mov	pc, r0
	.size	fp_exception, .-fp_exception
	.align	2
	.global	fp_exception_ds
	.type	fp_exception_ds, %function
fp_exception_ds:
	mov	r2, #0x90000000 /* Set high bit if delay slot */
	b	.fpe
	.size	fp_exception_ds, .-fp_exception_ds
	.align	2
	.global	jump_syscall
	.type	jump_syscall, %function
jump_syscall:
	ldr	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	mov	r3, #0x80000000
	str	r0, [fp, #reg_cop0+56-dynarec_local] /* EPC */
	orr	r1, #2
	mov	r2, #0x20
	str	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	str	r2, [fp, #reg_cop0+52-dynarec_local] /* Cause */
	add	r0, r3, #0x180
	bl	get_addr_ht
	mov	pc, r0
	.size	jump_syscall, .-jump_syscall
	.align	2
	.global	indirect_jump
	.type	indirect_jump, %function
indirect_jump:
	ldr	r3, [fp, #last_count-dynarec_local]
	add	r0, r0, r1, lsl #2
	add	r2, r2, r3 
	str	r2, [fp, #reg_cop0+36-dynarec_local] /* Count */
	ldr	pc, [r0]
	.size	indirect_jump, .-indirect_jump
	.align	2
	.global	new_dyna_start
	.type	new_dyna_start, %function
new_dyna_start:
	ldr	r12, .dlptr
	mov	r0, #0xa4000000
	stmia	r12, {r4, r5, r6, r7, r8, r9, sl, fp, lr}
	sub	fp, r12, #28
	add	r0, r0, #0x40
	bl	new_recompile_block
	ldr	r0, [fp, #next_interupt-dynarec_local]
	ldr	r10, [fp, #reg_cop0+36-dynarec_local] /* Count */
	str	r0, [fp, #last_count-dynarec_local]
	sub	r10, r10, r0
	mov	pc, #0x7000000
.dlptr:
	.word	dynarec_local+28
	.size	new_dyna_start, .-new_dyna_start
	.align	2
	.global	jump_eret
	.type	jump_eret, %function
jump_eret:
	ldr	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	ldr	r0, [fp, #reg_cop0+56-dynarec_local] /* EPC */
	bic	r1, r1, #2
	str	r1, [fp, #reg_cop0+48-dynarec_local] /* Status */
	mov	r5, #248
	add	r6, fp, #reg+256-dynarec_local
	mov	r1, #0
.L13:
	ldr	r2, [r6, #-8]!
	ldr	r3, [r6, #4]
	eor	r3, r3, r2, asr #31
	subs	r3, r3, #1
	adc	r1, r1, r1
	subs	r5, r5, #8
	bne	.L13
	ldr	r2, [fp, #hi-dynarec_local]
	ldr	r3, [fp, #hi+4-dynarec_local]
	eors	r3, r3, r2, asr #31
	ldr	r2, [fp, #lo-dynarec_local]
	ldreq	r3, [fp, #lo+4-dynarec_local]
	eoreq	r3, r3, r2, asr #31
	subs	r3, r3, #1
	adc	r1, r1, r1
	bl	get_addr_32
	mov	pc, r0
	.size	jump_eret, .-jump_eret
	.section	.note.GNU-stack,"",%progbits
