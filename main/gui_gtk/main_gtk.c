/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main_gtk.c                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
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

/* main_gtk.c - Handles the main window and 'glues' it with other windows */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>

#include <SDL_thread.h>
//#include <pthread.h>    // POSIX Thread library

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "main_gtk.h"
#include "aboutdialog.h"
#include "cheatdialog.h"
#include "configdialog.h"
#include "rombrowser.h"
#include "romproperties.h"

#include "../version.h"
#include "../main.h"
#include "../config.h"
#include "../util.h"
#include "../translate.h"
#include "../savestates.h"
#include "../plugin.h"
#include "../rom.h"

#ifdef DBG
#include "debugger/registers.h"     //temporary includes for the debugger menu
#include "debugger/breakpoints.h"   //these can be removed when the main gui
#include "debugger/regTLB.h"        //window no longer needs to know if each
#include "debugger/memedit.h"       //debugger window is open
#include "debugger/varlist.h"

#include "../../debugger/debugger.h"
#endif

/** function prototypes **/
static void callback_startEmulation(GtkWidget *widget, gpointer data);
static void callback_stopEmulation(GtkWidget *widget, gpointer data);
static void callback_openRom(GtkWidget *widget, gpointer data);
static int create_mainWindow(void);

/** globals **/
SMainWindow g_MainWindow;
Uint32 g_GuiThread; /* Main gui thread. */

static gboolean check_icon_theme()
{
    if(gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "document-open")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-start")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-pause")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-stop")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "view-fullscreen")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "preferences-system")&& 
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "video-display")&& 
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "audio-card")&& 
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "input-gaming")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-warning")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-error")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-question"))
        return TRUE;
    else
        return FALSE;
}

static void callback_theme_changed(GtkWidget *widget, gpointer data)
{
    printf("Theme changed\n");

    g_MainWindow.fallbackIcons = check_icon_theme();
    short size = config_get_number("ToolbarSize", 22);

    set_icon(g_MainWindow.openImage, "document-open", size, FALSE);
    set_icon(g_MainWindow.playImage, "media-playback-start", size, FALSE);
    set_icon(g_MainWindow.pauseImage, "media-playback-pause", size, FALSE);
    set_icon(g_MainWindow.stopImage, "media-playback-stop", size, FALSE);
    set_icon(g_MainWindow.fullscreenImage, "view-fullscreen", size, FALSE);
    set_icon(g_MainWindow.configureImage, "preferences-system", size, FALSE);

    set_icon(g_ConfigDialog.graphicsImage, "video-display", 32, FALSE);
    set_icon(g_ConfigDialog.audioImage, "audio-card", 32, FALSE);
    set_icon(g_ConfigDialog.inputImage, "input-gaming", 32, FALSE);
}

/*********************************************************************************************************
* GUI interfaces
*/

/** gui_init
 *    Parse gui-specific arguments, remove them from argument list, 
 *    and create gtk gui in thread-safe manner, but do not display.
 */
void gui_init(int *argc, char ***argv)
{
    // init multi-threading support
    g_thread_init(NULL);
    gdk_threads_init();

    // save main gui thread handle
    g_GuiThread = SDL_ThreadID();

    /* Call gtk to parse arguments. */
    gtk_init(argc, argv);

    /* Setup gtk theme. */
    g_MainWindow.iconTheme = gtk_icon_theme_get_default();
    g_MainWindow.fallbackIcons = check_icon_theme();
    g_signal_connect(G_OBJECT(g_MainWindow.iconTheme), "changed", G_CALLBACK(callback_theme_changed), NULL);

    create_mainWindow();
    create_configDialog();
    create_romPropDialog();
    create_aboutDialog();
}

/* Display GUI components to the screen. */
void gui_display(void)
{
    gtk_widget_show_all(g_MainWindow.toplevelVBox);
    if(!(config_get_bool("ToolbarVisible",TRUE)))
        gtk_widget_hide(g_MainWindow.toolBar);
    if(!(config_get_bool("FilterVisible",TRUE)))
        gtk_widget_hide(g_MainWindow.filterBar);
    if(!(config_get_bool("StatusbarVisible",TRUE)))
        gtk_widget_hide(g_MainWindow.statusBarHBox);
    gtk_widget_show(g_MainWindow.window);
}

/* Save GUI properties and destroy widgets. */
void gui_destroy(void)
{
    gint width, height, xposition, yposition;

    gtk_window_get_size(GTK_WINDOW(g_MainWindow.window), &width, &height);
    gtk_window_get_position(GTK_WINDOW(g_MainWindow.window), &xposition, &yposition); 

    config_put_number("MainWindowWidth",width);
    config_put_number("MainWindowHeight",height);
    config_put_number("MainWindowXPosition",xposition);
    config_put_number("MainWindowYPosition",yposition);

    gtk_widget_destroy(g_MainWindow.window);
    gtk_widget_destroy(g_AboutDialog.dialog);
    gtk_widget_destroy(g_ConfigDialog.dialog);
    gtk_widget_destroy(g_RomPropDialog.dialog);
}

/* gui_main_loop
 *   give control of thread to gtk
 */
void gui_main_loop(void)
{
    gtk_main();
    gui_destroy();
    gdk_threads_leave();
}

/* updaterombrowser() accesses g_romcahce.length and adds upto roms to the rombrowser. The clear
 * flag tells the GUI to clear the rombrowser.
 */
void updaterombrowser( unsigned int roms, unsigned short clear )
{
    Uint32 self = SDL_ThreadID();

    //if we're calling from a thread other than the main gtk thread, take gdk lock
    if(!(self==g_GuiThread))
        gdk_threads_enter();

    rombrowser_refresh(roms, clear);

    gdk_threads_leave();
    while(g_main_context_iteration(NULL, FALSE));
    gdk_threads_enter();

    if(!(self==g_GuiThread))
        gdk_threads_leave();

    return;
}

/* gui_message() uses messagetype to display either an informational message, for example to the 
 * status bar, a yes / no confirmation dialogue, or an error dialogue.
 */
int gui_message(unsigned char messagetype, const char *format, ...)
{
    if(!gui_enabled())
        return 0;

    va_list ap;
    char buffer[2049];
    Uint32 self = SDL_ThreadID();
    gint response = 0;

    va_start(ap, format);
    vsnprintf(buffer, 2048, format, ap);
    buffer[2048] = '\0';
    va_end(ap);

    //if we're calling from a thread other than the main gtk thread, take gdk lock
    if(!(self==g_GuiThread))
        gdk_threads_enter();

     if(messagetype==0)
        {
        int counter;
        for( counter = 0; counter < strlen(buffer); ++counter )
            {
            if(buffer[counter]=='\n')
                {
                buffer[counter]='\0';
                break;
                }
            }

        gtk_statusbar_pop(GTK_STATUSBAR(g_MainWindow.statusBar), gtk_statusbar_get_context_id( GTK_STATUSBAR(g_MainWindow.statusBar), "status"));
        gtk_statusbar_push(GTK_STATUSBAR(g_MainWindow.statusBar), gtk_statusbar_get_context_id( GTK_STATUSBAR(g_MainWindow.statusBar), "status"), buffer);
        }
    else if(messagetype>0)
        {
        GtkWidget *dialog, *hbox, *label, *icon;

        hbox = gtk_hbox_new(FALSE, 10);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

        GtkIconTheme *theme = gtk_icon_theme_get_default();
        GdkPixbuf *pixbuf;

        if(messagetype==1)
            {
            dialog = gtk_dialog_new_with_buttons(tr("Error"), GTK_WINDOW(g_MainWindow.window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_NONE,NULL);

            if(check_icon_theme())
                {
                pixbuf = gtk_icon_theme_load_icon(theme, "dialog-error", 32,  0, NULL);
                icon = gtk_image_new_from_pixbuf(pixbuf); 
                }
            else
                icon = gtk_image_new_from_file(get_iconpath("32x32/dialog-error.png"));
            }
        else if(messagetype==2)
            {
            dialog = gtk_dialog_new_with_buttons(tr("Confirm"), GTK_WINDOW(g_MainWindow.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_YES, GTK_RESPONSE_ACCEPT, GTK_STOCK_NO, GTK_RESPONSE_REJECT, NULL);

            if(check_icon_theme())
                {
                pixbuf = gtk_icon_theme_load_icon(theme, "dialog-question", 32,  0, NULL);
                icon = gtk_image_new_from_pixbuf(pixbuf); 
                }
            else
                icon = gtk_image_new_from_file(get_iconpath("32x32/dialog-question.png"));
            }

        gtk_misc_set_alignment(GTK_MISC(icon), 0, 0);
        gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);

        label = gtk_label_new(buffer);
        gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);

        gtk_widget_show_all(dialog);

        if(messagetype==1)
            {
            g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
            }
        else if(messagetype==2)
            {
            response = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            }
        }

    gdk_threads_leave();
    while(g_main_context_iteration(NULL, FALSE));
    gdk_threads_enter();

   if(!(self==g_GuiThread))
       gdk_threads_leave();

       return response == GTK_RESPONSE_ACCEPT;
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

    if(g_EmulationThread)
    {
        if(!gui_message(2, tr("Emulation is running. Do you want to\nstop it and load a rom?")))
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
    gtk_file_filter_set_name(file_filter, "N64 ROM (*.z64, *.v64, *.n64, *.gz, *.zip. *.bz2, *.lzma *.7z)");
    gtk_file_filter_add_mime_type(file_filter, "application/x-gzip");
    gtk_file_filter_add_mime_type(file_filter, "application/zip");
    gtk_file_filter_add_mime_type(file_filter, "application/x-bzip2");
    gtk_file_filter_add_mime_type(file_filter, "application/x-7z");
    gtk_file_filter_add_pattern(file_filter, "*.[zZ]64");
    gtk_file_filter_add_pattern(file_filter, "*.lzma");
    gtk_file_filter_add_pattern(file_filter, "*.7z");

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

        open_rom(filename, 0);

        g_free(filename);
    }

    gtk_widget_destroy (file_chooser);
}

// close rom
static void callback_closeRom(GtkWidget *widget, gpointer data)
{
    close_rom();
}

// reload windows
void reload()
{
    gui_destroy();
    create_mainWindow();
    create_aboutDialog();
    create_configDialog();
    rombrowser_refresh(g_romcache.length, FALSE);
    gui_display();
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

        if(!list) //Nothing selected.
            { 
            if(gui_message(2, tr("There is no Rom loaded. Do you want to load one?")))
                callback_openRom(NULL, NULL);
            return;
            }
        else
            {
            llist = g_list_first(list);

            gtk_tree_model_get_iter(model, &iter,(GtkTreePath *) llist->data);
            gtk_tree_model_get(model, &iter, 22, &entry, -1);

            g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
            g_list_free(list);

            if(open_rom(entry->filename, entry->archivefile)==0)
                startEmulation();
            else
                return;
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

//Save As
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
        error_message(tr("Emulation is not running."));
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
        error_message(tr("Emulation is not running."));
    }
}

// user changed savestate slot
static void cb_SaveSlotSelected(GtkMenuItem *item, int slot)
{
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item)))
        {
        if(slot!=savestates_get_slot()) //Only actually change slot when not a GUI update.
            savestates_select_slot(slot);
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
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(slotItem), TRUE);
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
        if(!gui_message(2, "Cannot configure while emulator is running!\nWould you like to stop the emulator?"))
            return;

        stopEmulation();
        }

    gtk_window_set_focus(GTK_WINDOW(g_ConfigDialog.dialog), g_ConfigDialog.okButton);
    gtk_widget_show_all(g_ConfigDialog.dialog);
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
        if(gui_message(2, tr("No graphics plugin selected! Do you\nwant to select one?")))
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
        if(gui_message(2, tr("No audio plugin selected! Do you\nwant to select one?")))
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
        if(gui_message(2, tr("No input plugin selected! Do you\nwant to select one?")))
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
        if(gui_message(2, tr("No RSP plugin selected! Do you\nwant to select one?")))
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

static void callback_toggle_toolbar( GtkWidget *widget, gpointer data )
{
    if(GTK_WIDGET_VISIBLE(g_MainWindow.toolBar))
        gtk_widget_hide(g_MainWindow.toolBar);
    else
        gtk_widget_show(g_MainWindow.toolBar);

    config_put_bool("ToolbarVisible",GTK_WIDGET_VISIBLE(g_MainWindow.toolBar));
}

static void callback_toggle_filter( GtkWidget *widget, gpointer data )
{
    if(GTK_WIDGET_VISIBLE(g_MainWindow.filterBar))
        {
        gtk_entry_set_text(GTK_ENTRY(g_MainWindow.filter),"");
        gtk_widget_hide(g_MainWindow.filterBar); 
        }
    else
        gtk_widget_show(g_MainWindow.filterBar);

    config_put_bool("FilterVisible",GTK_WIDGET_VISIBLE(g_MainWindow.filterBar));
}

static void callback_toggle_statusbar( GtkWidget *widget, gpointer data )
{
    if(GTK_WIDGET_VISIBLE(g_MainWindow.statusBarHBox))
        gtk_widget_hide(g_MainWindow.statusBarHBox);
    else
        gtk_widget_show(g_MainWindow.statusBarHBox);

    config_put_bool("StatusbarVisible",GTK_WIDGET_VISIBLE(g_MainWindow.statusBarHBox));
}

/** debugger **/
#ifdef DBG
static GtkWidget   *debuggerRegistersShow;
static GtkWidget   *debuggerBreakpointsShow;
static GtkWidget   *debuggerTLBShow;
static GtkWidget   *debuggerMemoryShow;
static GtkWidget   *debuggerVariablesShow;

// show
static void callback_debuggerEnableToggled( GtkWidget *widget, gpointer data )
{
    int emuRestart=0;

    if( g_EmulationThread )
    {
        if(gui_message(2, tr("Emulation needs to be restarted in order\nto activate the debugger. Do you want\nthis to happen?")))
        {
            callback_stopEmulation( NULL, NULL );
        emuRestart=1;
        }
    }

  g_DebuggerEnabled = gtk_check_menu_item_get_active((GtkCheckMenuItem *) widget);
  if (g_DebuggerEnabled == TRUE)
    {
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerRegistersShow), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerBreakpointsShow), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerTLBShow), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerMemoryShow), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerVariablesShow), TRUE);
    }
  else
    {
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerRegistersShow), FALSE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerBreakpointsShow), FALSE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerTLBShow), FALSE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerMemoryShow), FALSE);
      gtk_widget_set_sensitive( GTK_WIDGET(debuggerVariablesShow), FALSE);
    }

  if (emuRestart==1)
    callback_startEmulation(NULL,NULL);
}

static void callback_debuggerWindowShow( GtkWidget *widget, gpointer window )
{
  switch((long)window)
    {
    case 1:
      if(registers_opened==0)
    init_registers();
      break;
    case 2:
      if(breakpoints_opened==0)
    init_breakpoints();
      break;
    case 3:
      if(regTLB_opened==0)
    init_TLBwindow();
      break;
    case 4:
      if(memedit_opened==0)
    init_memedit();
      break;
    case 5:
      if(varlist_opened==0)
    init_varlist();
      break;
    default:
      break;
    }
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
    GtkWidget   *viewMenu;
    GtkWidget   *viewMenuItem;
    GtkWidget   *viewToolbar;
    GtkWidget   *viewFilter;
    GtkWidget   *viewStatusbar;
#ifdef DBG
    GtkWidget   *debuggerMenu;
    GtkWidget   *debuggerMenuItem;
    GtkWidget   *debuggerEnableItem;
    GtkWidget   *debuggerSeparator;
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

    GtkWidget   *helpAboutItem;

    GSList *group = NULL;
    list_t langList;
    list_node_t *node;
    char *language;
    const char *confLang = config_get_string( "Language", "English" );
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
    gtk_widget_add_accelerator(fileCloseRomItem, "activate", g_MainWindow.accelGroup,
                               GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    fileLanguageItem = gtk_menu_item_new_with_mnemonic(tr("_Language"));
    fileExitItem = gtk_menu_item_new_with_mnemonic(tr("_Quit"));
    gtk_widget_add_accelerator(fileExitItem, "activate", g_MainWindow.accelGroup,
                               GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_menu_append( GTK_MENU(fileMenu), fileLoadRomItem );
    gtk_menu_append( GTK_MENU(fileMenu), fileCloseRomItem );
    gtk_menu_append( GTK_MENU(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_append( GTK_MENU(fileMenu), fileLanguageItem );
    gtk_menu_append( GTK_MENU(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_append( GTK_MENU(fileMenu), fileExitItem );

    g_signal_connect(G_OBJECT(fileLoadRomItem), "activate", G_CALLBACK(callback_openRom), NULL);
    g_signal_connect(G_OBJECT(fileCloseRomItem), "activate", G_CALLBACK(callback_closeRom), NULL);
    g_signal_connect(G_OBJECT(fileExitItem), "activate", G_CALLBACK(callback_mainWindowDeleteEvent), NULL);

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
    emulationSaveItem = gtk_menu_item_new_with_mnemonic(tr("Sa_ve State"));
    gtk_widget_add_accelerator(emulationSaveItem, "activate", g_MainWindow.accelGroup,
                               GDK_F5, 0, GTK_ACCEL_VISIBLE);
    emulationSaveAsItem = gtk_menu_item_new_with_mnemonic(tr("Save State _as..."));
    emulationRestoreItem = gtk_menu_item_new_with_mnemonic(tr("_Restore State"));
    gtk_widget_add_accelerator(emulationRestoreItem, "activate", g_MainWindow.accelGroup,
                               GDK_F7, 0, GTK_ACCEL_VISIBLE);
    emulationLoadItem = gtk_menu_item_new_with_mnemonic(tr("_Load State from..."));
    emulationSeparator2 = gtk_menu_item_new();
    emulationSlotItem = gtk_menu_item_new_with_mnemonic(tr("_Current Save State Slot"));

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
    slotItem = gtk_radio_menu_item_new(NULL);
    char buffer[128];
    for (i = 0; i < 10; ++i)
        {
        snprintf(buffer, 128, tr(" Slot _%d"), i);

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

    viewMenu = gtk_menu_new();
    viewMenuItem = gtk_menu_item_new_with_mnemonic(tr("_View"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(viewMenuItem), viewMenu );
    viewToolbar = gtk_check_menu_item_new_with_mnemonic(tr(" _Toolbar"));
    viewFilter = gtk_check_menu_item_new_with_mnemonic(tr(" _Filter"));
    viewStatusbar = gtk_check_menu_item_new_with_mnemonic(tr(" _Statusbar"));

    if((i=config_get_bool("ToolbarVisible",2))==2)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewToolbar),TRUE); 
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewToolbar),i);

    if((i=config_get_bool("FilterVisible",2))==2)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewFilter),TRUE); 
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewFilter),i);

    if((i=config_get_bool("StatusBarVisible",2))==2)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewStatusbar),TRUE); 
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(viewStatusbar),i);

    gtk_signal_connect_object( GTK_OBJECT(viewToolbar), "toggled", GTK_SIGNAL_FUNC(callback_toggle_toolbar), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(viewFilter), "toggled", GTK_SIGNAL_FUNC(callback_toggle_filter), (gpointer)NULL );
    gtk_signal_connect_object( GTK_OBJECT(viewStatusbar), "toggled", GTK_SIGNAL_FUNC(callback_toggle_statusbar), (gpointer)NULL );

    gtk_menu_append( GTK_MENU(viewMenu), viewToolbar );
    gtk_menu_append( GTK_MENU(viewMenu), viewFilter );
    gtk_menu_append( GTK_MENU(viewMenu), viewStatusbar );

    // debugger menu
#ifdef DBG
    debuggerMenu = gtk_menu_new();
    debuggerMenuItem = gtk_menu_item_new_with_mnemonic(tr("_Debugger"));
    gtk_menu_item_set_submenu( GTK_MENU_ITEM(debuggerMenuItem), debuggerMenu );
    debuggerEnableItem = gtk_check_menu_item_new_with_mnemonic(tr(" _Enable"));
    debuggerSeparator = gtk_menu_item_new();
    debuggerRegistersShow = gtk_menu_item_new_with_mnemonic(tr("_Registers"));
    debuggerBreakpointsShow = gtk_menu_item_new_with_mnemonic(tr("_Breakpoints"));
    debuggerTLBShow = gtk_menu_item_new_with_mnemonic(tr("_TLB"));
    debuggerMemoryShow = gtk_menu_item_new_with_mnemonic(tr("_Memory"));
    debuggerVariablesShow = gtk_menu_item_new_with_mnemonic(tr("_Variables"));

    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerEnableItem );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerSeparator );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerRegistersShow );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerBreakpointsShow );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerTLBShow );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerMemoryShow );
    gtk_menu_append( GTK_MENU(debuggerMenu), debuggerVariablesShow );

    if(g_DebuggerEnabled)
      gtk_check_menu_item_set_active( (GtkCheckMenuItem *) debuggerEnableItem, TRUE );
    else 
      {
    gtk_widget_set_sensitive( GTK_WIDGET(debuggerRegistersShow), FALSE);
    gtk_widget_set_sensitive( GTK_WIDGET(debuggerBreakpointsShow), FALSE);
    gtk_widget_set_sensitive( GTK_WIDGET(debuggerTLBShow), FALSE);
    gtk_widget_set_sensitive( GTK_WIDGET(debuggerMemoryShow), FALSE);
    gtk_widget_set_sensitive( GTK_WIDGET(debuggerVariablesShow), FALSE);
      }

    gtk_signal_connect( GTK_OBJECT(debuggerEnableItem), "toggled", GTK_SIGNAL_FUNC(callback_debuggerEnableToggled), (gpointer)NULL );
    gtk_signal_connect( GTK_OBJECT(debuggerRegistersShow), "activate", GTK_SIGNAL_FUNC(callback_debuggerWindowShow), (gpointer)1 );
    gtk_signal_connect( GTK_OBJECT(debuggerBreakpointsShow), "activate", GTK_SIGNAL_FUNC(callback_debuggerWindowShow), (gpointer)2 );
    gtk_signal_connect( GTK_OBJECT(debuggerTLBShow), "activate", GTK_SIGNAL_FUNC(callback_debuggerWindowShow), (gpointer)3 );
    gtk_signal_connect( GTK_OBJECT(debuggerMemoryShow), "activate", GTK_SIGNAL_FUNC(callback_debuggerWindowShow), (gpointer)4 );
    gtk_signal_connect( GTK_OBJECT(debuggerVariablesShow), "activate", GTK_SIGNAL_FUNC(callback_debuggerWindowShow), (gpointer)5 );
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
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), viewMenuItem );
#ifdef DBG
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), debuggerMenuItem );
#endif
    gtk_menu_bar_append( GTK_MENU_BAR(g_MainWindow.menuBar), helpMenuItem );

    return 0;
}

void set_icon(GtkWidget* image, const gchar* icon, int size, gboolean force)
{
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    if(pixbuf)
        g_object_unref(pixbuf);

    if(g_MainWindow.fallbackIcons&&!force)
        pixbuf = gtk_icon_theme_load_icon(g_MainWindow.iconTheme, icon, size,  0, NULL);
    else
        {
        char buffer[128];
        snprintf(buffer, 128, "%dx%d/%s.png", size, size, icon);
        buffer[127] = '\0';
        pixbuf = gdk_pixbuf_new_from_file(get_iconpath(buffer), NULL);
        }
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
}

// toolbar
static int create_toolBar(void)
{
    g_MainWindow.toolBar = gtk_toolbar_new();
    gtk_toolbar_set_orientation(GTK_TOOLBAR(g_MainWindow.toolBar), GTK_ORIENTATION_HORIZONTAL);
    switch(config_get_number("ToolbarStyle", 0))
        {
        case 1:
            gtk_toolbar_set_style(GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_TEXT);
            break;
        case 2:
            gtk_toolbar_set_style(GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_BOTH);
            break;
        default:
            gtk_toolbar_set_style(GTK_TOOLBAR(g_MainWindow.toolBar), GTK_TOOLBAR_ICONS);
        }
    gtk_toolbar_set_tooltips( GTK_TOOLBAR(g_MainWindow.toolBar), TRUE );

    g_MainWindow.openImage = gtk_image_new();
    g_MainWindow.playImage = gtk_image_new();
    g_MainWindow.pauseImage = gtk_image_new();
    g_MainWindow.stopImage = gtk_image_new();
    g_MainWindow.fullscreenImage = gtk_image_new();
    g_MainWindow.configureImage = gtk_image_new();

    int size = config_get_number("ToolbarSize", 22);
    set_icon(g_MainWindow.openImage, "document-open", size, FALSE);
    set_icon(g_MainWindow.playImage, "media-playback-start", size, FALSE);
    set_icon(g_MainWindow.pauseImage, "media-playback-pause", size, FALSE);
    set_icon(g_MainWindow.stopImage, "media-playback-stop", size, FALSE);
    set_icon(g_MainWindow.fullscreenImage, "view-fullscreen", size, FALSE);
    set_icon(g_MainWindow.configureImage, "preferences-system", size, FALSE);

    // add icons to toolbar
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Open"),tr("Open Rom"),"",g_MainWindow.openImage,GTK_SIGNAL_FUNC(callback_openRom),NULL);
    gtk_toolbar_append_space(GTK_TOOLBAR(g_MainWindow.toolBar));
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Start"),tr("Start Emulation"),"",g_MainWindow.playImage,GTK_SIGNAL_FUNC(callback_startEmulation),NULL);
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Pause"),tr("Pause/ Continue Emulation"),"",g_MainWindow.pauseImage,GTK_SIGNAL_FUNC(callback_pauseContinueEmulation),NULL );
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Stop"),tr("Stop Emulation"),"",g_MainWindow.stopImage,GTK_SIGNAL_FUNC(callback_stopEmulation),NULL );
    gtk_toolbar_append_space(GTK_TOOLBAR(g_MainWindow.toolBar));
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Configure"),tr("Configure"),"",g_MainWindow.configureImage,GTK_SIGNAL_FUNC(callback_configure),NULL );
    gtk_toolbar_append_item(GTK_TOOLBAR(g_MainWindow.toolBar),tr("Fullscreen"),tr("Fullscreen"),"",g_MainWindow.fullscreenImage,GTK_SIGNAL_FUNC(callback_fullScreen),NULL);

    gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.toolBar, FALSE, FALSE, 0 );

    return 0;
}

// status bar
static int create_statusBar( void )
{
    g_MainWindow.statusBarHBox = gtk_hbox_new( FALSE, 5 );
    gtk_box_pack_end( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.statusBarHBox, FALSE, FALSE, 0 );

    g_MainWindow.statusBar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(g_MainWindow.statusBar), FALSE);
    gtk_box_pack_start( GTK_BOX(g_MainWindow.statusBarHBox), g_MainWindow.statusBar, TRUE , TRUE, 0 );

    return 0;
}

// main window
static int create_mainWindow(void)
{
    /* Setup main window. */
    gint width, height, xposition, yposition;

    width = config_get_number("MainWindowWidth", 600);
    height = config_get_number("MainWindowHeight", 400);
    xposition = config_get_number("MainWindowXPosition", 0);
    yposition = config_get_number("MainWindowYPosition", 0);

    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gint screenwidth = gdk_screen_get_width(screen);
    gint screenheight = gdk_screen_get_height(screen);

    if(xposition>screenwidth)
        xposition = 0;
    if(yposition>screenheight)
        yposition = 0;

    if(width>screenwidth)
        width = 600;
    if(height>screenheight)
        height = 400;

    if((xposition+width)>screenwidth)
        xposition = screenwidth - width;
    if((yposition+height)>screenheight)
        yposition = screenheight - height;

    g_MainWindow.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_MainWindow.window), MUPEN_NAME " v" MUPEN_VERSION);
    gtk_window_set_default_size(GTK_WINDOW(g_MainWindow.window), width, height);
    gtk_window_move(GTK_WINDOW(g_MainWindow.window), xposition, yposition);

    GdkPixbuf *mupen64plus16, *mupen64plus32;
    mupen64plus16 = gdk_pixbuf_new_from_file(get_iconpath("16x16/mupen64plus.png"), NULL);
    mupen64plus32 = gdk_pixbuf_new_from_file(get_iconpath("32x32/mupen64plus.png"), NULL);

    GList *iconlist = NULL;
    iconlist = g_list_append(iconlist, mupen64plus16);
    iconlist = g_list_append(iconlist, mupen64plus16);

    gtk_window_set_icon_list(GTK_WINDOW(g_MainWindow.window), iconlist);
    /* Edit gtk on quit. */
    gtk_signal_connect(GTK_OBJECT(g_MainWindow.window), "delete_event", GTK_SIGNAL_FUNC(callback_mainWindowDeleteEvent), (gpointer)NULL);

    /* Toplevel vbox, parent to all GUI widgets. */
    g_MainWindow.toplevelVBox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(g_MainWindow.window), g_MainWindow.toplevelVBox);

    create_menuBar();
    create_toolBar();
    create_filter();

    /* Setup rombrowser. */
    int value = config_get_number("RomSortType",16);
    if(value!=GTK_SORT_ASCENDING&&value!=GTK_SORT_DESCENDING)
        {
        g_MainWindow.romSortType = GTK_SORT_ASCENDING;
        config_put_number("RomSortType",GTK_SORT_ASCENDING);
        }
    else
        g_MainWindow.romSortType = value;
    value = config_get_number("RomSortColumn",17);
    if(value<0||value>16)
        {
        g_MainWindow.romSortColumn = 1;
        config_put_number("RomSortColumn",1);
        }
    else
        g_MainWindow.romSortColumn = value;

    create_romBrowser();

    create_statusBar();

    return 0;
}

