/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rsp plugin - config.c                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Dylan Wagstaff (Pyromanik)                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "config.h"
#include "extension.h"

#define CfgFileName "mupen64_rsp_hle.cfg"

char CfgPath[PATH_MAX];

void getCfgPath()
{
    char exename[PATH_MAX];
    char path[PATH_MAX];
    DIR *d;
    struct dirent *current;
    int e;

    CfgPath[0] = '\0';
    exename[0] = '\0';
    e = readlink("/proc/self/exe", exename, PATH_MAX);
    if(e != -1)
    {
        exename[e] = '\0';
        strncpy(exename, basename(exename), PATH_MAX);
        if(exename[0] != '.')
        {
            if((PATH_MAX - (strlen(getenv("HOME")) + strlen(exename) + 4)) > 0)
            {
                sprintf(path, "%s/.%s\0", getenv("HOME"), exename);
                d = opendir(path); /*test dir existance.*/
                if(d != NULL) /*actually assign the confdir*/
                {
                    strncpy(CfgPath, path, PATH_MAX);
                    strncat(CfgPath, "/", (PATH_MAX - strlen(CfgPath)));
                    strncat(CfgPath, CfgFileName, (PATH_MAX - strlen(CfgPath)));
                    closedir(d);
                }
            }
        }
    }
    if(CfgPath[0] != '/')/*fallback to a few default locations...*/
    {
        if(exename[0] != '\0' || exename[0] != '.')
        {
            if((PATH_MAX - (strlen(exename) + 19)) > 0)
            {
                sprintf(path, "/usr/share/%s/config\0", exename);
                d = opendir(path);
                if(d != NULL)
                {
                    strncpy(CfgPath, path, PATH_MAX);
                    closedir(d);
                }
                else
                {
                    if((PATH_MAX - (strlen(exename) + 25)) > 0)
                    {
                        sprintf(path, "/usr/local/share/%s/config\0", exename);
                        d = opendir(path);
                        if(d != NULL)
                        {
                            strncpy(CfgPath, path, PATH_MAX);
                            closedir(d);
                        }
                    }
                }
            }
        }
    }
    if(CfgPath[0] != '/')/*still falling back...*/
    {
        if(LibPath[0] == '/')/*try the same level as the plugins dir*/
        {
            strncpy(path, dirname(LibPath), PATH_MAX);
            d = opendir(path);
            if(d != NULL)
            {
                strncpy(CfgPath, path, PATH_MAX);
                closedir(d);
            }
            else /*try the plugins dir itself.*/
            {
                d = opendir(LibPath);
                if(d != NULL)
                {
                    while((current = readdir(d)) != NULL)
                    {
                        if(strncmp(current->d_name, CfgFileName, 19) == 0)
                        {
                            strncpy(CfgPath, LibPath, PATH_MAX);
                            break;
                        }
                    }
                    closedir(d);
                }
            }
        }
    }
    if(CfgPath[0] != '/')/*try the exe's dir*/
    {
        e = readlink("/proc/self/exe", path, PATH_MAX);
        if(e != -1)
        {
            path[e] = '\0';
            strncpy(path, dirname(path), PATH_MAX);
            if(path[0] != '.')
            {
                d = opendir(path);
                if(d != NULL)
                {
                    while((current = readdir(d)) != NULL)
                    {
                        if(strncmp(current->d_name, CfgFileName, 19) == 0)
                        {
                            strncpy(CfgPath, path, PATH_MAX);
                            break;
                        }
                    }
                    closedir(d);
                }
            }
        }
    }
    if(CfgPath[0] != '/')/*superfallback, try current dir.*/
    {
        fprintf(stderr, "CfgPath: %s\n", CfgPath);
        fprintf(stderr, "Failure to resolve config dir, resorting to ./\n");
        strncpy(CfgPath, CfgFileName, PATH_MAX);
    }

    /*If this 130 lines of monstrosity fail,
    the config will simply just not be found.
    This means the default settings will remain
    (process Alists inside RSP core, Dlists passed through).
    If they are changed before loading, the config file will
    end up in the users CWD, provided they can write there.*/
#ifdef DEBUG
    printf("[RSP] CfgPath: %s\n", CfgPath);
#endif
}

void LoadConfig()
{
    char line[256];
    FILE *f;
    f = fopen(CfgPath, "r");
    if(f == NULL)
    {
#ifdef DEBUG
       fprintf(stderr, "[RSP] Could not open RSP config file for loading!\n");
#endif
        return;
    }
    while(!feof(f))
    {
        if(fgets(line, 256, f) == NULL) { break; }
        if(line[0] == '\n' || line[0] == '\0' || line[0] == '[') { continue; }
        if(sscanf(line, "AudioHle=%i\n", &AudioHle) == 1) { continue; }
        if(sscanf(line, "GraphicsHle=%i\n", &GraphicsHle) == 1) { continue; }
        if(sscanf(line, "SpecificHle=%i\n", &SpecificHle) == 1) { continue; }
        if(strncmp(line, "Audio Plugin=", 13) == 0)
            {
            char* ptr = line;
            strcpy(audioname, (ptr+13));
            ptr = strchr(audioname, '\n');
            ptr[0] = '\0';
            }
    }
    fclose(f);
}

void SaveConfig()
{
    FILE* filepointer;
    filepointer = fopen(CfgPath, "w");

    if(filepointer==NULL)
         {
         fprintf(stderr, "[RSP] Could not open RSP config file for saving!\n");
         return;
         }

    fprintf(filepointer, "[Settings]\n");
    fprintf(filepointer, "AudioHle=%i\n", AudioHle);
    fprintf(filepointer, "GraphicsHle=%i\n", GraphicsHle);
    fprintf(filepointer, "SpecificHle=%i\n", SpecificHle);
    fprintf(filepointer, "Audio Plugin=%s\n", audioname);
    fclose(filepointer);
}
