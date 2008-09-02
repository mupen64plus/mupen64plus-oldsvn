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

#include "../memory/memory.h"
#include "cheat.h"
#include "main.h"
#include "rom.h"
#include "util.h" // list utilities
#include "config.h"
#include "ini_reader.h"

// this seems stupid
// #define CHEAT_CODE_MAGIC_VALUE 0xcafe // use this to know that old_value is uninitialized

#define CHEAT_FILENAME "mupen64plus.cht"
static ini_file *cheat_file = NULL;
static ini_section *current_rom_section = NULL;

// private functions
static unsigned short read_address_16bit(unsigned int address)
{
    return *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16)));
}

static unsigned short read_address_8bit(unsigned int address)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S8)));
}

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
// (only really used on conditional codes)
static int execute_cheat(unsigned int address, unsigned short value, unsigned short *old_value)
{
//     switch (address & 0xFF000000)
//     {
//         case 0x80000000:
//         case 0x88000000:
//         case 0xA0000000:
//         case 0xA8000000:
//         case 0xF0000000:
//             // if pointer to old value is valid and uninitialized, write current value to it
//             if(old_value && *old_value == CHEAT_CODE_MAGIC_VALUE)
//                 *old_value = read_address_8bit(address);
//             update_address_8bit(address,value);
//             return 1;
//             break;
//         case 0x81000000:
//         case 0x89000000:
//         case 0xA1000000:
//         case 0xA9000000:
//         case 0xF1000000:
//             // if pointer to old value is valid and uninitialized, write current value to it
//             if(old_value && *old_value == CHEAT_CODE_MAGIC_VALUE)
//                 *old_value = read_address_16bit(address);
//             update_address_16bit(address,value);
//             return 1;
//             break;
//         case 0xD0000000:
//         case 0xD8000000:
//             return address_equal_to_8bit(address,value);
//             break;
//         case 0xD1000000:
//         case 0xD9000000:
//             return address_equal_to_16bit(address,value);
//             break;
//         case 0xD2000000:
//         case 0xDB000000:
//             return !(address_equal_to_8bit(address,value));
//             break;
//         case 0xD3000000:
//         case 0xDA000000:
//             return !(address_equal_to_16bit(address,value));
//             break;
//         case 0xEE000000:
//             // most likely, this doesnt do anything.
//             execute_cheat(0xF1000318, 0x0040, NULL);
//             execute_cheat(0xF100031A, 0x0000, NULL);
//             return 1;
//             break;
//         default:
//             return 1;
//             break;
//     }
}

static int gs_button_pressed(void)
{
    return key_pressed('g') ||
           key_pressed('G') ||
           event_active(config_get_string("Joy Mapping GS Button", ""));
}

// public functions
void cheat_apply_cheats(int entry)
{
//     list_node_t *node1, *node2;
//     cheat_t *cheat;
//     cheat_code_t *code;
// 
//     // if no cheats for current rom, return
//     if(!g_Current) return;
// 
//     list_foreach(g_Current->cheats, node1)
//     {
//         cheat = (cheat_t *)node1->data;
//         if(cheat->always_enabled || cheat->enabled)
//         {
//             cheat->was_enabled = 1;
//             switch(entry)
//             {
//                 case ENTRY_BOOT:
//                     list_foreach(cheat->cheat_codes, node2)
//                     {
//                         code = (cheat_code_t *)node2->data;
// 
//                         // code should only be written once at boot time
//                         if((code->address & 0xF0000000) == 0xF0000000)
//                             execute_cheat(code->address, code->value, &code->old_value);
//                     }
//                     break;
//                 case ENTRY_VI:
//                     list_foreach(cheat->cheat_codes, node2)
//                     {
//                         code = (cheat_code_t *)node2->data;
// 
//                         // conditional cheat codes
//                         if((code->address & 0xF0000000) == 0xD0000000)
//                         {
//                             // if code needs GS button pressed and it's not, skip it
//                             if(((code->address & 0xFF000000) == 0xD8000000 ||
//                                 (code->address & 0xFF000000) == 0xD9000000 ||
//                                 (code->address & 0xFF000000) == 0xDA000000 ||
//                                 (code->address & 0xFF000000) == 0xDB000000) &&
//                                !gs_button_pressed())
//                             {
//                                 // skip next code
//                                 if(node2->next != NULL)
//                                     node2 = node2->next;
//                                 continue;
//                             }
// 
//                             // if condition true, execute next cheat code
//                             if(execute_cheat(code->address, code->value, NULL))
//                             {
//                                 node2 = node2->next;
//                                 code = (cheat_code_t *)node2->data;
// 
//                                 // if code needs GS button pressed, don't save old value
//                                 if(((code->address & 0xFF000000) == 0xD8000000 ||
//                                     (code->address & 0xFF000000) == 0xD9000000 ||
//                                     (code->address & 0xFF000000) == 0xDA000000 ||
//                                     (code->address & 0xFF000000) == 0xDB000000))
//                                    execute_cheat(code->address, code->value, NULL);
//                                 else
//                                    execute_cheat(code->address, code->value, &code->old_value);
//                             }
//                             // if condition false, skip next code
//                             else
//                             {
//                                 if(node2->next != NULL)
//                                     node2 = node2->next;
//                                 continue;
//                             }
//                         }
//                         // GS button triggers cheat code
//                         else if((code->address & 0xFF000000) == 0x88000000 ||
//                                 (code->address & 0xFF000000) == 0x89000000 ||
//                                 (code->address & 0xFF000000) == 0xA8000000 ||
//                                 (code->address & 0xFF000000) == 0xA9000000)
//                         {
//                             if(gs_button_pressed())
//                                 execute_cheat(code->address, code->value, NULL);
//                         }
//                         // normal cheat code
//                         else
//                         {
//                             // exclude boot-time cheat codes
//                             if((code->address & 0xF0000000) != 0xF0000000)
//                                 execute_cheat(code->address, code->value, &code->old_value);
//                         }
//                     }
//                     break;
//                 default:
//                     break;
//             }
//         }
//         // if cheat was enabled, but is now disabled, restore old memory values
//         else if(cheat->was_enabled)
//         {
//             cheat->was_enabled = 0;
//             switch(entry)
//             {
//                 case ENTRY_VI:
//                     list_foreach(cheat->cheat_codes, node2)
//                     {
//                         code = (cheat_code_t *)node2->data;
//               
//                         // set memory back to old value and clear saved copy of old value
//                         if(code->old_value != CHEAT_CODE_MAGIC_VALUE)
//                         {
//                             execute_cheat(code->address, code->old_value, NULL);
//                             code->old_value = CHEAT_CODE_MAGIC_VALUE;
//                         }
//                     }
//                     break;
//                 default:
//                     break;
//             }
//         }
//     }
}

void cheat_read_config(void)
{
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s%s", get_installpath(), CHEAT_FILENAME);
    cheat_file = ini_file_parse(buf);
}

void cheat_write_config(void)
{

}

void cheat_delete_all(void)
{
    ini_file_free(&cheat_file);
}

void cheat_unload_current_rom(void)
{
    current_rom_section = NULL;
}

/** cheat_load_current_rom
 *   sets pointer to cheats for currently loaded rom.
 */
void cheat_load_current_rom(void)
{
    /* The title is always 22 chars long. Add one for the \0 */
    char buf[23];
    list_node_t *node = NULL;
    ini_section *section = NULL;

    if (ROM_HEADER && cheat_file)
    {
        snprintf(buf, 23, "%X-%X-C:%X",
                 g_MemHasBeenBSwapped ? sl(ROM_HEADER->CRC1) : ROM_HEADER->CRC1,
                 g_MemHasBeenBSwapped ? sl(ROM_HEADER->CRC2) : ROM_HEADER->CRC2,
                 ROM_HEADER->Country_code);
        list_foreach(cheat_file->sections, node)
        {
            section = node->data;
            if (strcasecmp(buf, section->title) == 0)
            {
                puts("FOUND CHEAT ENTRY");
                current_rom_section = section;
                break;
            }
        }
    }
}

static cheat_t *find_or_create_cheat(list_t *list, int number)
{
    list_t node = NULL;
    cheat_t *cheat = NULL;
    int found = 0;

    list_foreach(*list, node)
    {
        cheat = node->data;
        if (cheat->number == number)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        cheat = malloc(sizeof(cheat_t));
        cheat->name = NULL;
        cheat->comment = NULL;
        cheat->number = number;
        cheat->codes = NULL;
        cheat->options = NULL;
        list_append(list, cheat);
    }

    return cheat;
}

static list_t tokenize_string(const char *string, const char* delim)
{
    list_t list = NULL;
    char *token = NULL, *wrk = NULL;
    char buf[4096]; // some of those strings are really long
    strncpy(buf, string, 4096);

    token = strtok(buf, delim);
    if (token)
    {
        wrk = malloc(strlen(token) + 1);
        strcpy(wrk, token);
        list_append(&list, wrk);
    }

    while ((token = strtok(NULL, delim)))
    {
        wrk = malloc(strlen(token) + 1);
        strcpy(wrk, token);
        list_append(&list, wrk);
    }
    return list;
}

list_t cheats_for_current_rom()
{
    list_t node = NULL;
    list_t node2 = NULL;
    list_t list = NULL;
    list_t temp = NULL;
    ini_entry *entry = NULL;
    cheat_t *cheat = NULL;
    int n = 0, len = 0;
    int value = 0;
    char buf[PATH_MAX];
    cheat_option_t *option;

    if (!current_rom_section)
    {
        return NULL;
    }

    list_foreach(current_rom_section->entries, node)
    {
        entry = node->data;
        len = strlen(entry->key);
        if (strcmp("_O", entry->key + len - 2) == 0) // Option for a cheat
        {
            if (sscanf(entry->key, "Cheat%d_O", &n) == 1)
            {
                cheat = find_or_create_cheat(&list, n);
                temp = tokenize_string(entry->value, ",");
                node2 = NULL;
                list_foreach(temp, node2)
                {
                    memset(buf, '\0', PATH_MAX);
                    if (sscanf(node2->data, "$%x %[a-zA-Z0-9 ]", &value, &buf) == 2)
                    {
                        printf("Parsed %02X %s\n", value, buf);
                        option = malloc(sizeof(cheat_option_t));
                        option->code = value;
                        option->description = malloc(strlen(buf) + 1);
                        strcpy(option->description, buf);
                        list_append(&cheat->options, option);
                    }
                    free(node2->data);
                }
                list_delete(&temp);
            }
        }
        else if (strcmp("_N", entry->key + len - 2) == 0) // Comment for a cheat
        {
            if (sscanf(entry->key, "Cheat%d_N", &n) == 1)
            {
                cheat = find_or_create_cheat(&list, n);
                cheat->comment = malloc(strlen(entry->value) + 1);
                strcpy(cheat->comment, entry->value);
            }
        }
        else if (sscanf(entry->key, "Cheat%d", &n) == 1)
        {
            cheat = find_or_create_cheat(&list, n);
            cheat->codes = tokenize_string(entry->value, ",");
            cheat->name = list_first_data(cheat->codes);
            list_node_delete(&cheat->codes, list_first_node(cheat->codes));
        }

        cheat = NULL;
    }

    return list;
}

void cheats_free(list_t *cheats)
{
    cheat_t *cheat = NULL;
    cheat_option_t *option = NULL;
    list_t node1, node2;
    node1 = node2 = NULL;

    list_foreach(*cheats, node1)
    {
        cheat = node1->data;

        free(cheat->name);
        free(cheat->comment);

        list_foreach(cheat->codes, node2)
        {
            free(node2->data);
        }
        list_delete(&cheat->codes);

        list_foreach(cheat->options, node2)
        {
            option = node2->data;
            free(option->description);
            free(option);
        }
        list_delete(&cheat->options);
    }

    list_delete(cheats);
}

/** cheat_new_rom
 *   creates a new rom_cheats_t structure, appends it to the global list and returns it.
 */
// rom_cheats_t *cheat_new_rom(void)
// {
//     rom_cheats_t *romcheat = malloc(sizeof(rom_cheats_t));
// 
//     if(!romcheat)
//         return NULL;
// 
//     memset(romcheat, 0, sizeof(rom_cheats_t));
//     list_append(&g_Cheats, romcheat);
// 
//     return romcheat;
// }

/** cheat_new_cheat
 *   creates a new cheat_t structure, appends it to the given rom_cheats_t struct and returns it.
 */
// cheat_t *cheat_new_cheat(rom_cheats_t *romcheat)
// {
//     cheat_t *cheat = malloc(sizeof(cheat_t));
// 
//     if(!cheat)
//         return NULL;
// 
//     memset(cheat, 0, sizeof(cheat_t));
//     list_append(&romcheat->cheats, cheat);
// 
//     return cheat;
// }

/** cheat_new_cheat_code
 *   creates a new cheat_code_t structure, appends it to the given cheat_t struct and returns it.
 */
// cheat_code_t *cheat_new_cheat_code(cheat_t *cheat)
// {
//     cheat_code_t *code = malloc(sizeof(cheat_code_t));
// 
//     if(!code)
//         return NULL;
// 
//     memset(code, 0, sizeof(cheat_code_t));
//     code->old_value = CHEAT_CODE_MAGIC_VALUE; // initialize old_value
//     list_append(&cheat->cheat_codes, code);
// 
//     return code;
// }

/** cheat_delete_rom
 *   deletes given rom structure and removes it from the global list.
 */
// void cheat_delete_rom(rom_cheats_t *romcheat)
// {
//     list_node_t *romnode, *node1, *node2;
//     cheat_t *cheat;
//     cheat_code_t *cheatcode;
// 
//     if(!romcheat)
//         return;
// 
//     if(romcheat->rom_name)
//         free(romcheat->rom_name);
// 
//     // remove any cheats associated with this rom
//     list_foreach(romcheat->cheats, node1)
//     {
//         cheat = (cheat_t *)node1->data;
// 
//         if(cheat->name)
//             free(cheat->name);
// 
//         // remove any codes associated with this cheat
//         list_foreach(cheat->cheat_codes, node2)
//         {
//             cheatcode = (cheat_code_t *)node2->data;
//             if(cheatcode)
//                 free(cheatcode);
//         }
//         list_delete(&cheat->cheat_codes);
//         free(cheat);
//     }
//     list_delete(&romcheat->cheats);
// 
//     // locate node associated with rom
//     romnode = list_find_node(g_Cheats, romcheat);
// 
//     // free rom and remove it from the list
//     free(romcheat);
//     list_node_delete(&g_Cheats, romnode);
// }

/** cheat_delete_cheat
 *   deletes given cheat structure and removes it from the given rom's cheat list.
 */
// void cheat_delete_cheat(rom_cheats_t *romcheat, cheat_t *cheat)
// {
//     list_node_t *cheatnode, *node;
//     cheat_code_t *cheatcode;
// 
//     if(!cheat)
//         return;
// 
//     if(cheat->name)
//         free(cheat->name);
// 
//     // remove any codes associated with this cheat
//     list_foreach(cheat->cheat_codes, node)
//     {
//         cheatcode = (cheat_code_t *)node->data;
//         if(cheatcode)
//             free(cheatcode);
//     }
//     list_delete(&cheat->cheat_codes);
// 
//     // locate node associated with cheat
//     cheatnode = list_find_node(romcheat->cheats, cheat);
// 
//     // free cheat and remove it from the rom's cheat list
//     free(cheat);
//     list_node_delete(&romcheat->cheats, cheatnode);
// }

/** cheat_delete_cheat_code
 *   deletes given cheat code structure and removes it from the given cheat's code list.
 */
// void cheat_delete_cheat_code(cheat_t *cheat, cheat_code_t *cheatcode)
// {
//     list_node_t *codenode;
// 
//     if(!cheatcode)
//         return;
// 
//     // locate node associated with cheat
//     codenode = list_find_node(cheat->cheat_codes, cheatcode);
// 
//     // free cheat code and remove it from the cheat's code list
//     free(cheatcode);
//     list_node_delete(&cheat->cheat_codes, codenode);
// }
