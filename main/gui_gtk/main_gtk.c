/***************************************************************************
 main_gtk.c - Handles the main window and 'glues' it with other windows
----------------------------------------------------------------------------
Began                : Fri Nov 8 2002
Copyright            : (C) 2002 by blight
Email                : blight@Ashitaka
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "../version.h"
#include "../winlnxdefs.h"
#include "../guifuncs.h"
#include "main_gtk.h"
#include "../main.h"
#include "../config.h"
#include "../util.h"
#include "aboutdialog.h"
#include "cheatdialog.h"
#include "configdialog.h"
#include "rombrowser.h"
#include "romproperties.h"
#include "../translate.h"
#include "vcrcomp_dialog.h"

#include "../plugin.h"
#include "../rom.h"
#include "../../r4300/r4300.h"
#include "../../r4300/recomph.h"
#include "../../memory/memory.h"
#include "../savestates.h"
#include "../vcr.h"
#include "../vcr_compress.h"

#ifdef DBG
#include "../../debugger/debugger.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <SDL.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>    // POSIX Thread library
#include <signal.h> // signals
#include <sys/stat.h>

#ifdef CONFIG_PATH
#include <dirent.h>
#endif

/** function prototypes **/
static void callback_startEmulation(GtkWidget *widget, gpointer data);
static void callback_stopEmulation(GtkWidget *widget, gpointer data);
static void callback_openRom(GtkWidget *widget, gpointer data);
static int create_mainWindow(void);

/** globals **/
SMainWindow g_MainWindow;

/** status bar **/
typedef struct
{
    const char *name;
    gint        id;
    GtkWidget  *bar;
} statusBarSection;

static pthread_t g_GuiThread = 0; // main gui thread

static statusBarSection statusBarSections[] = {
    { "status", -1, NULL },
    { NULL, -1, NULL }
};

/*********************************************************************************************************
* GUI interfaces (declared in ../guifuncs.h)
*/

/** gui_init
 *    Parse gui-specific arguments and remove them from argument list.
 */
void gui_init(int *argc, char ***argv)
{
    // init multi-threading support
    g_thread_init(NULL);
    gdk_threads_init();

    // save main gui thread handle
    g_GuiThread = pthread_self();

    // call gtk to parse arguments
    gtk_init(argc, argv);
}

/** gui_build
 *    Create GUI components, but do not display
 */
void gui_build(void)
{
    // info_message function can safely be used after we get the gdk lock
    gdk_threads_enter();

    create_mainWindow();
    create_configDialog();
#ifdef VCR_SUPPORT
    create_vcrCompDialog();
#endif
    create_aboutDialog();
}

/** gui_display
 *    Display GUI components to the screen
 */
void gui_display(void)
{
    gtk_widget_show_all(g_MainWindow.window);
}

/** gui_main_loop
 *    Give control of thread to gtk
 */
void gui_main_loop(void)
{
    gtk_main();
    gdk_threads_leave();
}

//External pulic function to update rombrowser.
void updaterombrowser()
{
    pthread_t self = pthread_self();

    if(gui_enabled())
        {
        // if we're calling from a thread other than the main gtk thread, take gdk lock
        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_enter();

        rombrowser_refresh();

        GUI_PROCESS_QUEUED_EVENTS();

        if(!pthread_equal(self, g_GuiThread))
        gdk_threads_leave();

        return;
        }
}

// prints informational message to status bar
void info_message(const char *fmt, ...)
{
    va_list ap;
    char buf[2049];
    int i;
    pthread_t self = pthread_self();

    va_start(ap, fmt);
    vsnprintf(buf, 2048, fmt, ap);
    va_end(ap);

    if(gui_enabled())
    {
        // if we're calling from a thread other than the main gtk thread, take gdk lock
        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_enter();

        for(i = 0; statusBarSections[i].name; i++)
        {
            if(!strcasecmp("status", statusBarSections[i].name))
            {
                gtk_statusbar_pop(GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id);
                gtk_statusbar_push(GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id, buf);
        
                // update status bar
                GUI_PROCESS_QUEUED_EVENTS();
        
                if(!pthread_equal(self, g_GuiThread))
                    gdk_threads_leave();

                return;
            }
        }

        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_leave();
    }
    // if gui not enabled, just print to console
    else
    {
        printf(tr("Info"));
        printf(": %s\n", buf);
    }
}

// pops up dialog box with error message and ok button
void alert_message(const char *fmt, ...)
{
    va_list ap;
    char buf[2049];
    GtkWidget *dialog = NULL,
              *hbox = NULL,
              *label = NULL,
              *icon = NULL;
    pthread_t self = pthread_self();

    va_start(ap, fmt);
    vsnprintf(buf, 2048, fmt, ap);
    va_end(ap);

    if(gui_enabled())
    {
        // if we're calling from a thread other than the main gtk thread, take gdk lock
        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_enter();

        dialog = gtk_dialog_new_with_buttons(tr("Error"),
                                             GTK_WINDOW(g_MainWindow.window),
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_OK, GTK_RESPONSE_NONE,
                                             NULL);
        
        g_signal_connect_swapped(dialog, "response",
                                 G_CALLBACK(gtk_widget_destroy), dialog);
        
        hbox = gtk_hbox_new(FALSE, 5);
        
        icon = gtk_image_new_from_file(get_iconpath("messagebox-error.png"));
        gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
        
        label = gtk_label_new(buf);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_size_request(label, 165, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);
        
        gtk_widget_show_all(dialog);

        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_leave();
    }
    // if gui not enabled, just print to console
    else
    {
        printf(tr("Error"));
        printf(": %s\n", buf);
    }
}

// pops up dialog box with question and yes/no buttons
int confirm_message(const char *fmt, ...)
{
    va_list ap;
    char buf[2049];
    gint response;
    GtkWidget *dialog = NULL,
              *hbox = NULL,
              *label = NULL,
              *icon = NULL;
    pthread_t self = pthread_self();

    va_start(ap, fmt);
    vsnprintf(buf, 2048, fmt, ap);
    va_end(ap);

    if(gui_enabled())
    {
        // if we're calling from a thread other than the main gtk thread, take gdk lock
        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_enter();

        dialog = gtk_dialog_new_with_buttons(tr("Confirm"),
                                             GTK_WINDOW(g_MainWindow.window),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_YES, GTK_RESPONSE_ACCEPT,
                                             GTK_STOCK_NO, GTK_RESPONSE_REJECT,
                                             NULL);
        
        hbox = gtk_hbox_new(FALSE, 5);
        
        icon = gtk_image_new_from_file(get_iconpath("messagebox-quest.png"));
        gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
        
        label = gtk_label_new(buf);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_size_request(label, 200, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
        
        gtk_widget_show_all(hbox);
        
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);
        
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        
        gtk_widget_destroy(dialog);
        
        if(!pthread_equal(self, g_GuiThread))
            gdk_threads_leave();

        return response == GTK_RESPONSE_ACCEPT;
    }
    // if gui not enabled, get input from the console
    else
    {
        char c;
        while(1)
        {
            printf(tr("Confirm"));
            printf(": %s(y/n) ", buf);

            c = fgetc(stdin);

            if(tolower(c) == 'y') return 1;
            else if(tolower(c) == 'n') return 0;

            printf(tr("Please answer 'y' (%s) or 'n' (%s).\n"), tr("Yes"), tr("No"));
        }
    }
}

/*********************************************************************************************************
* internal gui funcs
*/
// status bar
void statusbar_message(const char *section, const char *fmt, ...)
{
    va_list ap;
    char buf[2049];
    int i;

    va_start( ap, fmt );
    vsnprintf( buf, 2048, fmt, ap );
    va_end( ap );

    for( i = 0; statusBarSections[i].name; i++ )
    {
        if( !strcasecmp( section, statusBarSections[i].name ) )
        {
            gtk_statusbar_pop( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id );
            gtk_statusbar_push( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id, buf );
            return;
        }
    }

#ifdef _DEBUG
    printf( "statusbar_message(): unknown section '%s'!\n", section );
#endif
}

/*********************************************************************************************************
* callbacks
*/
/** rom **/
// open rom
static void callback_openRom(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter;

    if( g_EmulationThread )
    {
        if(!confirm_message(tr("Emulation is running. Do you want to\nstop it and load a rom?")))
            return;
        callback_stopEmulation( NULL, NULL );
    }

    /* get rom file from user */
    file_chooser = gtk_file_chooser_dialog_new(tr("Open Rom..."),
                                               GTK_WINDOW(g_MainWindow.window),
                                               GTK_FILE_CHOOSER_ACTION_OPEN,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                               NULL);

    // add filter for rom file types
    file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, "N64 ROM (*.z64, *.v64, *.n64, *.rom, *.gz, *.zip)");
    gtk_file_filter_add_mime_type(file_filter, "application/x-gzip");
    gtk_file_filter_add_mime_type(file_filter, "application/zip");
    gtk_file_filter_add_pattern(file_filter, "*.[zZ]64");

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                file_filter);

    // add filter for "all files"
    file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, "All files (*.*)");
    gtk_file_filter_add_pattern(file_filter, "*");

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                file_filter);

    if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

        // hide dialog while rom is loading
        gtk_widget_hide(file_chooser);

        if(open_rom(filename) == 0)
        {
            char buf[300];
            snprintf(buf, 300, MUPEN_NAME " v" MUPEN_VERSION " - %s", ROM_HEADER->nom);
            gtk_window_set_title(GTK_WINDOW(g_MainWindow.window), buf);
        }

        g_free(filename);
    }

    gtk_widget_destroy (file_chooser);
}

// close rom
static void callback_closeRom(GtkWidget *widget, gpointer data)
{
    if(close_rom() == 0)
    {
        gtk_window_set_title( GTK_WINDOW(g_MainWindow.window), MUPEN_NAME " v" MUPEN_VERSION );
    }
}

// language selected
static void callback_languageSelected(GtkWidget *widget, gpointer data)
{
    const char *name;
    widget = data; // ToDo: find out why this is needed

    if( !GTK_CHECK_MENU_ITEM(widget)->active )
        return;
    name = gtk_object_get_data( GTK_OBJECT(widget), "lang name" );
    tr_set_language( name );
    config_put_string( "Language", name );

    reload();
}

// reload windows
void reload()
{
    // recreate gui
    gtk_widget_destroy( g_MainWindow.window );
    gtk_widget_destroy( g_AboutDialog.dialog );
    gtk_widget_destroy( g_ConfigDialog.dialog );
    create_mainWindow();
    create_aboutDialog();
    create_configDialog();
    gtk_widget_show_all( g_MainWindow.window );
}

/** emulation **/
// start/continue emulation
static void callback_startEmulation(GtkWidget *widget, gpointer data)
{
    if(!rom)
        {
        GList *list = NULL, *llist = NULL;
        cache_entry *entry;
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_MainWindow.romDisplay));

        list = gtk_tree_selection_get_selected_rows (selection, &model);

        if( !list ) //Nothing selected.
            { 
            if(confirm_message(tr("There is no Rom loaded. Do you want to load one?")))
                { callback_openRom(NULL, NULL); }
            return;
            }
        else
            {

            llist = g_list_first (list);
         
            gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data);
            gtk_tree_model_get(model, &iter, 5, &entry, -1);
         
            g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
            g_list_free (list);
         
            if(open_rom( entry->filename ) == 0)
                { startEmulation(); }
            else
                { return; }
            }
        }
    startEmulation();
}

// pause/continue emulation
static void callback_pauseContinueEmulation(GtkWidget *widget, gpointer data)
{
    pauseContinueEmulation();
}

// stop emulation
static void callback_stopEmulation( GtkWidget *widget, gpointer data )
{
    stopEmulation();
}

// Save State
static void callback_Save( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
        savestates_job |= SAVESTATE;
}

// Save As
//Need to add default filename here...
static void callback_SaveAs( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        GtkWidget *file_chooser;

        file_chooser = gtk_file_chooser_dialog_new(tr("Save as..."),
                                                   GTK_WINDOW(g_MainWindow.window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                   NULL);

        if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
        {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
        
            savestates_select_filename( filename );
            savestates_job |= SAVESTATE;

            g_free(filename);
        }
        
        gtk_widget_destroy (file_chooser);
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}

// Restore
static void callback_Restore( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
        savestates_job |= LOADSTATE;
}

// Load
static void callback_Load( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        GtkWidget *file_chooser;

        file_chooser = gtk_file_chooser_dialog_new(tr("Load..."),
                                                   GTK_WINDOW(g_MainWindow.window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);
        
        if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
        {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
        
            savestates_select_filename( filename );
            savestates_job |= LOADSTATE;

            g_free(filename);
        }
        
        gtk_widget_destroy (file_chooser);
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}

// user changed savestate slot
static void cb_SaveSlotSelected(GtkMenuItem *item, int slot)
{
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item)))
        {
        if(slot!=savestates_get_slot()) //Only actually change slot when not a GUI update.
            { savestates_select_slot(slot); }
        }
}

// user opened save slot menu. Make sure current save slot is selected.
static void cb_UpdateSelectedSlot(GtkMenuItem *item, GSList *slots)
{
    unsigned int i, slot;
    GtkWidget *slotItem = GTK_WIDGET(g_slist_nth_data(slots, savestates_get_slot()));

    for(i=0; i<g_slist_length(slots); i++)
        {
        GtkWidget *slotItem = GTK_WIDGET(g_slist_nth_data(slots, i));
        slot = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(slotItem), "slot_num"));

        //Make menu item represent the current selected save slot.
        if(slot==savestates_get_slot())
            {
            if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(slotItem)))
                { gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(slotItem), TRUE); }
            break;
            }
        }
}

/** configuration **/
// configure
static void callback_configure( GtkWidget *widget, gpointer data )
{
    if(g_EmulationThread)
    {
        if(!confirm_message("Cannot configure while emulator is running!\nWould you like to stop the emulator?"))
            return;

        stopEmulation();
    }

    gtk_widget_show_all( g_ConfigDialog.dialog );
}

// configure gfx
static void callback_configureVideo( GtkWidget *widget, gpointer data )
{
    char *name;

    name = plugin_name_by_filename( config_get_string( "Gfx Plugin", "" ) );
    if( name )
    {
        plugin_exec_config( name );
    }
    else
    {
        if(confirm_message(tr("No graphics plugin selected! Do you\nwant to select one?")))
        {
            gtk_widget_show_all( g_ConfigDialog.dialog );
            gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
        }
    }
}

// configure audio
static void callback_configureAudio( GtkWidget *widget, gpointer data )
{
    char *name;

    name = plugin_name_by_filename( config_get_string( "Audio Plugin", "" ) );
    if( name )
    {
        plugin_exec_config( name );
    }
    else
    {
        if(confirm_message(tr("No audio plugin selected! Do you\nwant to select one?")))
        {
            gtk_widget_show_all( g_ConfigDialog.dialog );
            gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
        }
    }
}

// configure input
static void callback_configureInput( GtkWidget *widget, gpointer data )
{
    char *name;

    name = plugin_name_by_filename( config_get_string( "Input Plugin", "" ) );
    if( name )
    {
        plugin_exec_config( name );
    }
    else
    {
        if(confirm_message(tr("No input plugin selected! Do you\nwant to select one?")))
        {
            gtk_widget_show_all( g_ConfigDialog.dialog );
            gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
        }
    }
}

// configure RSP
static void callback_configureRSP( GtkWidget *widget, gpointer data )
{
    char *name;

    name = plugin_name_by_filename( config_get_string( "RSP Plugin", "" ) );
    if( name )
    {
        plugin_exec_config( name );
    }
    else
    {
        if(confirm_message(tr("No RSP plugin selected! Do you\nwant to select one?")))
        {
            gtk_widget_show_all( g_ConfigDialog.dialog );
            gtk_notebook_set_page( GTK_NOTEBOOK(g_ConfigDialog.notebook), gtk_notebook_page_num( GTK_NOTEBOOK(g_ConfigDialog.notebook), g_ConfigDialog.configPlugins ) );
        }
    }
}

// full screen
static void callback_fullScreen( GtkWidget *widget, gpointer data )
{
    if(g_EmulationThread)
    {
        changeWindow();
    }
}


/** VCR **/
#ifdef VCR_SUPPORT
static void callback_vcrStartRecord( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        GtkWidget *file_chooser;

        // get save file from user
        file_chooser = gtk_file_chooser_dialog_new(tr("Save Recording"),
                                                   GTK_WINDOW(g_MainWindow.window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                   NULL);
        
        if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
        {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
            char full_filename[PATH_MAX];

            strncpy(full_filename, filename, PATH_MAX);

            // if user didn't provide suffix, append .rec
            if(!strstr(filename, "."))
                strncat(full_filename, ".rec", PATH_MAX - strlen(full_filename));
        
            if (VCR_startRecord( full_filename ) < 0)
                alert_message(tr("Couldn't start recording."));

            g_free(filename);
        }
        
        gtk_widget_destroy (file_chooser);
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}


static void callback_vcrStopRecord( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        if (VCR_stopRecord() < 0)
            alert_message(tr("Couldn't stop recording."));
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}

static void callback_vcrStartPlayback( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        GtkWidget *file_chooser;
        GtkFileFilter *file_filter;

        // get recording file from user to playback
        file_chooser = gtk_file_chooser_dialog_new(tr("Load Recording"),
                                                   GTK_WINDOW(g_MainWindow.window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);
        
        // add filter for recording file types
        file_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(file_filter, "Recording file (*.rec)");
        gtk_file_filter_add_pattern(file_filter, "*.[rR][eE][cC]");
        
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                    file_filter);
        
        // add filter for "all files"
        file_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(file_filter, "All files (*.*)");
        gtk_file_filter_add_pattern(file_filter, "*");
        
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                    file_filter);

        if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
        {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
        
            if (VCR_startPlayback( filename ) < 0)
                alert_message(tr("Couldn't start playback."));
        
            g_free(filename);
        }
        
        gtk_widget_destroy (file_chooser);
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}


static void callback_vcrStopPlayback( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        if (VCR_stopPlayback() < 0)
            alert_message(tr("Couldn't stop playback."));
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}


static void callback_vcrStartCapture( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        GtkWidget *file_chooser;
        GtkFileFilter *file_filter;

        // load recording file to capture
        file_chooser = gtk_file_chooser_dialog_new(tr("Load Recording"),
                                                   GTK_WINDOW(g_MainWindow.window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                   NULL);
        
        // add filter for recording file types
        file_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(file_filter, "Recording file (*.rec)");
        gtk_file_filter_add_pattern(file_filter, "*.[rR][eE][cC]");
        
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                    file_filter);
        
        // add filter for "all files"
        file_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(file_filter, "All files (*.*)");
        gtk_file_filter_add_pattern(file_filter, "*");
        
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser),
                                    file_filter);

        if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
        {
            gchar *rec_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

            gtk_widget_destroy(file_chooser);

            // get avi filename from user to save recording to.
            file_chooser = gtk_file_chooser_dialog_new(tr("Save as..."),
                                                       GTK_WINDOW(g_MainWindow.window),
                                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                       NULL);

            if(gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT)
            {
                gchar *avi_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

                if (VCR_startCapture( rec_filename, avi_filename ) < 0)
                    alert_message(tr("Couldn't start capturing."));

                g_free(avi_filename);
            }
        
            g_free(rec_filename);
        }
        
        gtk_widget_destroy (file_chooser);
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}


static void callback_vcrStopCapture( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        if (VCR_stopCapture() < 0)
            alert_message(tr("Couldn't stop capturing."));
    }
    else
    {
        alert_message(tr("Emulation is not running."));
    }
}


static void callback_vcrSetup( GtkWidget *widget, gpointer data )
{
    gtk_widget_show_all( g_VcrCompDialog.dialog );
}
#endif // VCR_SUPPORT

/** debugger **/
#ifdef DBG
// show
static void callback_debuggerEnableToggled( GtkWidget *widget, gpointer data )
{
    if( g_EmulationThread )
    {
        if(confirm_message(tr("Emulation needs to be restarted in order\nto activate the debugger. Do you want\nthis to happen?")))
        {
            callback_stopEmulation( NULL, NULL );
            g_DebuggerEnabled = GTK_CHECK_MENU_ITEM(widget)->active;
            callback_startEmulation( NULL, NULL );
        }
        return;
    }

    g_DebuggerEnabled = gtk_check_menu_item_get_active(widget);
}
#endif // DBG

/** help **/
// about
static void callback_aboutMupen( GtkWidget *widget, gpointer data )
{
    gtk_widget_show_all(g_AboutDialog.dialog);
}

/** misc **/
// hide on delete
static gint callback_mainWindowDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    //Put new rombrowser config saving code here...
    gtk_main_quit();

    return TRUE; // undeleteable
}

/*********************************************************************************************************
* gui creation
*/
// static widgets to change their state from emulation thread
static GtkWidget       *slotItem;

// menuBar
static int create_menuBar( void )
{
    GtkWidget   *menuItem;
    GtkWidget   *fileMenu;
    GtkWidget   *fileMenuItem;
    GtkWidget   *emulationMenu;
    GtkWidget   *emulationMenuItem;
    GtkWidget   *optionsMenu;
    GtkWidget   *optionsMenuItem;
#ifdef VCR_SUPPORT
    GtkWidget   *vcrMenu;
    GtkWidget   *vcrMenuItem;
#endif
#ifdef DBG
    GtkWidget   *debuggerMenu;
    GtkWidget   *debuggerMenuItem;
#endif
    GtkWidget   *helpMenu;
    GtkWidget   *helpMenuItem;

    GtkWidget   *fileLoadRomItem;
    GtkWidget   *fileCloseRomItem;
    GtkWidget   *fileSeparator1;
    GtkWidget   *fileLanguageItem;
    GtkWidget   *fileLanguageMenu;
    GtkWidget   *fileSeparator2;
    GtkWidget   *fileExitItem;

    GtkWidget   *emulationStartItem;
    GtkWidget   *emulationPauseContinueItem;
    GtkWidget   *emulationStopItem;
    GtkWidget   *emulationSeparator1;
    GtkWidget   *emulationSaveItem;
    GtkWidget   *emulationSaveAsItem;
    GtkWidget   *emulationRestoreItem;
    GtkWidget   *emulationLoadItem;
    GtkWidget   *emulationSeparator2;
    GtkWidget   *emulationSlotItem;
    GtkWidget   *emulationSlotMenu;

    GtkWidget   *optionsConfigureItem;
    GtkWidget   *optionsSeparator1;
    GtkWidget   *optionsVideoItem;
    GtkWidget   *optionsAudioItem;
    GtkWidget   *optionsInputItem;
    GtkWidget   *optionsRSPItem;
    GtkWidget   *optionsSeparator2;
    GtkWidget   *optionsCheatsItem;
    GtkWidget   *optionsFullScreenItem;

#ifdef VCR_SUPPORT
    GtkWidget   *vcrStartRecordItem;
    GtkWidget   *vcrStopRecordItem;
    GtkWidget   *vcrSeparator1;
    GtkWidget   *vcrStartPlaybackItem;
    GtkWidget   *vcrStopPlaybackItem;
    GtkWidget   *vcrSeparator2;
    GtkWidget   *vcrStartCaptureItem;
    GtkWidget   *vcrStopCaptureItem;
    GtkWidget   *vcrSeparator3;
    GtkWidget   *vcrSetupItem;
#endif

#ifdef DBG
    GtkWidget   *debuggerEnableItem;
#endif

    GtkWidget   *helpAboutItem;

    GSList *group = NULL;
    list_t langList;
    list_node_t *node;
    char *language;
    const char *confLang = config_get_string( "Language", "English" );
    char buffer[1000];
    int i, lang_found;

    // accelerator group
    g_MainWindow.accelGroup = gtk_accel_group_new();
    g_MainWindow.accelUnsafe = gtk_accel_group_new();
    g_MainWindow.accelUnsafeActive = TRUE;
    gtk_window_add_accel_group(GTK_WINDOW(g_MainWindow.window), g_MainWindow.accelGroup);
    gtk_window_add_accel_group(GTK_WINDOW(g_MainWindow.window), g_MainWindow.accelUnsafe);

    // menubar
    g_MainWindow.menuBar = gtk_menu_bar_new();
    gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.menuBar, FALSE, FALSE, 0 );

    // file menu
    fileMenu = gtk_menu_new();
    fileMenuItem = gtk_menu_item_new_with_mnemonic(tr("_File"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(fileMenuItem), fileMenu );
    fileLoadRomItem = gtk_menu_item_new_with_mnemonic(tr("_Open Rom..."));
    gtk_widget_add_accelerator(fileLoadRomItem, "activate", g_MainWindow.accelGroup,
                               GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    fileCloseRomItem = gtk_menu_item_new_with_mnemonic(tr("_Close Rom"));
    fileSeparator1 = gtk_menu_item_new();
    fileLanguageItem = gtk_menu_item_new_with_mnemonic(tr("_Language"));
    fileSeparator2 = gtk_menu_item_new();
    fileExitItem = gtk_menu_item_new_with_mnemonic(tr("_Exit"));
    gtk_widget_add_accelerator(fileExitItem, "activate", g_MainWindow.accelGroup,
                               GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_menu_append( GTK_MENU(fileMenu), fileLoadRomItem );
    gtk_menu_append( GTK_MENU(fileMenu), fileCloseRomItem );
    gtk_menu_append( GTK_MENU(fileMenu), fileSeparator1 );
    gtk_menu_append( GTK_MENU(fileMenu), fileLanguageItem );
    gtk_menu_append( GTK_MENU(fileMenu), fileSeparator2 );
    gtk_menu_append( GTK_MENU(fileMenu), fileExitItem );

    gtk_signal_connect_object( GTK_OBJECT(fileLoadRomItem), "activate", GTK_SIGNAL_FUNC(callback_openRom), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(fileCloseRomItem), "activate", GTK_SIGNAL_FUNC(callback_closeRom), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(fileExitItem), "activate", GTK_SIGNAL_FUNC(callback_mainWindowDeleteEvent), (gpointer)NULL );

    // language menu
    fileLanguageMenu = gtk_menu_new();
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(fileLanguageItem), fileLanguageMenu );

    langList = tr_language_list();

    // first check if language specified in config file is in langList
    lang_found = 0;
    list_foreach(langList, node)
    {
        language = (char *)node->data;
        if(!strcasecmp(language, confLang))
            lang_found = 1;
    }

    // if config file language is not supported, default to English
    if(!lang_found)
    {
        printf("Warning: Language \"%s\" unsupported, or maybe the language name has changed.\n", confLang);
        config_put_string("Language", "English");
        confLang = config_get_string("Language", "English");
    }

    // create menu item for each language
    list_foreach(langList, node)
    {
        language = (char *)node->data;

        fileLanguageItem = gtk_radio_menu_item_new_with_label( group, language );
        gtk_object_set_data( GTK_OBJECT(fileLanguageItem), "lang name", language );
        group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(fileLanguageItem) );
        if(!strcasecmp(language, confLang))
        {
            gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(fileLanguageItem), 1 );
            tr_set_language(language);
        }

        gtk_signal_connect_object(GTK_OBJECT(fileLanguageItem), "toggled",
                                  GTK_SIGNAL_FUNC(callback_languageSelected), NULL);
        gtk_menu_append(GTK_MENU(fileLanguageMenu), fileLanguageItem );
    }
    // free language name list
    list_delete(&langList);

    // emulation menu
    emulationMenu = gtk_menu_new();
    emulationMenuItem = gtk_menu_item_new_with_mnemonic(tr("_Emulation"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationMenuItem), emulationMenu );
    emulationStartItem = gtk_menu_item_new_with_mnemonic(tr("_Start"));
    emulationPauseContinueItem = gtk_menu_item_new_with_mnemonic(tr("_Pause"));
    gtk_widget_add_accelerator(emulationPauseContinueItem, "activate", g_MainWindow.accelGroup,
                               GDK_Pause, 0, GTK_ACCEL_VISIBLE);
    emulationStopItem = gtk_menu_item_new_with_mnemonic(tr("S_top"));
    gtk_widget_add_accelerator(emulationStopItem, "activate", g_MainWindow.accelGroup,
                               GDK_Escape, 0, GTK_ACCEL_VISIBLE);
    emulationSeparator1 = gtk_menu_item_new();
    emulationSaveItem = gtk_menu_item_new_with_mnemonic(tr("Save State"));
    gtk_widget_add_accelerator(emulationSaveItem, "activate", g_MainWindow.accelGroup,
                               GDK_F5, 0, GTK_ACCEL_VISIBLE);
    emulationSaveAsItem = gtk_menu_item_new_with_mnemonic(tr("Save State As"));
    emulationRestoreItem = gtk_menu_item_new_with_mnemonic(tr("Restore State"));
    gtk_widget_add_accelerator(emulationRestoreItem, "activate", g_MainWindow.accelGroup,
                               GDK_F7, 0, GTK_ACCEL_VISIBLE);
    emulationLoadItem = gtk_menu_item_new_with_mnemonic(tr("Load State"));
    emulationSeparator2 = gtk_menu_item_new();
    emulationSlotItem = gtk_menu_item_new_with_mnemonic(tr("_Current save slot"));

    gtk_menu_append( GTK_MENU(emulationMenu), emulationStartItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationPauseContinueItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationStopItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationSeparator1 );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationSaveItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationSaveAsItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationRestoreItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationLoadItem );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationSeparator2 );
    gtk_menu_append( GTK_MENU(emulationMenu), emulationSlotItem);

    g_signal_connect(emulationStartItem, "activate", G_CALLBACK(callback_startEmulation), NULL );
    g_signal_connect(emulationPauseContinueItem, "activate", G_CALLBACK(callback_pauseContinueEmulation), NULL );
    g_signal_connect(emulationStopItem, "activate", G_CALLBACK(callback_stopEmulation), NULL );

    g_signal_connect(emulationSaveItem, "activate", G_CALLBACK(callback_Save), NULL );
    g_signal_connect(emulationSaveAsItem, "activate", G_CALLBACK(callback_SaveAs), NULL );
    g_signal_connect(emulationRestoreItem, "activate", G_CALLBACK(callback_Restore), NULL );
    g_signal_connect(emulationLoadItem, "activate", G_CALLBACK(callback_Load), NULL );

    // slot menu
    emulationSlotMenu = gtk_menu_new();

    gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationSlotItem), emulationSlotMenu );
    slotItem = gtk_radio_menu_item_new_with_mnemonic(NULL, buffer);
    for (i = 0; i < 10; ++i)
        {
        snprintf(buffer, 999, tr(" Slot _%d"), i);

        slotItem = gtk_radio_menu_item_new_with_mnemonic_from_widget(GTK_RADIO_MENU_ITEM(slotItem), buffer); 

        g_object_set_data(G_OBJECT(slotItem), "slot_num", GUINT_TO_POINTER(i));
        gtk_menu_append(GTK_MENU(emulationSlotMenu), slotItem);
        gtk_widget_add_accelerator(slotItem, "activate", g_MainWindow.accelUnsafe, GDK_0+i, 0, GTK_ACCEL_VISIBLE);
        g_signal_connect(slotItem, "activate", G_CALLBACK(cb_SaveSlotSelected), GUINT_TO_POINTER(i));
        }

    // set callback so selected save state slot is updated in the menu everytime it's opened
    g_signal_connect(emulationSlotItem, "activate", G_CALLBACK(cb_UpdateSelectedSlot),
                     gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(slotItem)));

    // options menu
    optionsMenu = gtk_menu_new();
    optionsMenuItem = gtk_menu_item_new_with_mnemonic(tr("_Options"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(optionsMenuItem), optionsMenu );
    optionsConfigureItem = gtk_menu_item_new_with_mnemonic(tr("_Configure..."));
    optionsSeparator1 = gtk_menu_item_new();
    optionsVideoItem = gtk_menu_item_new_with_mnemonic(tr("_Video Settings..."));
    optionsAudioItem = gtk_menu_item_new_with_mnemonic(tr("_Audio Settings..."));
    optionsInputItem = gtk_menu_item_new_with_mnemonic(tr("_Input Settings..."));
    optionsRSPItem = gtk_menu_item_new_with_mnemonic(tr("_RSP Settings..."));
    optionsSeparator2 = gtk_menu_item_new();
    optionsCheatsItem = gtk_menu_item_new_with_mnemonic(tr("C_heats..."));
    optionsFullScreenItem = gtk_menu_item_new_with_mnemonic(tr("_Full Screen"));
    gtk_widget_add_accelerator(optionsFullScreenItem, "activate", g_MainWindow.accelGroup,
                               GDK_Return, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_menu_append( GTK_MENU(optionsMenu), optionsConfigureItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator1 );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsVideoItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsAudioItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsInputItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsRSPItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator2 );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsCheatsItem );
    gtk_menu_append( GTK_MENU(optionsMenu), optionsFullScreenItem );

    g_signal_connect(optionsConfigureItem, "activate", G_CALLBACK(callback_configure), NULL );
    g_signal_connect(optionsVideoItem, "activate", G_CALLBACK(callback_configureVideo), NULL );
    g_signal_connect(optionsAudioItem, "activate", G_CALLBACK(callback_configureAudio), NULL );
    g_signal_connect(optionsInputItem, "activate", G_CALLBACK(callback_configureInput), NULL );
    g_signal_connect(optionsRSPItem, "activate", G_CALLBACK(callback_configureRSP), NULL );
    g_signal_connect(optionsCheatsItem, "activate", G_CALLBACK(cb_cheatDialog), NULL );
    g_signal_connect(optionsFullScreenItem, "activate", G_CALLBACK(callback_fullScreen), NULL );

    // vcr menu
#ifdef VCR_SUPPORT
    vcrMenu = gtk_menu_new();
    vcrMenuItem = gtk_menu_item_new_with_mnemonic(tr("_VCR"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(vcrMenuItem), vcrMenu );
    vcrStartRecordItem = gtk_menu_item_new_with_mnemonic(tr("Start _Record..."));
    vcrStopRecordItem = gtk_menu_item_new_with_mnemonic(tr("Stop Record"));
    vcrSeparator1 = gtk_menu_item_new();
    vcrStartPlaybackItem = gtk_menu_item_new_with_mnemonic(tr("Start _Playback..."));
    vcrStopPlaybackItem = gtk_menu_item_new_with_mnemonic(tr("Stop Playback"));
    vcrSeparator2 = gtk_menu_item_new();
    vcrStartCaptureItem = gtk_menu_item_new_with_mnemonic(tr("Start _Capture..."));
    vcrStopCaptureItem = gtk_menu_item_new_with_mnemonic(tr("Stop Capture"));
    vcrSeparator3 = gtk_menu_item_new();
    vcrSetupItem = gtk_menu_item_new_with_mnemonic(tr("Configure Codec..."));

    gtk_menu_append( GTK_MENU(vcrMenu), vcrStartRecordItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrStopRecordItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator1 );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrStartPlaybackItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrStopPlaybackItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator2 );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrStartCaptureItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrStopCaptureItem );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrSeparator3 );
    gtk_menu_append( GTK_MENU(vcrMenu), vcrSetupItem );

    gtk_signal_connect_object( GTK_OBJECT(vcrStartRecordItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStartRecord), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrStopRecordItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStopRecord), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrStartPlaybackItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStartPlayback), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrStopPlaybackItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStopPlayback), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrStartCaptureItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStartCapture), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrStopCaptureItem), "activate", GTK_SIGNAL_FUNC(callback_vcrStopCapture), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(vcrSetupItem), "activate", GTK_SIGNAL_FUNC(callback_vcrSetup), (gpointer)NULL );
#endif // VCR_SUPPORT

    // debugger menu
#ifdef DBG
    debuggerMenu = gtk_menu_new();
    debuggerMenuItem = gtk_menu_item_new_with_mnemonic(tr("_Debugger"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(debuggerMenuItem), debuggerMenu );
    debuggerEnableItem = gtk_check_menu_item_new_with_mnemonic(tr("_Enable"));
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerEnableItem );

    gtk_signal_connect( GTK_OBJECT(debuggerEnableItem), "toggled", GTK_SIGNAL_FUNC(callback_debuggerEnableToggled), (gpointer)NULL );
#endif // DBG

    // help menu
    helpMenu = gtk_menu_new();
    helpMenuItem = gtk_menu_item_new_with_mnemonic(tr("_Help"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(helpMenuItem), helpMenu );
    helpAboutItem = gtk_menu_item_new_with_mnemonic(tr("_About..."));
    gtk_menu_append( GTK_MENU(helpMenu), helpAboutItem );

    gtk_signal_connect_object( GTK_OBJECT(helpAboutItem), "activate", GTK_SIGNAL_FUNC(callback_aboutMupen), (gpointer)NULL );

    // add menus to menubar
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), fileMenuItem );
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), emulationMenuItem );
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), optionsMenuItem );
#ifdef VCR_SUPPORT
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), vcrMenuItem );
#endif
#ifdef DBG
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), debuggerMenuItem );
#endif
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), helpMenuItem );

    return 0;
}

// toolbar
static int create_toolBar( void )
{
    GtkWidget   *openImage = NULL;
    GtkWidget   *playImage = NULL;
    GtkWidget   *pauseImage = NULL;
    GtkWidget   *stopImage = NULL;
    GtkWidget   *fullscreenImage = NULL;
    GtkWidget   *configureImage = NULL;

    g_MainWindow.toolBar = gtk_toolbar_new();
    gtk_toolbar_set_orientation( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ORIENTATION_HORIZONTAL );
    switch(config_get_number( "ToolbarStyle", 0 ))
    {
        case 0:
            gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS );
        break;
        case 1:
            gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_TEXT );
        break;
        case 2:
            gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_BOTH );
        break;
        default:
            gtk_toolbar_set_style( GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS );
    }
    gtk_toolbar_set_tooltips( GTK_TOOLBAR(g_MainWindow.toolBar), TRUE );
    gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.toolBar, FALSE, FALSE, 0 );

    // load icons from memory
    if(config_get_number( "ToolbarSize", 1 ) == 1)
    {
        openImage = gtk_image_new_from_file( get_iconpath("open.png") );
        playImage = gtk_image_new_from_file( get_iconpath("play.png") );
        pauseImage = gtk_image_new_from_file( get_iconpath("pause.png") );
        stopImage = gtk_image_new_from_file( get_iconpath("stop.png") );
        fullscreenImage = gtk_image_new_from_file( get_iconpath("fullscreen.png") );
        configureImage = gtk_image_new_from_file( get_iconpath("configure.png") );
    }
    else
    {
        openImage = gtk_image_new_from_file( get_iconpath("open-small.png") );
        playImage = gtk_image_new_from_file( get_iconpath("play-small.png") );
        pauseImage = gtk_image_new_from_file( get_iconpath("pause-small.png") );
        stopImage = gtk_image_new_from_file( get_iconpath("stop-small.png") );
        fullscreenImage = gtk_image_new_from_file( get_iconpath("fullscreen-small.png") );
        configureImage = gtk_image_new_from_file( get_iconpath("configure-small.png") );
    }

    // add icons to toolbar
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Open"),tr("Open Rom"),"",openImage,GTK_SIGNAL_FUNC(callback_openRom),NULL);
    gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
    gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Start"),tr("Start Emulation"),"",playImage,GTK_SIGNAL_FUNC(callback_startEmulation),NULL);
    gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Pause"),tr("Pause/ Continue Emulation"),"",pauseImage,GTK_SIGNAL_FUNC(callback_pauseContinueEmulation),NULL );
    gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Stop"),tr("Stop Emulation"),"",stopImage,GTK_SIGNAL_FUNC(callback_stopEmulation),NULL );
    gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
    gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Fullscreen"),tr("Fullscreen"),"",fullscreenImage,GTK_SIGNAL_FUNC(callback_fullScreen),NULL );
    gtk_toolbar_append_space( GTK_TOOLBAR(g_MainWindow.toolBar) );
    gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Configure"),tr("Configure"),"",configureImage,GTK_SIGNAL_FUNC(callback_configure),NULL );

    return 0;
}

// status bar
static int create_statusBar( void )
{
    int i;

    //create status bar
    g_MainWindow.statusBarHBox = gtk_hbox_new( FALSE, 5 );
    gtk_box_pack_end( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.statusBarHBox, FALSE, FALSE, 0 );

    // request context ids
    for( i = 0; statusBarSections[i].name; ++i )
        {
        statusBarSections[i].bar = gtk_statusbar_new();
        gtk_box_pack_start( GTK_BOX(g_MainWindow.statusBarHBox), statusBarSections[i].bar, (i == 0) ? TRUE : FALSE, TRUE, 0 );
        statusBarSections[i].id = gtk_statusbar_get_context_id( GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].name );
        }

    return 0;
}

// main window
static int create_mainWindow( void )
{
    int width, height;

    width = config_get_number( "MainWindow Width", 600 );
    height = config_get_number( "MainWindow Height", 400 );

    // window
    g_MainWindow.window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(g_MainWindow.window), MUPEN_NAME " v" MUPEN_VERSION );
    gtk_window_set_default_size( GTK_WINDOW(g_MainWindow.window), width, height  );
    gtk_signal_connect(GTK_OBJECT(g_MainWindow.window), "delete_event", GTK_SIGNAL_FUNC(callback_mainWindowDeleteEvent), (gpointer)NULL);

    // toplevel vbox
    g_MainWindow.toplevelVBox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(g_MainWindow.window), g_MainWindow.toplevelVBox );

    // menu
    create_menuBar();

    // toolbar
    create_toolBar();

    // Filter
    create_filter();

    // rombrowser
    create_romBrowser();

    // rom properties
    create_romPropDialog();

    // statusbar
    create_statusBar();

    // fill rom browser
    rombrowser_refresh();
    return 0;
}
