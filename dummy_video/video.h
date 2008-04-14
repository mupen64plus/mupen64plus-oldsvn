#ifndef VIDEO_H
#define VIDEO_H

#ifdef USEPOSIX
#include "../main/wintypes.h"
#endif
#ifdef USEWIN32
#include <windows.h>
#endif

extern char         pluginName[];

extern void (*CheckInterrupts)( void );
extern char *screenDirectory;
extern char configdir[PATH_MAX];

#endif

