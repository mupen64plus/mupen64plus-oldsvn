/***************************************************************************
 configdialog.h - Handles the configuration dialog
----------------------------------------------------------------------------
Began                : Sat Nov 9 2002
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

#ifndef __CONFIGDIALOG_H__
#define __CONFIGDIALOG_H__

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget   *dialog;

    GtkWidget   *notebook;
    GtkWidget   *configMupen;
    GtkWidget   *configPlugins;
    GtkWidget   *configRomBrowser;
    GtkWidget   *configInputMappings;

    GtkWidget   *coreInterpreterCheckButton;
    GtkWidget   *coreDynaRecCheckButton;
    GtkWidget   *corePureInterpCheckButton;
    GtkWidget   *autoincSaveSlotCheckButton;
    GtkWidget   *noaskCheckButton;
    GtkWidget   *OsdEnabled;
    GtkWidget   *alwaysFullscreen;
    GList       *toolbarStyleGList;
    GtkWidget   *toolbarStyleCombo;
    GList       *toolbarSizeGList;
    GtkWidget   *toolbarSizeCombo;

    GtkWidget   *gfxCombo;
    GtkWidget   *audioCombo;
    GtkWidget   *inputCombo;
    GtkWidget   *RSPCombo;
    GList       *gfxPluginGList;
    GList       *audioPluginGList;
    GList       *inputPluginGList;
    GList       *RSPPluginGList;

    GtkWidget   *romDirectoryList;
    GtkWidget   *romDirsScanRecCheckButton;
    GtkWidget   *romShowFullPathsCheckButton;
    GtkWidget   *noCompiledJumpCheckButton;
    GtkWidget   *noMemoryExpansion;
} SConfigDialog;
extern SConfigDialog g_ConfigDialog;

int create_configDialog( void );

#endif // __CONFIGDIALOG_H__
