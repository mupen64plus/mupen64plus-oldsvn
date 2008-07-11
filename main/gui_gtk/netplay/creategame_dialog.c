/**
 * Mupen64Plus - creategame_dialog.c
 * Copyright (C) 2008 orbitaldecay
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include "../../rom.h"
#include "../rombrowser.h"
#include "../main_gtk.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../md5.h"
//#include "../../romcache.h"
#include "net_gui.h"
#include <SDL_net.h>
#include "../../../network/network.h"

static GtkWidget *l_CreateGameWindow, *romComboBox, *masterCheck;
static SRomEntry *romEntry;

//extern rom_settings ROM_SETTINGS;
//extern unsigned char *rom;

/*
  The following function quick_message() was a demo borrowed (only slightly modified) from the gnome development website:
      http://library.gnome.org/devel/gtk/unstable/GtkDialog.html

  A big thanks to them for a lot of useful tutorials and documentation :)
*/

void callback_game_create(GtkWidget *widget, gpointer data) {
    // This md5 should be gotten from the rom selection box!
    unsigned char md5[16];
    unsigned char temp[3];
    int           game_id, n, t;
    unsigned char checked;

    open_rom(romEntry->cFilename);
    // Very crappy hack, couldn't figure out an easier way to do it :*(
    temp[2] = 0;
    for (n = 0; n < 16; n++) {
       strncpy(temp, ROM_SETTINGS.MD5+(n*2), 2);
       sscanf(temp, "%X", &t);
       md5[n] = t;
    }

    checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(masterCheck));
    if (checked) {
      game_id = MasterServerCreateGame(md5, 1000);
      show_joingame_dialog(romEntry);
      if (game_id == -1) {
          alert_message(tr("Unable to contact master server."));
      }
    } else {
      show_joingame_dialog(romEntry);
    }
}

void hide_creategame_dialog() {
  gtk_widget_hide_all(l_CreateGameWindow);
}

static void clear_comboBox() {
    GtkTreeModel *model;
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(romComboBox));
    gtk_list_store_clear( GTK_LIST_STORE(model) );
}

void create_creategame_dialog() {
/* 
  ================================================================================

  Declare widgets

  ================================================================================
*/ 
  
  GtkWidget *optionFrame;                                                 // Frames
  GtkWidget *optionTable;                                                 // Tables
  GtkWidget *mainVBox, *bottomHBox, *emptyHBox, *buttonHBox;              // Boxes
  GtkWidget *pcbHBox;
  GtkWidget *romLabel, *coreLabel, *playerLabel, *pswdLabel;              // Labels
  GtkWidget *createButton, *cancelButton;                                 // Buttons
  GtkWidget *coreComboBox, *playerComboBox;                               // ComboBoxes
  GtkWidget *pswdEntry;                                                   // EntryBox

/* 
  ================================================================================

  Initialize widgets

  ================================================================================
*/ 

  // Main window
  l_CreateGameWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  // Buttons
  createButton       = gtk_button_new_with_label ("Create");
  cancelButton       = gtk_button_new_with_label ("Cancel");

  // Labels
  romLabel           = gtk_label_new("Rom");
  coreLabel          = gtk_label_new("Core");
  playerLabel        = gtk_label_new("Min Players");
  pswdLabel          = gtk_label_new("Password");

  // Frames
  optionFrame        = gtk_frame_new("Options");

  // Tables
  optionTable        = gtk_table_new(4, 2, FALSE);

  // ComboBoxes
  romComboBox        = gtk_combo_box_new_text();
  coreComboBox       = gtk_combo_box_new_text();
  playerComboBox     = gtk_combo_box_new_text();

  // Entries
  pswdEntry          = gtk_entry_new();

  // Boxes
  mainVBox           = gtk_vbox_new(FALSE, 0);
  bottomHBox         = gtk_hbox_new(FALSE, 0);
  emptyHBox          = gtk_hbox_new(FALSE, 0);
  pcbHBox            = gtk_hbox_new(FALSE, 0);

  // CheckButton
  masterCheck           = gtk_check_button_new_with_label("Notify Master Server");

/* 
  ================================================================================

  Populate combo boxes

  ================================================================================
*/ 

  gtk_combo_box_append_text(GTK_COMBO_BOX(coreComboBox), "Dynamic Recompilation");
  gtk_combo_box_append_text(GTK_COMBO_BOX(coreComboBox), "Interpreter");
  gtk_combo_box_append_text(GTK_COMBO_BOX(coreComboBox), "Pure Interpreter");
  gtk_combo_box_set_active (GTK_COMBO_BOX(coreComboBox), 0);

  gtk_combo_box_append_text(GTK_COMBO_BOX(playerComboBox), "1");
  gtk_combo_box_append_text(GTK_COMBO_BOX(playerComboBox), "2");
  gtk_combo_box_append_text(GTK_COMBO_BOX(playerComboBox), "3");
  gtk_combo_box_append_text(GTK_COMBO_BOX(playerComboBox), "4");
  gtk_combo_box_set_active (GTK_COMBO_BOX(playerComboBox), 0);

/* 
  ================================================================================

  Specify layout

  ================================================================================
*/ 

  // Add icons to buttons
  gtk_button_set_image(GTK_BUTTON(createButton), gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON));
  gtk_button_set_image(GTK_BUTTON(cancelButton), gtk_image_new_from_stock(GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));

  // Connected signals to callbacks
  g_signal_connect (G_OBJECT (l_CreateGameWindow), "delete_event", GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), NULL);
  g_signal_connect (G_OBJECT (createButton), "clicked", G_CALLBACK (callback_game_create), NULL);
  g_signal_connect_swapped (G_OBJECT (cancelButton), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), G_OBJECT (l_CreateGameWindow));

  // Set main window border size & set title
  gtk_container_set_border_width(GTK_CONTAINER(l_CreateGameWindow), 5);
  gtk_window_set_resizable (GTK_WINDOW(l_CreateGameWindow), FALSE);
  gtk_window_set_title(GTK_WINDOW(l_CreateGameWindow), "New Netplay Game");

  // Pack up boxes, frames, tables, etc.
  gtk_container_add(GTK_CONTAINER(l_CreateGameWindow), mainVBox);
  gtk_box_pack_start (GTK_BOX(mainVBox), optionFrame, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(optionFrame), optionTable);
  gtk_box_pack_start (GTK_BOX(mainVBox), bottomHBox, FALSE, FALSE, 5);

  // Pack buttons
  gtk_box_pack_start (GTK_BOX(bottomHBox), createButton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(bottomHBox), emptyHBox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX(bottomHBox), cancelButton, FALSE, FALSE, 10);

  // Populate option table with labels, comboboxes, and other widgets
  gtk_table_attach(GTK_TABLE(optionTable), romLabel,       0, 1, 0, 1,    0, 0, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), romComboBox,    1, 2, 0, 1,    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), coreLabel,      0, 1, 1, 2,    0, 0, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), coreComboBox,   1, 2, 1, 2,    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), playerLabel,    0, 1, 2, 3,    0, 0, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), pcbHBox,        1, 2, 2, 3,    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), pswdLabel,      0, 1, 3, 4,    0, 0, 5, 5);
  gtk_table_attach(GTK_TABLE(optionTable), pswdEntry,      1, 2, 3, 4,    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_box_pack_start (GTK_BOX(pcbHBox), playerComboBox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(pcbHBox), masterCheck, TRUE, TRUE, 5);

  // Make password entry box non-visible
  gtk_entry_set_visibility(GTK_ENTRY(pswdEntry), FALSE);
}

void show_creategame_dialog() {
    if(!rom) {

        GList *list = NULL, *llist = NULL;
        GtkTreeIter iter;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_MainWindow.romDisplay));
        list = gtk_tree_selection_get_selected_rows (selection, &model);

        if( !list ) {//Nothing selected.
            alert_message(tr("Select a ROM first."));
            return; // Should be displaying error dialog
        } else {
            llist = g_list_first (list);
            gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data);
            gtk_tree_model_get(model, &iter, 5, &romEntry, -1);
            g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
            g_list_free (list);

            clear_comboBox();         
            gtk_combo_box_append_text(GTK_COMBO_BOX(romComboBox), romEntry->info.cGoodName);
            gtk_combo_box_set_active (GTK_COMBO_BOX(romComboBox), 0);
            gtk_widget_show_all(l_CreateGameWindow);
        }
    }
}


