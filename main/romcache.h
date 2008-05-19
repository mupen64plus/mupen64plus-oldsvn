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
#include <limits.h> //PATH_MAX
//When finished, move to header.
typedef struct centry
{
    char filename[PATH_MAX];
    char MD5[33]; // md5 code
    time_t timestamp;//Should it be in m_time or something more human friendly???
    mupenEntry* inientry; 
    unsigned short countrycode;
    int romsize; //Hm... this should be unsigned everywhere.
    //comment* something to deal with comments.
    struct centry* next;
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
