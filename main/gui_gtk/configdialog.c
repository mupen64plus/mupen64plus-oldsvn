/***************************************************************************
 configdialog.c - Handles the configuration dialog
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

#include "configdialog.h"

#include "../main.h"
#include "main_gtk.h"
#include "rombrowser.h"
#include "../romcache.h"
#include "../config.h"
#include "../guifuncs.h"
#include "../translate.h"
#include "../util.h"

#include "../winlnxdefs.h"
#include "../plugin.h"

#include <gtk/gtk.h>
#include <SDL.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** globals **/
SConfigDialog g_ConfigDialog;

struct input_mapping {
    char *name;                     // human-readable name of emulation special function
    char *key_mapping;              // keyboard mapping of function
    char *joy_config_name;          // name of joystick mapping in config file
    GtkWidget *joy_mapping_textbox; // textbox displaying string representation of joystick mapping
};

#define mapping_foreach(mapping) for(mapping = mappings; mapping->name; mapping++)

// special function input mappings
static struct input_mapping mappings[] =
    {
        {
            "Toggle Fullscreen",
            "Alt+Enter",
            "Joy Mapping Fullscreen",
            NULL
        },
        {
            "Stop Emulation",
            "Esc",
            "Joy Mapping Stop",
            NULL
        },
        {
            "Pause Emulation",
            "Pause",
            "Joy Mapping Pause",
            NULL
        },
        {
            "Save State",
            "F5",
            "Joy Mapping Save State",
            NULL
        },
        {
            "Load State",
            "F7",
            "Joy Mapping Load State",
            NULL
        },
        {
            "Increment Save Slot",
            "0-9",
            "Joy Mapping Increment Slot",
            NULL
        },
        {
            "Save Screenshot",
            "F12",
            "Joy Mapping Screenshot",
            NULL
        },
        {
            "Mute/unmute volume",
            "M",
            "Joy Mapping Mute",
            NULL
        },
        {
            "Decrease volume",
            "[",
            "Joy Mapping Decrease Volume",
            NULL
        },
        {
            "Increase volume",
            "]",
            "Joy Mapping Increase Volume",
            NULL
        },
        {
            "Gameshark Button",
            "G",
            "Joy Mapping GS Button",
            NULL
        },
        { 0, 0, 0, 0 } // last entry
    };

/** callbacks **/
// gfx
static void callback_configGfx( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.gfxCombo) );
    if( name ) plugin_exec_config( name );
}

static void callback_testGfx( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.gfxCombo) );
    if( name ) plugin_exec_test( name );
}

static void callback_aboutGfx( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.gfxCombo) );
    if( name ) plugin_exec_about( name );
}

// audio
static void callback_configAudio( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.audioCombo) );
    if( name ) plugin_exec_config( name );
}

static void callback_testAudio( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.audioCombo) );
    if( name ) plugin_exec_test( name );
}

static void callback_aboutAudio( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.audioCombo) );
    if( name ) plugin_exec_about( name );
}

// input
static void callback_configInput( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.inputCombo) );
    if( name ) plugin_exec_config( name );
}

static void callback_testInput( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.inputCombo) );
    if( name ) plugin_exec_test( name );
}

static void callback_aboutInput( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.inputCombo) );
    if( name ) plugin_exec_about( name );
}

// RSP
static void callback_configRSP( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.RSPCombo) );
    if( name ) plugin_exec_config( name );
}

static void callback_testRSP( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.RSPCombo) );
    if( name ) plugin_exec_test( name );
}

static void callback_aboutRSP( GtkWidget *widget, gpointer data )
{
    const char *name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.RSPCombo) );
    if( name ) plugin_exec_about( name );
}

gboolean callback_deleteForEach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata)
{
    if(gtk_tree_model_get_iter_first(model, iter))
        { gtk_list_store_remove(GTK_LIST_STORE(model), iter); }
    return 0;
}
gboolean Match = 0;
gboolean callback_checkForEach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata)
{
    Match = 0;
    gchar *text;
    gtk_tree_model_get(model, iter, 0, &text, -1);
    if( !strcmp((gchar *)userdata, text ) )
    {
        Match = 1;
        return 1;
    }
    return 0;
}

// rombrowser
static void addRomDirectory(const gchar *dirname)
{
    // Local variables
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_ConfigDialog.romDirectoryList));

    gtk_tree_model_foreach(model, callback_checkForEach, (gpointer) dirname);
    if(Match)
    {
        alert_message(tr("The directory you selected is already in the list."));
        Match = 0;
        return;
    }

    // Add the item
    GtkTreeIter newiter;
    gtk_list_store_append (GTK_LIST_STORE(model), &newiter);
    gtk_list_store_set (GTK_LIST_STORE(model), &newiter, 0, dirname,-1);
}

static void callback_romDirectoryAdd( GtkWidget *widget, gpointer data )
{
    GtkWidget *file_chooser;

    // get a directory path from the user
    file_chooser = gtk_file_chooser_dialog_new(tr("Select Rom Directory"),
                                               GTK_WINDOW(g_ConfigDialog.dialog),
                                               GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                               NULL);

    if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
    {
        char *buf;
        gchar *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
        buf = malloc(strlen(path)+2);
        strcpy(buf, path);

        // add trailing '/'
        if(buf[strlen(buf)-1] != '/')
            strcat(buf, "/");

        addRomDirectory(buf);
    
        free(buf);
        g_free(path);
    }

    gtk_widget_destroy(file_chooser);
}

static void callback_romDirectoryRemove( GtkWidget *widget, gpointer data )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_ConfigDialog.romDirectoryList));
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW ( g_ConfigDialog.romDirectoryList ));

    while (gtk_tree_selection_count_selected_rows (selection) > 0)
    {
        GList *list = NULL, *llist = NULL;
        list = gtk_tree_selection_get_selected_rows (selection, &model);
        llist = g_list_first (list);
        do{
            GtkTreeIter iter;
            if(gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data))
                { gtk_list_store_remove(GTK_LIST_STORE(model), &iter); }
        } while ((llist = g_list_next (llist)));
        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);
    }
}

static void callback_romDirectoryRemoveAll( GtkWidget *widget, gpointer data )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_ConfigDialog.romDirectoryList));
    gtk_tree_selection_select_all(selection);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW ( g_ConfigDialog.romDirectoryList ));

    while (gtk_tree_selection_count_selected_rows (selection) > 0)
    {
        GList *list = NULL, *llist = NULL;
        list = gtk_tree_selection_get_selected_rows (selection, &model);
        llist = g_list_first (list);
        do
        {
            GtkTreeIter iter;
            if(gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data))
                { gtk_list_store_remove(GTK_LIST_STORE(model), &iter); }
        } while ((llist = g_list_next (llist)));
        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);
    }
}

// OK/Cancel Button
static void callback_okClicked( GtkWidget *widget, gpointer data )
{
    const char *filename, *name;
    int i;

    // save configuration
    // for plugins, don't automatically save plugin to config if user specified it at the commandline
    name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.gfxCombo) );
    if( !g_GfxPlugin && name )
    {
        filename = plugin_filename_by_name( name );
        if( filename ) config_put_string( "Gfx Plugin", filename );
    }
    name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.audioCombo) );
    if( !g_AudioPlugin && name )
    {
        filename = plugin_filename_by_name( name );
        if( filename ) config_put_string( "Audio Plugin", filename );
    }
    name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.inputCombo) );
    if( !g_InputPlugin && name )
    {
        filename = plugin_filename_by_name( name );
        if( filename ) config_put_string( "Input Plugin", filename );
    }
    name = gtk_combo_box_get_active_text( GTK_COMBO_BOX(g_ConfigDialog.RSPCombo) );
    if( !g_RspPlugin && name )
    {
        filename = plugin_filename_by_name( name );
        if( filename ) config_put_string( "RSP Plugin", filename );
    }

    i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romDirsScanRecCheckButton) );
    config_put_bool( "RomDirsScanRecursive", i );

    i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romShowFullPathsCheckButton) );
    config_put_bool( "RomBrowserShowFullPaths", i );

        i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noAudioDelayCheckButton) );
    config_put_bool( "NoAudioDelay", i );
        i = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noCompiledJumpCheckButton) );
    config_put_bool( "NoCompiledJump", i );

        savestates_set_autoinc_slot(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_ConfigDialog.autoincSaveSlotCheckButton)));
        config_put_bool("AutoIncSaveSlot", savestates_get_autoinc_slot());

    // if --noask was specified at the commandline, don't modify config
    if(!g_NoaskParam)
    {
        g_Noask = !gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noaskCheckButton) );
                config_put_bool( "No Ask", g_Noask );
    }

    i = 0;

    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW ( g_ConfigDialog.romDirectoryList ));
    GtkTreeIter iter; //= (GtkTreeIter *)malloc(sizeof(GtkTreeIter));

    // Get the first item
    gboolean keepGoing = gtk_tree_model_get_iter_first(model, &iter);

    // Iterate through the directory list and add each to the configuration
    while( keepGoing )
    {
        // Get the text
        gchar *text = NULL;
        gtk_tree_model_get(model, &iter, 0, &text, -1);

        // Add the directory to the configuration
        if( text != NULL )
        {
            char buf[30];
            sprintf( buf, "RomDirectory[%d]", i++ );
            config_put_string( buf, text );
        }

        // Get the next item
        keepGoing = gtk_tree_model_iter_next(model, &iter);
    }
    config_put_number( "NumRomDirs", i );
    g_RCSTask = RCS_RESCAN;

    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ) )
        config_put_number( "Core", CORE_INTERPRETER );
    else if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreDynaRecCheckButton) ) )
        config_put_number( "Core", CORE_DYNAREC );
    else
        config_put_number( "Core", CORE_PURE_INTERPRETER );

    // hide dialog
    gtk_widget_hide( g_ConfigDialog.dialog );

    switch(gtk_combo_box_get_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo)))
    {
        case 2:
            if(config_get_number("ToolbarSize",0)!=32)
            {
                config_put_number("ToolbarSize",32);
                gtk_toolbar_set_icon_size( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ICON_SIZE_SMALL_TOOLBAR);
                reload();
            }
        break;
        case 0:
            if(config_get_number("ToolbarSize",0)!=16)
            {
                config_put_number("ToolbarSize",16);
                gtk_toolbar_set_icon_size( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ICON_SIZE_SMALL_TOOLBAR);
                reload();
            }
        break;
        default:
            if(config_get_number("ToolbarSize",0)!=22)
            {
                config_put_number("ToolbarSize",22);
                gtk_toolbar_set_icon_size( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ICON_SIZE_SMALL_TOOLBAR);
                reload();
            }
    }

    switch(gtk_combo_box_get_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo)))
    {
        case 0:
            if(config_get_number( "ToolbarStyle", 0 ) != 0)
            {
                config_put_number( "ToolbarStyle", 0 );
                gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS );
                reload();
            }
        break;
        case 1:
            if(config_get_number( "ToolbarStyle", 0 ) != 1)
            {
                config_put_number( "ToolbarStyle", 1 );
                gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_TEXT );
                reload();
            }
        break;
        case 2:
            if(config_get_number( "ToolbarStyle", 0 ) != 2)
            {
                config_put_number( "ToolbarStyle", 2 );
                gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_BOTH );
                reload();
            }
        break;
        default:
            if(config_get_number( "ToolbarStyle", 0 ) != 0)
            {
                config_put_number( "ToolbarStyle", 0 );
                gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS );
                reload();
            }
    }

    struct input_mapping *mapping;
    gchar *text;

    // update special function input mappings
    mapping_foreach(mapping)
    {
        if(mapping->joy_mapping_textbox)
        {
            text = gtk_editable_get_chars(GTK_EDITABLE(mapping->joy_mapping_textbox), 0, -1);
            if(strcmp(text, config_get_string(mapping->joy_config_name, "")) != 0)
                config_put_string(mapping->joy_config_name, text);
            g_free(text);
        }
    }

    // write configuration
    config_write();
}

static void callback_cancelClicked( GtkWidget *widget, gpointer data )
{
    // hide dialog
    gtk_widget_hide( g_ConfigDialog.dialog );
}

// Initalize configuration dialog
static void callback_dialogShow( GtkWidget *widget, gpointer data )
{
    int i;
    char *name;

    // Load configuration

    callback_romDirectoryRemoveAll( NULL, NULL );
    for( i = 0; i < config_get_number( "NumRomDirs", 0 ); i++ )
    {
        char buf[30];
        sprintf( buf, "RomDirectory[%d]", i );
        addRomDirectory(config_get_string(buf, ""));
    }

    if(g_GfxPlugin)
        name = plugin_name_by_filename(g_GfxPlugin);
    else
        name = plugin_name_by_filename(config_get_string("Gfx Plugin", ""));
    if( name )
    {
        int index = 0;
        if( g_ConfigDialog.gfxPluginGList )
        {
            GList *element = g_list_first(g_ConfigDialog.gfxPluginGList);
            while(element)
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.gfxCombo), index);
                if(strcmp(gtk_combo_box_get_active_text(GTK_COMBO_BOX(g_ConfigDialog.gfxCombo)), name)==0)
                {
                    // if plugin was specified at the commandline, don't let user modify
                    if(g_GfxPlugin)
                        gtk_widget_set_sensitive(g_ConfigDialog.gfxCombo, FALSE);
                    break;
                }
                index++;
                element = g_list_next(element);
            }
        }
    }

    if(g_AudioPlugin)
        name = plugin_name_by_filename(g_AudioPlugin);
    else
        name = plugin_name_by_filename( config_get_string( "Audio Plugin", "" ) );
    if( name )
    {
        int index = 0;
        if( g_ConfigDialog.audioPluginGList )
        {
            GList *element = g_list_first(g_ConfigDialog.audioPluginGList);
            while(element)
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.audioCombo), index);
                if(strcmp(gtk_combo_box_get_active_text(GTK_COMBO_BOX(g_ConfigDialog.audioCombo)), name)==0)
                {
                    // if plugin was specified at the commandline, don't let user modify
                    if(g_AudioPlugin)
                        gtk_widget_set_sensitive(g_ConfigDialog.audioCombo, FALSE);
                    break;
                }
                index++;
                element = g_list_next(element);
            }
        }
    }

    if(g_InputPlugin)
        name = plugin_name_by_filename(g_InputPlugin);
    else
        name = plugin_name_by_filename( config_get_string( "Input Plugin", "" ) );
    if( name )
    {
        int index = 0;
        if( g_ConfigDialog.inputPluginGList )
        {
            GList *element = g_list_first(g_ConfigDialog.inputPluginGList);
            while(element)
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.inputCombo), index);
                if(strcmp(gtk_combo_box_get_active_text(GTK_COMBO_BOX(g_ConfigDialog.inputCombo)), name)==0)
                {
                    // if plugin was specified at the commandline, don't let user modify
                    if(g_InputPlugin)
                        gtk_widget_set_sensitive(g_ConfigDialog.inputCombo, FALSE);
                    break;
                }
                index++;
                element = g_list_next(element);
            }
        }
    }

    if(g_RspPlugin)
        name = plugin_name_by_filename(g_RspPlugin);
    else
        name = plugin_name_by_filename( config_get_string( "RSP Plugin", "" ) );
    if( name )
    {
        int index = 0;
        if( g_ConfigDialog.RSPPluginGList )
        {
            GList *element = g_list_first(g_ConfigDialog.RSPPluginGList);
            while(element)
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.RSPCombo), index);
                if(strcmp(gtk_combo_box_get_active_text(GTK_COMBO_BOX(g_ConfigDialog.RSPCombo)), name)==0)
                {
                    // if plugin was specified at the commandline, don't let user modify
                    if(g_RspPlugin)
                        gtk_widget_set_sensitive(g_ConfigDialog.RSPCombo, FALSE);
                    break;
                }
                index++;
                element = g_list_next(element);
            }
        }
    }
    switch (config_get_number( "Core", CORE_DYNAREC ))
    {
        case CORE_INTERPRETER:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreInterpreterCheckButton), 1 );
        break;
    
        case CORE_DYNAREC:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.coreDynaRecCheckButton), 1 );
        break;
    
        case CORE_PURE_INTERPRETER:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.corePureInterpCheckButton), 1 );
        break;
    }
    
    switch(config_get_number( "ToolbarStyle", 0 ))
    {
        case 0:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), 0);
        break;
        case 1:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), 1);
        break;
        case 2:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), 2);
        break;
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), 0);
        break;
    }

    switch(config_get_number( "ToolbarSize", 22))
    {
        case 32:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), 2);
        break;
        case 16:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), 0);
        break;
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), 1);
        break;
    }

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romDirsScanRecCheckButton), config_get_bool( "RomDirsScanRecursive", FALSE ) );
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.romShowFullPathsCheckButton), config_get_bool( "RomBrowserShowFullPaths", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noAudioDelayCheckButton), config_get_bool( "NoAudioDelay", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noCompiledJumpCheckButton), config_get_bool( "NoCompiledJump", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.autoincSaveSlotCheckButton), config_get_bool( "AutoIncSaveSlot", FALSE ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_ConfigDialog.noaskCheckButton), !g_Noask );
    // if --noask was specified at the commandline, disable checkbox
    gtk_widget_set_sensitive( g_ConfigDialog.noaskCheckButton, !g_NoaskParam );

    struct input_mapping *mapping;

    // set special function input mapping textbox values
    mapping_foreach(mapping)
    {
        if(mapping->joy_mapping_textbox)
        {
            gtk_editable_delete_text(GTK_EDITABLE(mapping->joy_mapping_textbox), 0, -1);
            gtk_editable_insert_text(GTK_EDITABLE(mapping->joy_mapping_textbox),
                                     config_get_string(mapping->joy_config_name, ""),
                                     strlen(config_get_string(mapping->joy_config_name, "")),
                                     &i);
        }
    }
}

/** hide on delete **/
static gint delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_widget_hide( widget );

    // Can not be deleted.
    return TRUE;
}

/** create treeview store **/
static GtkTreeModel *create_store (void)
{
    GtkListStore  *store;
    store = gtk_list_store_new (1, G_TYPE_STRING);
    
    return GTK_TREE_MODEL (store);
}

/** setup treeview **/
static void setup_view (GtkWidget *view)
{
    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1, "Directory", renderer, "text", 0, NULL);
    model = create_store();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);
    g_object_unref (model);
}

static int sdl_loop = 0;

static void callback_cancelSetInput(GtkWidget *widget, gint response, GtkWidget *textbox)
{
    // If user clicked the delete button, delete current input mapping
    if(response == GTK_RESPONSE_NO)
        gtk_editable_delete_text(GTK_EDITABLE(textbox), 0, -1);

    sdl_loop = 0;
    gtk_widget_destroy(widget);
}

static void callback_setInput( GtkWidget *widget, GdkEventAny *event, struct input_mapping *mapping )
{
    int i;
    char *mapping_str = NULL;
    char buf[512] = {0};
    SDL_Joystick *joys[10] = {0};
    SDL_Event sdl_event;
    GtkWidget *dialog;
    GtkWidget *label;
    GdkColor color;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        printf("%s: Could not init SDL video or joystick subsystem\n", __FUNCTION__);
        return;
    }

    SDL_JoystickEventState(SDL_ENABLE);

    // open all joystick devices (max of 10)
    for(i = 0; i < SDL_NumJoysticks() && i < 10; i++)
    {
        joys[i] = SDL_JoystickOpen(i);
        if(joys[i] == NULL)
        {
            printf("%s: Could not open joystick %d (%s): %s\n",
                   __FUNCTION__, i, SDL_JoystickName(i), SDL_GetError());
        }
    }

    // highlight mapping to change
    color.red = color.blue = 0;
    color.green = 0xffff;
    gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &color);
    // fix cursor position
    gtk_editable_select_region(GTK_EDITABLE(widget), 0, 0);

    // pop up dialog window telling user what to do
    dialog = gtk_dialog_new_with_buttons(tr("Map Special Function"),
                                         GTK_WINDOW(g_ConfigDialog.dialog),
                                         GTK_DIALOG_MODAL,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
                                         GTK_STOCK_DELETE, GTK_RESPONSE_NO,
                                         NULL);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    g_signal_connect(dialog,
                     "response",
                     G_CALLBACK(callback_cancelSetInput),
                     widget);

    snprintf(buf, 512, tr("Press a controller button for:\n\"%s\""), mapping->name);
    label = gtk_label_new(buf);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_size_request(label, 165, -1);

    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox), label);
    gtk_widget_show_all(dialog);

    // capture joystick input from user
    sdl_loop = 1;
    while(sdl_loop)
    {
        // let gtk work if it needs to. Need this so user can click buttons on the dialog.
        GUI_PROCESS_QUEUED_EVENTS();

        if(SDL_PollEvent(&sdl_event))
        {
            // only accept joystick events
            if(sdl_event.type == SDL_JOYAXISMOTION ||
               sdl_event.type == SDL_JOYBUTTONDOWN ||
               sdl_event.type == SDL_JOYHATMOTION)
            {
                if((mapping_str = event_to_str(&sdl_event)) != NULL)
                {
                    // change textbox to reflect new joystick input mapping
                    gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
                    gtk_editable_insert_text(GTK_EDITABLE(widget), mapping_str, strlen(mapping_str), &i);

                    struct input_mapping *mapping;

                    // if another function has the same input mapped to it, remove it
                    mapping_foreach(mapping)
                    {
                        gchar *text;

                        if(mapping->joy_mapping_textbox &&
                           mapping->joy_mapping_textbox != widget)
                        {
                            text = gtk_editable_get_chars(GTK_EDITABLE(mapping->joy_mapping_textbox), 0, -1);
                            if(strcmp(text, mapping_str) == 0)
                                gtk_editable_delete_text(GTK_EDITABLE(mapping->joy_mapping_textbox), 0, -1);

                            g_free(text);
                        }
                    }

                    free(mapping_str);
                                
                    sdl_loop = 0;
                    gtk_widget_destroy(dialog);
                }
            }
        }
        usleep(50000); // don't melt the cpu
    }

    // remove highlight on mapping to change
    gtk_widget_modify_base(widget, GTK_STATE_NORMAL, NULL);

    SDL_Quit();
}

/** create dialog **/
int create_configDialog( void )
{
    // Local Variables
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *button, *button_config, *button_test, *button_about;
    GdkPixbuf *pixbuf;
    GtkWidget *icon = NULL;
    GtkWidget *vbox;
    GtkWidget *hbox1, *hbox2;

    list_node_t *node;
    plugin *p;
    gboolean icontheme = check_icon_theme(); //Do we use theme icons or our own?
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    // Nullify the plugin lists (so we can fill them :) )
    g_ConfigDialog.gfxPluginGList = NULL;
    g_ConfigDialog.audioPluginGList = NULL;
    g_ConfigDialog.inputPluginGList = NULL;
    g_ConfigDialog.RSPPluginGList = NULL;

    // Iterate through all of the plugins
    list_foreach(g_PluginList, node)
    {
        p = (plugin *)node->data;

        switch(p->type)
        {
            case PLUGIN_TYPE_GFX:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!g_GfxPlugin ||
                   (g_GfxPlugin &&
                    (strcmp(g_GfxPlugin, p->file_name) == 0)))
                    g_ConfigDialog.gfxPluginGList = g_list_append( g_ConfigDialog.gfxPluginGList, p->plugin_name );
            break;
            case PLUGIN_TYPE_AUDIO:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!g_AudioPlugin ||
                   (g_AudioPlugin &&
                    (strcmp(g_AudioPlugin, p->file_name) == 0)))
                    g_ConfigDialog.audioPluginGList = g_list_append( g_ConfigDialog.audioPluginGList, p->plugin_name );
            break;
            case PLUGIN_TYPE_CONTROLLER:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!g_InputPlugin ||
                   (g_InputPlugin &&
                    (strcmp(g_InputPlugin, p->file_name) == 0)))
                    g_ConfigDialog.inputPluginGList = g_list_append( g_ConfigDialog.inputPluginGList, p->plugin_name );
            break;
            case PLUGIN_TYPE_RSP:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!g_RspPlugin ||
                   (g_RspPlugin &&
                    (strcmp(g_RspPlugin, p->file_name) == 0)))
                    g_ConfigDialog.RSPPluginGList = g_list_append( g_ConfigDialog.RSPPluginGList, p->plugin_name );
            break;
            default:
            break;
        }
    }

    // Create Window
    g_ConfigDialog.dialog = gtk_dialog_new();
    gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.dialog), 10 );
    gtk_window_set_title( GTK_WINDOW(g_ConfigDialog.dialog), tr("Configure") );
    // set main window as parent of config dialog window
    gtk_window_set_transient_for(GTK_WINDOW(g_ConfigDialog.dialog), GTK_WINDOW(g_MainWindow.window));
    gtk_signal_connect( GTK_OBJECT(g_ConfigDialog.dialog), "show", GTK_SIGNAL_FUNC(callback_dialogShow), (gpointer)NULL );
    gtk_signal_connect(GTK_OBJECT(g_ConfigDialog.dialog), "delete_event", GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

    // Create Notebook
    g_ConfigDialog.notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos( GTK_NOTEBOOK(g_ConfigDialog.notebook), GTK_POS_TOP );
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->vbox), g_ConfigDialog.notebook, TRUE, TRUE, 0 );

    // Create OK/Cancel button
    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
    gtk_signal_connect( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback_cancelClicked), (gpointer)NULL );

    button = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_ConfigDialog.dialog)->action_area), button, TRUE, TRUE, 0 );
    gtk_signal_connect( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback_okClicked), (gpointer)NULL );

    // Create Mupen64 configuration page
    {
        label = gtk_label_new( tr("General") );
        g_ConfigDialog.configMupen = gtk_vbox_new( FALSE, 6 );
        gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configMupen), 10 );
        gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configMupen, label );
    
        // Create a frame for CPU core options
        {
            frame = gtk_frame_new( tr("CPU Core Settings") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configMupen), frame, FALSE, FALSE, 0 );
            vbox = gtk_vbox_new( TRUE, 6 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );

            g_ConfigDialog.coreInterpreterCheckButton = gtk_radio_button_new_with_label( NULL, tr("Interpreter") );
            g_ConfigDialog.coreDynaRecCheckButton = gtk_radio_button_new_with_label( gtk_radio_button_group( GTK_RADIO_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ), tr("Dynamic Recompiler") );
            g_ConfigDialog.corePureInterpCheckButton = gtk_radio_button_new_with_label( gtk_radio_button_group( GTK_RADIO_BUTTON(g_ConfigDialog.coreInterpreterCheckButton) ), tr("Pure Interpreter") );
            gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.coreInterpreterCheckButton, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.coreDynaRecCheckButton, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), g_ConfigDialog.corePureInterpCheckButton, FALSE, FALSE, 0 );

            gtk_box_pack_start( GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0 );

            g_ConfigDialog.noAudioDelayCheckButton = gtk_check_button_new_with_label("Disable audio synchronization (For compatibility)");
            g_ConfigDialog.noCompiledJumpCheckButton = gtk_check_button_new_with_label("Disable compiled jump (For compatibility)");
            gtk_box_pack_start(GTK_BOX(vbox), g_ConfigDialog.noAudioDelayCheckButton, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(vbox), g_ConfigDialog.noCompiledJumpCheckButton, FALSE, FALSE, 0);
        }

        // Create a frame for misc GUI options
        {
            frame = gtk_frame_new( tr("Misc. GUI Settings") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configMupen), frame, FALSE, FALSE, 0 );
            vbox = gtk_vbox_new( TRUE, 6 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );

            hbox1 = gtk_hbox_new( TRUE, 2 );
            
            g_ConfigDialog.toolbarStyleCombo = gtk_combo_box_new_text();
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), (char *)tr("Icons") );
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), (char *)tr("Text") );
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarStyleCombo), (char *)tr("Icons & Text") );
    
            gtk_box_pack_start( GTK_BOX(hbox1), gtk_label_new(tr("Toolbar Style:")), 1, 1, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.toolbarStyleCombo, 1, 1, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );

            hbox1 = gtk_hbox_new( TRUE, 2 );
            
            g_ConfigDialog.toolbarSizeCombo = gtk_combo_box_new_text();
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), (char *)tr("Small") );
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), (char *)tr("Medium") );
            gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.toolbarSizeCombo), (char *)tr("Large") );
    
            gtk_box_pack_start( GTK_BOX(hbox1), gtk_label_new(tr("Toolbar Size:")), 1, 1, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.toolbarSizeCombo, 1, 1, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );

            g_ConfigDialog.autoincSaveSlotCheckButton = gtk_check_button_new_with_label(tr("Auto increment save slot"));
            gtk_box_pack_start(GTK_BOX(vbox), g_ConfigDialog.autoincSaveSlotCheckButton, FALSE, FALSE, 0);

            g_ConfigDialog.noaskCheckButton = gtk_check_button_new_with_label(tr("Ask before loading bad dump/hacked rom"));
            gtk_box_pack_start(GTK_BOX(vbox), g_ConfigDialog.noaskCheckButton, FALSE, FALSE, 0);
        }
    
        // Create some misc. core options
        {
        }
    }
    /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! **/
    /** !! The following line will disable the dynamic recompiler radio option !! **/
    /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! **/
    //gtk_widget_set_sensitive( g_ConfigDialog.coreDynaRecCheckButton, FALSE );
    // Yay, looks like this will not be needed in the near future.

    // Create plugin configuration page
    {
        label = gtk_label_new( tr("Plugins") );
        g_ConfigDialog.configPlugins = gtk_vbox_new( FALSE, 5 );
        gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configPlugins), 10 );
        gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins, label );
    
        // GFX Plugin Area
        {
            frame = gtk_frame_new( tr("Gfx Plugin") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );
        
            vbox = gtk_vbox_new( TRUE, 5 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            hbox1 = gtk_hbox_new( FALSE, 5 );
            hbox2 = gtk_hbox_new( FALSE, 5 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );
        
            if(icontheme)
                {
                pixbuf = gtk_icon_theme_load_icon(theme, "video-display", 32,  0, NULL);
                icon = gtk_image_new_from_pixbuf(pixbuf); 
                }
            else
                { icon = gtk_image_new_from_file( get_iconpath("32x32/video-diiplay.png") ); }
        
            g_ConfigDialog.gfxCombo = gtk_combo_box_new_text();
            if( g_ConfigDialog.gfxPluginGList )
            {
                GList *element = g_list_first(g_ConfigDialog.gfxPluginGList);
                while(element)
                {
                    gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.gfxCombo), (gchar *)g_list_nth_data(element, 0));
                    element = g_list_next(element);
                }
            }
            else
            {
                gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.gfxCombo), FALSE );
            }
        
            button_config = gtk_button_new_with_label( tr("Config") );
            button_test = gtk_button_new_with_label( tr("Test") );
            button_about = gtk_button_new_with_label( tr("About") );
        
            gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.gfxCombo, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );
        
            gtk_signal_connect( GTK_OBJECT(button_config), "clicked", GTK_SIGNAL_FUNC(callback_configGfx), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_test), "clicked", GTK_SIGNAL_FUNC(callback_testGfx), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_about), "clicked", GTK_SIGNAL_FUNC(callback_aboutGfx), (gpointer) NULL );
        }
    
        // Audio Plugin Area
        {
            frame = gtk_frame_new( tr("Audio Plugin") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );
        
            vbox = gtk_vbox_new( TRUE, 5 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            hbox1 = gtk_hbox_new( FALSE, 5 );
            hbox2 = gtk_hbox_new( FALSE, 5 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );

            if(icontheme)
                {
                pixbuf = gtk_icon_theme_load_icon(theme, "audio-card", 32,  0, NULL);
                icon = gtk_image_new_from_pixbuf(pixbuf); 
                }
            else
                { icon = gtk_image_new_from_file( get_iconpath("32x32/audio-card.png") ); }
        
            g_ConfigDialog.audioCombo = gtk_combo_box_new_text();
            if( g_ConfigDialog.audioPluginGList )
            {
                GList *element = g_list_first(g_ConfigDialog.audioPluginGList);
                while(element)
                {
                    gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.audioCombo), (gchar *)g_list_nth_data(element, 0));
                    element = g_list_next(element);
                }
            }
            else
            {
                gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.audioCombo), FALSE );
            }
        
            button_config = gtk_button_new_with_label( tr("Config") );
            button_test = gtk_button_new_with_label( tr("Test") );
            button_about = gtk_button_new_with_label( tr("About") );
        
            gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.audioCombo, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );
        
            gtk_signal_connect( GTK_OBJECT(button_config), "clicked", GTK_SIGNAL_FUNC(callback_configAudio), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_test), "clicked", GTK_SIGNAL_FUNC(callback_testAudio), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_about), "clicked", GTK_SIGNAL_FUNC(callback_aboutAudio), (gpointer) NULL );
        }
    
        // Input Plugin Area
        {
            frame = gtk_frame_new( tr("Input Plugin") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );
        
            vbox = gtk_vbox_new( TRUE, 5 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            hbox1 = gtk_hbox_new( FALSE, 5 );
            hbox2 = gtk_hbox_new( FALSE, 5 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );
        
            if(icontheme)
                {
                pixbuf = gtk_icon_theme_load_icon(theme, "input-gaming", 32,  0, NULL);
                icon = gtk_image_new_from_pixbuf(pixbuf); 
                }
            else
                { icon = gtk_image_new_from_file( get_iconpath("32x32/input-gaming.png") ); }
        
            g_ConfigDialog.inputCombo = gtk_combo_box_new_text();
            if( g_ConfigDialog.inputPluginGList )
            {
                GList *element = g_list_first(g_ConfigDialog.inputPluginGList);
                while(element)
                {
                    gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.inputCombo), (gchar *)g_list_nth_data(element, 0));
                    element = g_list_next(element);
                }
            }
            else
            {
                gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.inputCombo), FALSE );
            }
        
            button_config = gtk_button_new_with_label( tr("Config") );
            button_test = gtk_button_new_with_label( tr("Test") );
            button_about = gtk_button_new_with_label( tr("About") );
        
            gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.inputCombo, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );
        
            gtk_signal_connect( GTK_OBJECT(button_config), "clicked", GTK_SIGNAL_FUNC(callback_configInput), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_test), "clicked", GTK_SIGNAL_FUNC(callback_testInput), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_about), "clicked", GTK_SIGNAL_FUNC(callback_aboutInput), (gpointer) NULL );
        }
    
        // RSP Plugin Area
        {
            frame = gtk_frame_new( tr("RSP Plugin") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configPlugins), frame, FALSE, FALSE, 0 );
        
            vbox = gtk_vbox_new( TRUE, 5 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );
            hbox1 = gtk_hbox_new( FALSE, 5 );
            hbox2 = gtk_hbox_new( FALSE, 5 );
            gtk_container_add( GTK_CONTAINER(frame), vbox );
            gtk_box_pack_start( GTK_BOX(vbox), hbox1, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(vbox), hbox2, FALSE, FALSE, 0 );
        
            icon = gtk_image_new_from_file( get_iconpath("32x32/cpu.png") );

            g_ConfigDialog.RSPCombo = gtk_combo_box_new_text();
            if( g_ConfigDialog.RSPPluginGList )
            {
                GList *element = g_list_first(g_ConfigDialog.RSPPluginGList);
                while(element)
                {
                    gtk_combo_box_append_text( GTK_COMBO_BOX(g_ConfigDialog.RSPCombo), (gchar *)g_list_nth_data(element, 0));
                    element = g_list_next(element);
                }
            }
            else
            {
                gtk_widget_set_sensitive( GTK_WIDGET(g_ConfigDialog.RSPCombo), FALSE );
            }
        
            button_config = gtk_button_new_with_label( tr("Config") );
            button_test = gtk_button_new_with_label( tr("Test") );
            button_about = gtk_button_new_with_label( tr("About") );
        
            gtk_box_pack_start( GTK_BOX(hbox1), icon, FALSE, FALSE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox1), g_ConfigDialog.RSPCombo, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_config, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_test, TRUE, TRUE, 0 );
            gtk_box_pack_start( GTK_BOX(hbox2), button_about, TRUE, TRUE, 0 );
        
            gtk_signal_connect( GTK_OBJECT(button_config), "clicked", GTK_SIGNAL_FUNC(callback_configRSP), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_test), "clicked", GTK_SIGNAL_FUNC(callback_testRSP), (gpointer) NULL );
            gtk_signal_connect( GTK_OBJECT(button_about), "clicked", GTK_SIGNAL_FUNC(callback_aboutRSP), (gpointer) NULL );
        }
    }

    // Create the rom settings page
    {
        label = gtk_label_new( tr("Rom Browser") );
        g_ConfigDialog.configRomBrowser = gtk_vbox_new( FALSE, 6 );
        gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configRomBrowser), 10 );
        gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configRomBrowser, label );
    
        // Create a frame to hold the rom directory settings.
        frame = gtk_frame_new( tr("Rom Directories") );
        gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configRomBrowser), frame, TRUE, TRUE, 0 );
    
        // Create a new horizontal box to hold our rom browser settings tab together.
        hbox1 = gtk_hbox_new( FALSE, 5 );
        gtk_container_set_border_width( GTK_CONTAINER(hbox1), 10 );
        gtk_container_add( GTK_CONTAINER(frame), hbox1 );
    
        // Create a new rom list
        g_ConfigDialog.romDirectoryList = gtk_tree_view_new();
    
        // Get the GtkTreeSelection and flip the mode to GTK_SELECTION_MULTIPLE.
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_ConfigDialog.romDirectoryList));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
        
        // Create our model and set it.
        setup_view(g_ConfigDialog.romDirectoryList);
    
        // Create a scrolled window to contain the rom list, make scrollbar visibility automatic.
        GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start( GTK_BOX(hbox1), scrolled_window, TRUE, TRUE, 0 );
    
        // Add the romDirectoryList tree view into our scrolled window.
        gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window), g_ConfigDialog.romDirectoryList);
        gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
    
        // Instruct the widget to become visible.
        gtk_widget_show(scrolled_window);
    
        // Create a new vertical button box to hold the buttons that minipulate the rom list.
        vbox = gtk_vbutton_box_new();
    
        // Set it up so the buttons appear near the top of the list.
        gtk_button_box_set_layout( GTK_BUTTON_BOX(vbox),GTK_BUTTONBOX_START);
    
        // Add the buttonbox to the list.
        gtk_box_pack_start( GTK_BOX(hbox1), vbox, FALSE, FALSE, 0 );
    
        // Create the buttons and add it to the buttonbox.
        button = gtk_button_new_from_stock(GTK_STOCK_ADD);
        gtk_box_pack_start( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
        gtk_signal_connect( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback_romDirectoryAdd), (gpointer) NULL );
    
        button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
        gtk_box_pack_start( GTK_BOX(vbox), button, FALSE, FALSE, 0 );
        gtk_signal_connect( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback_romDirectoryRemove), (gpointer) NULL );
    
        // Create the "recursive" checkbox.
        g_ConfigDialog.romDirsScanRecCheckButton = gtk_check_button_new_with_label( tr("Recursively scan rom directories") );
        gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configRomBrowser), g_ConfigDialog.romDirsScanRecCheckButton, FALSE, FALSE, 0 );

        // Create the "full filename" checkbox.
        g_ConfigDialog.romShowFullPathsCheckButton = gtk_check_button_new_with_label( tr("Show full paths in filenames") );
        gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configRomBrowser), g_ConfigDialog.romShowFullPathsCheckButton, FALSE, FALSE, 0 );
    }

    // Create Hotkey Configuration page
    {
        label = gtk_label_new( "Hotkeys" );
        g_ConfigDialog.configInputMappings = gtk_vbox_new( FALSE, 6 );
        gtk_container_set_border_width( GTK_CONTAINER(g_ConfigDialog.configInputMappings), 10 );
        gtk_notebook_append_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configInputMappings, label );
    
        // Create a frame for shortcut key config
        {
            struct input_mapping *mapping;
            GtkTooltips *tooltips;

            tooltips = gtk_tooltips_new();

            frame = gtk_frame_new( tr("Input Mappings") );
            gtk_box_pack_start( GTK_BOX(g_ConfigDialog.configInputMappings), frame, FALSE, FALSE, 0 );

            hbox1 = gtk_hbox_new(FALSE, 5);
            gtk_container_add( GTK_CONTAINER(frame), hbox1 );

            vbox = gtk_vbox_new( TRUE, 6 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );

            // create column of all mapping names
            gtk_box_pack_start( GTK_BOX(vbox), gtk_label_new(""), FALSE, FALSE, 0 );
            mapping_foreach(mapping)
            {
                gtk_box_pack_start( GTK_BOX(vbox), gtk_label_new(tr(mapping->name)), FALSE, FALSE, 0 );
            }

            gtk_box_pack_start( GTK_BOX(hbox1), vbox, FALSE, FALSE, 0 );

            vbox = gtk_vbox_new( TRUE, 6 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );

            // create column of all keyboard shortcuts
            gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(tr("Keyboard")), FALSE, FALSE, 0);
            mapping_foreach(mapping)
            {
                gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(mapping->key_mapping), FALSE, FALSE, 0);
            }

            gtk_box_pack_start( GTK_BOX(hbox1), vbox, FALSE, FALSE, 0 );

            vbox = gtk_vbox_new( TRUE, 6 );
            gtk_container_set_border_width( GTK_CONTAINER(vbox), 10 );

            // create column of joystick mappings
            gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(tr("Controller")), FALSE, FALSE, 0);
            mapping_foreach(mapping)
            {
                mapping->joy_mapping_textbox = gtk_entry_new();
                gtk_widget_set_size_request(mapping->joy_mapping_textbox, 12, -1);
                gtk_editable_set_editable(GTK_EDITABLE(mapping->joy_mapping_textbox), FALSE);
                g_signal_connect(GTK_OBJECT(mapping->joy_mapping_textbox),
                                 "button-release-event",
                                 GTK_SIGNAL_FUNC(callback_setInput),
                         mapping);

                gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips),
                                     mapping->joy_mapping_textbox,
                                     tr("Click to change"),
                             "");

                gtk_box_pack_start(GTK_BOX(vbox), mapping->joy_mapping_textbox, FALSE, FALSE, 0);
            }

            gtk_box_pack_start( GTK_BOX(hbox1), vbox, FALSE, FALSE, 0 );
        }
    }

    // Initalize the widgets.
    callback_dialogShow( NULL, NULL );

    return 0;
}
