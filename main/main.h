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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <pthread.h>
#include "../network/network.h"

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

/* globals */
extern int g_Noask;
extern int g_NoaskParam;
extern int g_MemHasBeenBSwapped;
extern int g_TakeScreenshot;
extern int g_OsdEnabled;
extern int g_Fullscreen;
extern pthread_t g_EmulationThread;
extern pthread_t g_RomCacheThread;
extern char *g_GfxPlugin;
extern char *g_AudioPlugin;
extern char *g_InputPlugin;
extern char *g_RspPlugin;

char *get_configpath(void);
char *get_installpath(void);
char *get_savespath(void);
char *get_iconspath(void);
char *get_iconpath(char *iconfile);
int   gui_enabled(void);
void setSpeed(unsigned int);

void new_frame();
void new_vi();

void startEmulation(void);
void stopEmulation(void);
int pauseContinueEmulation(void);

void main_pause(void);
void main_advance_one(void);
void main_speedup(int percent);
void main_speeddown(int percent);
void main_draw_volume_osd(void);

void take_next_screenshot(void);
void main_message(unsigned int console, unsigned int statusbar, unsigned int osd, unsigned int osd_corner, const char *format, ...);
void error_message(const char *format, ...);

void netplayReady(void);

#endif // __MAIN_H__
