/**
 * Mupen64 - r4300/x86_64/regcache.h
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


#ifndef REGCACHE_H
#define REGCACHE_H

#include "../recomp.h"

void init_cache(precomp_instr* start);
void free_registers_move_start();
void free_all_registers();
void free_register(int reg);
void flush_registers();
void reload_registers();
void stack_save_registers();
void stack_load_registers();
int is64(unsigned int *addr);
int lru_register();
int lru_base_register();
void set_register_state(int reg, unsigned int *addr, int dirty, int is64bits);
int lock_register(int reg);
void unlock_register(int reg);
int allocate_register_32(unsigned int *addr);
int allocate_register_64(unsigned long long *addr);
int allocate_register_32_w(unsigned int *addr);
int allocate_register_64_w(unsigned long long *addr);
void allocate_register_32_manually(int reg, unsigned int *addr);
void allocate_register_32_manually_w(int reg, unsigned int *addr);
void simplify_access();
void build_wrapper(precomp_instr*, unsigned char*, precomp_block*);
void build_wrappers(precomp_instr*, int, int, precomp_block*);

#endif // REGCACHE_H

