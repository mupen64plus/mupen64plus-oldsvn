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

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "main.h"
#include "util.h"
#include "romcache.h"
#include "config.h"

#define CACHE_FILE "rombrowser2.cache"
#define DATABASE_FILE "mupen64plus.ini"

void *rom_cache_system(void *_arg);

static const char *g_romFileExtensions[] = 
{
    ".rom", ".v64", ".z64", ".gz", ".zip", ".n64", ".bin", NULL
};

list_t g_RomBrowserCache = NULL; // list of cache_entrys


void * rom_cache_system( void *_arg )
{
    int caching_done = 0;
    
    // if gui
    if (load_initial_cache() == 0)
    {
        printf("[error] load_initial_cache() returned 0\n");
    }
    else
    {
        printf("[rom cache] done loading initial cache\n");
    }
    
}


int load_initial_cache()
{
    char filename[PATH_MAX];
    FILE *f = NULL;
    long filesize = 0;
    char *cache_data;
    cache_entry cacheinfo = NULL;
    
    sprintf(filename, "%s%s", get_configpath(), CACHE_FILE);
    f = fopen(filename,"r");
    
    if (!f)
    {
    	printf("[error] Could not load %s\n",filename);
        return 0;
    }
    
    // Grab the filesize.
    fseek(f,0,SEEK_END);
    filesize = ftell(f);
    rewind(f);
    
    // Allocate enough memory for the cache data, then close the file.
    cache_data = malloc(filesize + 1);
    if (cache_data == NULL)
    {
        printf("[error] Could not allocate memory for cache_data. (%i bytes)\n",filesize);
        return 0;
    }
    
    // Read our data into our array.
    if(fread(cache_data,1,filesize,f) != filesize)
    {
        printf("[error] Could not read rom cache file.\n");
        return 0;
    }
    
    // Close our file, it's no longer needed.
    fclose(f);
        
    char *this_line = cache_data;
    
    while(this_line != NULL)
    {
        char *next_line = NULL;
        char *end_of_line = strchr(this_line, '\n');
        if (end_of_line != NULL)
        {
            *end_of_line = 0;
            next_line = end_of_line + 1;
            
            /* This next section should start building the g_RomBrowserCache */
            sscanf(this_line, "%s,%s,%s,%i",cacheinfo.MD5,cacheinfo.goodname,cacheinfo.path,cacheinfo.os_timestamp);
            
        }
        
        this_line = next_line;
    }
      
    // Free up the cache data
    free(cache_data);
    return 1;    
}



