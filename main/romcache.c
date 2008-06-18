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

#define UPDATE_FREQUENCY 12
#define DEFAULT 16
#define RCS_VERSION "RCS1.0"

void romdatabase_open();
void romdatabase_close();

_romdatabase g_romdatabase;
romdatabase_entry empty_entry;
rom_cache g_romcache;

static const char *romextensions[] = 
{
 ".v64", ".z64", ".gz", ".zip", ".n64", NULL //".rom" causes to many false positives.
};

static void scan_dir( const char *dirname );

void clear_cache()
{
    cache_entry *entry, *entrynext;

    if(g_romcache.length!=0)
        {
        entry = g_romcache.top;
        do
            { 
            entrynext = entry->next;
            free(entry);
            entry = entrynext;
            --g_romcache.length;
            }
        while (entry!=NULL);
        g_romcache.last = NULL;
        }
}

void* rom_cache_system(void* _arg)
{
    char* buffer;
    struct sched_param param;
    char cache_filename[PATH_MAX];

    // Setup job parser
    while (g_romcache.rcstask != RCS_SHUTDOWN)
        {
        //printf("Task: %d\n", g_romcache.rcstask);  
        switch(g_romcache.rcstask)
            {
            case RCS_INIT:
                g_romcache.rcstask = RCS_BUSY;
                romdatabase_open();
                buffer = (char*)config_get_string("RomCacheFile", NULL);
                if(buffer==NULL)
                    {
                    printf("[rcs] Cache not in config.\n");
                    buffer = (char*)malloc(PATH_MAX*sizeof(char));
                    snprintf(buffer, PATH_MAX, "%s%s", get_configpath(), "rombrowser.cache");
                    config_put_string("RomCacheFile", buffer);
                    }

                snprintf(cache_filename, PATH_MAX, "%s", buffer);

                if(load_initial_cache(cache_filename))
                    { updaterombrowser(g_romcache.length, 1); }

                param.sched_priority = 0;
                pthread_attr_setschedparam (_arg, &param);

                remove(cache_filename);
                rebuild_cache_file(cache_filename);
                updaterombrowser(g_romcache.length, 0);

                printf("[rcs] Cache file up to date.\n");
                if(g_romcache.rcstask==RCS_BUSY)
                   { g_romcache.rcstask = RCS_SLEEP; }
                break;
            case RCS_WRITE_CACHE:
                g_romcache.rcstask = RCS_BUSY;
                remove(cache_filename);
                write_cache_file(cache_filename);
                if (g_romcache.rcstask == RCS_BUSY)
                    { g_romcache.rcstask = RCS_SLEEP; }
                break;
            case RCS_RESCAN:
                g_romcache.rcstask = RCS_BUSY;

                printf("[rcs] Rescanning rom cache!\n");
                rebuild_cache_file(cache_filename);
                updaterombrowser(g_romcache.length, 0);
                printf("[rcs] Cache file up to date.\n");

                if(g_romcache.rcstask==RCS_BUSY)
                    { g_romcache.rcstask = RCS_SLEEP; }
                break;
            case RCS_SLEEP:
                // Sleep to not use any CPU power.
                usleep(1000);
                break;
            }
        }

    romdatabase_close();
    printf("[rcs] RCS Terminated!\n");
}

int write_cache_file(char* cache_filename)
{
    gzFile *gzfile;

    if((gzfile = gzopen(cache_filename,"wb"))==NULL)
        {
        printf("[rcs] Could not create cache file %s\n",cache_filename);
        return 0;
        }

    gzwrite(gzfile, RCS_VERSION, 6*sizeof(char));
    gzwrite(gzfile, &g_romcache.length, sizeof(unsigned int));

    if(g_romcache.length!=0)
       {
       cache_entry *entry;
       entry = g_romcache.top;
       do
            {
            gzwrite(gzfile, entry->md5, 16*sizeof(md5_byte_t));
            gzwrite(gzfile, &entry->timestamp, sizeof(time_t));
            gzwrite(gzfile, entry->filename, PATH_MAX*sizeof(char));
            gzwrite(gzfile, entry->usercomments, COMMENT_MAXLENGTH*sizeof(char));
            gzwrite(gzfile, &entry->internalname, 80*sizeof(char));
            gzwrite(gzfile, &entry->countrycode, sizeof(unsigned short));
            gzwrite(gzfile, &entry->compressiontype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->imagetype, sizeof(unsigned short));
            gzwrite(gzfile, &entry->cic, sizeof(unsigned short));
            gzwrite(gzfile, &entry->archivefile, sizeof(unsigned int));
            gzwrite(gzfile, &entry->crc1, sizeof(unsigned int));
            gzwrite(gzfile, &entry->crc2, sizeof(unsigned int));
            gzwrite(gzfile, &entry->romsize, sizeof(int));
            entry = entry->next;
            }
        while (entry!=NULL);
        }

    gzclose(gzfile);
    return 1;
}

int rebuild_cache_file(char* cache_filename)
{
    char real_path[PATH_MAX];
    char buffer[32];
    const char *directory; 
    int i;

    printf("[rcs] Rebuilding cache file.\n",g_romcache.length);

    for ( i = 0; i < config_get_number("NumRomDirs",0); ++i )
        {
        sprintf(buffer,"RomDirectory[%d]",i);
        directory = config_get_string(buffer,"");
        printf("Scanning... %s\n",directory);
        scan_dir(directory);
        }

    write_cache_file(cache_filename);
}

static void scan_dir(const char *directoryname)
{
    char filename[PATH_MAX];
    char fullpath[PATH_MAX];
    char *extension;

    DIR *directory; 
    struct dirent *directoryentry;
    struct stat filestatus;

    cache_entry *entry;
    int found;

    directory = opendir(directoryname);
    if(!directory)
        {
        printf( "[rcs] Could not open directory '%s': %s\n", directoryname, strerror(errno) );
        return;
        }

    int counter;
    int romcounter=0;

    while((directoryentry=readdir(directory)))
        {
        if( directoryentry->d_name[0] == '.' ) // .., . or hidden file
            { continue; }
        snprintf(filename, PATH_MAX-1, "%s%s", directoryname, directoryentry->d_name);
        filename[PATH_MAX-1] = '\0';

        //Use real path (maybe it's a link)
        if(realpath(filename,fullpath))
            {
            strncpy(filename,fullpath,PATH_MAX-1); 
            filename[PATH_MAX-1] = '\0';
            }

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
        for( counter = 0; romextensions[counter]; ++counter )
            {
            if(!strcasecmp(extension,romextensions[counter]))
                { break; }
            }
        if(!romextensions[counter])
            { continue; }

        found = 0;
        //Found controls whether its found in the cache.
        if(g_romcache.length!=0)
            {
            entry = g_romcache.top;
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

        entry = (cache_entry*)calloc(1,sizeof(cache_entry));
        if(entry==NULL)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            continue;
            }

        if(found==0)
            {
            strncpy(entry->filename,filename,PATH_MAX-1);
            entry->filename[PATH_MAX-1] = '\0';

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
                {
                strncpy(entry->internalname,(char*)localrom+0x20,20); 
                entry->internalname[20]='\0';
                }
            else
                {
                char *shiftjis, *shiftjisstart, *utf8, *utf8start;
                size_t shiftjislength = 20;
                size_t utf8length = 80; 
                shiftjisstart = shiftjis = (char*)calloc(20,sizeof(char));
                utf8start = utf8 = (char*)calloc(81,sizeof(char));

                strncpy(shiftjis, (char*)localrom+0x20, 20);

                iconv(conversion, &shiftjis, &shiftjislength, &utf8, &utf8length);
                iconv_close(conversion);

                strncpy(entry->internalname , utf8start, 80);
                entry->internalname[80]='\0';

                free(shiftjisstart);
                free(utf8start);
                }

            //Detect CIC copy protection boot chip by CRCing the boot code.
            long long CRC = 0;

            for ( counter = 0x40/4; counter < 0x1000/4; ++counter )
                { CRC += ((unsigned int*)localrom)[counter]; }

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
            if(g_romcache.length==0)
                {
                g_romcache.top = entry; 
                g_romcache.last = entry; 
                ++g_romcache.length;
                }
            else
                {
                g_romcache.last->next = entry;
                g_romcache.last = entry;
                ++g_romcache.length;
                }
            printf("Added ROM: %s\n", entry->inientry->goodname);
            ++romcounter;
            if(romcounter%UPDATE_FREQUENCY==0)
                { updaterombrowser(g_romcache.length, 0); }

            free(localrom);
            }
        }
    closedir(directory);
}

int load_initial_cache(char* cache_filename)
{
    int counter;
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

    gzread(gzfile, &g_romcache.length, sizeof(unsigned int));
    for ( counter = 0; counter < g_romcache.length; ++counter )
        { 
        entry = (cache_entry*)calloc(1,sizeof(cache_entry));

        if(!entry)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
            return 0;
            }

        gzread(gzfile, entry->md5, 16*sizeof(md5_byte_t));
        gzread(gzfile, &entry->timestamp, sizeof(time_t));
        gzread(gzfile, entry->filename, PATH_MAX*sizeof(char));
        gzread(gzfile, entry->usercomments, COMMENT_MAXLENGTH*sizeof(char));
        gzread(gzfile, entry->internalname, 80*sizeof(char));
        gzread(gzfile, &entry->countrycode, sizeof(unsigned short));
        gzread(gzfile, &entry->compressiontype, sizeof(unsigned short));
        gzread(gzfile, &entry->imagetype, sizeof(unsigned short));
        gzread(gzfile, &entry->cic, sizeof(unsigned short));
        gzread(gzfile, &entry->archivefile, sizeof(unsigned int));
        gzread(gzfile, &entry->crc1, sizeof(unsigned int));
        gzread(gzfile, &entry->crc2, sizeof(unsigned int));
        gzread(gzfile, &entry->romsize, sizeof(int));

        //Check rom is valid.
        if(stat(entry->filename,&filestatus)==-1)
            {
            free(entry); //If we can't get into make sure to
            --counter;   //Update counters before continuing.
            --g_romcache.length;
            continue; 
            }

        entry->next = NULL;

        if(entry->timestamp==filestatus.st_mtime)
            {
            //Actually add rom to cache.
            entry->inientry = ini_search_by_md5(entry->md5);
            if(entry->inientry==&empty_entry)
                { printf("Empty\n"); }

            if(counter==0)
                {
                g_romcache.top = entry; 
                g_romcache.last = entry; 
                }
            else
                {
                g_romcache.last->next = entry;
                g_romcache.last = entry;
                }
            //printf("Added from cache: %d %s\n", counter, entry->inientry->goodname);
            }
        }

    gzclose(gzfile);

    printf("[rcs] Cache file processed.\n");

    return 1;
}

static int split_property(char *string)
{
    int counter = 0;
    while(string[counter] != '=' && string[counter] != '\0')
        { ++counter; }
    if(string[counter]=='\0')
        { return -1; }
    string[counter] = '\0';
    return counter;
}

void romdatabase_open()
{
    gzFile gzfile;
    char buffer[256];
    romdatabase_search *search = NULL;
    romdatabase_entry *entry = NULL;

    int stringlength, totallength, namelength, index, counter, value;
    char hashtemp[3];
    hashtemp[2] = '\0';

    if(g_romdatabase.comment!=NULL)
        { return; }

    //Query config system and open romdatabase.
    char *pathname = (char*)config_get_string("RomDatabaseFile", NULL);
    if(pathname==NULL)
        {
        printf("[rcs] Database not in config.\n");
        pathname = (char*)malloc(PATH_MAX*sizeof(char));
        snprintf(pathname, PATH_MAX, "%s%s", get_configpath(), "mupen64plus.ini");
        config_put_string("RomDatabaseFile", pathname);
        }

    //printf("Database file: %s \n", pathname);
    gzfile = gzopen(pathname, "rb");

    if(gzfile==NULL)
        {
        printf("[rcs] Unable to open rom database.\n");
        return;
        }

    //Move through opening comments, set romdatabase.comment to non-NULL 
    //to signal we have a database.
    totallength = 0;
    do
        {
        gzgets(gzfile, buffer, 255);
        if(buffer[0]!='[')
            {
            stringlength=strlen(buffer);
            totallength+=stringlength;
            if(g_romdatabase.comment==NULL) 
                {
                g_romdatabase.comment = (char*)malloc(stringlength+2);
                strncpy(g_romdatabase.comment, buffer, stringlength);
                buffer[stringlength+1] = '\0';
                }
            else
                {
                g_romdatabase.comment = (char*)realloc(g_romdatabase.comment, totallength+2);
                snprintf(g_romdatabase.comment, totallength+1, "%s%s", g_romdatabase.comment, buffer);
                buffer[totallength+1] = '\0';
                }
            }
        }
    while (buffer[0] != '[' && !gzeof(gzfile));

    //Clear premade indices.
    for ( counter = 0; counter < 255; ++counter )
        { g_romdatabase.crc_lists[counter] = NULL; }
    for ( counter = 0; counter < 255; ++counter )
        { g_romdatabase.md5_lists[counter] = NULL; }
    g_romdatabase.list = NULL;

    do
        {
        if(buffer[0]=='[')
            {
            if(g_romdatabase.list==NULL)
                {
                g_romdatabase.list = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                g_romdatabase.list->next_entry = NULL;
                g_romdatabase.list->next_crc = NULL;
                g_romdatabase.list->next_md5 = NULL;
                search = g_romdatabase.list;
                }
            else
                {
                search->next_entry = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                search = search->next_entry;
                search->next_entry = NULL;
                search->next_crc = NULL;
                search->next_md5 = NULL;
                }
            for (counter=0; counter < 16; ++counter)
              {
              hashtemp[0] = buffer[counter*2+1];
              hashtemp[1] = buffer[counter*2+2];
              sscanf(hashtemp, "%X", search->entry.md5+counter); 
              }
            //Index MD5s by first 8 bits.
            index = search->entry.md5[0];
            if(g_romdatabase.md5_lists[index]==NULL)
                { g_romdatabase.md5_lists[index] = search; }
            else
                {
                romdatabase_search *aux = g_romdatabase.md5_lists[index];
                search->next_md5 = aux;
                g_romdatabase.md5_lists[index] = search;
                }
            search->entry.status=0; //Set default to 0 stars.
            search->entry.savetype=DEFAULT; //Set default to NULL 
            search->entry.rumble=DEFAULT; 
            search->entry.players=DEFAULT; 
            }
        else
            {
            stringlength = split_property(buffer);
            if(stringlength!=-1)
                {
                if(!strcmp(buffer, "GoodName"))
                    {
                    //Get length of GoodName since we dynamically allocate.
                    namelength = strlen(buffer+stringlength+1);
                    search->entry.goodname = (char*)malloc(namelength*sizeof(char));
                    //Make sure we're null terminated.
                    if(buffer[stringlength+namelength]=='\n'||buffer[stringlength+namelength]=='\r')
                        { buffer[stringlength+namelength] = '\0'; }
                    strncpy(search->entry.goodname, buffer+stringlength+1, namelength);
                    search->entry.goodname[namelength-1] = '\0';
                    //printf("Name: %s, Length: %d\n", search->entry.goodname, namelength-1);
                    }
                else if(!strcmp(buffer, "CRC"))
                    {
                    buffer[stringlength+19] = '\0';
                    sscanf(buffer+stringlength+10, "%X", &search->entry.crc2);
                    buffer[stringlength+9] = '\0';
                    sscanf(buffer+stringlength+1, "%X", &search->entry.crc1);
                    buffer[stringlength+3] = '\0';
                    sscanf(buffer+stringlength+1, "%X", &index);
                    //Index CRCs by first 8 bits.
                    if(g_romdatabase.crc_lists[index]==NULL)
                        { g_romdatabase.crc_lists[index] = search; }
                    else
                        {
                        romdatabase_search *aux = g_romdatabase.crc_lists[index];
                        search->next_crc = aux;
                        g_romdatabase.crc_lists[index] = search;
                        }
                    }
                else if(!strcmp(buffer, "RefMD5"))
                    {
                    //If we have a refernce MD5, dynamically allocate.
                    search->entry.refmd5 = (md5_byte_t*)malloc(16*sizeof(md5_byte_t));
                    for (counter=0; counter < 16; ++counter)
                        {
                        hashtemp[0] = buffer[stringlength+1+counter*2];
                        hashtemp[1] = buffer[stringlength+2+counter*2];
                        sscanf(hashtemp, "%X", search->entry.refmd5+counter); 
                        }
                    //Lookup reference MD5 and replace non-default entries.
                    if((entry = ini_search_by_md5(search->entry.refmd5))!=&empty_entry)
                        {
                        //printf("RefMD5: %s\n", aux->goodname);
                        if(entry->savetype!=DEFAULT)
                            { search->entry.savetype = entry->savetype; }
                        if(entry->status!=0)
                            { search->entry.status = entry->status; }
                        if(entry->players!=DEFAULT)
                            { search->entry.players = entry->players; }
                        if(entry->rumble!=DEFAULT)
                            { search->entry.rumble = entry->rumble; }
                        }
                    }
                else if(!strcmp(buffer, "SaveType"))
                    {
                    if(!strncmp(buffer+stringlength+1, "Eeprom 4KB", 10))
                        { search->entry.savetype = EEPROM_4KB; }
                    else if(!strncmp(buffer+stringlength+1, "Eeprom 16KB", 10))
                        { search->entry.savetype = EEPROM_16KB; }
                    else if(!strncmp(buffer+stringlength+1, "SRAM", 4))
                        { search->entry.savetype = SRAM; }
                    else if(!strncmp(buffer+stringlength+1, "Flash RAM", 9))
                        { search->entry.savetype = FLASH_RAM; }
                    else if(!strncmp(buffer+stringlength+1, "Controller Pack", 15))
                        { search->entry.savetype = CONTROLLER_PACK; }
                    else if(!strncmp(buffer+stringlength+1, "None", 4))
                        { search->entry.savetype = NONE; }
                    }
                else if(!strcmp(buffer, "Status"))
                    {
                    value = atoi(buffer+stringlength+1);
                    if(value>-1&&value<6)
                        { search->entry.status = value; }
                    }
                else if(!strcmp(buffer, "Players"))
                    {
                    value = atoi(buffer+stringlength+1);
                    if(value>0&&value<8)
                        { search->entry.players = value; }
                    }
                else if(!strcmp(buffer, "Rumble"))
                    {
                    if(!strncmp(buffer+stringlength+1, "Yes", 3))
                        { search->entry.rumble = 1; }
                    if(!strncmp(buffer+stringlength+1, "No", 2))
                        { search->entry.rumble = 0; }
                    }
                }
            }

        gzgets(gzfile, buffer, 255);
        }
   while (!gzeof(gzfile));

   gzclose(gzfile);
}

void romdatabase_close()
{
    if (g_romdatabase.comment == NULL)
        { return; }

    free(g_romdatabase.comment);

    while(g_romdatabase.list != NULL)
        {
        romdatabase_search *search = g_romdatabase.list->next_entry;
        free(g_romdatabase.list);
        g_romdatabase.list = search;
        }
}

romdatabase_entry* ini_search_by_md5(md5_byte_t* md5)
{
   // if(g_romdatabase.comment==NULL)
     //   { return &empty_entry; }
    romdatabase_search *search;
    search = g_romdatabase.md5_lists[md5[0]];

    while (search!=NULL&&memcmp(search->entry.md5, md5, 16)!=0)
        { search = search->next_md5; }

    if(search==NULL)
        { return &empty_entry; }
    else
        { return &(search->entry); }
}

romdatabase_entry* ini_search_by_crc(unsigned int crc1, unsigned int crc2)
{
    if(g_romdatabase.comment==NULL) 
        { return &empty_entry; }
    romdatabase_search *search;
    search = g_romdatabase.crc_lists[(unsigned short)crc1];

    while (search!=NULL&&search->entry.crc1!=crc1&search->entry.crc2!=crc2)
        { search = search->next_crc; }

    if(search == NULL) 
        { return &empty_entry; }
    else
        { return &(search->entry); }
}


