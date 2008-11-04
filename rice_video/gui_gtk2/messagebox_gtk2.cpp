/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - messagebox_gtk2.c                                       *
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

#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../messagebox.h"
#include "icontheme.h"

int messagebox(const char* title, int flags, const char* fmt, ...)
{
    va_list ap;
    char buf[2048];
    int ret = 0;

    GtkWidget* dialog;
    GtkWidget* hbox;
    GtkWidget* icon = NULL;
    GtkWidget* label;
    GtkWidget *button1, *button2 = NULL, *button3 = NULL;

    va_start(ap, fmt);
    vsnprintf(buf, 2048, fmt, ap);
    va_end(ap);

    dialog = gtk_dialog_new();
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 0);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_policy(GTK_WINDOW(dialog), 0, 0, 0 );
    g_signal_connect(dialog, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    hbox = gtk_hbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 0);

    /* Check flags. */
    switch(flags&0x000000FF)
    {
    case MB_ABORTRETRYIGNORE:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Abort", 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", 2);
        button3 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Ignore", 3);
        break;
    case MB_CANCELTRYCONTINUE:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", 2);
        button3 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Continue", 3);
        break;
    case MB_OKCANCEL:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, 2);
        break;
    case MB_RETRYCANCEL:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, 2);
        break;
    case MB_YESNO:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_YES, 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_NO, 2);
        break;
    case MB_YESNOCANCEL:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_YES, 1);
        button2 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_NO, 2);
        button3 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, 3);
        break;
    case MB_OK:
    default:
        button1 = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, 1);
    }

    switch(flags&0x00000F00)
    {
    case MB_ICONWARNING:
        icon = gtk_image_new();
        set_icon(icon, "dialog-warning", 32, FALSE);
        break;
    case MB_ICONQUESTION:
    case MB_ICONINFORMATION:
       icon = gtk_image_new();
       set_icon(icon, "dialog-question", 32, FALSE);
       break;
    case MB_ICONERROR:
       icon = gtk_image_new();
       set_icon(icon, "dialog-error", 32, FALSE);
    }

    if(GTK_IS_IMAGE(icon))
        {
        gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
        gtk_misc_set_alignment(GTK_MISC(icon), 0, 0);
        }

    label = gtk_label_new(buf);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);

    gtk_widget_show_all(dialog);
    ret = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return ret;
}

