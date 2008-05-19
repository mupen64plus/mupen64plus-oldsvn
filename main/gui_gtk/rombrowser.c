/***************************************************************************
rombrowser.c - Handles VCR mode GUI elements
----------------------------------------------------------------------------
Began                : Sat Nov 9 2002
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


#include "rombrowser.h"
#include "romproperties.h"
#include "main_gtk.h"

#include "../main.h"
#include "../util.h"
#include "../config.h"
#include "../translate.h"
#include "../rom.h"
#include "../mupenIniApi.h"

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


// rom file extensions
static const char *g_romFileExtensions[] = 
{
    ".rom", ".v64", ".z64", ".gz", ".zip", ".n64", NULL
};

static void setup_view (GtkWidget *view);

/*********************************************************************************************************
* globals
*/
static GdkPixbuf *australia, *europe, *france, *germany, *italy, *japan, *spain, *usa, *japanusa, *n64cart;
static GtkWidget *playRomItem;
static GtkWidget *romPropertiesItem;
static GtkWidget *refreshRomBrowserItem;
static int g_iNumRoms, g_iSortColumn = 0; // sort column
static GtkSortType g_SortType = GTK_SORT_ASCENDING; // sort type (ascending/descending)

static void countrycodeflag(unsigned short int countrycode, GdkPixbuf **flag)
{
    switch(countrycode)
    {
    case 0x41: /* Japan / USA */
        *flag = japanusa; 
        break;

    case 0x44: /* Germany */
        *flag = germany;
        break;

    case 0x45: /* USA */
        *flag = usa;
        break;

    case 0x46: /* France */
        *flag = france;
        break;

    case 'I':  /* Italy */
        *flag = italy;
        break;

    case 0x4A: /* Japan */
        *flag = japan;
        break;

    case 'S':  /* Spain */
        *flag = spain;
        break;

    case 0x55: case 0x59:  /* Australia */
        *flag = australia;
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        *flag = europe;
        break;

    default:
        *flag = n64cart;
        break;
    }
}

/*********************************************************************************************************
* rombrowser functions
*/
// compare function
gint rombrowser_compare( GtkTreeModel *model, GtkTreeIter *ptr1, GtkTreeIter *ptr2, gpointer user_data )
{
    GtkSortType SortType;
    short int returnvalue, column;
    char *buffer1, *buffer2;
    float size1, size2, difference;

    switch( g_iSortColumn )
        {
        case 0: //Flag, so sort by country name.
           column = 1;
            break;
        case 1: //ROM Name
            column = 0;
            break;
        case 2: //Country
            column = 1;
            break;
        case 3: //Size
            column = 2;
            break;
        case 4: //Comments
            column = 3;
            break;
        case 5: //Filename
            column = 4;
            break;
        default:
            return 0;
        };

    gtk_tree_model_get ( model, ptr1, column, &buffer1, -1 );
    gtk_tree_model_get ( model, ptr2, column, &buffer2, -1 );

    if(column==2)
        {
        sscanf ( buffer1, "%f", &size1 );
        sscanf ( buffer2, "%f", &size2 );

        difference = size2 - size1;

        if( difference < 0 )
            { return 1; }
        else if( difference > 0 )
            { return -1; }
        else
            { return  0; }
        }
    else
        { return strcasecmp(buffer1, buffer2); }
}

char *sub_string(const char *string, int start, int end)
{
    int length = end-start;
    if(length <= 0)
    {
        return NULL;
    }
    char *alloc = (char *)malloc(length+1);
    int i;
    for(i = 0; i < length; i++)
    {
        alloc[i] = string[i+start];
    }
    alloc[length] = '\0';

    return alloc;
}

//Add romcache to GUI.
void fillrombrowser()
{
    gboolean fullpaths;
    char *line[5];

    GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
    GdkPixbuf *flag;
    line[1] = malloc(32*sizeof(char));
    line[2] = malloc(16*sizeof(char));

    fullpaths = config_get_bool( "RomBrowserShowFullPaths", FALSE);

    printf("GUI thinks there are %d roms in cache.\n", romcache.length);

    if(romcache.length!=0)
        {
        cache_entry *entry;
        entry = romcache.top;
        do
            {
            line[0] = entry->inientry->goodname;
            countrycodestring(entry->countrycode, line[1]);
            sprintf(line[2], "%.1f MBits", (float)(entry->romsize / (float)0x20000) );
            line[3] = NULL; //Comments.
            countrycodeflag(entry->countrycode, &flag);
            if(fullpaths)
                { line[4] = entry->filename; }
            else
                {
                int fnlen = strlen(entry->filename);
                char *newfn= NULL;
                int i;
                for(i=fnlen; i > 0; i--)
                    {
                    if(entry->filename[i] == '/')
                        {
                        newfn = sub_string(entry->filename, i+1, fnlen);
                        break;
                        }
                    }
                    line[4] = newfn;
                }

            /*
            //DEBUG
            printf("ROM: %s\n", line[0]);
            printf("Country: %s\n", line[1]);
            printf("Size: %s\n", line[2]);
            printf("File: %s\n", line[4]);
            */

            //Add entries to TreeModel
            GtkTreeModel *model =  gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);
            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, line[0], 1, line[1], 2, line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, -1);

            model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);
            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, line[0], 1, line[1], 2,    line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, -1);

            printf("Added ROM to GUI: %s\n", entry->inientry->goodname);
            entry = entry->next;
            }
        while (entry!=NULL);
        }

}




//Load a fresh TreeView by re-scanning directories.
void rombrowser_refresh( void )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
    gtk_tree_selection_select_all(selection);
    GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
    gtk_list_store_clear( GTK_LIST_STORE(model) );

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_tree_selection_select_all(selection);
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_list_store_clear( GTK_LIST_STORE(model) );

   //We'd actually query the rcs system here, insetad of just adding the current cache to the GUI.
   fillrombrowser();

   //Do an initial sort.
   GtkTreeSortable *sortable = GTK_TREE_SORTABLE(model);
   gtk_tree_sortable_set_sort_func ( sortable, g_iSortColumn, rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
   gtk_tree_sortable_set_sort_column_id ( sortable, g_iSortColumn, g_SortType );
   gtk_tree_view_set_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay), GTK_TREE_MODEL(sortable) );
}

//We're going our own filtering, but this is a GtkTreeModelFilter function.
//We do filtering manually since GtkTreeModelFilter can not implement automatic 
//sorting and GtkTreeModelSort can't implement automatic filtering.
gboolean filter_function( GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    const gchar *filter;
    char *buffer1, *buffer2;
    short int column;

    filter = gtk_entry_get_text ( GTK_ENTRY(g_MainWindow.filter) );
    //printf("Filter: %s\n", filter);

    switch( g_iSortColumn )
        {
        case 0: //Flag, so sort by country name.
            column = 1;
            break;
        case 1: //ROM Name
            column = 0;
            break;
        case 2: //Country
            column = 1;
            break;
        case 3: //Size
            column = 2;
            break;
        case 4: //Comments
            column = 3;
            break;
        case 5: //Filename
            column = 4;
            break;
        };

    gtk_tree_model_get (model, iter, column, &buffer1, -1);
    //printf("Value %s\n", buffer1);
    if( buffer1 == NULL || filter == NULL)
        { return TRUE; }
    else
        {
        buffer2 = strcasestr( buffer1, filter );
        if ( buffer2 != NULL )
            { return TRUE; }
       else
            { return FALSE; }
       }
}


/*********************************************************************************************************
* callbacks
*/
// column clicked (title) -> sort
static void callback_columnClicked(GtkTreeViewColumn *treeviewcolumn, gpointer user_data)
{
    if( g_iSortColumn == gtk_tree_view_column_get_sort_column_id(treeviewcolumn) )
        { g_SortType = ( g_SortType == GTK_SORT_ASCENDING ) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING; }
    else
        {
        g_iSortColumn = gtk_tree_view_column_get_sort_column_id(treeviewcolumn); 
        g_SortType = GTK_SORT_ASCENDING;
        }

    apply_filter();
}

// row double clicked -> play rom
static gboolean callback_rowSelected(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    cache_entry *entry;
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get (model, &iter, 5, &entry, -1);

    if(open_rom( entry->filename ) == 0)
        { startEmulation(); }

    return FALSE;
}

// right click -> show menu
gboolean callback_buttonPressed( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_MainWindow.romDisplay));

    if( event->type == GDK_BUTTON_PRESS )
        {
        if( event->button == 3 ) // right click
            {
            if( gtk_tree_selection_count_selected_rows (selection) > 0 )
                {
                gtk_widget_set_sensitive( playRomItem, TRUE );
                gtk_widget_set_sensitive( romPropertiesItem, TRUE );
                }
            else
                {
                gtk_widget_set_sensitive( playRomItem, FALSE );
                gtk_widget_set_sensitive( romPropertiesItem, FALSE );
                }
            gtk_menu_popup( GTK_MENU(data), NULL, NULL, NULL, NULL,
            event->button, event->time );

            return TRUE;
            }
        }

    return FALSE;
}

// activate filter widget -> filter and resort.
static void callback_apply_filter( GtkWidget *widget, gpointer data )
{
    apply_filter();
}

void apply_filter( void )
{
    GtkTreeModel *model, *source, *destination;
    GtkTreeIter sourceiter;
    gboolean validiter;

    //Clear the Display Tree View.
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_tree_selection_select_all(selection);
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_list_store_clear( GTK_LIST_STORE(model) );

    source = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList) );
    g_iNumRoms = gtk_tree_model_iter_n_children(source, NULL);
    validiter = gtk_tree_model_get_iter_first(source, &sourceiter);
    destination = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );

    if(validiter)
        {
        GdkPixbuf *flag;
       cache_entry *entry;
        short int counter;
        gchar *line[5];

        for ( counter = 0; counter < g_iNumRoms; ++counter )
            {
            if ( filter_function( source, &sourceiter, (gpointer)NULL) )
                 {
                 gtk_tree_model_get ( GTK_TREE_MODEL(source), &sourceiter, 0, &line[0], 1, &line[1], 2, &line[2], 3, &line[3], 4, &line[4], 5, &entry, 6, &flag, -1 );

                 GtkTreeIter destinationiter;
                 gtk_list_store_append ( GTK_LIST_STORE(destination), &destinationiter );
                 gtk_list_store_set ( GTK_LIST_STORE(model), &destinationiter, 0, line[0], 1, line[1], 2, line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, -1 );
                 }
            if(!gtk_tree_model_iter_next(source, &sourceiter))
                 { break; }
            }
        }

    destination = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    g_iNumRoms = gtk_tree_model_iter_n_children(destination, NULL);

    GtkTreeSortable *sortable = GTK_TREE_SORTABLE(destination);
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(sortable), g_iSortColumn,  rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(sortable), g_iSortColumn, g_SortType );
}

static gboolean callback_filter_selected( GtkWidget *widget, gpointer data )
{
    if(g_MainWindow.accelUnsafeActive)
        { gtk_window_remove_accel_group(GTK_WINDOW(g_MainWindow.window), g_MainWindow.accelUnsafe); }
    return FALSE;
}


static gboolean callback_filter_unselected( GtkWidget *widget, gpointer data )
{
    if(g_MainWindow.accelUnsafeActive)
        { gtk_window_add_accel_group(GTK_WINDOW(g_MainWindow.window), g_MainWindow.accelUnsafe); }
    return FALSE;
}

static gboolean callback_filter_grab_unselected( GtkWidget *widget, gpointer data )
{
    if(!g_MainWindow.accelUnsafeActive)
        { gtk_window_add_accel_group(GTK_WINDOW(g_MainWindow.window), g_MainWindow.accelUnsafe); }
    gtk_window_set_focus(GTK_WINDOW(g_MainWindow.window), NULL);
    return FALSE;
}

// create GUI filter widgets.
int create_filter( void )
{
    GtkWidget *Hbox;
    GtkWidget *label;

    Hbox = gtk_hbox_new ( FALSE, 5 );

    label = gtk_label_new_with_mnemonic ( tr("F_ilter:") );
    g_MainWindow.filter = gtk_entry_new();
    gtk_entry_set_text ( GTK_ENTRY(g_MainWindow.filter), "" );
    gtk_signal_connect ( GTK_OBJECT(g_MainWindow.filter), "changed",
                       GTK_SIGNAL_FUNC(callback_apply_filter), (gpointer)NULL );
    gtk_signal_connect ( GTK_OBJECT(g_MainWindow.filter), "focus-in-event",
                        GTK_SIGNAL_FUNC(callback_filter_selected), (gpointer)NULL );
    gtk_signal_connect ( GTK_OBJECT(g_MainWindow.filter), "focus-out-event",
                      GTK_SIGNAL_FUNC(callback_filter_unselected), (gpointer)NULL );
    gtk_signal_connect ( GTK_OBJECT(g_MainWindow.filter), "grab-notify",
                     GTK_SIGNAL_FUNC(callback_filter_grab_unselected), (gpointer)NULL );

    gtk_label_set_mnemonic_widget ( GTK_LABEL(label), g_MainWindow.filter );

    gtk_box_pack_start ( GTK_BOX(Hbox), label, FALSE, FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX(Hbox), g_MainWindow.filter, TRUE, TRUE, 5 );

    gtk_box_pack_start ( GTK_BOX(g_MainWindow.toplevelVBox), Hbox, FALSE, FALSE, 0 );
}

// play rom menu item
static void callback_playRom( GtkWidget *widget, gpointer data )
{
    GList *list = NULL, *llist = NULL;
    cache_entry *entry;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_MainWindow.romDisplay));

    list = gtk_tree_selection_get_selected_rows (selection, &model);

    if( !list ) // should never happen since the item is only active when a row is selected
        { return; }

    llist = g_list_first (list);

    gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data);
    gtk_tree_model_get(model, &iter, 5, &entry, -1);

    g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list);

    if(open_rom( entry->filename ) == 0)
        { startEmulation(); }
}

// rom properties
static void callback_romProperties( GtkWidget *widget, gpointer data )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_MainWindow.romDisplay));
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));

    if (gtk_tree_selection_count_selected_rows (selection) > 0)
        {
        cache_entry *entry;

        GList *list = NULL, *llist = NULL;
        list = gtk_tree_selection_get_selected_rows (selection, &model);
        llist = g_list_first (list);

        GtkTreeIter iter;
        gtk_tree_model_get_iter (model, &iter,(GtkTreePath *) llist->data);
        gtk_tree_model_get (model, &iter, 5, &entry, -1);

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list);

        show_romPropDialog( entry );
        }
}

// refresh rom browser
static void callback_refreshRomBrowser( GtkWidget *widget, gpointer data )
{
    rombrowser_refresh();
}



static void setup_view (GtkWidget *view)
{
    gchar *titles[] = 
        {
        (gchar *)tr("Flag"),
        (gchar *)tr("Good Name"),
        (gchar *)tr("Country"),
        (gchar *)tr("Size"),
        (gchar *)tr("Comments"),
        (gchar *)tr("File Name")
        };

    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    GtkTreeViewColumn   *column;

    //Create an empty ListStore as we never use tree childs and format for ROM data.
    GtkListStore *store = gtk_list_store_new (7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, GDK_TYPE_PIXBUF, -1);

    model = GTK_TREE_MODEL (store);

    renderer = gtk_cell_renderer_pixbuf_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[0], renderer, "pixbuf", 6, NULL); 
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 0);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[1], renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 1);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[2], renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 2);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[3], renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 3);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 3);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[4], renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 4);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 4);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (titles[5], renderer, "text", 4, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 5);
    g_signal_connect (G_OBJECT (column), "clicked", G_CALLBACK (callback_columnClicked), model);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 5);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model );
    g_object_unref (model);
}

/*********************************************************************************************************
* gui functions
*/
//Create rom browser.
int create_romBrowser( void )
{
    GtkWidget *rightClickMenu;
    GtkWidget *separatorItem;

    g_iSortColumn = 5;
    g_SortType = GTK_SORT_ASCENDING;

    GError* err = NULL;
    //Load flags.
    australia = gdk_pixbuf_new_from_file( get_iconpath("australia.png"), &err);
    europe = gdk_pixbuf_new_from_file( get_iconpath("europe.png"), &err );
    france = gdk_pixbuf_new_from_file( get_iconpath("france.png"), &err );
    germany = gdk_pixbuf_new_from_file( get_iconpath("germany.png"), &err );
    italy = gdk_pixbuf_new_from_file( get_iconpath("italy.png"), &err );
    japan = gdk_pixbuf_new_from_file( get_iconpath("japan.png"), &err );
    spain = gdk_pixbuf_new_from_file( get_iconpath("spain.png"), &err );
    usa = gdk_pixbuf_new_from_file( get_iconpath("usa.png"), &err );
    japanusa = gdk_pixbuf_new_from_file( get_iconpath("japanusa.png"), &err );
    n64cart = gdk_pixbuf_new_from_file( get_iconpath("n64cart.xpm"), &err );

    //Setup right-click menu.
    rightClickMenu = gtk_menu_new();
    playRomItem = gtk_menu_item_new_with_label( tr("Play Rom") );
    romPropertiesItem = gtk_menu_item_new_with_label( tr("Rom Properties") );
    separatorItem = gtk_menu_item_new();
    refreshRomBrowserItem = gtk_menu_item_new_with_label( tr("Refresh") );

    gtk_menu_append ( GTK_MENU(rightClickMenu), playRomItem );
    gtk_menu_append ( GTK_MENU(rightClickMenu), romPropertiesItem );
    gtk_menu_append ( GTK_MENU(rightClickMenu), separatorItem );
    gtk_menu_append ( GTK_MENU(rightClickMenu), refreshRomBrowserItem );

    gtk_widget_show ( rightClickMenu );
    gtk_widget_show ( playRomItem );
    gtk_widget_show ( romPropertiesItem );
    gtk_widget_show ( separatorItem );
    gtk_widget_show ( refreshRomBrowserItem );

    gtk_signal_connect ( GTK_OBJECT(playRomItem), "activate", GTK_SIGNAL_FUNC(callback_playRom), (gpointer)NULL );
    gtk_signal_connect ( GTK_OBJECT(romPropertiesItem), "activate", GTK_SIGNAL_FUNC(callback_romProperties), (gpointer)NULL );
    gtk_signal_connect ( GTK_OBJECT(refreshRomBrowserItem), "activate", GTK_SIGNAL_FUNC(callback_refreshRomBrowser), (gpointer)NULL );

    g_MainWindow.romFullList = gtk_tree_view_new();
    g_MainWindow.romDisplay = gtk_tree_view_new();

    //This is needed due to the way we clear the TreeViews.
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
    gtk_tree_selection_set_mode ( selection, GTK_SELECTION_MULTIPLE );
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_tree_selection_set_mode ( selection, GTK_SELECTION_MULTIPLE );

    //Initialize TreeViews - i.e. setup column info.
    setup_view(g_MainWindow.romFullList);
    setup_view(g_MainWindow.romDisplay);

    //Create a scrolled window to contain the rom list, make scrollbar visibility automatic.
    g_MainWindow.romScrolledWindow = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(g_MainWindow.romScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

    //Add the Display TreeView into our scrolled window.
    gtk_container_add (GTK_CONTAINER (g_MainWindow.romScrolledWindow), g_MainWindow.romDisplay );
    gtk_container_add (GTK_CONTAINER (g_MainWindow.toplevelVBox), g_MainWindow.romScrolledWindow );

    //Instruct the widget to become visible.
    gtk_widget_show(g_MainWindow.romScrolledWindow);

    gtk_tree_view_set_headers_clickable ( GTK_TREE_VIEW(g_MainWindow.romDisplay), TRUE );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(g_MainWindow.romDisplay), TRUE );
    gtk_tree_view_set_reorderable   (GTK_TREE_VIEW(g_MainWindow.romDisplay), TRUE );
 
    gtk_signal_connect( GTK_OBJECT(g_MainWindow.romDisplay), "row-activated", GTK_SIGNAL_FUNC(callback_rowSelected), (gpointer)NULL );
    gtk_signal_connect( GTK_OBJECT(g_MainWindow.romDisplay), "button-press-event", GTK_SIGNAL_FUNC(callback_buttonPressed), (gpointer)rightClickMenu );

    return 0;
}
