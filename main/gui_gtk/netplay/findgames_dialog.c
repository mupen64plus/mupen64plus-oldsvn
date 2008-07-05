/**
 * Mupen64Plus - findgames_dialog.c
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
//#include "../../romcache.h"
#include "net_gui.h"
#include "../../../network/network.h"

#define MAX_COL_NAME_LEN		20
#define COL_COUNT                       6

gchar l_ColumnNames[COL_COUNT][MAX_COL_NAME_LEN] = {"Created By",
                                                    "IP",
                                                    "Players",
                                                    "Core",
                                                    "Password",
                                                    "P2P Only"
                                                    };
static GtkWidget    *l_FindgamesWindow;
static GtkWidget    *l_TreeView;
static GtkWidget    *l_ComboBox;

static void Callback_DoubleClick(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
static void Callback_ShowFindGamesWindow(GtkWidget *widget, gpointer data);
static void Callback_Refresh(GtkWidget *widget, gpointer data);
static void Callback_Create(GtkWidget *widget, gpointer data);
static void Callback_Join(GtkWidget *widget, gpointer data);
/*
  The following function quick_message() was a demo borrowed (only slightly modified) from the gnome development website:
      http://library.gnome.org/devel/gtk/unstable/GtkDialog.html

  A big thanks to them for a lot of useful tutorials and documentation :)
*/
void quick_message (gchar *message) {
   GtkWidget *dialog, *label;
   dialog = gtk_dialog_new_with_buttons ("Message", (GtkWindow *)l_FindgamesWindow, GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
   label = gtk_label_new (message);
   g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
   gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
   gtk_widget_show_all (dialog);
}

void popFindGamesCB() {
/*    int n = 0;
    static int alreadyPopulated = 0;
    char temp[32];
    cache_entry* rom = g_romcache.top;
    romdatabase_entry *romdb;
    
    if (!alreadyPopulated) {
        while (rom) {
           romdb = ini_search_by_md5(rom->md5);
           strncpy(temp, romdb->goodname, sizeof(temp) - 1);
           if (strlen(romdb->goodname) > sizeof(temp)) {
               temp[29] = '.';
               temp[30] = '.';
               temp[31] = '.';
           }
           gtk_combo_box_append_text((GtkComboBox *)l_ComboBox, (gchar *)temp);
           rom = rom->next;
        }
        alreadyPopulated = 1;
    }
    gtk_combo_box_set_active ((GtkComboBox *)l_ComboBox, 0);*/
}

static void append_list_entry(gchar *creator, gchar *players, gchar *core, gchar *password, gchar *p2p, gchar *ip) {
    GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
    GtkTreeModel *model;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(l_TreeView));
    gtk_list_store_append ((GtkListStore *)model, iter);
    gtk_list_store_set ((GtkListStore *)model, iter, 0, creator, 1, ip, 2, players, 3, core, 4, password, 5, p2p, -1);
    free(iter);
}

static void clear_list() {
    GtkTreeModel *model;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(l_TreeView));
    gtk_list_store_clear( GTK_LIST_STORE(model) );
}

static void join_selected_game() {
   GtkTreePath *path;
   GtkTreeViewColumn *column;
   GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
   GValue *value = (GValue *)malloc(sizeof(GValue));
   GtkTreeModel *model;

   memset((void *)value, 0, sizeof(GValue));
   gtk_tree_view_get_cursor ((GtkTreeView *)l_TreeView, &path, &column);
   model = gtk_tree_view_get_model(GTK_TREE_VIEW(l_TreeView));
   if (path) {
      gtk_tree_model_get_iter (model, iter, path);

      // Need to change core to defined type
      gtk_tree_model_get_value (model, iter, 1, value);
      g_print("Join game @ %s\n", g_value_get_string(value));
      show_joingame_dialog();
      gtk_tree_path_free(path);
   } else {
      quick_message("Select a game from the list that you wish to join.\n");
   }
   free(value);
   free(iter);
}

static void refresh_list() {
    MD5ListNode  *md5_list, *md5_temp;
    GameListNode *game_list, *game_temp;
    char addy_buffer[128];

    clear_list();

    // This is temporary, these functions will be used to populate the dropdown box
    // and game list
    md5_temp = (md5_list = get_md5_list_test());
    game_temp = (game_list = find_games_test());
    printf("[Master Server] Open Game List:\n");
    while (game_temp) {

       // game_temp->host will be contacted and append_list_entry will reflect it's status
       // This is code for testing
       sprintf(addy_buffer, "%d.%d.%d.%d:%d", GET_IP(game_temp->host), game_temp->port);
       printf("    %s\n", addy_buffer);
       append_list_entry("Demo", "2/4", "Dynarec", "Yes", "No", (gchar *)addy_buffer);     // For testing  

       game_temp = game_temp->next;
    }

    freeMD5List(md5_list);
    freeGameList(game_list);



    /*       // gtk_combo_box_get_active (check if db has changed?)
             
             in /network/masterserver.c:
                 query master server with MD5
                 recv() response (wait 1s)

             if no reponse, notify user, return false
             Set window title

             in /network/masterserver.c:
                 status query each server in response list
                 loop for 1 second, recv() server responses
                 for each server response, append_list_entry() (if gui enabled, otherwise print to prompt)
    */
}

void create_new_tree_view_text_column(GtkWidget *tree, gchar *title, gint index) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", index, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
}

GtkWidget *setup_tree (void)
{
   GtkListStore *listStore;
   GtkWidget *tree;
   GType columnTypes[COL_COUNT];
   int n;

   for (n = 0; n < COL_COUNT; n++) columnTypes[n] = GTK_TYPE_STRING;
   listStore = gtk_list_store_newv (COL_COUNT, columnTypes);
   tree = gtk_tree_view_new_with_model ((GtkTreeModel *)listStore);
   for (n = 0; n < COL_COUNT; n++) create_new_tree_view_text_column(tree, l_ColumnNames[n], n);
//   gtk_tree_view_set_grid_lines ((GtkTreeView *)tree, GTK_TREE_VIEW_GRID_LINES_VERTICAL);
   gtk_tree_view_set_headers_clickable((GtkTreeView *)tree, TRUE);
   return tree;
}


static void destroy( GtkWidget *widget, GdkEvent *event, gpointer data ) {
}

void show_findgames_dialog() {
  popFindGamesCB();
  refresh_list(); // If unsuccessful, don't display findgames window, display error dialog
  gtk_widget_show_all(l_FindgamesWindow);
}

void hide_findgames_dialog() {
  gtk_widget_hide_all(l_FindgamesWindow);
}

GtkWidget *create_findgames_dialog () {
    GtkWidget *joinButton, *createButton, *refreshButton, *closeButton;
    GtkWidget *vbox, *hbox, *lhbox, *chbox, *rhbox, *label;
    GtkWidget *tophsep, *bottomhsep, *bframe;
    GtkWidget *romSelectBox, *romSelectFrame;
    GtkWidget *openGameBox, *openGameFrame;

    l_ComboBox = gtk_combo_box_new_text();
    l_TreeView = setup_tree();

    vbox    = gtk_vbox_new (FALSE, 1);
    romSelectBox = gtk_vbox_new (FALSE, 1);
    romSelectFrame = gtk_frame_new("Select Rom");
    openGameBox = gtk_vbox_new (FALSE, 1);
    openGameFrame = gtk_frame_new("Open Games");

    hbox    = gtk_hbox_new (FALSE, 1);
    lhbox    = gtk_hbox_new (FALSE, 1);
    chbox    = gtk_hbox_new (FALSE, 1);
    rhbox    = gtk_hbox_new (FALSE, 1);
    l_FindgamesWindow  = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    joinButton  = gtk_button_new_with_label ("Join");
    gtk_button_set_image((GtkButton *)joinButton, gtk_image_new_from_stock(GTK_STOCK_NETWORK, GTK_ICON_SIZE_BUTTON));
    createButton  = gtk_button_new_with_label ("Create");
    gtk_button_set_image((GtkButton *)createButton, gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON));
    refreshButton  = gtk_button_new_with_label ("Refresh");
    gtk_button_set_image((GtkButton *)refreshButton, gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON));
    closeButton  = gtk_button_new_with_label ("Cancel");
    gtk_button_set_image((GtkButton *)closeButton, gtk_image_new_from_stock(GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));

    g_signal_connect (G_OBJECT (l_FindgamesWindow), "delete_event", GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), NULL);
    g_signal_connect (G_OBJECT (l_FindgamesWindow), "show", G_CALLBACK (Callback_ShowFindGamesWindow), NULL);
    g_signal_connect (G_OBJECT (l_FindgamesWindow), "destroy", G_CALLBACK (destroy), NULL);
    g_signal_connect (G_OBJECT (l_TreeView), "row-activated", G_CALLBACK (Callback_DoubleClick), NULL);
    g_signal_connect (G_OBJECT (joinButton),    "clicked", G_CALLBACK (Callback_Join), NULL);
    g_signal_connect (G_OBJECT (refreshButton), "clicked", G_CALLBACK (Callback_Refresh), NULL);

    g_signal_connect_swapped (G_OBJECT (closeButton), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), G_OBJECT (l_FindgamesWindow));

    gtk_container_set_border_width (GTK_CONTAINER (l_FindgamesWindow), 5);
    gtk_widget_set_size_request  (l_TreeView, 400, 400);

    gtk_box_set_spacing ((GtkBox *)vbox, 10);
    gtk_tree_view_set_show_expanders ((GtkTreeView *)l_TreeView, FALSE);
    gtk_window_set_title ((GtkWindow *)l_FindgamesWindow, "Find Netplay Games");

    gtk_container_add (GTK_CONTAINER (l_FindgamesWindow), vbox);
    gtk_container_add (GTK_CONTAINER (romSelectFrame), romSelectBox);
    gtk_container_add (GTK_CONTAINER (openGameFrame), openGameBox);
    gtk_container_set_border_width (GTK_CONTAINER (romSelectBox), 10);
    gtk_container_set_border_width (GTK_CONTAINER (openGameBox), 10);

    gtk_box_pack_start ((GtkBox *)vbox, romSelectFrame, FALSE, TRUE, 0);
    gtk_box_pack_start ((GtkBox *)romSelectBox, l_ComboBox, FALSE, TRUE, 0);

    gtk_box_pack_start ((GtkBox *)vbox, openGameFrame, TRUE, TRUE, 0);
    gtk_box_pack_start ((GtkBox *)openGameBox, l_TreeView, TRUE, TRUE, 0);

    gtk_box_set_homogeneous((GtkBox *)lhbox, TRUE);

    gtk_box_pack_start ((GtkBox *)vbox, hbox,  FALSE,  TRUE, 0);
    gtk_box_pack_start ((GtkBox *)hbox, lhbox, FALSE, TRUE, 0);
    gtk_box_pack_start ((GtkBox *)hbox, chbox, TRUE,  TRUE, 0);
    gtk_box_pack_start ((GtkBox *)hbox, rhbox, FALSE, TRUE, 0);

    gtk_container_set_border_width (GTK_CONTAINER (rhbox), 0);
    gtk_box_pack_start ((GtkBox *)lhbox, joinButton,    TRUE, TRUE, 5);
//    gtk_box_pack_start ((GtkBox *)rhbox, createButton,  TRUE, TRUE, 5);
    gtk_box_pack_start ((GtkBox *)lhbox, refreshButton, TRUE, TRUE, 5);
    gtk_box_pack_start ((GtkBox *)rhbox, closeButton,   TRUE, TRUE, 5);
    
    return l_FindgamesWindow;
}

/* callback functions for findgames dialog
======================================================================================
*/
static void Callback_DoubleClick(GtkTreeView *tree_view, GtkTreePath *p, GtkTreeViewColumn *c, gpointer user_data) {
  join_selected_game();
}

static void Callback_ShowFindGamesWindow(GtkWidget *widget, gpointer data) {
}

static void Callback_Refresh(GtkWidget *widget, gpointer data) {
  refresh_list();
}

static void Callback_Create(GtkWidget *widget, gpointer data) {
}

static void Callback_Join(GtkWidget *widget, gpointer data) {
  join_selected_game();
}

