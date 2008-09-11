/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gui_gtk2.c                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Dylan Wagstaff (Pyromanik)                         *
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

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gui.h"
#include "extension.h"
#include "config.h"

BOOL pseudospecifichle;
BOOL pseudoaudiohle;
BOOL pseudographicshle;
char pseudoaudioname[100];

void enablerCallback(GtkWidget *widget, gpointer *object);
void radioCallback(GtkWidget *widget, gpointer *object);
void comboCallback(GtkWidget *widget, gpointer *object);
void checkCallback(GtkWidget *widget, gpointer *object);
void configCallback(GtkWidget *widget, gpointer *object);

void configDialog(HWND handle)
{
    startup();
    char* plugname = next_plugin();
    int cnt;
    gpointer* parent;
    GtkWidget *dialog, *box, *OKbutton, *CancelButton, *Aframe, *Gframe, *radioA1, *radioA2, *radioA3, *dropbox, *Abox, *Gbox, *Gcheck;

    dialog = gtk_dialog_new();
    if(handle != 0) /*backwards compatibility (vanilla mupen, & old versions) without crying*/
        {
        gdk_window_get_user_data(gdk_window_lookup(handle), (gpointer*)&parent); /* Get GtkWindow from XID or HWND. */
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
        }

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(dialog), 350, 300);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_title(GTK_WINDOW(dialog), "RSP Configuration");

    box = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), box);

    Aframe = gtk_frame_new("Audio Processing:");
    gtk_container_add(GTK_CONTAINER(box), Aframe);
    Abox = gtk_vbox_new(FALSE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(Abox), 10);
    gtk_container_add(GTK_CONTAINER(Aframe), Abox);
    radioA1 = gtk_radio_button_new_with_label (NULL, "Process Alists Inside the RSP");
    gtk_widget_set_tooltip_text(radioA1, "This is the default and most stable option.");
    gtk_box_pack_start(GTK_BOX(Abox), radioA1, FALSE, TRUE, 2);
    radioA2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radioA1), "Process Alists with emu defined plugin");
    gtk_widget_set_tooltip_text(radioA2, "You will not hear any sound if your chosen plugin does not support HLE processing.\nSide effects of this may provide you with a small speed up at the cost of audio.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioA2), pseudoaudiohle);
    gtk_box_pack_start(GTK_BOX(Abox), radioA2, FALSE, TRUE, 2);
    radioA3 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radioA1), "Process Alists with RSP defined plugin");
    gtk_widget_set_tooltip_text(radioA3, "You will not hear any sound if your chosen plugin does not support HLE processing.\nAvoid choosing the same plugin as defined in the emulator settings, as this could have unpredictable results.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioA3), pseudospecifichle);
    gtk_box_pack_start(GTK_BOX(Abox), radioA3, FALSE, TRUE, 2);
    dropbox = gtk_combo_box_new_text();

    for(cnt = 0; plugname != NULL; cnt++)
        {
        gtk_combo_box_append_text(GTK_COMBO_BOX(dropbox), plugname);
        if(strcmp(plugname, pseudoaudioname) == 0)
            gtk_combo_box_set_active(GTK_COMBO_BOX(dropbox), cnt);
        plugname = next_plugin();
        }

    gtk_widget_set_sensitive(dropbox, pseudospecifichle);
    gtk_box_pack_start(GTK_BOX(Abox), dropbox, FALSE, TRUE, 2);

    Gframe = gtk_frame_new("Graphics Processing:");
    gtk_container_add(GTK_CONTAINER(box), Gframe);
    Gbox = gtk_vbox_new(FALSE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(Gbox), 10);
    gtk_container_add(GTK_CONTAINER(Gframe), Gbox);
    Gcheck = gtk_check_button_new_with_label("Process Dlists inside the RSP");
    gtk_widget_set_tooltip_text(Gcheck, "The RSP does not currently support the decoding of Dlists. This would disable video, which makes the whole thing kind of pointless.\nThis is disabled for now.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Gcheck), !pseudographicshle);
    gtk_widget_set_sensitive(Gcheck, FALSE);
    gtk_box_pack_start(GTK_BOX(Gbox), Gcheck, FALSE, TRUE, 2);

    CancelButton = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
    OKbutton = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);

    g_signal_connect(radioA1, "toggled", G_CALLBACK(radioCallback), radioA1);
    g_signal_connect(radioA2, "toggled", G_CALLBACK(radioCallback), radioA1);
    g_signal_connect(radioA3, "toggled", G_CALLBACK(enablerCallback), dropbox);
    g_signal_connect(dropbox, "changed", G_CALLBACK(comboCallback), dropbox);
    g_signal_connect(Gcheck, "toggled", G_CALLBACK(checkCallback), Gcheck);

    g_signal_connect(dialog, "response", G_CALLBACK(configCallback), dialog);
    gtk_widget_show_all(dialog);
}

/*Message box handler, to simplify info dialog calls.*/
void MessageBox(HWND handle, char* message, char* title, int flags)
{
    GtkWidget *dialog, *box, *img, *label, *button;
    gpointer* parent;
    char* icon;

    dialog = gtk_dialog_new();

    if(handle != 0) /*backwards compatibility (vanilla mupen, & old versions) without crying*/
        {
        gdk_window_get_user_data(gdk_window_lookup(handle), (gpointer*)&parent); /* Get GtkWindow from XID or HWND. */
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
        }

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_title(GTK_WINDOW(dialog), title);

    box = gtk_hbox_new(FALSE, 30);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), box);

    switch (flags)
        {
        case 1:
            icon = GTK_STOCK_DIALOG_INFO; break;
        case 2:
            icon = GTK_STOCK_DIALOG_ERROR; break;
        case 3:
            icon = GTK_STOCK_DIALOG_WARNING; break;
        default:
            icon = NULL;
        }

    if(icon != NULL)
        {
        img = gtk_image_new_from_stock(icon, GTK_ICON_SIZE_DIALOG);
        gtk_container_add(GTK_CONTAINER(box), img);
        }

    label = gtk_label_new(message);
    gtk_container_add(GTK_CONTAINER(box), label);

    button = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);

    gtk_widget_show_all(dialog);
}

/* Callbacks (5) for GTK config dialog. */
void enablerCallback(GtkWidget* widget, gpointer* data)
{
    pseudoaudiohle = TRUE;
    pseudospecifichle = !pseudospecifichle;
    gtk_widget_set_sensitive(GTK_WIDGET(data), pseudospecifichle);
}

void radioCallback(GtkWidget* widget, gpointer* data)
{
    pseudoaudiohle = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data));
}

void comboCallback(GtkWidget* widget, gpointer* data)
{
    strcpy(pseudoaudioname, gtk_combo_box_get_active_text(GTK_COMBO_BOX(data)));
}

void checkCallback(GtkWidget* widget, gpointer* data)
{
    pseudographicshle = !pseudographicshle;
}

void configCallback(GtkWidget* widget, gpointer* data)
{
    int result = (int)data;
    if(result==GTK_RESPONSE_ACCEPT)
        {
        AudioHle = pseudoaudiohle;
        GraphicsHle = pseudographicshle;
        SpecificHle = pseudospecifichle;

        if(SpecificHle)
            AudioHle = TRUE;

        strcpy(audioname, pseudoaudioname);
        SaveConfig();
        }

    gtk_widget_destroy(widget);
}
