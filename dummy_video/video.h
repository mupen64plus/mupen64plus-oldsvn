#ifndef VIDEO_H
#define VIDEO_H

#ifndef __LINUX__
#include <windows.h>
#else
# include "../main/winlnxdefs.h"
#endif

extern char			pluginName[];

extern void (*CheckInterrupts)( void );
extern char *screenDirectory;
extern char configdir[PATH_MAX];

#endif

