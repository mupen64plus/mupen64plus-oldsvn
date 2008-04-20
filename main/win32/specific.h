// Generic Platform Specific Header for Win32

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

static inline char *ss(const char *string, int start, int end)
{
    int length = end-start;
    if(length <= 0)
    {
        return NULL;
    }
    char *alloc = (char *)malloc(length+1);
    int i;
    for(i = 0; i < length; i++)
    {
        alloc[i] = string[i+start];
    }
    alloc[length] = '\0';
    
    return alloc;
}

static inline char *getexedir()
{
    char *path = (char *)malloc(PATH_MAX);
    GetModuleFileNameA(NULL, path, PATH_MAX);

    int fnlen = strlen(path);
    char *newfn = NULL;
    int i;
    for(i=fnlen; i > 0; i--)
    {
        if(path[i] == dirsep[0])
        {
            newfn = ss(path, 0, i);
            break;
        }
    }

    return newfn;
}

static inline char *gethomedir()
{
    char *path = (char *)malloc(PATH_MAX);
    memset(path,0,PATH_MAX);
    SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,NULL,path);
    strncat(path, "\\Mupen64Plus\\", PATH_MAX - strlen(path));
    //strncat(path, ".\\config", PATH_MAX);
    return path;
}


