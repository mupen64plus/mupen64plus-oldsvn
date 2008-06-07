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
#include <limits.h> //PATH_MAX
#define COMMENT_MAXLENGTH 256

#include "md5.h"

typedef struct
{
   char* goodname;
   md5_byte_t md5[16];
   md5_byte_t refmd5[16];
   unsigned int crc1;
   unsigned int crc2;
   unsigned short status;
   unsigned short savetype;
} mupenEntry;

void ini_openFile();
void romdatabase_close();
mupenEntry* ini_search_by_md5(md5_byte_t* md5);
mupenEntry* ini_search_by_crc(unsigned int crc1, unsigned int crc2);

enum RCS_TASK
{
    RCS_INIT = 1,
    RCS_RESCAN,
    RCS_SLEEP,
    RCS_BUSY,
    RCS_SHUTDOWN,
    RCS_WRITE_CACHE //For user comments.
};

enum RCS_TASK g_RCSTask;
//When finished, move to header.

//Needs to be rearranged.
typedef struct _cache_entry
{
    char filename[PATH_MAX];
    md5_byte_t md5[16];
    time_t timestamp;
    unsigned short countrycode;
    unsigned short compressiontype;
    unsigned short imagetype;
    unsigned short cic;
    unsigned int archivefile; //Not currently used, for locating file inside zip or 7zip archives.
    int romsize;
    char comment[COMMENT_MAXLENGTH]; 
    char internalname[80]; //Needs to be 4 times the stored value for UTF8 conversion. 
    unsigned int crc1;
    unsigned int crc2;
    mupenEntry *inientry;
    struct _cache_entry *next;
} cache_entry;

//Use custom linked list. 
typedef struct
{
    unsigned int length; 
    cache_entry *top;
    cache_entry *last;
} rom_cache;

rom_cache romcache;

int rebuild_cache_file();
int load_initial_cache();
void *rom_cache_system(void *_arg);
