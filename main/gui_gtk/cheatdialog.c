/**
 * Mupen64 - cheatdialog.c
 * Copyright (C) 2008 ebenblues
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

#include "main_gtk.h"
#include "cheatdialog.h"
#include "../cheat.h"
#include "../translate.h"

/** globals **/

/** private functions **/

// sets text to render for each combobox entry
static void cb_rom_combo_data_func(GtkCellLayout *cell_layout,
                                   GtkCellRenderer *cell,
                                   GtkTreeModel *tree_model,
                                   GtkTreeIter *iter,
                                   gpointer data)
{
    rom_cheats_t *romcheat;

    // get pointer to rom_cheats_t from model
    gtk_tree_model_get(tree_model, iter,
                       0, &romcheat,
                       -1);

    // set combobox cell text to rom name
    g_object_set(G_OBJECT(cell),
                 "text", romcheat->rom_name,
                 NULL);
}

// populate combo box with list of rom names
static void populate_rom_combo(GtkComboBox *combo)
{
    list_node_t *node;

    GtkListStore *list;
    GtkTreeIter iter;
    GtkCellRenderer *rend;

    // create a list store and populate it with pointers to all rom_cheats_t's
    list = gtk_list_store_new(1, G_TYPE_POINTER);
    list_foreach(g_Cheats, node)
    {
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter,
                           0, node->data,
                           -1);
    }

    gtk_combo_box_set_model(combo, GTK_TREE_MODEL(list));

    // setup cell renderer to display rom name in each cell of combobox
    rend = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), rend, TRUE);
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(combo), rend, cb_rom_combo_data_func, NULL, NULL);

    // set first rom as active (TODO: if a rom is loaded, set it as active)
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
}

// get currently selected rom cheat structure from rom combo box
static rom_cheats_t *get_selected_rom(GtkComboBox *combo)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    rom_cheats_t *romcheat = NULL;

    model = gtk_combo_box_get_model(combo);
    if(gtk_combo_box_get_active_iter(combo, &iter))
        gtk_tree_model_get(model, &iter, 0, &romcheat, -1);

    return romcheat;
}

// populate combo box with list of cheat names
static void init_cheat_combo(GtkComboBox *combo)
{
    // TODO
}

// get currently selected cheat structure from cheat combo box
static cheat_t *get_selected_cheat(GtkComboBox *combo)
{
    // TODO
}

/** cheat enable dialog callbacks **/
static void cb_cheatDisabled(GtkWidget *button, cheat_t *cheat)
{
    cheat->always_enabled = cheat->enabled = 0;
}

static void cb_cheatEnabled(GtkWidget *button, cheat_t *cheat)
{
    cheat->enabled = 1;
    cheat->always_enabled = 0;
}

static void cb_cheatEnabledAlways(GtkWidget *button, cheat_t *cheat)
{
    cheat->enabled = 0;
    cheat->always_enabled = 1;
}

static void cb_updateEnableCheats(GtkWidget *widget, GtkWidget *frame)
{
    GtkWidget *viewport;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *button;
    GtkTooltips *tooltips;

    tooltips = gtk_tooltips_new();

    int i;
    cheat_t *cheat;
    rom_cheats_t *selected_rom;

    // if we're being called to update the cheat list, destroy the current one.
    viewport = gtk_bin_get_child(GTK_BIN(frame));
    if(viewport)
        gtk_widget_destroy(viewport);

    viewport = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(viewport), 5);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(viewport),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    // create list of all cheats for currently selected rom
    selected_rom = get_selected_rom(GTK_COMBO_BOX(widget));

    if(selected_rom)
    {
        table = gtk_table_new(list_length(selected_rom->cheats)+1, 4, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table), 5);

        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(viewport), table);
        gtk_widget_set_size_request(viewport, -1, 200);

        label = gtk_label_new(tr("Disable"));
        gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
        label = gtk_label_new(tr("Enable"));
        gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
        label = gtk_label_new(tr("Enable\nAlways"));
        gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 2, 2);

        // list all cheats
        for(i=0; i<list_length(selected_rom->cheats); i++)
        {
            cheat = list_nth_node_data(selected_rom->cheats, i);

            // Disable button
            button = gtk_radio_button_new(NULL);
            g_signal_connect(GTK_OBJECT(button), "toggled", G_CALLBACK(cb_cheatDisabled), cheat);
            gtk_table_attach(GTK_TABLE(table), button, 0, 1, i+1, i+2, GTK_SHRINK, GTK_SHRINK, 2, 2);
            gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), button, tr("Disable cheat"), "");

            // Enable (this session) button
            button = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(button));
            g_signal_connect(GTK_OBJECT(button), "toggled", G_CALLBACK(cb_cheatEnabled), cheat);
            if(cheat->enabled) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            gtk_table_attach(GTK_TABLE(table), button, 1, 2, i+1, i+2, GTK_SHRINK, GTK_SHRINK, 2, 2);
            gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), button, tr("Enable cheat (this session only)"), "");

            // Enable (always) button
            button = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(button));
            g_signal_connect(GTK_OBJECT(button), "toggled", G_CALLBACK(cb_cheatEnabledAlways), cheat);
            if(cheat->always_enabled) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            gtk_table_attach(GTK_TABLE(table), button, 2, 3, i+1, i+2, GTK_SHRINK, GTK_SHRINK, 2, 2);
            gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), button, tr("Enable cheat always"), "");

            // cheat name
            label = gtk_label_new(cheat->name);
            gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
            gtk_widget_set_size_request(label, 150, -1);
            gtk_table_attach(GTK_TABLE(table), label, 3, 4, i+1, i+2, GTK_FILL, GTK_FILL, 5, 5);
        }
    }

    gtk_container_add(GTK_CONTAINER(frame), viewport);

    // if we're being called to update the cheat list, display the updated list
    if(GTK_WIDGET_VISIBLE(frame))
        gtk_widget_show_all(viewport);
}

static void cb_response(GtkWidget *widget, gint response, gpointer data)
{
    cheat_write_config();
    gtk_widget_destroy(widget);
}

/** public functions **/

// create and display "Enable Cheats" dialog
void cb_enableCheatDialog(GtkWidget *widget)
{
    // Local Variables
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *combo;
    GtkWidget *hbox;

    // Create Dialog Window
    dialog = gtk_dialog_new_with_buttons(tr("Enable Cheats"),
                                         GTK_WINDOW(g_MainWindow.window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_container_set_border_width( GTK_CONTAINER(dialog), 10 );
    g_signal_connect(GTK_OBJECT(dialog), "response",
                     G_CALLBACK(cb_response), NULL);

    // Rom drop-down list
    hbox = gtk_hbox_new(FALSE, 10);
    label = gtk_label_new(tr("Rom"));
    combo = gtk_combo_box_new();
    populate_rom_combo(GTK_COMBO_BOX(combo));

    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 0);

    // create frame for cheats
    frame = gtk_frame_new(tr("Cheats"));
    gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, TRUE, TRUE, 0 );
   
    g_signal_connect(GTK_OBJECT(combo), "changed",
                     G_CALLBACK(cb_updateEnableCheats), frame);

    // list all cheats in a table format
    cb_updateEnableCheats(combo, frame);

    gtk_widget_show_all(dialog);
}

// create and display "Edit Cheats" dialog
void cb_editCheatDialog(GtkWidget *widget)
{
    // TODO
}
