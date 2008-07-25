/**
 * Mupen64Plus - joingame_dialog.c
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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "../../md5.h"
#include "../rombrowser.h"
#include "../../rom.h"
//#include "../../romcache.h"
#include "net_gui.h"
#include "../main_gtk.h"
#define MAX_COL_NAME_LEN		20
#define COL_COUNT                       4

SMainWindow g_MainWindow;
GtkWidget *gameLabel;
gchar l_JoinGameColumnNames[COL_COUNT][MAX_COL_NAME_LEN] = {"Player",
                                                            "Nick",
                                                            "P2P Only",
                                                            "Status"
                                                           };
GtkWidget *l_JoinGameWindow;
GtkWidget *l_JoinGameTreeView;
GtkWidget *l_StartButton;


static void Callback_CancelJoinGame(GtkWidget *widget, gpointer data) {
  gtk_widget_hide_all(l_JoinGameWindow);
  MasterServerCloseGame();
  close_rom(rom);
  gtk_widget_set_sensitive(g_MainWindow.window, TRUE);
}

static void Callback_ReadyJoinGame(GtkWidget *widget, gpointer data) {
  gtk_button_set_label(GTK_BUTTON(l_StartButton), "Waiting");
  gtk_widget_set_sensitive(l_StartButton, FALSE);
}


void hide_joingame_dialog() {
  gtk_widget_hide_all(l_JoinGameWindow);
}

static void append_list_entry(gchar *player, gchar *nick, gchar *p2ponly, gchar *status) {
    GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
    GtkTreeModel *model;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(l_JoinGameTreeView));
    gtk_list_store_append ((GtkListStore *)model, iter);
    gtk_list_store_set ((GtkListStore *)model, iter, 0, player, 1, nick, 2, p2ponly, 3, status, -1);
    free(iter);
}

void create_new_join_game_tree_view_text_column(GtkWidget *tree, gchar *title, gint index) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", index, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
}

GtkWidget *setup_join_game_tree (void)
{
   GtkListStore *listStore;
   GtkWidget *tree;
   GType columnTypes[COL_COUNT];
   int n;

   for (n = 0; n < COL_COUNT; n++) columnTypes[n] = GTK_TYPE_STRING;
   listStore = gtk_list_store_newv (COL_COUNT, columnTypes);
   tree = gtk_tree_view_new_with_model ((GtkTreeModel *)listStore);
   for (n = 0; n < COL_COUNT; n++) create_new_join_game_tree_view_text_column(tree, l_JoinGameColumnNames[n], n);
   gtk_tree_view_set_headers_clickable((GtkTreeView *)tree, TRUE);
   return tree;
}

void create_joingame_dialog() {
  
  GtkWidget *mainWindowBox, *bottomHBox, *emptyHBox, *quitButton, *bottomSeparator, *topSeparator;

  l_JoinGameWindow         = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  l_JoinGameTreeView       = setup_join_game_tree();
  gameLabel                = gtk_label_new("Now Playing Mario Kart (U) [!]");
  mainWindowBox            = gtk_vbox_new(FALSE, 0);
  topSeparator             = gtk_hseparator_new();
  bottomSeparator          = gtk_hseparator_new();
  bottomHBox               = gtk_hbox_new(FALSE, 0);
  emptyHBox                = gtk_hbox_new(FALSE, 0);
  l_StartButton              = gtk_button_new_with_label ("Ready");
  quitButton               = gtk_button_new_with_label ("Quit");


  gtk_button_set_image(GTK_BUTTON(l_StartButton), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON));
  gtk_button_set_image(GTK_BUTTON(quitButton), gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_BUTTON));

  gtk_window_set_title(GTK_WINDOW(l_JoinGameWindow), "Waiting For Players...");
  gtk_container_set_border_width(GTK_CONTAINER(l_JoinGameWindow), 5);
  gtk_container_add(GTK_CONTAINER(l_JoinGameWindow), mainWindowBox);
  gtk_box_pack_start (GTK_BOX(mainWindowBox), gameLabel, FALSE, FALSE, 0);  
  gtk_box_pack_start (GTK_BOX(mainWindowBox), topSeparator, FALSE, FALSE, 10);  
  gtk_box_pack_start (GTK_BOX(mainWindowBox), l_JoinGameTreeView, TRUE, TRUE, 0);  
  gtk_box_pack_start (GTK_BOX(mainWindowBox), bottomSeparator, FALSE, FALSE, 10);  
  gtk_box_pack_start (GTK_BOX(mainWindowBox), bottomHBox, FALSE, FALSE, 0);  
  gtk_box_pack_start (GTK_BOX(bottomHBox), l_StartButton, FALSE, FALSE, 0);  
  gtk_box_pack_start (GTK_BOX(bottomHBox), emptyHBox, TRUE, TRUE, 0);  
  gtk_box_pack_start (GTK_BOX(bottomHBox), quitButton, FALSE, FALSE, 0);  

  g_signal_connect (G_OBJECT (l_JoinGameWindow), "delete_event", G_CALLBACK (Callback_CancelJoinGame), NULL);
  g_signal_connect (G_OBJECT (quitButton), "clicked", G_CALLBACK (Callback_CancelJoinGame), NULL);
  g_signal_connect (G_OBJECT (l_StartButton), "clicked", G_CALLBACK (Callback_ReadyJoinGame), NULL);

  append_list_entry("1", "Bob", "No", "Ready");
  append_list_entry("2", "",    "No", "Not Connected");
  append_list_entry("3", "",    "No", "Not Connected");
  append_list_entry("4", "",    "No", "Not Connected");
}

void show_joingame_dialog(SRomEntry *romEntry) {
    char buffer[128];
    if (romEntry != NULL) {
      sprintf(buffer, "Now playing %s.", romEntry->info.cGoodName);
      gtk_label_set_label(GTK_LABEL(gameLabel), (gchar *)buffer);
    }

    hide_creategame_dialog();
    hide_findgames_dialog();
    gtk_button_set_label(GTK_BUTTON(l_StartButton), "Ready");

    gtk_widget_set_sensitive(l_StartButton, TRUE);
    gtk_widget_set_sensitive(g_MainWindow.window, FALSE);
    gtk_widget_show_all(l_JoinGameWindow);
}

