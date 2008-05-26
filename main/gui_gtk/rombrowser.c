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
//static unsigned int times;
//if(times%5000==0)
//{ printf("Calling sort function! %d\n", times); }
//++times;

    GtkSortType SortType;
    short int returnvalue, column;
    gchar *buffer1, *buffer2;

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
        case 7: //Filename
            column = 7;
            break;
        case 8: //Filename
            column = 8;
            break;
        default:
            return 0;
        };

    gtk_tree_model_get ( model, ptr1, column, &buffer1, -1 );
    gtk_tree_model_get ( model, ptr2, column, &buffer2, -1 );

    returnvalue = strcasecmp(buffer1, buffer2); 
    g_free(buffer1);
    g_free(buffer2);

    return returnvalue;
}

char* filefrompath(const char *string)
{
    char *buffer1, *buffer2;
    buffer1=(char*)string;
    while((buffer2=strstr(buffer1, "/"))!=NULL)
        { buffer1=buffer2+1; }

    if(buffer1==(char*)string)
        { return buffer1; }
    else
        {
        int size = strlen(buffer1)+1;
        buffer2 = malloc(size*sizeof(char));
        if(buffer2==NULL)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ ); 
            return buffer1;
            }
        snprintf(buffer2, size, "%s\0", buffer1);
        return buffer2;
        }
}

void call_rcs(void)
{
    g_RCSTask = RCS_RESCAN;
}

//Load a fresh TreeView after re-scanning directories.
void rombrowser_refresh( void )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
    gtk_tree_selection_select_all(selection);
    GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
    gtk_list_store_clear( GTK_LIST_STORE(model) );
    selection = gtk_tree_view_get_selection (  GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_tree_selection_select_all(selection);
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    gtk_list_store_clear( GTK_LIST_STORE(model) );

    gboolean fullpaths;
    char *line[7];

    GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
    GdkPixbuf *flag;
    line[1] = malloc(32*sizeof(char));
    line[2] = malloc(16*sizeof(char));
    line[5] = malloc(16*sizeof(char));
    line[6] = malloc(16*sizeof(char));
    if(iter==NULL||line[1]==NULL||line[2]==NULL)
        {
        fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ ); 
        return;
        }

    fullpaths = config_get_bool( "RomBrowserShowFullPaths", FALSE);

    if(romcache.length!=0)
        {
        cache_entry *entry;
        entry = romcache.top;
        do
            {
            line[0] = entry->inientry->goodname;
            countrycodestring(entry->countrycode, line[1]);
            sprintf(line[2], "%.1f MBits", (float)(entry->romsize / (float)0x20000) );
            line[3] = entry->comment;
            countrycodeflag(entry->countrycode, &flag);
            if(fullpaths)
                { line[4] = entry->filename; }
            else
                { line[4] = filefrompath(entry->filename); }
            compressionstring(entry->compressiontype, line[5]);
            imagestring(entry->imagetype, line[6]);

            //Add entries to TreeModel
            model =  gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);
            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, line[0], 1, line[1], 2, line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, 7, line[5], 8, line[6], -1);

            model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);
            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, line[0], 1, line[1], 2,    line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, 7, line[5], 8, line[6], -1);

            entry = entry->next;
            }
        while (entry!=NULL);
        }

      free(line[1]);
      free(line[2]);
      free(line[5]);
      free(line[6]);
      free(iter);
      if(fullpaths==1)
         { free(line[4]); }

    printf("DOING INITIAL SORT!!!\n");

     //Do an initial sort.
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), g_iSortColumn, rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(model), g_iSortColumn, g_SortType );
    gtk_tree_view_set_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay), model );
}

//We're going our own filtering, but this is a GtkTreeModelFilter function.
//We do filtering manually since GtkTreeModelFilter can not implement automatic 
//sorting and GtkTreeModelSort can't implement automatic filtering.
//NOTE: we should add contingencies, i.e. secondary sort columns.
gboolean filter_function( GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
//static unsigned int times;
//printf("Calling filter function! %d\n", times);
//++times;

    const gchar *filter;
    char *buffer1, *buffer2;
    short int column;
    gboolean returnvalue;

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
        case 7: //Filename
            column = 7;
            break;
        case 8: //Filename
            column = 8;
            break;
        };

    gtk_tree_model_get (model, iter, column, &buffer1, -1);
    //printf("Value %s\n", buffer1);
    if( buffer1 == NULL || filter == NULL)
        { returnvalue = TRUE; }
    else
        {
        buffer2 = strcasestr( buffer1, filter );
        if ( buffer2 != NULL )
            { returnvalue =  TRUE; }
       else
            { returnvalue =  FALSE; }
       }

    free(buffer1);
    return returnvalue;
}


/*********************************************************************************************************
* callbacks
*/
// column clicked (title) -> sort
static void callback_column_sort(GtkTreeViewColumn *treeviewcolumn, gpointer data)
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

//Rombrowser right click context menu.
gboolean callback_rombrowser_context( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
    GtkTreeIter sourceiter, destinationiter;
    gboolean validiter;
    GdkPixbuf *flag;
    cache_entry *entry;
    short int counter;
    gchar *line[7];

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
        for ( counter = 0; counter < g_iNumRoms; ++counter )
            {
            if ( filter_function( source, &sourceiter, (gpointer)NULL) )
                 {
                 gtk_tree_model_get ( GTK_TREE_MODEL(source), &sourceiter, 0, &line[0], 1, &line[1], 2, &line[2], 3, &line[3], 4, &line[4], 5, &entry, 6, &flag, 7, &line[5], 8, &line[6], -1 );

                 gtk_list_store_append ( GTK_LIST_STORE(destination), &destinationiter );
                 gtk_list_store_set ( GTK_LIST_STORE(model), &destinationiter, 0, line[0], 1, line[1], 2, line[2], 3, line[3], 4, line[4], 5, entry, 6, flag, 7, line[5], 8, line[6], -1 );
                 }
            if(!gtk_tree_model_iter_next(source, &sourceiter))
                 { break; }
            }
        }

    for ( counter = 0; counter < 7; ++counter )
        {
        if(line[counter]!=NULL)
            { free(line[counter]); }
        }

    destination = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
    g_iNumRoms = gtk_tree_model_iter_n_children(destination, NULL);

    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(destination), g_iSortColumn,  rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(destination), g_iSortColumn, g_SortType );

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
    GtkWidget *filter;
    GtkToolItem *toolitem;
    GtkWidget *label;
    GtkWidget *Hbox;

    Hbox = gtk_hbox_new(FALSE, 0);
    toolitem = gtk_tool_item_new();
    gtk_tool_item_set_expand(toolitem, TRUE);

    filter = gtk_toolbar_new();
    gtk_toolbar_set_orientation( GTK_TOOLBAR(filter), GTK_ORIENTATION_HORIZONTAL );

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

    gtk_box_pack_start ( GTK_BOX(Hbox), label, FALSE, FALSE, 5);
    gtk_box_pack_start ( GTK_BOX(Hbox), g_MainWindow.filter, TRUE, TRUE, 5);

    gtk_container_add(GTK_CONTAINER(toolitem), Hbox);
    gtk_toolbar_insert ( GTK_TOOLBAR(filter), toolitem, 0);

    gtk_box_pack_start ( GTK_BOX(g_MainWindow.toplevelVBox), filter, FALSE, FALSE, 0 );
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










static void  callback_column_visible(GtkWidget *widget, int column)
{
    int i;
    gboolean visible = gtk_tree_view_column_get_visible(GTK_TREE_VIEW_COLUMN(g_MainWindow.column[column]));
    gtk_tree_view_column_set_visible(g_MainWindow.column[column], !visible);

    //Control if emptry header column is visibile.
    gtk_tree_view_column_set_visible(g_MainWindow.column[11], FALSE);
    for ( i = 0; i < 7; ++i )
        {
        if(gtk_tree_view_column_get_visible(GTK_TREE_VIEW_COLUMN(g_MainWindow.column[i])))
            { return; }
        }
    gtk_tree_view_column_set_visible(g_MainWindow.column[11], TRUE);
}

static void  callback_header_clicked(GtkWidget *widget, GdkEventButton *event, gpointer column)
{
    if(event->type==GDK_BUTTON_PRESS)
        {
        if(event->button==3) //Right click.
            {
            gtk_menu_popup( GTK_MENU(g_MainWindow.romHeaderMenu), NULL, NULL, NULL, NULL,
            event->button, event->time );
            }
        else if(event->button==1) //Left click.
            { callback_column_sort(column, NULL); }
        }
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
    gchar *titles[7] = 
        {
        (gchar*)tr("Country"),
        (gchar*)tr("Good Name"),
        (gchar*)tr("Size"),
        (gchar*)tr("Comments"),
        (gchar*)tr("File Name"),
        (gchar*)tr("Compression"),
        (gchar*)tr("Image Type"),
        };

    GtkListStore *store = gtk_list_store_new (9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, -1);

    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    GtkTreeViewColumn   *column;

    model = GTK_TREE_MODEL (store);

    renderer = gtk_cell_renderer_pixbuf_new ();
    g_MainWindow.column[0] = gtk_tree_view_column_new();
    column = g_MainWindow.column[0];
    gtk_tree_view_column_set_title(column, titles[0]); 
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", 6);
    g_object_set(renderer, "xpad", 5, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", 1);

    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 0);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[1] = gtk_tree_view_column_new_with_attributes (titles[1], renderer, "text", 0, NULL);
    column = g_MainWindow.column[1];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 1);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[2] = gtk_tree_view_column_new_with_attributes (titles[2], renderer, "text", 2, NULL);
    column = g_MainWindow.column[2];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 3);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 3);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[3] = gtk_tree_view_column_new_with_attributes (titles[3], renderer, "text", 3, NULL);
    column = g_MainWindow.column[3];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 4);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 4);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[4] = gtk_tree_view_column_new_with_attributes (titles[4], renderer, "text", 4, NULL);
    column = g_MainWindow.column[4];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 5);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 5);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[5] = gtk_tree_view_column_new_with_attributes(titles[5], renderer, "text", 7, NULL);
    column = g_MainWindow.column[5];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 7);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 7);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[6] = gtk_tree_view_column_new_with_attributes (titles[6], renderer, "text", 8, NULL);
    column = g_MainWindow.column[6];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 8);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 8);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[11] = gtk_tree_view_column_new();
    column = g_MainWindow.column[11];
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 12);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model );
    g_object_unref (model);

    g_MainWindow.romHeaderMenu = gtk_menu_new();

    int i;
    GtkWidget *item;
    for (i = 0; i < 7; ++i)
        {
        item = gtk_check_menu_item_new_with_label(titles[i]); 
        //This needs to be integrated with config system.
        if(i<5) 
             { gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE); }
        gtk_menu_append(GTK_MENU(g_MainWindow.romHeaderMenu), item);
        gtk_widget_show(item);
        g_signal_connect(item, "activate", G_CALLBACK(callback_column_visible), GUINT_TO_POINTER(i));
        }

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
    gtk_signal_connect ( GTK_OBJECT(refreshRomBrowserItem), "activate", GTK_SIGNAL_FUNC(call_rcs), (gpointer)NULL );

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
    gtk_signal_connect( GTK_OBJECT(g_MainWindow.romDisplay), "button-press-event", GTK_SIGNAL_FUNC(callback_rombrowser_context), (gpointer)rightClickMenu);

    return 0;
}
