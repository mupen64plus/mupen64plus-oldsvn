/**
 * Mupen64 - romcache.c
 * Copyright (C) 2008 okaygo Tillin9
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <pthread.h>

#include <zlib.h>

#include <limits.h> //PATH_MAX
#include <dirent.h> //Directory support.
//Includes for POSIX file status.
#include <sys/stat.h>
#include <sys/types.h>

#include "main.h"
#include "util.h"
#include "romcache.h"
#include "config.h"
#include "rom.h"
#include "translate.h"
#include "../memory/memory.h"    // sl() Where is sl? Its not in memory.c

void clear_cache()
{
    cache_entry *entry, *entrynext;

    if(g_RomCache.length!=0)
        {
        entry = g_RomCache.top;
        do
            { 
            entrynext = entry->next;
            free(entry);
            entry = entrynext;
            --g_RomCache.length;
            }
        while (entry!=NULL);
        g_RomCache.last = NULL;
        }
}

void * rom_cache_system( void *_arg )
{
    int i;
    int caching_done = 0;
    int rebuild_cache = 0;

    sprintf(cache_filename, "%s%s", get_configpath(), CACHE_FILE);

    if(!load_initial_cache())
    { 
    printf("[rcs] load_initial_cache() returned 0\n"); 

    cache_entry *entry;

    remove(cache_filename);
    rebuild_cache_file();
    }
   
    printf("[rom cache] done loading initial cache\n"); 

}

int rebuild_cache_file()
{
    char real_path[PATH_MAX];
    char buffer[32];
    const char *directory; 
    int i;

    clear_cache();

    printf("[rcs] Rebuilding cache file. %s\n",cache_filename);

    for ( i = 0; i < config_get_number("NumRomDirs",0); ++i )
        {
        sprintf(buffer,"RomDirectory[%d]",i);
        directory = config_get_string(buffer,"");
        printf("Scanning... %s\n",directory);
        scan_dir2(directory);
        }

    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"wb"))==NULL)
        {
        printf("[error] could not create %s\n",cache_filename);
        return 0;
        }
        
    gzwrite( gzfile, MAGIC_HEADER, 4);
    gzwrite( gzfile, &g_RomCache.length, sizeof(unsigned int) );

    if(g_RomCache.length!=0)
       {
       cache_entry *entry;
       entry = g_RomCache.top;
       do
            {
            gzwrite( gzfile, entry->filename, sizeof(char)*PATH_MAX);
            gzwrite( gzfile, &entry->timestamp, sizeof(time_t));
            gzwrite( gzfile, &entry->info, sizeof(rominfo));
             
            entry = entry->next;
            }
        while (entry!=NULL);
        }

    gzclose(gzfile);

}

static void scan_dir2( const char *dirname )
{
    int rom_size, i;
    char filename[PATH_MAX];
    char real_path[PATH_MAX];
    char *extension;

    DIR *dir; 
    struct dirent *de;
    struct stat sb; //Give sb and de better names...

    cache_entry *entry;
    int found;

    dir = opendir( dirname );
    if(!dir)
        {
        printf( "Couldn't open directory '%s': %s\n", dirname, strerror( errno ) );
        return;
        }

    while( (de = readdir( dir )) )
        {
        if( de->d_name[0] == '.' ) // .., . or hidden file
            { continue; }
        snprintf( filename, PATH_MAX, "%s%s", dirname, de->d_name );

        //Use real path (maybe it's a link)
        if(realpath(filename,real_path))
            { strncpy(real_path, filename, PATH_MAX); }

        //If we can't get information, move to next file.
        if(stat(real_path,&sb)==-1)
            { continue; }

        //Maybe probe errno and provide feedback?  ^
        //To be consistent or removed from above   |

        //Handle recursive scanning.
        if( config_get_bool( "RomDirsScanRecursive", 0 ) )
            {
            if( S_ISDIR(sb.st_mode) )
                {
                strncat(filename, "/", PATH_MAX);
                scan_dir2(filename);
                continue;
                }
            }

        //Check if file has a supported extension. 
        extension = strrchr( filename, '.' );
        if(!extension)
            { continue; }
        for( i = 0; romextensions[i]; ++i )
            {
            if(!strcasecmp(extension,romextensions[i]))
                { break; }
            }
        if(!romextensions[i])
            { continue; }

        entry = (cache_entry*)calloc(1,sizeof(cache_entry));
        if( !entry )
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            continue;
            }

        //This needs to be converted to something more useful...
        //printf("Modified %d\n", sb.st_mtime);

        //Found controls whether its found in the cache.
        //We need to add this.
        found = 0;
        if(!found)
            {
            strcpy(entry->filename,filename);
            //Test if we're a valid rom and compute MD5.
            
             memcpy(&entry->info,(rominfo *)fill_rominfo(entry->filename),sizeof(rominfo));
          
            /* Something is VERY WRONG here, even with fill_header, 
           the rcs version of getting the files takes far too long.
           This may be a pthread issue. */
            // rom_size = entry->info.iSize;
            if(&entry->info == NULL)
                {
                free(entry);
                continue;
                }
                     
            //Something to deal with custom roms.
            //if(entry.inientry==NULL)
            //    { custom_search_by_md5(entry->MD5);

            entry->timestamp = sb.st_mtime;
            //Add code to search comment database.

            //Actually add rom to cache.
            if(g_RomCache.length==0)
                {
                g_RomCache.top = entry; 
                g_RomCache.last = entry; 
                ++g_RomCache.length;
                }
            else
                {
                g_RomCache.last->next = entry;
                g_RomCache.last = entry;
                ++g_RomCache.length;
                }
            }
            printf("[rcs] rom cached\n");
            update_rombrowser(entry);
            usleep(10);
        }
    closedir(dir);

}

int load_initial_cache()
{
    int i;
    char magicheader[8] = "";
    cache_entry *entry, *entrynext;

    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"rb"))==NULL)
        {
        printf("[rcs] Error, could not open %s\n",cache_filename);
        return 0;
        }
    if(gzread(gzfile, &magicheader, 4))
    {
    	printf("[rcs] reading header...\n");
    	printf("[rcs] header: %s\n",(magicheader));
        if (strcmp(MAGIC_HEADER,magicheader))
        {
        	printf("[rcs] header mismatch, this is normal for a first time boot.\n");
            return 0;
        }
    } else { printf("[rcs] cant read file...\n"); return 0; }
    printf("[rcs] reading romcache length\n");
    
    if(gzread(gzfile, &g_RomCache.length, sizeof(unsigned int)))
        {
        	
        for ( i = 0; i < g_RomCache.length; ++i )
            { 
            entry = (cache_entry*)calloc(1,sizeof(cache_entry));

            if(!entry)
                {
                fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                return 0;
                }
            
            if(!gzread(gzfile, entry->filename, sizeof(char)*PATH_MAX)||
               !gzread(gzfile, &entry->timestamp, sizeof(time_t))||
               !gzread(gzfile, &entry->info, sizeof(rominfo)))
                {
                if(!gzeof(gzfile)) //gzread error.
                    { 
                    clear_cache();
                    printf("[rcs] Error, cache file corrupt.\n");
                    return 0;
                    }
                }
            
            //Actually add rom to cache.
            if(entry->MD5!=NULL)
                {
                // entry->inientry = ini_search_by_md5(entry->MD5);
                update_rombrowser(entry);
                
                if(i==0)
                    {
                    g_RomCache.top = entry; 
                    g_RomCache.last = entry; 
                    }
                else
                    {
                    g_RomCache.last->next = entry;
                    g_RomCache.last = entry;
                    }
                
                }
            }
        }

    gzclose(gzfile);

    printf("[rcs] Cache file read.\n");
    return 1;
}

