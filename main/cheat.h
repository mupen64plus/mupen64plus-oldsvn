/**
 * Mupen64 - rom.h
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

// TODO: finish file, move appropiate code to cheat.h

// this struct might come in handy, who knows...

typedef struct {
    unsigned int address;
    unsigned short value;
    int enabled;
    struct cheatcode *next_cheat;
} cheatcode;

#define ENTRY_BOOT 0
#define ENTRY_VI 1

extern struct cheatcode *cheats;
extern int numcheats;

// creation/list handeling functions
cheatcode parse_cheat_string(char* sz_cheat);

// actual memory modifer function
void update_address_8bit(unsigned int address, unsigned char new_value);
void update_address_16bit(unsigned int address, unsigned short new_value);

// loop functions (used for going through the list)
void apply_boot_cheats();
void apply_cheats(int entry);

void load_cheats();

// individual application - returns 0 if we are supposed to skip the next cheat
// (only happens on conditional codes)
int execute_cheat(unsigned int address, unsigned short value);

// special cheat command functions - some aren't really needed at all.
int address_equal_to_8bit(unsigned int address, unsigned char value);
int address_equal_to_16bit(unsigned int address, unsigned short value);
void disable_expansion_pack();
void additional_enable_code();
void change_exception_handler();
void enabler(unsigned int address);
void set_store_location(unsigned int address);

/* im not sure how to write the base for this function
 * Repeater/Patch Code
 * 5000XXYY ????
 * 8ZZZZZZZ VVVV
 * Used to make extremely long, sequenced codes shorter. 
 * XX is the number of addresses to be written,
 *  YY is the offset between addresses, 
 * ???? is the amount to increment the value (VVVV).
 * Note that this increment is signed, so values higher than 0x8000 will
 * subtract from VVVV rather than being added.
 * These can be preceded by any 'D' code type (EG 0xD0144604 0022)
 * to enable them on demand. The code to be repeated is not limited to constant writes.
 * Any 80/81/88/89 code type appearing below a '50' code should work.
 */


