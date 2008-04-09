/**
 * Mupen64 - cheat.c
 * Copyright (C) 2008 okaygo
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

// gameshark and xploder64 reference: http://doc.kodewerx.net/hacking_n64.html 

#include <stdio.h>
#include <errno.h>
#include <zlib.h> // TODO: compress cfg file

#include "../memory/memory.h"
#include "cheat.h"
#include "main.h"
#include "rom.h"
#include "util.h" // list utilities

#define CHEAT_FILENAME "cheats.cfg"

// public globals
list_t g_Cheats = NULL; // list of all supported cheats

// static globals
static rom_cheats_t *g_Current = NULL; // current loaded rom

// private functions
static void update_address_16bit(unsigned int address, unsigned short new_value)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16))) = new_value;
}

static void update_address_8bit(unsigned int address, unsigned char new_value)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S8))) = new_value;
}

static int address_equal_to_8bit(unsigned int address, unsigned char value)
{
    unsigned char value_read;
    value_read = *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S8)));
    return value_read == value;
}

static int address_equal_to_16bit(unsigned int address, unsigned short value)
{
    unsigned short value_read;
    value_read = *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16)));
    return value_read == value;
}

// individual application - returns 0 if we are supposed to skip the next cheat
// (only happens on conditional codes)
static int execute_cheat(unsigned int address, unsigned short value)
{
    switch (address & 0xFF000000)
    {
        case 0x80000000:
            address = 0x80000000 | (address & 0xFFFFFF);
            update_address_8bit(address,value);
            return 1;
            break;
        case 0x81000000:
            address = 0x81000000 | (address & 0xFFFFFF);
            update_address_16bit(address,value);
            return 1;
            break;
        case 0xA0000000:
            address = 0xA0000000 | (address & 0xFFFFFF);
            update_address_8bit(address,value);
            return 1;
            break;
        case 0xA1000000:
            address = 0xA1000000 | (address & 0xFFFFFF);
            update_address_16bit(address,value);
            return 1;
            break;
        case 0xD0000000:
            address = 0xD0000000 | (address & 0xFFFFFF);
            return address_equal_to_8bit(address,value);
            break;
        case 0xD1000000:
            address = 0xD1000000 | (address & 0xFFFFFF);
            return address_equal_to_16bit(address,value);
            break;
        case 0xD2000000:
            address = 0xD2000000 | (address & 0xFFFFFF);
            return !(address_equal_to_8bit(address,value));
            break;
        case 0xD3000000:
            address = 0xD3000000 | (address & 0xFFFFFF);
            return !(address_equal_to_16bit(address,value));
            break;
        case 0xF0000000:
            address = 0xF0000000 | (address & 0xFFFFFF);
            update_address_8bit(address,value);
            return 1;
            break;
        case 0xF1000000:
            address = 0xF1000000 | (address & 0xFFFFFF);
            update_address_16bit(address,value);
            return 1;
            break;
        case 0xEE000000:
            // most likely, this doesnt do anything.
            execute_cheat(0xF1000318, 0x0040);
            execute_cheat(0xF100031A, 0x0000);
            return 1;
            break;
    }
}  

// public functions
void cheat_apply_cheats(int entry)
{
    int do_next, check_next = 0;
    list_node_t *node1, *node2;
    cheat_t *cheat;
    cheat_code_t *code;

    // if no cheats for current rom, return
    if(!g_Current) return;

    list_foreach(g_Current->cheats, node1)
    {
        cheat = (cheat_t *)node1->data;
        if(cheat->enabled)
        {
            switch(entry)
            {
                case ENTRY_BOOT:
                    list_foreach(cheat->cheat_codes, node2)
                    {
                        code = (cheat_code_t *)node2->data;
                        execute_cheat(code->address, code->value);
                    }
                    break;
                case ENTRY_VI:
                    list_foreach(cheat->cheat_codes, node2)
                    {
                        code = (cheat_code_t *)node2->data;

                        if(check_next)
                        {
                            if(do_next)
                                execute_cheat(code->address, code->value);

                            check_next = 0;
                        }
                        else if((code->address & 0xF0000000) == 0xD0000000)
                        {
                            do_next = execute_cheat(code->address, code->value);
                            check_next = 1;
                        }
                        else
                            execute_cheat(code->address, code->value);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/** cheat_read_config
 *   Read config file and populate list of supported cheats. Format of cheat file is:
 *
 *   {Some Game's CRC}
 *   name=Some Game
 *
 *   [Cheat Name 1]
 *   enabled=1
 *   XXXXXXXX YYYY <-- cheat code (address, new value)
 *   XXXXXXXX YYYY
 *   XXXXXXXX YYYY
 *
 *   [Cheat Name 2]
 *   enabled=0
 *   XXXXXXXX YYYY
 *   XXXXXXXX YYYY
 *   XXXXXXXX YYYY
 *
 *   {Another Game's CRC}
 *   name=Another Game
 *   ...
 */
void cheat_read_config(void)
{
    char path[PATH_MAX];
    FILE *f = NULL;
    char line[2048];

    rom_cheats_t *romcheat = NULL;
    cheat_t *cheat = NULL;
    cheat_code_t *cheatcode = NULL;

    snprintf(path, PATH_MAX, "%s%s", get_configpath(), CHEAT_FILENAME);
    f = fopen(path, "r");

    // if no cheat config file installed, exit quietly
    if(!f) return;

    // parse file lines
    while(!feof(f))
    {
        if( !fgets( line, 2048, f ) )
            break;

        trim(line);

        if(strlen(line) == 0 ||
           line[0] == '#')     // comment
            continue;

        // beginning of new rom section
        if (line[0] == '{' && line[strlen(line)-1] == '}')
        {
            romcheat = malloc(sizeof(rom_cheats_t));
            list_append(&g_Cheats, romcheat);
            memset(romcheat, 0, sizeof(rom_cheats_t));
            sscanf(line, "{%x %x}", &romcheat->crc1, &romcheat->crc2);
            continue;
        }

        // rom name (just informational)
        if(strncasecmp(line, "name=", 5) == 0)
        {
            romcheat->rom_name = strdup(strstr(line, "=")+1);
            continue;
        }

        // name of cheat
        if(line[0] == '[' && line[strlen(line)-1] == ']')
        {
            cheat = malloc(sizeof(cheat_t));
            memset(cheat, 0, sizeof(cheat_t));
            list_append(&romcheat->cheats, cheat);
            line[strlen(line)-1] = '\0'; // get rid of trailing ']'
            cheat->name = strdup(line+1);
            continue;
        }

        // cheat always enabled?
        if(strncasecmp(line, "enabled=", 8) == 0)
        {
            sscanf(line, "enabled=%d", &cheat->enabled);
            continue;
        }

        // else, line must be a cheat code
        cheatcode = malloc(sizeof(cheat_code_t));
        memset(cheatcode, 0, sizeof(cheat_code_t));
        list_append(&cheat->cheat_codes, cheatcode);
        sscanf(line, "%x %hx", &cheatcode->address, &cheatcode->value);
    }
    fclose(f);
}

/** cheat_write_config
 *   Write out all cheats to file
 */
void cheat_write_config(void)
{
    char path[PATH_MAX];
    FILE *f = NULL;

    list_node_t *node1, *node2, *node3;
    rom_cheats_t *romcheat = NULL;
    cheat_t *cheat = NULL;
    cheat_code_t *cheatcode = NULL;

    // if no cheats, don't bother writing out file
    if(list_empty(g_Cheats)) return;

    snprintf(path, PATH_MAX, "%s%s", get_configpath(), CHEAT_FILENAME);
    f = fopen(path, "w");
    if(!f)
        return;

    list_foreach(g_Cheats, node1)
    {
        romcheat = (rom_cheats_t *)node1->data;

        fprintf(f, "{%.8x %.8x}\n"
                "name=%s\n",
                romcheat->crc1,
                romcheat->crc2,
                romcheat->rom_name);

        list_foreach(romcheat->cheats, node2)
        {
            cheat = (cheat_t *)node2->data;

            fprintf(f, "\n[%s]\n", cheat->name);
            fprintf(f, "enabled=%d\n", cheat->enabled? 1 : 0);

            list_foreach(cheat->cheat_codes, node3)
            {
                cheatcode = (cheat_code_t *)node3->data;

                fprintf(f, "%.8x %.4hx\n", cheatcode->address, cheatcode->value);
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

/** cheat_delete_all
 *   Delete all cheat-related structures
 */
void cheat_delete_all(void)
{
    list_node_t *node1, *node2, *node3;
    rom_cheats_t *romcheat = NULL;
    cheat_t *cheat = NULL;
    cheat_code_t *cheatcode = NULL;

    list_foreach(g_Cheats, node1)
    {
        romcheat = (rom_cheats_t *)node1->data;

        free(romcheat->rom_name);
        list_foreach(romcheat->cheats, node2)
        {
            cheat = (cheat_t *)node2->data;

            free(cheat->name);
            list_foreach(cheat->cheat_codes, node3)
            {
                cheatcode = (cheat_code_t *)node3->data;
                free(cheatcode);
            }
            list_delete(&cheat->cheat_codes);
            free(cheat);
        }
        list_delete(&romcheat->cheats);
        free(romcheat);
    }
    list_delete(&g_Cheats);
    g_Current = NULL;
}

/** cheat_load_current_rom
 *   sets pointer to cheats for currently loaded rom.
 */
void cheat_load_current_rom(void)
{
    list_node_t *node;
    rom_cheats_t *rom_cheat = NULL;
    unsigned int crc1, crc2;

    if(!ROM_HEADER) return;

    list_foreach(g_Cheats, node)
    {
        rom_cheat = (rom_cheats_t *)node->data;

        if(g_MemHasBeenBSwapped)
        {
            crc1 = sl(rom_cheat->crc1);
            crc2 = sl(rom_cheat->crc2);
        }
        else
        {
            crc1 = rom_cheat->crc1;
            crc2 = rom_cheat->crc2;
        }

        if(crc1 == ROM_HEADER->CRC1 &&
           crc2 == ROM_HEADER->CRC2)
        {
            g_Current = rom_cheat;
            return;
        }
    }
    // not found
    g_Current = NULL;
}

void additional_enable_code()
{
    
    // for 0x80000200 - 0x80000300
    // new_value = 0;
}

void set_store_location(unsigned int address)
{
    return;
}

void enabler(unsigned int address)
{
    /* Used to select the executable entry point (0x80XXXXXX). 
    * This is necessary with games that utilize certain protection chips.
    *  This code is typically used in conjunction with a key code. 
    * The address specified can only be 0x80000000 - 0x80100000.
    *  Any address above 0x80100000 (EG 0xDE100400 0000)
    *  will default the entry point down to 0x80000400.
    */
     
    return;
}

