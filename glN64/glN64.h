#ifndef GLN64_H
#define GLN64_H

#ifdef USEPOSIX
#include "../main/wintypes.h"
#endif
#ifdef USEWIN32
#include <windows.h>
#endif

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

//#define DEBUG
//#define RSPTHREAD

#ifdef USEWIN32
extern HWND         hWnd;
extern HWND         hFullscreen;
extern HWND         hStatusBar;
extern HWND         hToolBar;
extern HINSTANCE    hInstance;
#endif // !__LINUX__

extern char         pluginName[];

extern void (*CheckInterrupts)( void );
extern char *screenDirectory;
extern char configdir[PATH_MAX];

#endif

