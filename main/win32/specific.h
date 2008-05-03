// Generic Platform Specific Header for Win32

#ifndef __WIN32_SPECIFIC_H__
#define __WIN32_SPECIFIC_H__

#include <windows.h>
#include <direct.h>
#include <commctrl.h>
#include <limits.h>
#include <shlobj.h>

static char *dirsep="\\";
static char *curdir=".\\";

static inline void usleep(unsigned long time)
{
    Sleep(time/1000);
}

static inline int mkdirwp(const char *path, int mode)
{
    return mkdir(path);
}

static inline char *getexedir()
{
    char *path = (char *)malloc(PATH_MAX);
    memset(path,0,PATH_MAX);
    GetModuleFileNameA(NULL, path, PATH_MAX);

    /*int fnlen = strlen(path);
    char *newfn = NULL;
    int i;

    while(path[strlen(path)-1] != dirsep[0])
        path[strlen(path)-1] = '\0';*/

    return path;
}

static inline char *gethomedir()
{
    char *path = (char *)malloc(PATH_MAX);
    memset(path,0,PATH_MAX);
    SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,0,path);
    strncat(path, "\\Mupen64Plus\\", PATH_MAX - strlen(path));
    //strncat(path, ".\\config", PATH_MAX);
    return path;
}

#endif
