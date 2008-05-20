/***************************************************************************
 romproperties.c - Handles rom properties dialog
----------------------------------------------------------------------------
Began                : Wed Nov 13 2002
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

#include "romproperties.h"
#include "rombrowser.h"

#include "main_gtk.h"
#include "rombrowser.h"
#include "../translate.h"

#include "../../memory/memory.h"    // sl()

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

/*********************************************************************************************************
 * globals
 */
SRomPropertiesDialog g_RomPropDialog;
static cache_entry *g_RomEntry;

/*********************************************************************************************************
 * callbacks
 */
static void callback_apply_changes( GtkWidget *widget, gpointer data )
{
    char crc_code[200];

    gtk_widget_hide( g_RomPropDialog.dialog );
    gtk_grab_remove( g_RomPropDialog.dialog );

    /* save properties */
    //strcpy( g_RomEntry->comments, gtk_entry_get_text( GTK_ENTRY(g_RomPropDialog.commentsEntry) ) );

    //WTF! We shouldn't ever be able to modify the INI.
    //if( g_RomEntry->iniEntry )
   // {
     //   strcpy( g_RomEntry->iniEntry->comments, g_RomEntry->info.cComments );
      //  ini_updateFile(1);
    //}

    // update rombrowser
    rombrowser_refresh();
}

static void callback_cancelClicked( GtkWidget *widget, gpointer data )
{
    gtk_widget_hide( g_RomPropDialog.dialog );
    gtk_grab_remove( g_RomPropDialog.dialog );
}

/*********************************************************************************************************
 * show dialog
 */
void show_romPropDialog( cache_entry *entry )
{
    char ini_code[200];
    char country[32];
    char size[16];

    countrycodestring(entry->romsize, country);
    sprintf(size, "%.1f MBits", (float)(entry->romsize / (float)0x20000) );

    // fill dialog
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.romNameEntry), entry->inientry->goodname );
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.sizeEntry), size );
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.countryEntry), country );
    sprintf(ini_code, "%s", entry->inientry->CRC);
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.iniCodeEntry), ini_code );
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.md5Entry), entry->inientry->MD5 );
    gtk_entry_set_text( GTK_ENTRY(g_RomPropDialog.commentsEntry), entry->comment );
    g_RomEntry = entry;

    // show dialog
    gtk_widget_show_all( g_RomPropDialog.dialog );
    gtk_grab_add( g_RomPropDialog.dialog );
}

/*********************************************************************************************************
 * dialog creation
 */
int create_romPropDialog( void )
{
    GtkWidget *button_ok, *button_cancel;
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *button;

    PangoAttrList *Bold = pango_attr_list_new(); 
    PangoAttribute *Attribute = NULL;
    Attribute = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert ( Bold, Attribute );

    g_RomPropDialog.dialog = gtk_dialog_new();
    gtk_container_set_border_width( GTK_CONTAINER(g_RomPropDialog.dialog), 10 );
    gtk_window_set_title( GTK_WINDOW(g_RomPropDialog.dialog), tr("Rom Properties") );
    gtk_widget_set_size_request( g_RomPropDialog.dialog, 450, -1);

    // rom info
    frame = gtk_frame_new( tr("Rom Info") );
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, TRUE, TRUE, 0 );

    table = gtk_table_new( 7, 2, FALSE );
    gtk_container_set_border_width( GTK_CONTAINER(table), 10 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 10 );
    gtk_container_add( GTK_CONTAINER(frame), table );

    label = gtk_label_new( tr("Rom Name:") );
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_label_set_attributes( GTK_LABEL(label), Bold );
    g_RomPropDialog.romNameEntry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.romNameEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.romNameEntry, 1, 2, 0, 1 );

    label = gtk_label_new( tr("Size:") );
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_label_set_attributes( GTK_LABEL(label), Bold );
    g_RomPropDialog.sizeEntry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.sizeEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 1, 2, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.sizeEntry, 1, 2, 1, 2 );

    label = gtk_label_new( tr("Country:") );
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_label_set_attributes( GTK_LABEL(label), Bold );
    g_RomPropDialog.countryEntry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.countryEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.countryEntry, 1, 2, 2, 3 );

    label = gtk_label_new( tr("Ini Code:") );
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_label_set_attributes( GTK_LABEL(label), Bold );
    g_RomPropDialog.iniCodeEntry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.iniCodeEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 3, 4, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.iniCodeEntry, 1, 2, 3, 4 );

    label = gtk_label_new( tr("MD5 Checksum:") );
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_label_set_attributes( GTK_LABEL(label), Bold );
    g_RomPropDialog.md5Entry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.md5Entry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 4, 5, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.md5Entry, 1, 2, 4, 5 );

    frame = gtk_frame_new( tr("Comments") );
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, TRUE, TRUE, 0 );

    table = gtk_table_new( 1, 1, FALSE );
    gtk_container_set_border_width( GTK_CONTAINER(table), 10 );

    g_RomPropDialog.commentsEntry = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(g_RomPropDialog.commentsEntry), 199 );
    gtk_signal_connect( GTK_OBJECT(g_RomPropDialog.commentsEntry), "activate",
                       GTK_SIGNAL_FUNC(callback_apply_changes), (gpointer)NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.commentsEntry, 0, 1, 0, 1 );
    gtk_container_add( GTK_CONTAINER(frame), table );

    // ok/cancel button
    button_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button_cancel, TRUE, TRUE, 0 );
    gtk_signal_connect( GTK_OBJECT(button_cancel), "clicked",
                GTK_SIGNAL_FUNC(callback_cancelClicked), (gpointer)NULL );

    button_ok = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button_ok, TRUE, TRUE, 0 );
    gtk_signal_connect( GTK_OBJECT(button_ok), "clicked",
                GTK_SIGNAL_FUNC(callback_apply_changes), (gpointer)NULL );

    return 0;
}
