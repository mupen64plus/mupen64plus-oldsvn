/**
 * Mupen64 - mupenIniApi.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
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

#include "mupenIniApi.h"
#include "main.h"

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>

typedef struct _romdatabase_search
{
    mupenEntry entry;
    struct _romdatabase_search* next_entry;
    struct _romdatabase_search* next_crc;
    struct _romdatabase_search* next_MD5;
} romdatabase_search;

typedef struct
{
    char *comment;
    romdatabase_search* CRC_lists[256];
    romdatabase_search* MD5_lists[256];
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
    gzFile gzfile;
    char buf[256];
    int i=0;
    romdatabase_search *cur = NULL;
    int free_buffer = 0;

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
        { romdatabase.CRC_lists[i] = NULL; }
    for ( i = 0; i < 255; ++i )
        { romdatabase.MD5_lists[i] = NULL; }
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
                romdatabase.list->next_MD5 = NULL;
                cur = romdatabase.list;
                }
            else
                {
                cur->next_entry = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                cur = cur->next_entry;
                cur->next_entry = NULL;
                cur->next_crc = NULL;
                cur->next_MD5 = NULL;
                }
            i = strlen(buf);
            while(buf[i] != ']') i--;
            buf[i] = 0;
            strncpy(cur->entry.MD5, buf+1, 32);
            cur->entry.MD5[32] = '\0';
            buf[3] = 0;
            sscanf(buf+1, "%X", &i);

            if(romdatabase.MD5_lists[i]==NULL)
                { romdatabase.MD5_lists[i] = cur; }
            else
                {
                romdatabase_search *aux = romdatabase.MD5_lists[i];
                cur->next_MD5 = aux;
                romdatabase.MD5_lists[i] = cur;
                }
            cur->entry.eeprom16kb = 0;
            strcpy(cur->entry.refMD5, "");
            strcpy(cur->entry.comments, "");
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
                    strncpy(cur->entry.CRC, buf+i+1, 21);
                    cur->entry.CRC[21] = '\0';
                    buf[i+3] = 0;
                    sscanf(buf+i+1, "%X", &i);

                    if(romdatabase.CRC_lists[i]==NULL)
                        { romdatabase.CRC_lists[i] = cur; }
                    else
                        {
                        romdatabase_search *aux = romdatabase.CRC_lists[i];
                        cur->next_crc = aux;
                        romdatabase.CRC_lists[i] = cur;
                        }
                    }
                else if(!strcmp(buf, "Reference"))
                    {
                    strncpy(cur->entry.refMD5, buf+i+1, 32);
                    cur->entry.refMD5[32] = '\0';
                    }
                else if(!strcmp(buf, "Eeprom"))
                    {
                    if(!strncmp(buf+i+1, "16k", 3))
                        { cur->entry.eeprom16kb = 1; }
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

mupenEntry* ini_search_by_md5(const char *md5)
{
    char buffer[3];
    int i;
    romdatabase_search *search;

    //If no database, return empty.
    if(romdatabase.comment==NULL)
        { return &emptyEntry; }

    //Convert first 2 hex digits to search premade index.
    buffer[0] = md5[0];
    buffer[1] = md5[1];
    buffer[2] = '\0';
    sscanf(buffer, "%X", &i);
    search = romdatabase.MD5_lists[i];

    while (search != NULL && strncmp(search->entry.MD5, md5, 32))
        { search = search->next_MD5; }

    //If found return pointer, if not return empty.
    if(search==NULL)
        { return &emptyEntry; }
    else 
        { return &(search->entry); }
}

mupenEntry* ini_search_by_CRC(const char *crc)
{
    char buffer[3];
    int i;
    romdatabase_search *search;

    //If no database, return empty.
    if(romdatabase.comment==NULL) 
        { return &emptyEntry; }

    //Convert first 2 hex digits to search premade index.
    buffer[0] = crc[0];
    buffer[1] = crc[1];
    buffer[2] = '\0';
    sscanf(buffer, "%X", &i);
    search = romdatabase.CRC_lists[i];

    while (search != NULL && strncmp(search->entry.CRC, crc, 21))
        { search = search->next_crc; }

    if(search == NULL) 
        { return &emptyEntry; }

    //Return CRC of reference rom if different.
    if(strcmp(search->entry.refMD5, ""))
        {
        mupenEntry* temp = ini_search_by_md5(search->entry.refMD5);
        if(strncmp(search->entry.CRC, temp->CRC, 21))
            { return &(search->entry); }
        else
            { return temp; }
        }
   else
        { return &(search->entry); }
}
