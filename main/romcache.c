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
#define MAGIC_HEADER "RCS}"
    
void *rom_cache_system(void *_arg);
char cache_filename[PATH_MAX];

static const char *g_romFileExtensions[] = 
{
    ".rom", ".v64", ".z64", ".gz", ".zip", ".n64", ".bin", NULL
};

list_t g_RomBrowserCache = NULL; // list of cache_entrys


void * rom_cache_system( void *_arg )
{
    int caching_done = 0;
    int rebuild_cache = 0;
    
    sprintf(cache_filename, "%s%s", get_configpath(), CACHE_FILE);
    
    if (load_initial_cache() == 0)
    {
        printf("[error] load_initial_cache() returned 0\n");
        remove(cache_filename);
        rebuild_cache_file();
    }
    else
    {
        printf("[rom cache] done loading initial cache\n");
    }
    
}

int rebuild_cache_file()
{
	printf("[rcs] rebuilding the cache file\n");
    FILE *f = NULL;
    unsigned int dummy_zero = 10;
    f = fopen(cache_filename,"wb");
    if (!f)
    {
    	printf("[error] could not create %s\n",cache_filename);
        return 0;
    }
    
    fwrite(MAGIC_HEADER,sizeof(MAGIC_HEADER),1,f);
    fwrite(dummy_zero,1,sizeof(int),f);
}

int load_initial_cache()
{
    FILE *f = NULL;
    long filesize = 0;
    cache_header header;
    cache_entry *cache_data;
    int num_entrys = 0;
    
    f = fopen(cache_filename,"r");
    
    if (!f)
    {
    	printf("[error] Could not load %s\n",cache_filename);
        return 0;
    }
    
    // Grab the filesize.
    fseek(f,0,SEEK_END);
    filesize = ftell(f);
    rewind(f);
    
    // Allocate enough memory for the cache data, then close the file.
    if(fread(&header,sizeof(header),1,f) != sizeof(header))
    {
    	
        if (strcmp(header.MAGIC,MAGIC_HEADER))
        {
        	printf("[error] the rom cache header is malformed.\n");
            return 0;
        }
        else
        {
        	printf("[rcs] rom cache header is correct\n", header.MAGIC);
        }
        
        num_entrys = header.entries;
        cache_data = malloc(num_entrys * sizeof(cache_entry));
        if (cache_data == NULL)
        {
            printf("[error] Could not allocate memory for cache_data. (%i bytes)\n",(sizeof(cache_entry)*num_entrys));
            return 0;
        }        
    }

    // Read our data into our array.
    if(fread(cache_data,sizeof(cache_entry),num_entrys,f) != (sizeof(cache_entry)*num_entrys))
    {
        printf("[error] malformed rom cache structure.\n");
        return 0;
    }
    
    // Close our file, it's no longer needed.
    fclose(f);

    // Free up the cache data
    free(cache_data);
    
    return 1;    
}

int cache_rom_list()
{
	char filename[PATH_MAX];
	FILE *f = NULL;
	// note: check headers first
	return 1;
}



