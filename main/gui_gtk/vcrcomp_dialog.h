/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - vcrcomp_dialog.h                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Blight                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* vcrcomp_dialog.h - Handles VCR mode GUI elements */

#ifdef VCR_SUPPORT

#ifndef __VCRCOMP_DIALOG_H__
#define __VCRCOMP_DIALOG_H__

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget   *dialog;
    GtkWidget   *notebook;

    GtkWidget   *videoPage;
    GtkWidget   *videoCodecCombo;
    GList       *videoCodecGList;
    GtkWidget   *videoAttribCombo;
    GList       *videoAttribGList;
    GtkWidget   *videoAttribEntry;
    GtkWidget   *videoAttribChangeButton;

    GtkWidget   *audioPage;
    GtkWidget   *audioCodecCombo;
    GList       *audioCodecGList;
    GtkWidget   *audioAttribCombo;
    GList       *audioAttribGList;
    GtkWidget   *audioAttribEntry;
    GtkWidget   *audioAttribChangeButton;
} SVcrCompDialog;

extern SVcrCompDialog g_VcrCompDialog;

int create_vcrCompDialog( void );

#endif // __VCRCOMP_DIALOG_H__

#endif // VCR_SUPPORT
