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

#ifndef __MAIN_GTK_H__
#define __MAIN_GTK_H__

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget   *window;
    GtkWidget   *toplevelVBox;  // vbox containing menubar, toolbar, rombrowser, statusbar
    GtkWidget   *menuBar;
    GtkWidget   *toolBar;
    GtkWidget   *filter;
    GtkWidget   *romScrolledWindow;
//Make two TreeViews, a visable manually filtered one for the Display, and a
//Non-visable FullList from which we can filter.
    GtkWidget   *romDisplay, *romFullList;
    GtkWidget   *statusBarHBox;
    GtkAccelGroup *accelGroup;
    GtkAccelGroup *accelUnsafe; //GtkAccelGroup for keys without Metas. Prevents GtkEntry widgets.
    gboolean accelUnsafeActive; //From getting keypresses, so must be deactivated.
} SMainWindow;

extern SMainWindow g_MainWindow;

void reload();
void statusbar_message( const char *section, const char *fmt, ... );

// helper macro
#define GUI_PROCESS_QUEUED_EVENTS() \
{ \
    gdk_threads_leave(); \
    while(g_main_context_iteration(NULL, FALSE)); \
    gdk_threads_enter(); \
}

#endif // __MAIN_GTK_H__
