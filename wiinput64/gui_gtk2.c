/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Wiinput64 plugin : gui_gtk2.c                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Lionel Fuentes (Funto)                             *
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

#ifdef USE_GTK

#include "wiinput64.h"
#include "gui.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

void OnConfigOK(GtkWidget* dialog)
{
	gtk_widget_destroy(dialog);
}

void ConfigDialog()
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *addr_hboxes[4];
	GtkWidget *addr_labels[4];
	GtkWidget *addr_entries[4];
	int i=0;
	char buffer[100] = "";
	char* pfile_name = NULL;
	size_t len = strlen(config_dir);
	FILE* f = NULL;

	// Dialog
	dialog = gtk_dialog_new_with_buttons("Wiinput64 plugin configuration", NULL, GTK_DIALOG_MODAL,
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				NULL);

	// Label
	label = gtk_label_new(	"Wiimote(s) bluetooth address(es)\n"
							"(get them by typing \"lswm\" in a shell prompt) :");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 0);

	// Wiimotes addresses : label + entry
	for(i=0 ; i<4 ; i++)
	{
		// - hbox
		addr_hboxes[i] = gtk_hbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(addr_hboxes[i]), 10);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), addr_hboxes[i], TRUE, TRUE, 0);

		// - label
		sprintf(buffer, "Wiimote %d : ", i);
		addr_labels[i] = gtk_label_new(buffer);
		gtk_box_pack_start(GTK_BOX(addr_hboxes[i]), addr_labels[i], TRUE, TRUE, 0);

		// - entry
		addr_entries[i] = gtk_entry_new_with_max_length(sizeof(bluetooth_addr) - 1);
		gtk_box_pack_start(GTK_BOX(addr_hboxes[i]), addr_entries[i], TRUE, TRUE, 0);
		gtk_entry_set_text(GTK_ENTRY(addr_entries[i]), wiimote_addresses[i]);
	}

	// Show the dialog
	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
	{
		// Memorize the new wiimote addresses
		for(i=0 ; i<4 ; i++)
			strcpy(wiimote_addresses[i], gtk_entry_get_text(GTK_ENTRY(addr_entries[i])));

		// Save them in the config file
		pfile_name = (char*)malloc(len + 50);
		sprintf(pfile_name, "%s/%s", config_dir, CONFIG_FILE_NAME);
		if((f = fopen(pfile_name, "w")) == NULL)
		{
			GtkWidget* err_dialog;
			err_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"Unable to open " CONFIG_FILE_NAME " for writing !");
			gtk_dialog_run(GTK_DIALOG(err_dialog));
			gtk_widget_destroy(err_dialog);

			fprintf(stderr, "[Wiinput64] : error : unable to open %s for writing !", pfile_name);
		}
		else
		{
			fprintf(f,	"addr0=%s\n"
						"addr1=%s\n"
						"addr2=%s\n"
						"addr3=%s\n",	wiimote_addresses[0], wiimote_addresses[1],
										wiimote_addresses[2], wiimote_addresses[3]);
			fclose(f);
		}

		free(pfile_name);
	}

	gtk_widget_destroy(dialog);
}

void AboutDialog()
{
	GtkWidget* dialog;
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, PLUGIN_FULL_NAME);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

#endif // USE_GTK
