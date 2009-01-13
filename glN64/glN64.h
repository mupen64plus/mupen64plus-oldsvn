#ifndef GLN64_H
#define GLN64_H

#if !defined(__LINUX__) && !defined(__sgi)
#include <windows.h>
//#include <commctrl.h>
#else
# include "../main/winlnxdefs.h"
#endif

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

//#define DEBUG
//#define RSPTHREAD

#if !defined(__LINUX__) && !defined(__sgi)
extern HWND         hWnd;
//extern HWND           hFullscreen;
extern HWND         hStatusBar;
extern HWND         hToolBar;
extern HINSTANCE    hInstance;
#endif // !__LINUX__ && !__sgi

extern char         pluginName[];

extern void (*CheckInterrupts)( void );
extern char *screenDirectory;
extern char configdir[PATH_MAX];
extern void (*renderCallback)();

#endif

