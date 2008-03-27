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
#include <glib.h>
#include "../../debugger/debugger.h"
#endif

#include <gtk/gtk.h>

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
#include <pthread.h>	// POSIX Thread library
#include <signal.h>	// signals
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
SMainWindow	g_MainWindow;

/** status bar **/
typedef struct
{
	const char *name;
	gint        id;
	GtkWidget  *bar;
} statusBarSection;

static statusBarSection statusBarSections[] = {
	{ "status", -1, NULL },
	{ "num_roms", -1, NULL },
	{ NULL, -1, NULL }
};

/*********************************************************************************************************
* GUI interfaces (declared in ../guifuncs.h)
*/

/** gui_parseArgs
 *    Parse gui-specific arguments and remove them from argument list.
 */
void gui_parseArgs(int *argc, char ***argv)
{
	// call gtk to parse arguments
	gtk_init(argc, argv);
}

/** gui_build
 *    Create GUI components, but do not display
 */
void gui_build(void)
{
	rombrowser_readCache();

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
	gtk_widget_show_all( g_MainWindow.window );

	// perform any queued gui actions
	while( g_main_iteration( FALSE ) );
}

/** gui_main_loop
 *    Give control of thread to gtk
 */
void gui_main_loop(void)
{
	gtk_main();
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

	if(gui_enabled() &&
	   !pthread_equal(self, g_EmulationThread))
	{
		for(i = 0; statusBarSections[i].name; i++)
		{
			if(!strcasecmp("status", statusBarSections[i].name))
			{
				gtk_statusbar_pop(GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id);
				gtk_statusbar_push(GTK_STATUSBAR(statusBarSections[i].bar), statusBarSections[i].id, buf);
        
				// update status bar
				while( g_main_iteration( FALSE ) );
        
				return;
			}
		}
	}
	// if gui not enabled, just print to console
	else
	{
		printf(tr("Info"));
		printf(": %s\n", buf);
	}

#ifdef _DEBUG
	printf("statusbar_message(): unknown section '%s'!\n", section);
#endif
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

	if(gui_enabled() &&
	   !pthread_equal(self, g_EmulationThread))
	{
		dialog = gtk_dialog_new_with_buttons(tr("Error"),
		                                     GTK_WINDOW(g_MainWindow.window),
						     GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_STOCK_OK,
						     GTK_RESPONSE_NONE,
						     NULL);
        
		g_signal_connect_swapped(dialog,
		                         "response",
					 G_CALLBACK(gtk_widget_destroy),
					 dialog);
        
		hbox = gtk_hbox_new(FALSE, 5);
        
		icon = gtk_image_new_from_file(get_iconpath("messagebox-error.png"));
		gtk_container_add(GTK_CONTAINER(hbox), icon);
        
		label = gtk_label_new(buf);
		gtk_container_add(GTK_CONTAINER(hbox), label);
        
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
        
		gtk_widget_show_all(dialog);
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

	if(gui_enabled() &&
	   !pthread_equal(self, g_EmulationThread))
	{
		dialog = gtk_dialog_new_with_buttons(tr("Confirm"),
		                                     GTK_WINDOW(g_MainWindow.window),
						     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						     tr("Yes"),
						     GTK_RESPONSE_ACCEPT,
						     tr("No"),
						     GTK_RESPONSE_REJECT,
						     NULL);
        
		hbox = gtk_hbox_new(FALSE, 5);
        
		icon = gtk_image_new_from_file(get_iconpath("messagebox-quest.png"));
		gtk_container_add(GTK_CONTAINER(hbox), icon);
        
		label = gtk_label_new(buf);
		gtk_container_add(GTK_CONTAINER(hbox), label);
        
		gtk_widget_show_all(hbox);
        
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
        
		response = gtk_dialog_run(GTK_DIALOG(dialog));
        
		gtk_widget_destroy(dialog);
        
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
static void callback_openRomFileSelected(GtkWidget *widget, gpointer data)
{
	const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

	gtk_widget_hide( GTK_WIDGET(data) );
	// really hide dialog (let gtk work)
	while( g_main_iteration( FALSE ) );

	if(open_rom(filename) == 0)
	{
		char buf[300];
		snprintf(buf, 300, MUPEN_NAME " v" MUPEN_VERSION " - %s", ROM_HEADER->nom);
		gtk_window_set_title(GTK_WINDOW(g_MainWindow.window), buf);
	}
}

static void callback_openRom(GtkWidget *widget, gpointer data)
{
	GtkWidget *file_selector;

	if( g_EmulationThread )
	{
		if(!confirm_message(tr("Emulation is running. Do you want to\nstop it and load a rom?")))
			return;
		callback_stopEmulation( NULL, NULL );
	}

	/* Create the selector */
	file_selector = gtk_file_selection_new( tr("Open Rom...") );

	// set main window as parent of file selection window
	gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

	gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
			    GTK_SIGNAL_FUNC(callback_openRomFileSelected), (gpointer)file_selector );

	/* Ensure that the dialog box is destroyed when the user clicks a button. */
	gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

	gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
				   GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

	/* Display that dialog */
	gtk_widget_show( file_selector );
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

	// recreate gui
	gtk_widget_destroy( g_MainWindow.window );
	gtk_widget_destroy( g_AboutDialog.dialog );
	gtk_widget_destroy( g_ConfigDialog.dialog );
	create_mainWindow();
	create_aboutDialog();
	create_configDialog();
	gtk_widget_show_all( g_MainWindow.window );
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
// start emulation
static void callback_startEmulation(GtkWidget *widget, gpointer data)
{
	if(!rom)
	{
		if(confirm_message(tr("There is no Rom loaded.\nDo you want to load one?")))
			callback_openRom(NULL, NULL);

		return;
	}
	startEmulation();
}

// pause/continue emulation
static void callback_pauseContinueEmulation(GtkWidget *widget, gpointer data)
{
	int isRunning = pauseContinueEmulation();

	if(widget && GTK_IS_TOOL_BUTTON (widget))
	{
		GtkToolButton *widget = GTK_TOOL_BUTTON(widget);
		if(isRunning)
		{
			gtk_tool_button_set_label(widget, tr("Pause"));
		}
		else
		{
			gtk_tool_button_set_label(widget, tr("Resume"));
		}
	}
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
static void callback_saveAsFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		savestates_select_filename( filename );
		savestates_job |= SAVESTATE;
	}
}

static void callback_SaveAs( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save as...") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
				    GTK_SIGNAL_FUNC(callback_saveAsFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked",
					   GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked",
					   GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}

// Restore
static void callback_Restore( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
		savestates_job |= LOADSTATE;
}

// Load
static void callback_loadFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		const gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		savestates_select_filename( filename );
		savestates_job |= LOADSTATE;
	}
}

static void callback_Load( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load...") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC(callback_loadFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}

/** Slot **/
static void callback_Default( GtkWidget *widget, gpointer data )
{
	savestates_select_slot( 0 );
}

static void callback_slot( GtkObject *data )
{
	savestates_select_slot( (long) data );
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
static void callback_vcrStartRecord_fileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );

		// Allow GTK to process everything (Causing the widget to truely disappear)
		while( g_main_iteration( FALSE ) );

		if (VCR_startRecord( filename ) < 0)
			alert_message(tr("Couldn't start recording."));
	}
}


static void callback_vcrStartRecord( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save .rec file") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC(callback_vcrStartRecord_fileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display alert_message
}


static void callback_vcrStopRecord( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopRecord() < 0)
			alert_message(tr("Couldn't stop recording."));
	}
}


static void callback_vcrStartPlayback_fileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		if (VCR_startPlayback( filename ) < 0)
			alert_message(tr("Couldn't start playback."));
	}
}


static void callback_vcrStartPlayback( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load .rec file") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC(callback_vcrStartPlayback_fileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display alert_message
}


static void callback_vcrStopPlayback( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopPlayback() < 0)
			alert_message(tr("Couldn't stop playback."));
	}
}


static char m_startCaptureRecFilename[PATH_MAX];

static void callback_vcrStartCapture_aviFileSelected( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		if (VCR_startCapture( m_startCaptureRecFilename, filename ) < 0)
			alert_message(tr("Couldn't start capturing."));
	}
}

static void callback_vcrStartCapture_recFileSelected( GtkWidget *widget, gpointer data )
{
	GtkWidget *file_selector;

	if( g_EmulationThread )
	{
		gchar *filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(data) );

		gtk_widget_hide( GTK_WIDGET(data) );
		// really hide dialog (let gtk work)
		while( g_main_iteration( FALSE ) );

		strncpy( m_startCaptureRecFilename, filename, PATH_MAX );

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Save .avi file") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC(callback_vcrStartCapture_aviFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
}


static void callback_vcrStartCapture( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		GtkWidget *file_selector;

		/* Create the selector */
		file_selector = gtk_file_selection_new( tr("Load .rec file") );

		// set main window as parent of file selection window
		gtk_window_set_transient_for(GTK_WINDOW(file_selector), GTK_WINDOW(g_MainWindow.window));

		gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC(callback_vcrStartCapture_recFileSelected), (gpointer)file_selector );

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer)file_selector );

		gtk_signal_connect_object( GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)file_selector );

		/* Display that dialog */
		gtk_widget_show( file_selector );
	}
	// else maybe display alert_message
}


static void callback_vcrStopCapture( GtkWidget *widget, gpointer data )
{
	if( g_EmulationThread )
	{
		if (VCR_stopCapture() < 0)
			alert_message(tr("Couldn't stop capturing."));
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
			callback_emulationStop( NULL, NULL );
			g_DebuggerEnabled = GTK_CHECK_MENU_ITEM(widget)->active;
			callback_emulationStart( NULL, NULL );
		}
		return;
	}

	g_DebuggerEnabled = GTK_CHECK_MENU_ITEM(widget)->active;
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
	int i, w, h;

	// save configuration
	w = g_MainWindow.window->allocation.width;
	h = g_MainWindow.window->allocation.height;
	if (w != 0 && h != 0)
	{
		config_put_number( "MainWindow Width", w );
		config_put_number( "MainWindow Height", h );
	}
	for( i = 0; i < 6; i++ )
	{
		w = gtk_tree_view_get_column( GTK_TREE_VIEW(g_MainWindow.romTreeView), i )->width;
		if (w != 0)
		{
			char buf[30];
			sprintf( buf, "RomBrowser ColWidth[%d]", i );
			config_put_number( buf, w );
		}
	}

	gtk_main_quit();

	return TRUE; // undeleteable
}

/*********************************************************************************************************
* gui creation
*/
// create a menu item with an accelerator
static GtkWidget *menu_item_new_with_accelerator( GtkAccelGroup *group, const char *label )
{
	GtkWidget *item;
	gint key;

	item = gtk_menu_item_new_with_label( "" );
	key = gtk_label_parse_uline( GTK_LABEL(GTK_BIN(item)->child), label );
//  gtk_widget_add_accelerator( item, "activate_item", group, key, GDK_MOD1_MASK, 0 );

	return item;
}

// static widgets to change their state from emulation thread
static GtkWidget       *slotDefaultItem;
static GtkWidget       *slotItem[9];

// menuBar
static int create_menuBar( void )
{
	GtkWidget	*fileMenu;
	GtkWidget	*fileMenuItem;
	GtkWidget	*emulationMenu;
	GtkWidget	*emulationMenuItem;
	GtkWidget	*optionsMenu;
	GtkWidget	*optionsMenuItem;
#ifdef VCR_SUPPORT
	GtkWidget	*vcrMenu;
	GtkWidget	*vcrMenuItem;
#endif
#ifdef DBG
	GtkWidget	*debuggerMenu;
	GtkWidget	*debuggerMenuItem;
#endif
	GtkWidget	*helpMenu;
	GtkWidget	*helpMenuItem;

	GtkWidget	*fileLoadRomItem;
	GtkWidget	*fileCloseRomItem;
	GtkWidget	*fileSeparator1;
	GtkWidget	*fileLanguageItem;
	GtkWidget	*fileLanguageMenu;
	GtkWidget	*fileSeparator2;
	GtkWidget	*fileExitItem;

	GtkWidget	*emulationStartItem;
	GtkWidget	*emulationPauseContinueItem;
	GtkWidget	*emulationStopItem;
	GtkWidget	*emulationSeparator1;
	GtkWidget	*emulationSaveItem;
	GtkWidget	*emulationSaveAsItem;
	GtkWidget	*emulationRestoreItem;
	GtkWidget	*emulationLoadItem;
	GtkWidget	*emulationSeparator2;
	GtkWidget	*emulationSlotItem;
	GtkWidget	*emulationSlotMenu;

	GtkWidget	*slotSeparator;

	GtkWidget	*optionsConfigureItem;
	GtkWidget	*optionsSeparator1;
	GtkWidget	*optionsVideoItem;
	GtkWidget	*optionsAudioItem;
	GtkWidget	*optionsInputItem;
	GtkWidget	*optionsRSPItem;
	GtkWidget	*optionsSeparator2;
	GtkWidget	*optionsFullScreenItem;

#ifdef VCR_SUPPORT
	GtkWidget	*vcrStartRecordItem;
	GtkWidget	*vcrStopRecordItem;
	GtkWidget	*vcrSeparator1;
	GtkWidget	*vcrStartPlaybackItem;
	GtkWidget	*vcrStopPlaybackItem;
	GtkWidget	*vcrSeparator2;
	GtkWidget	*vcrStartCaptureItem;
	GtkWidget	*vcrStopCaptureItem;
	GtkWidget	*vcrSeparator3;
	GtkWidget	*vcrSetupItem;
#endif

#ifdef DBG
	GtkWidget	*debuggerEnableItem;
#endif

	GtkWidget	*helpAboutItem;

	GtkAccelGroup *menuAccelGroup;
	GtkAccelGroup *fileAccelGroup;
	GtkAccelGroup *emulationAccelGroup;
	GtkAccelGroup *optionsAccelGroup = NULL;
#ifdef VCR_SUPPORT
	GtkAccelGroup *vcrAccelGroup;
#endif
#ifdef DBG
	GtkAccelGroup *debuggerAccelGroup;
#endif
	GtkAccelGroup *helpAccelGroup;

	GSList *group = NULL;
	GSList *slot_group = NULL;
	list_t langList;
	list_node_t *node;
	char *language;
	const char *confLang = config_get_string( "Language", "English" );
	char buffer[1000];
	int i, lang_found;

	// menubar
	g_MainWindow.menuBar = gtk_menu_bar_new();
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.menuBar, FALSE, FALSE, 0 );
	menuAccelGroup = gtk_accel_group_new();

	// file menu
	fileAccelGroup = gtk_accel_group_new();
	fileMenu = gtk_menu_new();
	fileMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_File") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(fileMenuItem), fileMenu );
	fileLoadRomItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Open Rom...") );
	fileCloseRomItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Close Rom") );
	fileSeparator1 = gtk_menu_item_new();
	fileLanguageItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Language") );
	fileSeparator2 = gtk_menu_item_new();
	fileExitItem = menu_item_new_with_accelerator( fileAccelGroup, tr("_Exit") );
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

		gtk_signal_connect_object( GTK_OBJECT(fileLanguageItem), "toggled",
					   GTK_SIGNAL_FUNC(callback_languageSelected), (gpointer)NULL );
		gtk_menu_append( GTK_MENU(fileLanguageMenu), fileLanguageItem );
	}
	// free language name list
	list_delete(&langList);

	// emulation menu
	emulationAccelGroup = gtk_accel_group_new();
	emulationMenu = gtk_menu_new();
	emulationMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Emulation") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationMenuItem), emulationMenu );
	emulationStartItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("_Start") );
	emulationPauseContinueItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Pause/_Continue") );
	emulationStopItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Sto_p") );
	emulationSeparator1 = gtk_menu_item_new();
	emulationSaveItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Save State") );
	emulationSaveAsItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Save State As") );
	emulationRestoreItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Restore State") );
	emulationLoadItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Load State") );
	emulationSeparator2 = gtk_menu_item_new();
	emulationSlotItem = menu_item_new_with_accelerator( emulationAccelGroup, tr("Current save state") );

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

	gtk_signal_connect_object( GTK_OBJECT(emulationStartItem), "activate", GTK_SIGNAL_FUNC(callback_startEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationPauseContinueItem), "activate", GTK_SIGNAL_FUNC(callback_pauseContinueEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationStopItem), "activate", GTK_SIGNAL_FUNC(callback_stopEmulation), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationSaveItem), "activate", GTK_SIGNAL_FUNC(callback_Save), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationSaveAsItem), "activate", GTK_SIGNAL_FUNC(callback_SaveAs), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationRestoreItem), "activate", GTK_SIGNAL_FUNC(callback_Restore), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(emulationLoadItem), "activate", GTK_SIGNAL_FUNC(callback_Load), (gpointer)NULL );

	// slot menu
	emulationSlotMenu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(emulationSlotItem), emulationSlotMenu );
	slotDefaultItem = gtk_radio_menu_item_new_with_label( slot_group, tr("Default") );
	slot_group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(slotDefaultItem) );
	slotSeparator = gtk_menu_item_new();
	for (i = 0; i < 9; i++)
	{
		snprintf( buffer, 1000, tr( "Slot %d" ), i+1 );
		slotItem[i] = gtk_radio_menu_item_new_with_label( slot_group, buffer );
		slot_group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(slotItem[i]) );
	}

	gtk_menu_append( GTK_MENU(emulationSlotMenu), slotDefaultItem );
	gtk_menu_append( GTK_MENU(emulationSlotMenu), slotSeparator );
	for (i = 0; i < 9; i++)
		gtk_menu_append( GTK_MENU(emulationSlotMenu), slotItem[i] );

	gtk_signal_connect_object( GTK_OBJECT(slotDefaultItem), "activate", GTK_SIGNAL_FUNC(callback_Default), (gpointer)NULL );

	for (i = 0; i < 9; i++)
		gtk_signal_connect_object( GTK_OBJECT(slotItem[i]), "activate",
					   GTK_SIGNAL_FUNC(callback_slot), (gpointer) (long) i );

	// options menu
	optionsMenu = gtk_menu_new();
	optionsMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Options") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(optionsMenuItem), optionsMenu );
	optionsConfigureItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Configure...") );
	optionsSeparator1 = gtk_menu_item_new();
	optionsVideoItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Video Settings...") );
	optionsAudioItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Audio Settings...") );
	optionsInputItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Input Settings...") );
	optionsRSPItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_RSP Settings...") );
	optionsSeparator2 = gtk_menu_item_new();
	optionsFullScreenItem = menu_item_new_with_accelerator( optionsAccelGroup, tr("_Full Screen") );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsConfigureItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator1 );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsVideoItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsAudioItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsInputItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsRSPItem );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsSeparator2 );
	gtk_menu_append( GTK_MENU(optionsMenu), optionsFullScreenItem );

	gtk_signal_connect_object( GTK_OBJECT(optionsConfigureItem), "activate", GTK_SIGNAL_FUNC(callback_configure), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsVideoItem), "activate", GTK_SIGNAL_FUNC(callback_configureVideo), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsAudioItem), "activate", GTK_SIGNAL_FUNC(callback_configureAudio), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsInputItem), "activate", GTK_SIGNAL_FUNC(callback_configureInput), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsRSPItem), "activate", GTK_SIGNAL_FUNC(callback_configureRSP), (gpointer)NULL );
	gtk_signal_connect_object( GTK_OBJECT(optionsFullScreenItem), "activate",GTK_SIGNAL_FUNC(callback_fullScreen), (gpointer)NULL );

	// vcr menu
#ifdef VCR_SUPPORT
	vcrAccelGroup = gtk_accel_group_new();
	vcrMenu = gtk_menu_new();
	vcrMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_VCR") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(vcrMenuItem), vcrMenu );
	vcrStartRecordItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Record...") );
	vcrStopRecordItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Record") );
	vcrSeparator1 = gtk_menu_item_new();
	vcrStartPlaybackItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Playback...") );
	vcrStopPlaybackItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Playback") );
	vcrSeparator2 = gtk_menu_item_new();
	vcrStartCaptureItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Start _Capture...") );
	vcrStopCaptureItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Stop Capture") );
	vcrSeparator3 = gtk_menu_item_new();
	vcrSetupItem = menu_item_new_with_accelerator( vcrAccelGroup, tr("Configure Codec...") );

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
	debuggerAccelGroup = gtk_accel_group_new();
	debuggerMenu = gtk_menu_new();
	debuggerMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Debugger") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(debuggerMenuItem), debuggerMenu );
	debuggerEnableItem = menu_item_new_with_accelerator( debuggerAccelGroup, tr("_Enable") );
	gtk_menu_append( GTK_MENU(debuggerMenu), debuggerEnableItem );

	gtk_signal_connect_object( GTK_OBJECT(debuggerEnableItem), "toggled", GTK_SIGNAL_FUNC(callback_debuggerEnableToggled), (gpointer)NULL );
#endif // DBG

	// help menu
	helpAccelGroup = gtk_accel_group_new();
	helpMenu = gtk_menu_new();
	helpMenuItem = menu_item_new_with_accelerator( menuAccelGroup, tr("_Help") );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(helpMenuItem), helpMenu );
	helpAboutItem = menu_item_new_with_accelerator( helpAccelGroup, tr("_About...") );
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

	// Add accelerators to window
/*	gtk_menu_set_accel_group( GTK_MENU(fileMenu), menuAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(fileMenu), fileAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(emulationMenu), emulationAccelGroup );
	gtk_menu_set_accel_group( GTK_MENU(optionsMenu), optionsAccelGroup );
#ifdef VCR_SUPPORT
	gtk_menu_set_accel_group( GTK_MENU(vcrMenu), vcrAccelGroup );
#endif
#ifdef DBG
	gtk_menu_set_accel_group( GTK_MENU(debuggerMenu), debuggerAccelGroup );
#endif
	gtk_menu_set_accel_group( GTK_MENU(helpMenu), helpAccelGroup );*/

	return 0;
}

// toolbar
static int create_toolBar( void )
{
	GtkWidget	*openImage = NULL;
	GtkWidget	*playImage = NULL;
	GtkWidget	*pauseImage = NULL;
	GtkWidget	*stopImage = NULL;
	GtkWidget	*fullscreenImage = NULL;
	GtkWidget	*configureImage = NULL;

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
	gtk_toolbar_append_item( GTK_TOOLBAR(g_MainWindow.toolBar),tr("Pause"),tr("Pause/continue Emulation"),"",pauseImage,GTK_SIGNAL_FUNC(callback_pauseContinueEmulation),NULL );
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

	// create status bar
	g_MainWindow.statusBarHBox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.statusBarHBox, FALSE, FALSE, 0 );

	// request context ids
	for( i = 0; statusBarSections[i].name; i++ )
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
	g_MainWindow.toplevelVBox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER(g_MainWindow.window), g_MainWindow.toplevelVBox );

	// menu
	create_menuBar();

	// toolbar
	create_toolBar();

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
