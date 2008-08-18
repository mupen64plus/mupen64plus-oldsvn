/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - romproperties.h                                         *
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

/* romproperties.c - Handles rom properties dialog */

#include "romproperties.h"
#include "rombrowser.h"
#include "main_gtk.h"

#include "../translate.h"
#include "../main.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

/*********************************************************************************************************
 * globals
 */
SRomPropertiesDialog g_RomPropDialog;

/*********************************************************************************************************
 * callbacks
 */
static void callback_apply_changes(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(g_RomPropDialog.dialog);

    strncpy(g_RomPropDialog.entry->usercomments, gtk_entry_get_text(GTK_ENTRY(g_RomPropDialog.commentsEntry)),255);

    // update rombrowser
    g_romcache.rcstask = RCS_WRITE_CACHE;
    gtk_list_store_set (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(g_MainWindow.romDisplay))),&g_RomPropDialog.iter,3,g_RomPropDialog.entry->usercomments,-1);
}

static void callback_cancel_clicked(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(g_RomPropDialog.dialog);
}

// hide on delete
static gint delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_widget_hide(widget);
    gtk_grab_remove(g_RomPropDialog.dialog);

    return TRUE;
}

/*********************************************************************************************************
 * show dialog
 */
void show_romPropDialog()
{
    char *filename;
    char country[32];
    char md5hash[33];
    char crc1[9];
    char crc2[9];
    char savetype[16];
    char players[16];
    char size[16];
    char compressiontype[16];
    char imagetype[32];
    char cicchip[16];
    char rumble[8];
    int counter;
    GdkPixbuf *flag;

    filename = filefrompath(g_RomPropDialog.entry->filename); 
    countrycodeflag(g_RomPropDialog.entry->countrycode, &flag);
    countrycodestring(g_RomPropDialog.entry->countrycode, country);
    for ( counter = 0; counter < 16; ++counter ) 
        sprintf(md5hash+counter*2, "%02X", g_RomPropDialog.entry->md5[counter]);
    sprintf(crc1, "%X", g_RomPropDialog.entry->crc1);
    sprintf(crc2, "%X", g_RomPropDialog.entry->crc2);
    savestring(g_RomPropDialog.entry->inientry->savetype, savetype);
    playersstring(g_RomPropDialog.entry->inientry->players, players);
    sprintf(size, "%.1f MBits", (float)(g_RomPropDialog.entry->romsize / (float)0x20000));
    compressionstring(g_RomPropDialog.entry->compressiontype, compressiontype);
    imagestring(g_RomPropDialog.entry->imagetype, imagetype);
    cicstring(g_RomPropDialog.entry->cic, cicchip);
    rumblestring(g_RomPropDialog.entry->inientry->rumble, rumble);

    // fill dialog
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.filenameEntry), filename);
    free(filename);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.goodnameEntry), g_RomPropDialog.entry->inientry->goodname);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.countryEntry), country);
    gtk_image_set_from_pixbuf(GTK_IMAGE(g_RomPropDialog.flag), flag); 
    for( counter = 0; counter < 5; ++counter )
        {
        if(g_RomPropDialog.entry->inientry->status>counter)
            gtk_image_set_from_pixbuf(GTK_IMAGE(g_RomPropDialog.status[counter]), star); 
        else 
            gtk_image_set_from_pixbuf(GTK_IMAGE(g_RomPropDialog.status[counter]), NULL); 
        }
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.fullpathEntry), g_RomPropDialog.entry->filename);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.md5Entry), md5hash);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.crc1Entry), crc1);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.crc2Entry), crc2);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.internalnameEntry), g_RomPropDialog.entry->internalname);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.savetypeEntry), savetype);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.playersEntry), players);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.sizeEntry), size);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.compressiontypeEntry), compressiontype);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.imagetypeEntry), imagetype);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.cicchipEntry), cicchip);
    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.rumbleEntry), rumble);

    gtk_entry_set_text(GTK_ENTRY(g_RomPropDialog.commentsEntry), g_RomPropDialog.entry->usercomments);

    // show dialog
    gtk_widget_show_all(g_RomPropDialog.dialog);
}

/*********************************************************************************************************
 * dialog creation
 */
int create_romPropDialog()
{
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *statushbox;
    GtkWidget *countryhbox;
    int i;
    char buffer[32];

    g_RomPropDialog.dialog = gtk_dialog_new();
    gtk_container_set_border_width( GTK_CONTAINER(g_RomPropDialog.dialog), 10 );
    gtk_window_set_title(GTK_WINDOW(g_RomPropDialog.dialog), tr("Rom Properties"));
    gtk_signal_connect(GTK_OBJECT(g_RomPropDialog.dialog), "delete_event",                GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL);
    gtk_widget_set_size_request( g_RomPropDialog.dialog, 500, -1);

    frame = gtk_frame_new(tr("Rom Info"));
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(12, 4, FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table), 2);
    gtk_container_add(GTK_CONTAINER(frame), table);

    snprintf(buffer,31,"%s:",tr(column_names[4]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.filenameEntry = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.filenameEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.filenameEntry, 1, 4, 0, 1);

    snprintf(buffer,31,"%s:",tr(column_names[0]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0 );
    g_RomPropDialog.countryEntry = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.countryEntry), FALSE);
    g_RomPropDialog.flag = gtk_image_new(); 
    countryhbox = gtk_hbox_new(FALSE, 0);
    gtk_box_set_spacing(GTK_BOX(countryhbox), 5);
    gtk_box_pack_start(GTK_BOX(countryhbox), g_RomPropDialog.flag, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(countryhbox), g_RomPropDialog.countryEntry, TRUE, TRUE, 0);
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 1, 2, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), countryhbox, 1, 4, 1, 2);

    snprintf(buffer,31,"%s:",tr(column_names[1]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.goodnameEntry = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.goodnameEntry), FALSE );
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.goodnameEntry, 1, 4, 2, 3);

    snprintf(buffer,31,"%s:",tr(column_names[2]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4, GTK_FILL, GTK_EXPAND, 0, 5);
    statushbox = gtk_hbox_new(FALSE, 0);
    gtk_box_set_spacing(GTK_BOX(statushbox), 2);
    for( i = 0; i < 5; ++i)
        {
        g_RomPropDialog.status[i] = gtk_image_new();
        gtk_box_pack_start(GTK_BOX(statushbox), g_RomPropDialog.status[i], FALSE, FALSE, 0);
        }
    gtk_table_attach(GTK_TABLE(table), statushbox, 1, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);

    label = gtk_label_new( tr("Full Path:"));
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0);
    g_RomPropDialog.fullpathEntry = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.fullpathEntry), FALSE );
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.fullpathEntry, 1, 4, 4, 5);

    snprintf(buffer,31,"%s:",tr(column_names[8]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.internalnameEntry = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.internalnameEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 5, 6, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.internalnameEntry, 1, 4, 5, 6);

    snprintf(buffer,31,"%s:",tr(column_names[5]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.md5Entry = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.md5Entry), FALSE );
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 6, 7, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.md5Entry, 1, 4, 6, 7);

    snprintf(buffer,31,"%s:",tr(column_names[6]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.crc1Entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.crc1Entry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.crc1Entry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 7, 8, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.crc1Entry, 1, 2, 7, 8);

    snprintf(buffer,31,"%s:",tr(column_names[7]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.crc2Entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.crc2Entry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.crc2Entry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 2, 3, 7, 8, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.crc2Entry, 3, 4, 7, 8);

    snprintf(buffer,31,"%s:",tr(column_names[11]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.sizeEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.sizeEntry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.sizeEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 8, 9, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.sizeEntry, 1, 2, 8, 9);

    snprintf(buffer,31,"%s:",tr(column_names[9]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.savetypeEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.savetypeEntry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.savetypeEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 2, 3, 8, 9, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.savetypeEntry, 3, 4, 8, 9);

    snprintf(buffer,31,"%s:",tr(column_names[12]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.compressiontypeEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.compressiontypeEntry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.compressiontypeEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 9, 10, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.compressiontypeEntry, 1, 2, 9, 10);

    snprintf(buffer,31,"%s:",tr(column_names[10]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.playersEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.playersEntry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.playersEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 2, 3, 9, 10, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.playersEntry, 3, 4, 9, 10);

    snprintf(buffer,31,"%s:",tr(column_names[13]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment(GTK_MISC(label), 1, 0);
    g_RomPropDialog.imagetypeEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.imagetypeEntry), 8);
    gtk_entry_set_editable(GTK_ENTRY(g_RomPropDialog.imagetypeEntry), FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 10, 11, GTK_FILL, GTK_EXPAND, 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), g_RomPropDialog.imagetypeEntry, 1, 2, 10, 11);

    snprintf(buffer,31,"%s:",tr(column_names[15]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    g_RomPropDialog.rumbleEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.rumbleEntry), 8);
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.rumbleEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 2, 3, 10, 11, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.rumbleEntry, 3, 4, 10, 11 );

    snprintf(buffer,31,"%s:",tr(column_names[14]));
    buffer[31]='\0';
    label = gtk_label_new(buffer);
    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    g_RomPropDialog.cicchipEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(g_RomPropDialog.cicchipEntry), 8);
    gtk_entry_set_editable( GTK_ENTRY(g_RomPropDialog.cicchipEntry), FALSE );
    gtk_table_attach( GTK_TABLE(table), label, 0, 1, 11, 12, GTK_FILL, GTK_EXPAND, 0, 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.cicchipEntry, 1, 2, 11, 12 );

    snprintf(buffer,31,"%s",tr(column_names[3]));
    buffer[31]='\0';
    frame = gtk_frame_new(buffer);
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->vbox), frame, TRUE, TRUE, 0 );

    table = gtk_table_new( 1, 1, FALSE );
    gtk_container_set_border_width( GTK_CONTAINER(table), 10 );

    g_RomPropDialog.commentsEntry = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(g_RomPropDialog.commentsEntry), 255 );
    gtk_signal_connect( GTK_OBJECT(g_RomPropDialog.commentsEntry), "activate",
                       GTK_SIGNAL_FUNC(callback_apply_changes), (gpointer)NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), g_RomPropDialog.commentsEntry, 0, 1, 0, 1 );
    gtk_container_add( GTK_CONTAINER(frame), table );

    /* Ok/ Cancel buttons. */
    GtkWidget *button;

    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
                       GTK_SIGNAL_FUNC(callback_cancel_clicked), (gpointer)NULL);

    button = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(g_RomPropDialog.dialog)->action_area), button, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
                       GTK_SIGNAL_FUNC(callback_apply_changes), (gpointer)NULL);

    return 0;
}
