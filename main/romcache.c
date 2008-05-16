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
typedef struct
{
    char filename[PATH_MAX];
    char MD5[33]; // md5 code
    //OS timestamp... ?Should it be in m_time or something more human friendly..
    mupenEntry* inientry; //Okay since ini database is persistent and global, no romdata. GUI can handle numbering.
    //comment* something to deal with comments.
} cacheentry;

/* Okay... new paradigm.

romcache - the cache.
romdatabase - the database, currently linked lists from mupenIniApi.c
comments - user comments.
customdatabase - user editable ini for non-Goodnamed ROMS to handle corner cases.

GUI polls romcache, using the database entry pointer to build 
its GtkTreeView or KDE ListView.
*/

//Linked list, why is #include <list.h> not needed???
//Possible better data structure...
//Possible better name... 
list_t romcache = NULL; 

static void scan_dir2( const char *dirname );

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
        { printf("[rom cache] done loading initial cache\n"); }
}

int rebuild_cache_file()
{
    char filename[PATH_MAX];
    char real_path[PATH_MAX];
    char buffer[30];
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

    //Write the cache to file.
    //Right now we don't generate the linked list, so this doesn't work.
    FILE *file = NULL;
    unsigned int dummy_zero = 10;
    file = fopen(cache_filename,"wb");
    if (!file)
        {
        printf("[error] could not create %s\n",cache_filename);
        return 0;
        }

    fwrite(MAGIC_HEADER,1,sizeof(MAGIC_HEADER),file);
    fwrite(&dummy_zero,1,sizeof(int),file);
}


//RENAME, this function chops off the file from a path,
//also better to move to some kind of common GUI function.
char *sub_string2(const char *string, int start, int end)
{
    int length = end-start;
    if(length <= 0)
        { return NULL; }

    char *alloc = (char*)malloc(length+1);
    int i;
    for(i = 0; i < length; i++)
        { alloc[i] = string[i+start]; }
    alloc[length] = '\0';

    return alloc;
}


static void scan_dir2( const char *dirname )
{
    int rom_size, i;
    char filename[PATH_MAX];
    char real_path[PATH_MAX];
    char *line, *extension; //line just temp buffer for debugging
    mupenEntry* inientry; //We need better names for types >.<

    DIR *dir; 
    struct dirent *de;
    struct stat sb; //Give sb and de better names...

    cacheentry *entry;
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
            { strcpy(real_path, filename); }

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
                strncat(filename,"/",PATH_MAX);
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
                {
                //This needs to be converted to something more useful...
                printf("Modified %d\n", sb.st_mtime);
                break; 
                }
            }
        if(!romextensions[i])
            { continue; }

        entry = (cacheentry*)calloc(1,sizeof(cacheentry));
        if( !entry )
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            continue;
            }

        //Found controls whether its found in the cache.
        //We need to add this.
        found = 0;
        if( !found )
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
           }

        //This needs to go further to the GUI side.
        //I.e. rcs should always store the whole path.
        if(config_get_bool( "RomBrowserShowFullPaths", 0 ))
            { line = entry->filename; }
        else
            {
            int fnlen = strlen(entry->filename);
            char *newfn= NULL;
            for(i=fnlen; i > 0; i--)
                {
                if(entry->filename[i] == '/')
                    {
                    newfn = sub_string2(entry->filename, i+1, fnlen);
                    break;
                    }
                }
            line = newfn;
            }

      printf("MD5: %s\t ROM: %s\n", entry->MD5, line);

      inientry = ini_search_by_md5(entry->MD5);
      if(inientry!=NULL)
          { printf("ROM in ini, GoodName: %s\n", inientry->goodname); }

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

        num_entrys = header.entries;
        romcache = malloc(num_entrys*sizeof(cacheentry));
        if (romcache == NULL)
            {
            printf("[error] Could not allocate memory for cache_data. (%i bytes)\n",(sizeof(cacheentry)*num_entrys));
            return 0;
            }
        }

    // Read our data into our array.
    if(fread(romcache,sizeof(cacheentry),num_entrys,f) != (sizeof(cacheentry)*num_entrys))
        {
        printf("[error] malformed rom cache structure.\n");
        return 0;
        }

    // Close our file, it's no longer needed.
    fclose(f);

    //NO NO NO!!!
    //free(romdata);

    return 1;
}

int cache_rom_list()
{
   char filename[PATH_MAX];
   FILE *f = NULL;

   // note: check headers first
   return 1;
}



