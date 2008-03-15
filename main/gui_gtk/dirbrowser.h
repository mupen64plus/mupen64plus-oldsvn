/***************************************************************************
 dirbrowser.c - Handles directory browsing
----------------------------------------------------------------------------
Began            : Sat Nov 9 2002
Copyright        : (C) 1998-2002 Peter Alm, Mikael Alm, Olle Hallnas,
                                 Thomas Nilsson and 4Front Technologies
Email            : <Occluded for size reasons>
---------------------------------------------------------------------------
Notice: This file was derived from the XMMS source code.
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef XMMS_DIRBROWSER_H
#define XMMS_DIRBROWSER_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

GtkWidget *create_dir_browser(gchar * title, gchar * current_path, GtkSelectionMode mode, void (*handler) (gchar *));

#ifdef __cplusplus
};
#endif


#endif
