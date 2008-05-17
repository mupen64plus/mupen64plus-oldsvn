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

//These functions need to be moved.
#include "mupenIniApi.h"

//Should move into config file system.
#define CACHE_FILE "rombrowser2.cache"
#define DATABASE_FILE "mupen64plus.ini"
//This must be fixed...
#define MAGIC_HEADER "RCS}" 

void *rom_cache_system(void *_arg);
char cache_filename[PATH_MAX];

static const char *romextensions[] = 
{
 ".v64", ".z64", ".gz", ".zip", ".n64", NULL //".rom" causes to many false positives.
};

//When finished, move to header.
typedef struct centry
{
    char filename[PATH_MAX];
    char MD5[33]; // md5 code
    time_t timestamp;//Should it be in m_time or something more human friendly???
    mupenEntry* inientry; 
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

rom_cache romcache;

static void scan_dir2( const char *dirname );

void * rom_cache_system( void *_arg )
{
    int i;
    int caching_done = 0;
    int rebuild_cache = 0;

    sprintf(cache_filename, "%s%s", get_configpath(), CACHE_FILE);

       printf("[error] load_initial_cache() returned 0\n");
       remove(cache_filename);
       rebuild_cache_file();

    printf("[rom cache] done loading initial cache\n"); 

}

int rebuild_cache_file()
{
    char real_path[PATH_MAX];
    char buffer[32];
    const char *directory; 
    int i;

    printf("[rcs] Rebuilding cache file.\n");

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

    cache_entry *entry;

    gzwrite( gzfile, &romcache.length, sizeof(unsigned int) );

    if(romcache.length!=0)
       {
       entry = romcache.top;
       while (entry->next!=NULL)
            {
            printf("Saving ROM: %s to cache.\n", entry->inientry->goodname);

            gzwrite( gzfile, entry->filename, sizeof(char)*PATH_MAX);
            gzwrite( gzfile, entry->MD5, sizeof(char)*33);
            gzwrite( gzfile, &entry->timestamp, sizeof(time_t));

            entry = entry->next;
            }
        }

    gzclose( gzfile );

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
            rom_size = calculateMD5(entry->filename , entry->MD5);
            /* Something is VERY WRONG here, even with fill_header, 
           the rcs version of getting the files takes far too long.
           This may be a pthread issue. */

            if(!rom_size)
                {
                free(entry);
                continue;
                }

            entry->inientry = ini_search_by_md5(entry->MD5);

            //Something to deal with custom roms.
            //if(entry.inientry==NULL)
            //    { custom_search_by_md5(entry->MD5);

            entry->timestamp = sb.st_mtime;
            //Add code to search comment database.

            //Actually add rom to cache.
            if(romcache.length==0)
                {
                romcache.top = entry; 
                romcache.last = entry; 
                romcache.length++;
                }
            else
                {
                romcache.last->next = entry;
                romcache.last = entry;
                romcache.length++;
                }
            }
        }
    closedir( dir );
}

int load_initial_cache()
{
    FILE *f = NULL;
    long filesize = 0;
    cache_header header;
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
            { printf("[rcs] rom cache header is correct\n", header.MAGIC); }
        }

    // Close our file, it's no longer needed.
    fclose(f);

    return 1;
}

int cache_rom_list()
{
   char filename[PATH_MAX];
   FILE *f = NULL;

   // note: check headers first
   return 1;
}

/*
void rombrowser_readCache( void )
{
    char filename[PATH_MAX];
    gzFile *f;
    int i;
    SRomEntry *entry;

    snprintf( filename, PATH_MAX, "%srombrowser.cache", get_configpath() );
    f = gzopen( filename, "rb" );
    if( !f )
        return;

    // free old list
    for( i = 0; i < g_list_length( g_RomListCache ); i++ )
    {
        entry = (SRomEntry *)g_list_nth_data( g_RomListCache, i );
        free( entry );
    }
    g_list_free( g_RomListCache );
    g_RomListCache = NULL;

    // number of entries
    gzread( f, &g_iNumRoms, sizeof( g_iNumRoms ) );

    // entries
    for( i = 0; i < g_iNumRoms; i++ )
    {
        entry = malloc( sizeof( SRomEntry ) );
        if( !entry )
        {
            printf( "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            continue;
        }
        gzread( f, entry->cFilename, sizeof( char ) * PATH_MAX );
        gzread( f, &entry->info, sizeof( entry->info ) );

        if( access( entry->cFilename, F_OK ) < 0 )
            continue;

        romentry_fill( entry );

        // append to list
        g_RomListCache = g_list_append( g_RomListCache, entry );
    }

    gzclose( f );
}

*/