#ifndef VIDEO_H
#define VIDEO_H

#include <specific.h>

extern char         pluginName[];

extern void (*CheckInterrupts)( void );
extern char *screenDirectory;
extern char configdir[PATH_MAX];

#endif

