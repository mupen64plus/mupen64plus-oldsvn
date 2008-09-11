/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rsp plugin - extension.c                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Dylan Wagstaff (Pyromanik)                         *
 *   Linked list and plugin loading done by Hacktarux, 2002.               *
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

#include "wintypes.h"
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <stdio.h>
#include "Rsp_1.1.h"
#include "Audio_1.1.h"
#include "extension.h"
#include "gui.h"

extern RSP_INFO rsp;

static AUDIO_INFO audio_info;
static BOOL (*initiateAudio)(AUDIO_INFO Audio_Info);
void (*getDllInfo)(PLUGIN_INFO *PluginInfo);
static HMODULE audiohandle = NULL; /* HMODULE is of type void*   */
char audioname[100];


char LibPath[PATH_MAX];

/*this function finds the full path from root to specified dir.
Since the configs and plugins can be stored seperately
Find function is weak due to reliance on plugins dir being named plugins, and
also that all plugins wern't loaded specifically at the command line (unpredictable results).*/
void getLibPath ()
{
    char line[PATH_MAX];
    char exe[PATH_MAX];
    char* ptr;
    DIR *d;
    int e;
    FILE *f;

    LibPath[0] = '\0';

    /*this is the main search. it should almost never fail.*/
    f = fopen("/proc/self/maps", "r");
    if(f != NULL)
    {
        while(!feof(f))
            {
            if(fgets(line, PATH_MAX, f)==NULL)
                continue;
            if(line[0]=='\n'||line[0]=='\0')
                continue;
            if(strstr(line, "/plugins/") != NULL)
                {
                /* Qt also has a plugins dir. We don't want this. */
                if(strstr(line, "/qt/") != NULL)
                    continue;

                ptr = strchr(line, '/');
                strncpy(line, dirname(ptr), PATH_MAX);
                break;
            }
        }
        fclose(f);
        /*we had better check it.*/
        d = opendir(line);
        if(d != NULL)
        {
            strncpy(LibPath, line, PATH_MAX);
            closedir(d);
        }
    }
    else /* main search failed :/  */
    {    /* check the exe directory for a plugins folder */
        e = readlink("/proc/self/exe", line, PATH_MAX);
        if(e != -1)
        {
            line[e] = '\0';
            strncpy(line, dirname(line), PATH_MAX);
            if(line[0] != '.')
            {
                strncat(line, "/plugins\0", (PATH_MAX - strlen(line)));
                d = opendir(line);
                if(d != NULL)
                {
                    strncpy(LibPath, line, PATH_MAX);
                    closedir(d);
                }
            }
        }
    }
    if(LibPath[0] != '/') /*main search catastrophic failure; fallback :S */
    {
        /*if exe has been renamed,
        or we're not plugged into mupen64plus...
        we need to account for these things too.*/
        e = readlink("/proc/self/exe", exe, PATH_MAX);
        if(e != -1)
        {
            exe[e] = '\0';
            strncpy(exe, basename(exe), PATH_MAX);
            if((PATH_MAX - (strlen(exe) + 20)) > 0)
            {
                sprintf(line, "/usr/share/%s/plugins\0", exe);
                d = opendir(line);
                if(d != NULL)
                {
                    strncpy(LibPath, line, PATH_MAX);
                    closedir(d);
                }
                else
                {
                    if((PATH_MAX - (strlen(exe) + 26)) > 0)
                    {
                        sprintf(line, "/usr/local/share/%s/plugins\0", exe);
                        d = opendir(line);
                        if(d != NULL)
                        {
                            strncpy(LibPath, line, PATH_MAX);
                            closedir(d);
                        }
                    }
                }
            }
        }
    }
    /*thankfully if all of these searches fail,
    the search function will default to looking
    in ./plugins (or fail gracefully)*/
}

/*definition of a chain link for a linked list of audio plugins*/
typedef struct _plugins plugins;
struct _plugins
{
    char *file_name;
    char *plugin_name;
    HMODULE handle;/* HMODULE is typedef'd to void*  */
    int type;
    plugins *next;
};
static plugins *liste_plugins = NULL, *current;

/* This function makes a linked list of audio plugins and their info. */
void insert_plugin(plugins* p, char* file_name, char* plugin_name, void* handle, int type,int num)
{
    if (p->next)
        insert_plugin(p->next, file_name, plugin_name, handle, type, (p->type == type) ? num+1 : num);
    else
    {
        p->next = malloc(sizeof(plugins));
        p->next->type = type;
        p->next->handle = handle;
        p->next->file_name = malloc(strlen(file_name)+1);
        strcpy(p->next->file_name, file_name);
        p->next->plugin_name = malloc(strlen(plugin_name)+7);
        sprintf(p->next->plugin_name, "%s", plugin_name);
        p->next->next=NULL;
    }
}

/*gets next chain link from linked list, if there is one*/
char *next_plugin()
{
    if (!current->next) return NULL;
    current = current->next;
    return current->plugin_name;
}

/*returns the memory address handle to the plugin*/
char *get_handle(plugins *p, char *name)
{
    if (!p->next) return NULL;
    if (!strncmp(p->next->plugin_name, name, strlen(name)))
        return p->next->file_name;
    else  
        return get_handle(p->next, name);
}

/*gets the last 3 digits from the end of a string
(presumably path to library, checks for .dll eventually)*/
char* getExtension(char *str)
{
    if (strlen(str) > 3) return str + strlen(str) - 3;
    else return NULL;
}

/*search through plugin dir to find audio plugins
uses all the helping functions and structs defined above*/
void search_plugins()
{
    DIR *dir;
    char cwd[PATH_MAX];
    char name[PATH_MAX];
    struct dirent *entry;

    liste_plugins = malloc(sizeof(plugins));
    liste_plugins->type = -1;
    liste_plugins->next = NULL;

    if(LibPath[0] != '/')
        sprintf(cwd, "./plugins");
    else
        sprintf(cwd, LibPath);

#ifdef DEBUG
    printf("[RSP] plugins dir: %s\n", cwd);
#endif
    dir = opendir(cwd);
    if(dir != NULL)
        {
        while((entry = readdir(dir)) != NULL)
            {
            HMODULE handle;/*has been typedef'd as void*  */

            strcpy(name, cwd);
            strcat(name, "/");
            strcat(name, entry->d_name);
            if (getExtension(entry->d_name) != NULL && strcmp(getExtension(entry->d_name),".so")==0)
                {
                handle = dlopen(name, RTLD_NOW); /* Load the library. */
                if (handle)
                    {
                    PLUGIN_INFO PluginInfo;
                    getDllInfo = dlsym(handle, "GetDllInfo"); /* Get function address. */
                    if (getDllInfo)
                        {
                        getDllInfo(&PluginInfo);
                        if(PluginInfo.Type==PLUGIN_TYPE_AUDIO)
                            insert_plugin(liste_plugins, name, PluginInfo.Name, handle, PluginInfo.Type, 0);
                        }
                    }
                }
            }

        closedir(dir);
        }

    current = liste_plugins;
}

/*declare the alist processer*/
void (*processAList)();

/*entry point for accessing the extra functionality of the RSP*/
void startup()
{
    getLibPath();     /* get plugins dir         */
    search_plugins(); /* list the audio plugins  */
    getCfgPath();     /* get config file         */
    LoadConfig();     /* load the config         */

    /*setup variables for our GUI interface.*/
    pseudospecifichle = SpecificHle;
    pseudoaudiohle = AudioHle;
    pseudographicshle = GraphicsHle;
    strcpy(pseudoaudioname, audioname);
}

/* These dummy functions/variables prevent mupen from crashing if they're called. */
static BYTE fake_header[0x1000];
static DWORD fake_AI_DRAM_ADDR_REG;
static DWORD fake_AI_LEN_REG;
static DWORD fake_AI_CONTROL_REG;
static DWORD fake_AI_STATUS_REG;
static DWORD fake_AI_DACRATE_REG;
static DWORD fake_AI_BITRATE_REG;

/*loads our new RSP audio plugin... or not.*/
BOOL loadPlugin() 
{
    audiohandle = dlopen(get_handle(liste_plugins, audioname), RTLD_NOW);
    if(!audiohandle)
        return FALSE;

    audio_info.hinst = rsp.hInst;
    audio_info.MemoryBswaped = TRUE;
    audio_info.HEADER = fake_header;
    audio_info.RDRAM = rsp.RDRAM;
    audio_info.DMEM = rsp.DMEM;
    audio_info.IMEM = rsp.IMEM;
    audio_info.MI_INTR_REG = rsp.MI_INTR_REG;
    audio_info.AI_DRAM_ADDR_REG = &fake_AI_DRAM_ADDR_REG;
    audio_info.AI_LEN_REG = &fake_AI_LEN_REG;
    audio_info.AI_CONTROL_REG = &fake_AI_CONTROL_REG;
    audio_info.AI_STATUS_REG = &fake_AI_STATUS_REG;
    audio_info.AI_DACRATE_REG = &fake_AI_DACRATE_REG;
    audio_info.AI_BITRATE_REG = &fake_AI_BITRATE_REG;
    audio_info.CheckInterrupts = rsp.CheckInterrupts;

    initiateAudio = dlsym(audiohandle, "InitiateAudio");
    if(initiateAudio==NULL)
        {
        fprintf(stderr, "RSP failed to load audio plugin initilization\n");
        return FALSE;
        }

    processAList = dlsym(audiohandle, "ProcessAList");
    if(processAList!=NULL)
        {
        fprintf(stderr, "RSP failed to load alist processer\n");
        return FALSE;
        }

    initiateAudio(audio_info);
    printf("RSP audio plugin loaded!\n");
    return TRUE;
}
