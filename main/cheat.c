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
#include <stdlib.h>
#include <zlib.h>
#ifndef __WIN32__
#include "../main/winlnxdefs.h"
#else
#include <windows.h>
#endif
#include "../memory/memory.h"
#include "cheat.h"

extern char *rdramb;
cheatcode *cheats;
int numcheats;

void apply_cheats(int entry)
{
    int x;
    int do_next = 0;
    cheatcode *next_cheat;
    for (x = 0;x < numcheats;x++)
    {
       next_cheat = NULL;
       do_next = 1;
       if (entry == ENTRY_BOOT)
       {
           if ((cheats[x].address & 0xF0000000) == 0xF0000000)
           {
               if(cheats[x].enabled == 1)
               {
                   do
                   {
                       execute_cheat(cheats[x].address,cheats[x].value);
                       next_cheat = (cheatcode *)cheats[x].next_cheat;
                   } while (next_cheat != NULL);
               }
           }
       }
       if (entry == ENTRY_VI)
       {
           if(cheats[x].enabled == 1)
           {
               do
               {
                   if (do_next != 0)
                   {
                       do_next = execute_cheat(cheats[x].address,cheats[x].value);
                   }
                   else
                   {
                       do_next = 1;
                   }
                   next_cheat = (cheatcode *)cheats[x].next_cheat;
               } while (next_cheat != NULL);
           }
       }
    }
}

void load_cheats(char crcmatch[22])
{
    char buf[256];
    char buf2[256];
    char crc[22];
    char *cheatname;
    int i=0;
    int enabled;
    char *path = malloc(strlen(get_configpath())+1+strlen("cheats.gs"));
    strcpy(path, get_configpath());
    strcat(path, "cheats.gs");
    //gzFile f = gzopen(path, "rb");
    FILE *f = fopen(path, "rb");
    free(path);
    
    if (f==NULL)
    {
        fprintf(stderr,"File Error\n");
        return;
    }

    //gzgets(f, buf, 255);
    fgets(buf, 255, f);
    numcheats = strtol(buf, NULL, 16);
    cheats = (cheatcode *)malloc(strtol(buf, NULL, 10)*sizeof(cheatcode));
    memset(cheats,0,strtol(buf, NULL, 10)*sizeof(cheatcode));
    
    do
    {
nextcheat:
        //gzgets(f, buf, 255);
        fgets(buf, 255, f);
        if (buf[0] == ':')
        {
            strncpy(crc, buf+1, 21);
            printf ("CRC: %s\n",buf);
            
            if(strncmp(crc,crcmatch,21)==0) goto nextcheat;
            
            //gzgets(f, buf, 255);
            fgets(buf, 255, f);
            cheatname = (char *)malloc(255);
            strncpy(cheatname, buf, strlen(buf));
            printf ("Name: %s\n",buf);
            
            //gzgets(f, buf, 255);
            fgets(buf, 255, f);
            enabled = strtol(buf, NULL, 10);
            printf ("Enabled: %i\n",enabled);
            
            //gzgets(f, buf, 255);
            fgets(buf, 255, f);
            if(buf[0] == '<')
            {
                fgets(buf, 255, f);
                do
                {
                    //gzgets(f, buf, 255);
                    strncpy(buf2, buf, 8);
                    cheats[i].address = strtoul(buf2, NULL, 16);
                    //printf("address: %x vs. %s", cheats[i].address, buf2);
                    cheats[i].value = strtol(buf+8, NULL, 16);
                    //printf("address: %x vs. %s", cheats[i].value, buf);
                    cheats[i].cheatname = cheatname;
                    cheats[i].enabled = enabled;
                    //memcpy(cheats[i].crc,crc,21);
                    i++;
                    fgets(buf, 255, f);
                }
                while(buf[0] != '>');
                
            }
        }
    }
    //while (!gzeof(f));
    while (!feof(f));
    printf("Done loading cheats.\n");
    //gzclose(f);
    fclose(f);
}
    
int execute_cheat(unsigned int address, unsigned short value)
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
            disable_expansion_pack();
            return 1;
            break;
    }
}  

int address_equal_to_8bit(unsigned int address, unsigned char value)
{
    unsigned char value_read;
    value_read = *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S8)));
    if (value_read == value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int address_equal_to_16bit(unsigned int address, unsigned short value)
{
    unsigned short value_read;
    value_read = *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16)));
    if (value_read == value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// most likely, this doesnt do anything.
void disable_expansion_pack()
{
    execute_cheat(0xF1000318, 0x0040);
    execute_cheat(0xF100031A, 0x0000);
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

void update_address_16bit(unsigned int address, unsigned short new_value)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16))) = new_value;
}

void update_address_8bit(unsigned int address, unsigned char new_value)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S8))) = new_value;
}

