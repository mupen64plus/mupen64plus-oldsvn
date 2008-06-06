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

#include <zlib.h>
#include <errno.h>
#include <pthread.h>

#include <limits.h> //PATH_MAX
#include <dirent.h> //Directory support.
//Includes for POSIX file status.
#include <sys/stat.h>
#include <sys/types.h>

#include "../memory/memory.h" //sl

#include "md5.h"
#include "main.h"
#include "util.h"
#include "romcache.h"
#include "config.h"
#include "rom.h"
#include "translate.h"

//This if for updaterombrowser(), which needs the same type of abstraction as info_message().
#include "gui_gtk/rombrowser.h"

#define RCS_VERSION "RCS1.0"

void *rom_cache_system(void *_arg);
char cache_filename[PATH_MAX];

static const char *romextensions[] = 
{
 ".v64", ".z64", ".gz", ".zip", ".n64", NULL //".rom" causes to many false positives.
};

static void scan_dir( const char *dirname );

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
                ini_openFile();
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

                if(load_initial_cache())
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
                    printf("[rcs] Cache file up to date.\n");
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
    romdatabase_close();
    printf("[rcs] RCS Terminated!\n");
}

int write_cache_file()
{
    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"wb"))==NULL)
        {
        printf("[rcs] Could not create cache file %s\n",cache_filename);
        return 0;
        }

    gzwrite(gzfile, RCS_VERSION, 6*sizeof(char));
    gzwrite(gzfile, &romcache.length, sizeof(unsigned int));

    if(romcache.length!=0)
       {
       cache_entry *entry;
       entry = romcache.top;
       do
            {
            gzwrite(gzfile, entry->filename, PATH_MAX*sizeof(char));
            gzwrite(gzfile, entry->md5, 16*sizeof(md5_byte_t));
            gzwrite(gzfile, &entry->timestamp, sizeof(time_t));
            gzwrite(gzfile, &entry->countrycode, sizeof(unsigned short));
            gzwrite(gzfile, &entry->romsize, sizeof(int));
            gzwrite(gzfile, entry->comment, COMMENT_MAXLENGTH*sizeof(char));
            gzwrite(gzfile, &entry->compressiontype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->imagetype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->internalname, 80*sizeof(char));
            gzwrite(gzfile, &entry->crc1, sizeof(unsigned int));
            gzwrite(gzfile, &entry->crc2, sizeof(unsigned int));
            gzwrite(gzfile, &entry->archivefile, sizeof(unsigned int));

            entry = entry->next;
            }
        while (entry!=NULL);
        }

    gzclose(gzfile);
    return 1;
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

    write_cache_file();
}

static void scan_dir( const char *directoryname )
{
    static short failures = 0;
    int i;
    char filename[PATH_MAX];
    char fullpath[PATH_MAX];
    char *extension;

    DIR *directory; 
    struct dirent *directoryentry;
    struct stat filestatus;

    cache_entry *entry;
    int found = 0;

    directory = opendir(directoryname);
    if(!directory)
        {
        printf( "[rcs] Could not open directory '%s': %s\n", directoryname, strerror(errno) );
        return;
        }

    while((directoryentry=readdir(directory)))
        {
        if( directoryentry->d_name[0] == '.' ) // .., . or hidden file
            { continue; }
        snprintf(filename, PATH_MAX, "%s%s", directoryname, directoryentry->d_name);

        //Use real path (maybe it's a link)
        if(realpath(filename,fullpath))
            { strncpy(fullpath,filename,PATH_MAX); }

        //If we can't get information, move to next file.
        if(stat(fullpath,&filestatus)==-1)
            {
            printf( "[rcs] Could not open file '%s': %s\n", fullpath, strerror(errno) );
            continue; 
            }

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

            //When we support multifile archives load_rom needs a archivefile field.
            entry->archivefile = 0; //For now we're the 1st valid ROM only.

            //Test if we're a valid rom.
            if((localrom=load_rom(filename, &entry->romsize, &entry->compressiontype, &entry->imagetype, &entry->romsize))==NULL)
                { 
                free(entry);
                continue;  
                }

            //Compute md5.
            md5_state_t state;

            md5_init(&state);
            md5_append(&state, (const md5_byte_t *)localrom, entry->romsize);
            md5_finish(&state, entry->md5);

            entry->inientry = ini_search_by_md5(entry->md5);

            //See rom.h for header layout.
            entry->countrycode = (unsigned short)*(localrom+0x3E);
            entry->crc1 = sl(*(unsigned int*)(localrom+0x10));
            entry->crc2 = sl(*(unsigned int*)(localrom+0x14));

            //Internal name is encoded in SHIFT-JIS. Attempt to convert to UTF-8 so that
            //GUIs and Rice can use this for Japanese titles in a moderm *NIX environment.
            iconv_t conversion = iconv_open ("UTF-8", "SHIFT-JIS");
            if(conversion==(iconv_t)-1)
                { strncpy(entry->internalname,(char*)localrom+0x20,20); }
            else
                {
                char *shiftjis, *shiftjisstart, *utf8, *utf8start;
                size_t shiftjislength = 20;
                size_t utf8length = 80; 
                shiftjisstart = shiftjis = (char*)calloc(20,sizeof(char));
                utf8start = utf8 = (char*)calloc(80,sizeof(char));

                strncpy(shiftjis, (char*)localrom+0x20, 20);

                iconv(conversion, &shiftjis, &shiftjislength, &utf8, &utf8length);
                iconv_close(conversion);

                strncpy(entry->internalname , utf8start, 80);

                free(shiftjisstart);
                free(utf8start);
                }

            //Detect CIC copy protection boot chip by CRCing the boot code.
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

            entry->timestamp = filestatus.st_mtime;

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
    closedir(directory);
}

int load_initial_cache()
{
    int i, j;
    char header[6];
    cache_entry *entry, *entrynext;
    struct stat filestatus;

    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"rb"))==NULL)
        {
        printf("[rcs] Could not open %s\n",cache_filename);
        return 0;
        }

    if(!gzread(gzfile, &header, 6*sizeof(char))||(strstr(header, RCS_VERSION)==NULL))
        {
        printf("[rcs] Cache corrupt or from previous version.\n");
        return 0;
        }

    gzread(gzfile, &romcache.length, sizeof(unsigned int));
    for ( i = 0; i < romcache.length; ++i )
        { 
        entry = (cache_entry*)calloc(1,sizeof(cache_entry));

        if(!entry)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            return 0;
            }

        gzread(gzfile, entry->filename, PATH_MAX*sizeof(char));
        gzread(gzfile, entry->md5, 16*sizeof(md5_byte_t));
        gzread(gzfile, &entry->timestamp, sizeof(time_t));
        gzread(gzfile, &entry->countrycode, sizeof(unsigned short));
        gzread(gzfile, &entry->romsize, sizeof(int));
        gzread(gzfile, entry->comment, COMMENT_MAXLENGTH*sizeof(char));
        gzread(gzfile, &entry->compressiontype, sizeof(unsigned short));
        gzread(gzfile, &entry->imagetype, sizeof(unsigned short));
        gzread(gzfile, entry->internalname, 80*sizeof(char));
        gzread(gzfile, &entry->crc1, sizeof(unsigned int));
        gzread(gzfile, &entry->crc2, sizeof(unsigned int));
        gzread(gzfile, &entry->archivefile, sizeof(unsigned int));

        //Check rom is valid.
        //If we can't get information, move to next file.
        if(stat(entry->filename,&filestatus)==-1)
            { continue; }

        entry->next = NULL;

        if(entry->timestamp==filestatus.st_mtime)
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
            //printf("Added from cache: %s\n", entry->inientry->goodname);
            }
        }

    gzclose(gzfile);

    printf("[rcs] Cache file processed.\n");

    return 1;
}


typedef struct _romdatabase_search
{
    mupenEntry entry;
    struct _romdatabase_search* next_entry;
    struct _romdatabase_search* next_crc;
    struct _romdatabase_search* next_md5;
} romdatabase_search;

typedef struct
{
    char *comment;
    romdatabase_search* crc_lists[256];
    romdatabase_search* md5_lists[256];
    romdatabase_search* list;
} _romdatabase;

static _romdatabase romdatabase;

typedef mupenEntry romdatabase_entry;

romdatabase_entry emptyEntry;

static int split_property(char *s)
{
    int i = 0;
    while(s[i] != '=' && s[i] != 0) i++;
    if (s[i] == 0) return -1;
    s[i] = 0;
    return i;
}

void ini_openFile()
{
    static short first = 0;
    gzFile gzfile;
    char buf[256];
    int j,i=0;
    romdatabase_search *cur = NULL;
    int free_buffer = 0;

    int counter;
    char temp[3];
    temp[2] = '\0';


    if(romdatabase.comment!=NULL)
        { return; }

    //Query config system and open romdatabase.
    char *pathname = (char*)config_get_string("RomDatabaseFile", NULL);
    if(pathname==NULL)
        {
        printf("[rcs] Database not in config.\n");
        pathname = (char*)malloc(PATH_MAX*sizeof(char));
        snprintf(pathname, PATH_MAX, "%s%s", get_configpath(), "mupen64plus.ini");
        config_put_string("RomDatabaseFile", pathname);
        free_buffer = 1;
        }

    //printf("Database file: %s \n", pathname);
    gzfile = gzopen(pathname, "rb");
    if(free_buffer)
        { free(pathname); }

    if(gzfile==NULL)
        {
        printf("[rcs] Unable to open rom database.\n");
        return;
        }

    //Move through opening comments, set romdatabase.comment to non-NULL 
    //to signal we have a database.
    do
        {
        gzgets(gzfile, buf, 255);
        if(buf[0]!='[')
            {
            i+= strlen(buf);
            if(romdatabase.comment==NULL) 
                {
                romdatabase.comment = (char*)malloc(i+1);
                strcpy(romdatabase.comment, buf);
                }
            else
                {
                romdatabase.comment = (char*)realloc(romdatabase.comment, i+1);
                strcat(romdatabase.comment, buf);
                }
            }
        }
    while (buf[0] != '[' && !gzeof(gzfile));

    //Clear premade indices.
    for ( i = 0; i < 255; ++i )
        { romdatabase.crc_lists[i] = NULL; }
    for ( i = 0; i < 255; ++i )
        { romdatabase.md5_lists[i] = NULL; }
    romdatabase.list = NULL;

    do
        {
        if(buf[0]=='[')
            {
            if(romdatabase.list==NULL)
                {
                romdatabase.list = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                romdatabase.list->next_entry = NULL;
                romdatabase.list->next_crc = NULL;
                romdatabase.list->next_md5 = NULL;
                cur = romdatabase.list;
                }
            else
                {
                cur->next_entry = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                cur = cur->next_entry;
                cur->next_entry = NULL;
                cur->next_crc = NULL;
                cur->next_md5 = NULL;
                }
            i = strlen(buf);
            while(buf[i] != ']') i--;
            buf[i] = 0;

            for (counter=0; counter < 16; ++counter)
              {
              temp[0] = buf[counter*2+1];
              temp[1] = buf[counter*2+2];
              sscanf(temp, "%X", cur->entry.md5+counter); 
              }

            //strncpy(cur->entry.md5, buf+1, 32);

            i = cur->entry.md5[0];

            if(romdatabase.md5_lists[i]==NULL)
                { romdatabase.md5_lists[i] = cur; }
            else
                {
                romdatabase_search *aux = romdatabase.md5_lists[i];
                cur->next_md5 = aux;
                romdatabase.md5_lists[i] = cur;
                }
            cur->entry.status=3;
            cur->entry.savetype = 0;
            for (counter=0; counter < 16; ++counter)
                { cur->entry.refmd5[counter] = 0; }
            }
        else
            {
            i = split_property(buf);
            if(i!=-1)
                {
                if(!strcmp(buf, "Good Name"))
                    {
                    if(buf[i+1+strlen(buf+i+1)-1]=='\n')
                        { buf[i+1+strlen(buf+i+1)-1] = '\0'; }
                    if (buf[i+1+strlen(buf+i+1)-1]=='\r')
                        { buf[i+1+strlen(buf+i+1)-1] = '\0'; }
                    strncpy(cur->entry.goodname, buf+i+1, 99);
                    }
                else if(!strcmp(buf, "Header Code"))
                    {
                    buf[i+19] = 0;
                    sscanf(buf+i+10, "%X", &cur->entry.crc2);
                    buf[i+9] = 0;
                    sscanf(buf+i+1, "%X", &cur->entry.crc1);

                    buf[i+3] = 0;
                    sscanf(buf+i+1, "%X", &i);

                    if(romdatabase.crc_lists[i]==NULL)
                        { romdatabase.crc_lists[i] = cur; }
                    else
                        {
                        romdatabase_search *aux = romdatabase.crc_lists[i];
                        cur->next_crc = aux;
                        romdatabase.crc_lists[i] = cur;
                        }
                    }
                else if(!strcmp(buf, "Reference"))
                    {
                    for (counter=0; counter < 16; ++counter)
                        {
                        temp[0] = buf[i+counter*2+1];
                        temp[1] = buf[i+counter*2+2];
                        sscanf(temp, "%X", cur->entry.refmd5+counter); 
                        }

                    //strncpy(cur->entry.refmd5, buf+i+1, 32);
                    //cur->entry.refmd5[32] = '\0';
                    }
                else if(!strcmp(buf, "Eeprom"))
                    {
                    if(!strncmp(buf+i+1, "16k", 3))
                        { cur->entry.savetype = 1;
 }
                    }
                }
            }

        gzgets(gzfile, buf, 255);
        }
   while (!gzeof(gzfile));

   gzclose(gzfile);
}

void romdatabase_close()
{
    if (romdatabase.comment == NULL)
        { return; }

    free(romdatabase.comment);

    while(romdatabase.list != NULL)
        {
        romdatabase_search *search = romdatabase.list->next_entry;
        free(romdatabase.list);
        romdatabase.list = search;
        }
}

mupenEntry* ini_search_by_md5(md5_byte_t* md5)
{
    if(romdatabase.comment==NULL)
        { return &emptyEntry; }
    romdatabase_search *search;
    search = romdatabase.md5_lists[md5[0]];

    while (search!=NULL&&memcmp(search->entry.md5, md5, 16)!=0)
        { search = search->next_md5; }

    if(search==NULL)
        { return &emptyEntry; }
    else
        { return &(search->entry); }
}

mupenEntry* ini_search_by_crc(unsigned int crc1, unsigned int crc2)
{
    if(romdatabase.comment==NULL) 
        { return &emptyEntry; }
    romdatabase_search *search;
    search = romdatabase.crc_lists[(unsigned short)crc1];

    while (search!=NULL&&search->entry.crc1!=crc1&search->entry.crc2!=crc2)
        { search = search->next_crc; }

    if(search == NULL) 
        { return &emptyEntry; }
    else
        { return &(search->entry); }
}


