// Generic Platform Specific Header for POSIX

#ifndef __POSIX_SPECIFIC_H__
#define __POSIX_SPECIFIC_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include "../wintypes.h"
#include <string.h>

static char *dirsep="/";
static char *curdir="./";

static inline int mkdirwp(const char *path, mode_t mode)
{
    return mkdir(path,mode);
}

static inline char *getexedir()
{
    char *path = (char *)malloc(PATH_MAX);
    int n = readlink("/proc/self/exe", path, PATH_MAX);
    if (n == -1)
        strcpy(path, "./");
    else
    {
        path[n] = '\0';
        while(path[strlen(path)-1] != '/')
            path[strlen(path)-1] = '\0';
    }

    return path;
}

static inline char *gethomedir()
{
    char *path = (char *)malloc(PATH_MAX);
    memset(path,0,PATH_MAX);
    strncpy(path, getenv("HOME"), PATH_MAX);
    strncat(path, "/.mupen64plus/", PATH_MAX-strlen(path));
    return path;
}

#endif
