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
#include <iconv.h>

#include <errno.h>
#include <pthread.h> //Actually not currently using...

#include <limits.h> //PATH_MAX
#include <dirent.h> //Directory support.
//Includes for POSIX file status.
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "md5.h"
#include "main.h"
#include "util.h"
#include "romcache.h"
#include "config.h"
#include "rom.h"
#include "translate.h"

//This if for updaterombrowser(), which needs the same type of abstraction as info_message().
#include "gui_gtk/rombrowser.h"

//These functions need to be moved.
#include "mupenIniApi.h"

#define DATABASE_FILE "mupen64plus.ini"

void *rom_cache_system(void *_arg);
char cache_filename[PATH_MAX];

static const char *romextensions[] = 
{
 ".v64", ".z64", ".gz", ".zip", ".n64", NULL //".rom" causes to many false positives.
};

static void scan_dir( const char *dirname );
void showhex (const char *a, int len)
{
  int i;
  for (i = 0; i < len; i++)
    {
      printf ("%X", (unsigned char) a[i]);
      if (i < len - 1)
        printf ("/");
    }
  printf ("\n");
}
void clear_cache()
{
    cache_entry *entry, *entrynext;

    if(romcache.length!=0)
        {
        entry = romcache.top;
        do
            { 
            entrynext = entry->next;
            free(entry);
            entry = entrynext;
            --romcache.length;
            }
        while (entry!=NULL);
        romcache.last = NULL;
        }
}

void * rom_cache_system( void *_arg )
{
    int i;
    int caching_done = 0;
    int rebuild_cache = 0;
    int rcs_initialized = 0;
    char *buffer;
    int free_buffer = 0;
    struct sched_param param;

    // Setup job parser
    while (g_RCSTask != RCS_SHUTDOWN)
    {
    //printf("Task: %d\n", g_RCSTask);  
        switch(g_RCSTask)
        {
            case RCS_INIT:
            {
                g_RCSTask = RCS_BUSY;
                rcs_initialized = 1;
                buffer = (char*)config_get_string("RomCacheFile", NULL);
                if(buffer==NULL)
                    {
                    printf("[rcs] Cache not in config.\n");
                    buffer = (char*)malloc(PATH_MAX*sizeof(char));
                    snprintf(buffer, PATH_MAX, "%s%s", get_configpath(), "rombrowser.cache");
                    config_put_string("RomCacheFile", buffer);
                    free_buffer = 1;
                    }

                snprintf(cache_filename, PATH_MAX, "%s", buffer);
                if(free_buffer)
                    { free(buffer); }

                if(!load_initial_cache())
                {
                    printf("[rcs] load_initial_cache() returned 0\n");
                }
                else
                {
                    //Send current cache to rombrowser.
                    updaterombrowser();
                }

                param.sched_priority = 0;
                //pthread_attr_setschedparam (_arg, &param);

                remove(cache_filename);
                rebuild_cache_file();
                // Should be done every n roms, but this should work for now.
                updaterombrowser();

                printf("[rcs] Cache file up to date.\n");
                if (g_RCSTask == RCS_BUSY)
                    g_RCSTask = RCS_SLEEP; 
            }
            break;
            case RCS_RESCAN:
            {
                g_RCSTask = RCS_BUSY;

                if (rcs_initialized)
                {
                    printf("[rcs] Rescanning rom cache!\n");
                    rebuild_cache_file();
                     // Should be done every n roms, but this should work for now.
                    updaterombrowser();
                }

                if (g_RCSTask == RCS_BUSY)
                    g_RCSTask = RCS_SLEEP;
            }
            break;
            case RCS_SLEEP:
            {
                // Sleep to not use any CPU power.
                usleep(1000);
            }
            break;
        }
    }
    printf("[rcs] RCS Terminated!\n");
}

int rebuild_cache_file()
{
    char real_path[PATH_MAX];
    char buffer[32];
    const char *directory; 
    int i;

    printf("[rcs] Rebuilding cache file.\n",romcache.length);

    for ( i = 0; i < config_get_number("NumRomDirs",0); ++i )
        {
        sprintf(buffer,"RomDirectory[%d]",i);
        directory = config_get_string(buffer,"");
        printf("Scanning... %s\n",directory);
        scan_dir(directory);
        }

    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"wb"))==NULL)
        {
        printf("[rcs] Could not create cache file %s\n",cache_filename);
        return 0;
        }

    gzwrite( gzfile, &romcache.length, sizeof(unsigned int) );

    if(romcache.length!=0)
       {
       cache_entry *entry;
       entry = romcache.top;
       do
            {
            gzwrite(gzfile, entry->filename, sizeof(char)*PATH_MAX);
            gzwrite(gzfile, entry->md5, sizeof(char)*33);
            gzwrite(gzfile, &entry->timestamp, sizeof(time_t));
            gzwrite(gzfile, &entry->countrycode, sizeof(unsigned short));
            gzwrite(gzfile, &entry->romsize, sizeof(int));
            gzwrite(gzfile, entry->comment, COMMENT_MAXLENGTH*sizeof(char));
            gzwrite(gzfile, &entry->compressiontype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->imagetype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->internalname, 80*sizeof(char));

            entry = entry->next;
            }
        while (entry!=NULL);
        }

    gzclose(gzfile);

}

static void scan_dir( const char *dirname )
{
    static short failures = 0;
    int i;
    char filename[PATH_MAX];
    char real_path[PATH_MAX];
    char *extension;

    DIR *dir; 
    struct dirent *de; //Give de a better name.
    struct stat filestatus;

    cache_entry *entry;
    int found = 0;

    dir = opendir( dirname );
    if(!dir)
        {
        printf( "[rcs] Could not open directory '%s': %s\n", dirname, strerror( errno ) );
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
        if(stat(real_path,&filestatus)==-1)
            { continue; }

        //Maybe probe errno and provide feedback?  ^
        //To be consistent or removed from above   |

        //Handle recursive scanning.
        if( config_get_bool( "RomDirsScanRecursive", 0 ) )
            {
            if( S_ISDIR(filestatus.st_mode) )
                {
                strncat(filename, "/", PATH_MAX);
                scan_dir(filename);
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

        //This needs to be converted to something more useful...
        //printf("Modified %d\n", filestatus.st_mtime);

        //Found controls whether its found in the cache.
        if(romcache.length!=0)
            {
            entry = romcache.top;
            do
               {
               if(strncmp(entry->filename,filename,PATH_MAX)==0)
                     {
                     found = 1;
                     break;
                     }
               entry = entry->next;
               }
            while (entry!=NULL);
            }
        else
            { found = 0; }

        entry = (cache_entry*)calloc(1,sizeof(cache_entry));
        if(entry==NULL)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            continue;
            }

        if(found==0)
            {
            strcpy(entry->filename,filename);

            unsigned char* localrom;

            //Test if we're a valid rom and compute md5.
            if((localrom=load_rom(filename, &entry->romsize, &entry->compressiontype, &entry->imagetype, &entry->romsize))==NULL)
                { 
                free(entry);
                continue;  
                }

            md5_state_t state;
            md5_byte_t digest[16];

            md5_init(&state);
            md5_append(&state, (const md5_byte_t *)localrom, entry->romsize);
            md5_finish(&state, digest);

            for ( i = 0; i < 16; ++i ) 
                { sprintf(entry->md5+i*2, "%02X", digest[i]); }

            entry->inientry = ini_search_by_md5(entry->md5);

            //See rom.h for header layout.
            entry->countrycode = (unsigned short)*(localrom+0x3E);

            char *buffer1 = calloc(21,sizeof(char));
                strncpy(buffer1, (char*)localrom+0x20, 20);
        


char* euc = buffer1;
  iconv_t euc2utf8t;
  size_t iconv_value;
  char *euc_start, *utf8, *utf8start;
  size_t len, utf8len, utf8lenstart, start_len;

  euc_start = euc;

  euc2utf8t = iconv_open ("UTF-8", "SHIFT-JIS");
  if (euc2utf8t == (iconv_t) -1)
     { printf("Error\n"); }

  start_len = len = strlen (euc);
  utf8lenstart = utf8len = 80;
  utf8start =   utf8 = (char*) calloc (utf8len, 1);
  //printf ("before: in string '%s', length %d, out string '%s', length %d\n",
          //euc, len, utf8, utf8len);
  //showhex (euc, start_len);
  //showhex (utf8start, utf8lenstart);
  iconv_value = iconv (euc2utf8t, & euc, & len, & utf8, & utf8len);
  //printf ("after: in string '%s', length %d, out string '%s', length %d\n",
    //      euc, len, utf8start, utf8lenstart);
  //showhex (euc_start, start_len);
  //showhex (utf8start, utf8lenstart);
  if (iconv_value == (size_t) -1)
    {
      printf ("failed: in string '%s', length %d, "
              "out string '%s', length %d\n",
              euc, len, utf8start, utf8len);
      switch (errno)
        {
        case EILSEQ:
          printf ("Invalid character sequence\n");
          break;
        case EINVAL:
          printf ("EINVAL\n");
          break;
        case E2BIG:
          printf ("E2BIG\n");
          break;
        default:
          printf ("unknown error");
        }
    ++failures;
    printf("Failures: %d\n", failures);
    }
  iconv_close (euc2utf8t);
  
  //char * instring = "金文体"; // Save this file as Japanese EUC in emacs
  //char * instring = "Test"; // Save this file as Japanese EUC in emacs
  printf ("%s\n", utf8start);
  strncpy(entry->internalname , utf8start, 80);

            long long CRC = 0;

            for ( i = 0x40/4; i < 0x1000/4; ++i )
                { CRC += ((unsigned int*)localrom)[i]; }

            switch(CRC)
                {
                case 0x000000A0F26F62FELL:
                    entry->cic = CIC_NUS_6101;
                    break;
                case 0x000000A316ADC55ALL:
                    entry->cic = CIC_NUS_6102;
                    break;
                case 0x000000A9229D7C45LL:
                    entry->cic = CIC_NUS_6103;
                    break;
                case 0x000000F8B860ED00LL:
                    entry->cic = CIC_NUS_6105;
                    break;
                case 0x000000BA5BA4B8CDLL:
                    entry->cic = CIC_NUS_6106;
                    break;
                default:
                    entry->cic = CIC_NUS_6102;
                }

           //Something to deal with custom roms.
            //if(entry.inientry==NULL)
            //    { custom_search_by_md5(entry->md5);

            entry->timestamp = filestatus.st_mtime;
            //Add code to search comment database.

            //Actually add rom to cache.
            if(romcache.length==0)
                {
                romcache.top = entry; 
                romcache.last = entry; 
                ++romcache.length;
                }
            else
                {
                romcache.last->next = entry;
                romcache.last = entry;
                ++romcache.length;
                }
            printf("Added ROM: %s\n", entry->inientry->goodname);

            free(localrom);
            }
        }
    closedir(dir);
}

int load_initial_cache()
{
    int i;
    cache_entry *entry, *entrynext;
    struct stat filestatus;

    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"rb"))==NULL)
        {
        printf("[rcs] Could not open %s\n",cache_filename);
        return 0;
        }

    if(gzread(gzfile, &romcache.length, sizeof(unsigned int)))
        {
        for ( i = 0; i < romcache.length; ++i )
            { 
            entry = (cache_entry*)calloc(1,sizeof(cache_entry));

            if(!entry)
                {
                fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                return 0;
                }

            if(!gzread(gzfile, entry->filename, sizeof(char)*PATH_MAX)||
               !gzread(gzfile, entry->md5, sizeof(char)*33)||
               !gzread(gzfile, &entry->timestamp, sizeof(time_t))||
               !gzread(gzfile, &entry->countrycode, sizeof(unsigned short))||
               !gzread(gzfile, &entry->romsize, sizeof(int))||
               !gzread(gzfile, entry->comment, COMMENT_MAXLENGTH*sizeof(char))||
               !gzread(gzfile, &entry->compressiontype, sizeof(unsigned short))||
               !gzread(gzfile, &entry->imagetype, sizeof(unsigned short))||
               !gzread(gzfile, &entry->internalname, 80*sizeof(char)))
                {
                if(!gzeof(gzfile)) //gzread error.
                    { 
                    clear_cache();
                    printf("[rcs] Cache file corrupt.\n");
                    return 0;
                    }
                }

            //Check rom is valid.
            //If we can't get information, move to next file.
            if(stat(entry->filename,&filestatus)==-1)
                { continue; }

            entry->next = NULL;

            //Maybe add size here too?
            if(entry->md5!=NULL&&entry->timestamp==filestatus.st_mtime)
                {
                //Actually add rom to cache.
                entry->inientry = ini_search_by_md5(entry->md5);
                if(i==0)
                    {
                    romcache.top = entry; 
                    romcache.last = entry; 
                    }
                else
                    {
                    romcache.last->next = entry;
                    romcache.last = entry;
                    }
                }
            }
        }

    gzclose(gzfile);

    printf("[rcs] Cache file processed.\n");

    return 1;
}

