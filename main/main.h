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

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

/* globals */
extern int autoinc_slot;
extern int *autoinc_save_slot;
extern int g_Noask;
extern int g_NoaskParam;
extern pthread_t g_EmulationThread;
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

void new_frame();
void new_vi();

int open_rom(const char *filename);
int close_rom(void);
void startEmulation(void);
void stopEmulation(void);
void pauseContinueEmulation(void);
void screenshot(void);

#endif // __MAIN_H__
