/***************************************************************************
 main_gtk.h - Handles the main window and 'glues' it with other windows
----------------------------------------------------------------------------
Began                : Fri Nov 8 2002
Copyright            : (C) 2002 by blight
Email                : blight@Ashitaka
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <signal.h>
#include <float.h>
#include <windows.h>
#include <shlobj.h>
#include <limits.h>

#ifndef _FPE_INVALID
#define _FPE_INVALID        0x81
#define _FPE_DENORMAL       0x82
#define _FPE_ZERODIVIDE     0x83
#define _FPE_OVERFLOW       0x84
#define _FPE_UNDERFLOW      0x85
#define _FPE_INEXACT        0x86
#define _FPE_UNEMULATED     0x87
#define _FPE_SQRTNEG        0x88
#define _FPE_STACKOVERFLOW  0x8a
#define _FPE_STACKUNDERFLOW 0x8b
#define _FPE_EXPLICITGEN    0x8c    /* raise( SIGFPE ); */
#endif

/* globals */
extern HANDLE g_EmulationThread;
extern HINSTANCE hinst;

HWND InitWindow(HINSTANCE hinstance);
BOOL InitInstance(HINSTANCE hinstance);
int file_exists(const char *fileName);

