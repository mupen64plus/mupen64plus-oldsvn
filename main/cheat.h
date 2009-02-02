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

#ifndef CHEAT_H
#define CHEAT_H

#include "util.h" // list_t

#define ENTRY_BOOT 0
#define ENTRY_VI 1

void cheat_apply_cheats(int entry);
void cheat_read_config(void);
void cheat_write_config(void);
void cheat_delete_all(void);
void cheat_load_current_rom(void);
void cheat_unload_current_rom(void);
void cheat_enable_current_rom(int);

typedef struct cheat_option {
    int code; /* e.g. 0xFF */
    char *description; /* e.g. Music Off */
} cheat_option_t;

typedef struct cheat_code {
    unsigned int address;
    int value;
    int old_value;
} cheat_code_t;

typedef struct cheat {
    char *name;
    char *comment;
    int number;
    int enabled;
    int always_enabled;
    int was_enabled;
    list_t codes;
    list_t options;
} cheat_t;

list_t cheats_for_current_rom(); // use cheats_free to free returned list
cheat_t *cheat_find_current_rom(int);

void cheats_free(list_t *cheats); // list_t becomes invalid after this!

// rom_cheats_t *cheat_new_rom(void);
// cheat_t *cheat_new_cheat(rom_cheats_t *);
// cheat_code_t *cheat_new_cheat_code(cheat_t *);
// void cheat_delete_rom(rom_cheats_t *);
// void cheat_delete_cheat(rom_cheats_t *, cheat_t *);
// void cheat_delete_cheat_code(cheat_t *, cheat_code_t *);

#endif // #define CHEAT_H
