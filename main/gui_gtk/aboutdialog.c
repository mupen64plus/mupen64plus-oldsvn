/***************************************************************************
 aboutdialog.c - Handles the about box
----------------------------------------------------------------------------
Began                : Mon Nov 11 2002
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

#include "../main.h"
#include "../version.h"
#include "../translate.h"
#include "aboutdialog.h"
#include "main_gtk.h"

#include <gtk/gtk.h>


/** globals **/
SAboutDialog g_AboutDialog;

/** callbacks **/
static void
callback_okClicked( GtkWidget *widget, gpointer data )
{
	gtk_widget_hide( g_AboutDialog.dialog );
}

// hide on delete
static gint delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_widget_hide( widget );

	return TRUE; // undeleteable
}

/** function to create the about dialog **/
int
create_aboutDialog( void )
{
	GtkWidget *mupenImage = NULL;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *button;

	g_AboutDialog.dialog = gtk_dialog_new();
	gtk_container_set_border_width( GTK_CONTAINER(g_AboutDialog.dialog), 10 );
	gtk_window_set_title( GTK_WINDOW(g_AboutDialog.dialog), tr("About " MUPEN_NAME) );
	// set main window as parent of about window
	gtk_window_set_transient_for(GTK_WINDOW(g_AboutDialog.dialog), GTK_WINDOW(g_MainWindow.window));
	gtk_signal_connect(GTK_OBJECT(g_AboutDialog.dialog), "delete_event",
				GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_AboutDialog.dialog)->vbox), hbox, FALSE, FALSE, 0 );

	frame = gtk_frame_new( MUPEN_NAME " v" MUPEN_VERSION );
	gtk_container_set_border_width( GTK_CONTAINER(frame), 10 );
	gtk_box_pack_start( GTK_BOX(hbox), frame, TRUE, TRUE, 0 );

	label = gtk_label_new(	tr("About Mupen64Plus\n"
                "Original Mupen64 code by Hacktarux\n"
				"Gtk GUI by blight\n"
				"Mupen and RiceVideo 64-bit port by Richard42 and nmn\n"
                "Glide64 port by Gunther\n"
                "Fixes and features by Ebenblues, DarkJezter, nmn, Richard42, and others\n"
				"\n") );
	gtk_misc_set_padding( GTK_MISC(label), 10, 10 );
	gtk_container_add( GTK_CONTAINER(frame), label );

	mupenImage = gtk_image_new_from_file( get_iconpath("logo.png") );
	gtk_box_pack_start( GTK_BOX(hbox), mupenImage, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( tr("Ok") );
	gtk_box_pack_start( GTK_BOX(GTK_BOX(GTK_DIALOG(g_AboutDialog.dialog)->action_area)), button, TRUE, TRUE, 0 );
	gtk_signal_connect( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(callback_okClicked), (gpointer)NULL );

	return 0;
}
