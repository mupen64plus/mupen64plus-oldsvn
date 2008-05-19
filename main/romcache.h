/**
 * Mupen64 - romcache.h
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
//These functions need to be moved.
#include "mupenIniApi.h"

//Should move into config file system.
#define CACHE_FILE "rombrowser.cache"
#define DATABASE_FILE "mupen64plus.ini"
//This must be fixed...
#define MAGIC_HEADER "RCS}" 

void *rom_cache_system(void *_arg);
char cache_filename[PATH_MAX];

typedef struct {
    char MAGIC[4];
    int entries;
} cache_header;


static const char *romextensions[] = 
{
 ".v64", ".z64", ".gz", ".zip", ".n64", NULL //".rom" causes to many false positives.
};
    // rom info
typedef struct
{
        char          cName[20];                    // rom name
        int           iSize;                            // size in bytes
        unsigned char cCountry;                     // country id
        unsigned int  iCRC1;                            // crc part 1
        unsigned int  iCRC2;                            // crc part 2
        char          cMD5[33];                     // md5 code
        char          cGoodName[256];           // from ini
} rominfo; // data saved in cache
//When finished, move to header.
typedef struct centry
{
    char filename[PATH_MAX];
    char MD5[33]; // md5 code
    time_t timestamp;//Should it be in m_time or something more human friendly???
    rominfo info; // lots of information for quick building of rombrowser
    // mupenEntry* inientry; 
    //comment* something to deal with comments.
    struct centry* next;
} cache_entry;

/* Okay... new paradigm.

romcache - the cache.
romdatabase - the database, currently linked lists from mupenIniApi.c
comments - user comments.
customdatabase - user editable ini for non-Goodnamed ROMS to handle corner cases.

GUI polls romcache, using the database entry pointer to build 
its GtkTreeView or KDE ListView.
*/

//Use custom linked list. 
typedef struct
{
    unsigned int length; 
    cache_entry *top;
    cache_entry *last;
} rom_cache;

static void scan_dir2( const char *dirname );
int rebuild_cache_file();
int load_initial_cache();
void *rom_cache_system(void *_arg);
rom_cache g_RomCache;
