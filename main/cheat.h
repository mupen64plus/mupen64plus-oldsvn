/**
 * Mupen64 - cheat.h
 * Copyright (C) 2008 okaygo
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
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

#include "util.h" // list_t

#define ENTRY_BOOT 0
#define ENTRY_VI 1

extern list_t g_Cheats;

// represents all cheats for a given rom
typedef struct {
    char *rom_name;
    unsigned int crc1;
    unsigned int crc2;
    list_t cheats; // list of cheat_t's
} rom_cheats_t;

// represents a cheat for a game
typedef struct {
    char *name;
    int enabled; // cheat enabled until mupen64plus is closed
    int always_enabled; // always enabled (written to config)
    list_t cheat_codes; // list of cheat_code_t's
} cheat_t;

// represents a cheatcode associated with a cheat
typedef struct {
    unsigned int address;
    unsigned short value;
} cheat_code_t;

void cheat_apply_cheats(int entry);
void cheat_read_config(void);
void cheat_write_config(void);
void cheat_delete_all(void);
void cheat_load_current_rom(void);

rom_cheats_t *cheat_new_rom(void);
cheat_t *cheat_new_cheat(rom_cheats_t *);
cheat_code_t *cheat_new_cheat_code(cheat_t *);
void cheat_delete_rom(rom_cheats_t *);
void cheat_delete_cheat(rom_cheats_t *, cheat_t *);
void cheat_delete_cheat_code(cheat_t *, cheat_code_t *);

