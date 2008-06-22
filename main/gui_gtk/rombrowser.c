/***************************************************************************
rombrowser.c - Handles rombrowser GUI elements
----------------------------------------------------------------------------
Began                : Sat Nov 9 2002
Copyright            : (C) 2002 by blight, 2008 by Tillin9
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

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*********************************************************************************************************
* globals
*/
GtkWidget *playRomItem;
GtkWidget *romPropertiesItem;
GtkWidget *refreshRomBrowserItem;
GdkPixbuf *australia, *europe, *france, *germany, *italy, *japan, *spain, *usa, *japanusa, *n64cart, *star;

char *column_names[16] =
    {
    "Country",
    "Good Name",
    "Status",
    "User Comments",
    "File Name",
    "MD5 Hash",
    "CRC1",
    "CRC2",
    "Internal Name",
    "Save Type",
    "Players",
    "Size",
    "Compression",
    "Image Type",
    "CIC Chip",
    "Rumble",
    };

void countrycodeflag(unsigned short int countrycode, GdkPixbuf **flag)
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

gint return_zero( GtkTreeModel *model, GtkTreeIter *ptr1, GtkTreeIter *ptr2, gpointer user_data )
{
    return 0;
}

// compare function
gint rombrowser_compare( GtkTreeModel *model, GtkTreeIter *ptr1, GtkTreeIter *ptr2, gpointer user_data )
{
    short returnvalue;

    if(g_MainWindow.romSortColumn==2||g_MainWindow.romSortColumn==11||g_MainWindow.romSortColumn==14)
        {
        cache_entry *entry1;
        cache_entry *entry2;
        int item1, item2;

        gtk_tree_model_get ( model, ptr1, 22, &entry1, -1 );
        gtk_tree_model_get ( model, ptr2, 22, &entry2, -1 );

        if(g_MainWindow.romSortColumn==2)
            {
            item1 = entry1->inientry->status;
            item2 = entry2->inientry->status;
            }
        else if(g_MainWindow.romSortColumn==11)
            {
            item1 = entry1->romsize;
            item2 = entry2->romsize;
            }
        else if(g_MainWindow.romSortColumn==14)
            {
            item1 = entry1->cic;
            item2 = entry2->cic;
            }

        if(item1==item2)
            { returnvalue = 0; }
        else if(item1>item2)
            { returnvalue = -1; }
        else
            { returnvalue = 1; }
        }
    else
        {
        gchar *buffer1, *buffer2;

        gtk_tree_model_get ( model, ptr1, g_MainWindow.romSortColumn, &buffer1, -1 );
        gtk_tree_model_get ( model, ptr2, g_MainWindow.romSortColumn, &buffer2, -1 );

        if (buffer1 == NULL && buffer2 == NULL)
            returnvalue = 0;
        else if (buffer1 == NULL)
            returnvalue = 1;
        else if (buffer2 == NULL)
            returnvalue = -1;
        else
            returnvalue = strcasecmp(buffer1, buffer2);

        if (buffer1)
            g_free(buffer1);
        if (buffer2)
            g_free(buffer2);
        }

    //If items are equal, fallback on Good Name and then File Name.
    if(returnvalue==0&&g_MainWindow.romSortColumn!=1)
        {
        gchar *buffer1, *buffer2;

        gtk_tree_model_get ( model, ptr1, 1, &buffer1, -1 );
        gtk_tree_model_get ( model, ptr2, 1, &buffer2, -1 );

        if (buffer1 == NULL && buffer2 == NULL)
            returnvalue = 0;
        else if (buffer1 == NULL)
            returnvalue = 1;
        else if (buffer2 == NULL)
            returnvalue = -1;
        else
            returnvalue = strcasecmp(buffer1, buffer2);

        if (buffer1)
            g_free(buffer1);
        if (buffer2)
            g_free(buffer2);
        }

    if(returnvalue==0&&g_MainWindow.romSortColumn!=4)
        {
        gchar *buffer1, *buffer2;

        gtk_tree_model_get ( model, ptr1, 4, &buffer1, -1 );
        gtk_tree_model_get ( model, ptr2, 4, &buffer2, -1 );

        if (buffer1 == NULL && buffer2 == NULL)
            returnvalue = 0;
        else if (buffer1 == NULL)
            returnvalue = 1;
        else if (buffer2 == NULL)
            returnvalue = -1;
        else
            returnvalue = strcasecmp(buffer1, buffer2);

        if (buffer1)
            g_free(buffer1);
        if (buffer2)
            g_free(buffer2);
        }

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
    g_romcache.rcstask = RCS_RESCAN;
}

//Load a fresh TreeView after re-scanning directories.
void rombrowser_refresh( unsigned int roms, unsigned short clear )
{
    int arrayroms;
    GtkTreeModel *model;

    if(clear)
        {
        GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
        gtk_tree_selection_select_all(selection);
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romFullList) );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), g_MainWindow.romSortColumn,  return_zero, (gpointer)NULL, (gpointer)NULL );
        gtk_list_store_clear( GTK_LIST_STORE(model) );
        selection = gtk_tree_view_get_selection (  GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_tree_selection_select_all(selection);
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), g_MainWindow.romSortColumn,  return_zero, (gpointer)NULL, (gpointer)NULL );
        gtk_list_store_clear( GTK_LIST_STORE(model) );
        }

    model =  gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList));
    arrayroms = gtk_tree_model_iter_n_children(model, NULL); 

    if(roms>arrayroms)
        {
        gboolean fullpaths;

        GtkTreeIter *iter = (GtkTreeIter *)malloc(sizeof(GtkTreeIter));
        char *country, *goodname, *usercomments, *filename, *md5hash, *crc1, *crc2, *internalname, *savetype, *players, *size, *compressiontype, *imagetype, *cicchip, *rumble;
        GdkPixbuf *flag;
        GdkPixbuf *status[5];
        unsigned int romcounter;
        int counter;

        country = (char*)calloc(32,sizeof(char));
        md5hash = (char*)calloc(33,sizeof(char));
        crc1 = (char*)calloc(9,sizeof(char));
        crc2 = (char*)calloc(9,sizeof(char));
        savetype = (char*)calloc(16,sizeof(char));
        players = (char*)calloc(16,sizeof(char));
        size = (char*)calloc(16,sizeof(char));
        compressiontype = (char*)calloc(16,sizeof(char));
        imagetype = (char*)calloc(32,sizeof(char));
        cicchip = (char*)calloc(16,sizeof(char));
        rumble = (char*)calloc(8,sizeof(char));

        if(iter==NULL||country==NULL||md5hash==NULL||crc1==NULL||crc2==NULL||size==NULL||compressiontype==NULL||imagetype==NULL||cicchip==NULL)
            {
            fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ ); 
            return;
            }

        fullpaths = config_get_bool( "RomBrowserShowFullPaths", FALSE);
        cache_entry *entry;
        entry = g_romcache.top;

        for ( romcounter=0; romcounter < arrayroms; ++romcounter )
            {
            entry = entry->next;
            if((entry==NULL))
                { printf("NULL\n"); return; }
            }

        for ( romcounter=0; (romcounter<roms)&&(entry!=NULL); ++romcounter )
          {
          countrycodestring(entry->countrycode, country);
          countrycodeflag(entry->countrycode, &flag);
          goodname = entry->inientry->goodname;

          for ( counter = 0; counter < 5; ++counter )
                {
                if(entry->inientry->status>counter)
                    { status[counter] = star; }
                else
                    { status[counter] = NULL; }
                }
            usercomments = entry->usercomments;
            if(fullpaths)
                { filename = entry->filename; }
            else
                { filename = filefrompath(entry->filename); }

            for ( counter = 0; counter < 16; ++counter ) 
                { sprintf(md5hash+counter*2, "%02X", entry->md5[counter]); }
            sprintf(crc1, "%08X", entry->crc1);
            sprintf(crc2, "%08X", entry->crc2);
            internalname=entry->internalname;
            savestring(entry->inientry->savetype, savetype);
            playersstring(entry->inientry->players, players); 
            sprintf(size, "%.1f MBits", (float)(entry->romsize / (float)0x20000) );
            compressionstring(entry->compressiontype, compressiontype);
            imagestring(entry->imagetype, imagetype);
            cicstring(entry->cic, cicchip);
            rumblestring(entry->inientry->rumble, rumble);

            //Add entries to TreeModel
            model =  gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);

            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, country, 1, goodname, 2, NULL, 3, usercomments, 4, filename, 5, md5hash, 6, crc1, 7, crc2, 8, internalname, 9, savetype, 10, players, 11, size, 12, compressiontype, 13, imagetype, 14, cicchip, 15, rumble, 16, status[0], 17, status[1], 18, status[2], 19, status[3], 20, status[4], 21, flag, 22, entry, -1);

            model = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romDisplay));
            gtk_list_store_append ( GTK_LIST_STORE(model), iter);
            gtk_list_store_set ( GTK_LIST_STORE(model), iter, 0, country, 1, goodname, 2, NULL, 3, usercomments, 4, filename, 5, md5hash, 6, crc1, 7, crc2, 8, internalname, 9, savetype, 10, players, 11, size, 12, compressiontype, 13, imagetype, 14, cicchip, 15, rumble, 16, status[0], 17, status[1], 18, status[2], 19, status[3], 20, status[4], 21, flag, 22, entry, -1);

            //printf("Added: %s\n", goodname);
            entry = entry->next;
            }

        free(country);
        free(md5hash);
        free(crc1);
        free(crc2);
        free(savetype);
        free(players);
        free(size);
        free(compressiontype);
        free(imagetype);
        free(cicchip);
        free(rumble);
        if(fullpaths==1)
           { free(filename); }
        free(iter);
        //Do an initial sort.
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(model), g_MainWindow.romSortColumn, rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
        gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(model), g_MainWindow.romSortColumn, g_MainWindow.romSortType );
        gtk_tree_view_set_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay), model );
        }
}

//We're going our own filtering, but this is a GtkTreeModelFilter function.
//We do filtering manually since GtkTreeModelFilter can not implement automatic 
//sorting and GtkTreeModelSort can't implement automatic filtering.
gboolean filter_function( GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    const gchar *filter;
    filter = gtk_entry_get_text ( GTK_ENTRY(g_MainWindow.filter) );
    //printf("Filter: %s\n", filter);

    if(g_MainWindow.romSortColumn==2)
        {
        cache_entry *entry;
        gtk_tree_model_get ( model, iter, 22, &entry, -1 );

        if(filter[0]=='\0')
            { return TRUE; }

        if(atoi(filter)==entry->inientry->status)
            { return TRUE; }
        else
            { return FALSE; }
        }

    char *buffer1, *buffer2;
    gboolean returnvalue;

    gtk_tree_model_get (model, iter, g_MainWindow.romSortColumn, &buffer1, -1);
    //printf("Value %s\n", buffer1);
    if(buffer1==NULL||filter==NULL)
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

    if( g_MainWindow.romSortColumn == gtk_tree_view_column_get_sort_column_id(treeviewcolumn) )
        { g_MainWindow.romSortType = ( g_MainWindow.romSortType == GTK_SORT_ASCENDING ) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING; }
    else
        {
        g_MainWindow.romSortColumn = gtk_tree_view_column_get_sort_column_id(treeviewcolumn); 
        g_MainWindow.romSortType = GTK_SORT_ASCENDING;
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
    gtk_tree_model_get (model, &iter, 22, &entry, -1);

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
    int g_iNumRoms;
    static short resort = 0;
    GtkTreeModel *destination;
    const gchar *filter;

    filter = gtk_entry_get_text ( GTK_ENTRY(g_MainWindow.filter) );
    if(filter[0]!='\0'||resort==1)
        {
        if(filter[0]!='\0')
            { resort=1; }
        else
            { resort=0; }

        GtkTreeModel *model, *source;
        GtkTreeIter sourceiter, destinationiter;
        gboolean validiter;

        char *country, *goodname, *usercomments, *filename, *md5hash, *crc1, *crc2, *internalname, *savetype, *players, *size, *compressiontype, *imagetype, *cicchip, *rumble;
        GdkPixbuf *status[5];
        GdkPixbuf *flag;
        cache_entry *entry; 
        short int counter;

        //Clear the Display Tree View.
        GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_tree_selection_select_all(selection);
        model = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_list_store_clear( GTK_LIST_STORE(model) );

        source = gtk_tree_view_get_model(GTK_TREE_VIEW(g_MainWindow.romFullList) );
        g_iNumRoms = gtk_tree_model_iter_n_children(source, NULL);
        validiter = gtk_tree_model_get_iter_first(source, &sourceiter);

        destination = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(destination), g_MainWindow.romSortColumn,  return_zero, (gpointer)NULL, (gpointer)NULL );

        if(validiter)
            {
            for ( counter = 0; counter < g_iNumRoms; ++counter )
                {
                if ( filter_function( source, &sourceiter, (gpointer)NULL) )
                    {
                    gtk_tree_model_get ( GTK_TREE_MODEL(source), &sourceiter, 0, &country, 1, &goodname, 3, &usercomments, 4, &filename, 5, &md5hash, 6, &crc1, 7, &crc2, 8, &internalname, 9, &savetype, 10, &players, 11, &size, 12, &compressiontype, 13, &imagetype, 14, &cicchip, 15, &rumble, 16, &status[0], 17, &status[1], 18, &status[2], 19, &status[3], 20, &status[4], 21, &flag, 22, &entry, -1);

                    gtk_list_store_append ( GTK_LIST_STORE(destination), &destinationiter);
                    gtk_list_store_set ( GTK_LIST_STORE(destination), &destinationiter, 0, country, 1, goodname, 2, NULL, 3, usercomments, 4, filename, 5, md5hash, 6, crc1, 7, crc2, 8, internalname, 9, savetype, 10, players, 11, size, 12, compressiontype, 13, imagetype, 14, cicchip, 15, rumble, 16, status[0], 17, status[1], 18, status[2], 19, status[3], 20, status[4], 21, flag, 22, entry, -1);
                    }
                if(!gtk_tree_model_iter_next(source, &sourceiter))
                     { break; }
                }
           }

        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(destination), g_MainWindow.romSortColumn,  rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
        gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(destination), g_MainWindow.romSortColumn, g_MainWindow.romSortType );
        }
    else
        {
        destination = gtk_tree_view_get_model ( GTK_TREE_VIEW(g_MainWindow.romDisplay) );
        gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(destination), g_MainWindow.romSortColumn,  rombrowser_compare, (gpointer)NULL, (gpointer)NULL );
        gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(destination), g_MainWindow.romSortColumn, g_MainWindow.romSortType );
        }

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
    GtkToolItem *toolitem;
    GtkWidget *label;
    GtkWidget *Hbox;

    Hbox = gtk_hbox_new(FALSE, 0);
    toolitem = gtk_tool_item_new();
    gtk_tool_item_set_expand(toolitem, TRUE);

    g_MainWindow.filterBar = gtk_toolbar_new();
    gtk_toolbar_set_orientation( GTK_TOOLBAR(g_MainWindow.filterBar), GTK_ORIENTATION_HORIZONTAL );

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
    gtk_toolbar_insert ( GTK_TOOLBAR(g_MainWindow.filterBar), toolitem, 0);

    gtk_box_pack_start ( GTK_BOX(g_MainWindow.toplevelVBox), g_MainWindow.filterBar, FALSE, FALSE, 0 );
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
    gtk_tree_model_get(model, &iter, 22, &entry, -1);

    g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list);

    if(open_rom( entry->filename ) == 0)
        { startEmulation(); }
}

static void callback_column_visible(GtkWidget *widget, int column)
{
    int i;
    char buffer1[32], buffer2[32];

    gboolean visible = gtk_tree_view_column_get_visible(GTK_TREE_VIEW_COLUMN(g_MainWindow.column[column]));
    gtk_tree_view_column_set_visible(g_MainWindow.column[column], !visible);
    g_MainWindow.columnVisible[column]=!g_MainWindow.columnVisible[column];
    strncpy(buffer1,column_names[column],31);
    buffer1[31]='\0';
    snprintf(buffer2, 31, "Column%sVisible", strnstrip(buffer1,31));

    config_put_bool(buffer2,g_MainWindow.columnVisible[column]); 

    //Control if emptry header column is visibile.
    gtk_tree_view_column_set_visible(g_MainWindow.column[16], FALSE);
    for ( i = 0; i < 16; ++i )
        {
        if(g_MainWindow.columnVisible[i])
            { return; }
        }
    gtk_tree_view_column_set_visible(g_MainWindow.column[16], TRUE);
}

static void callback_header_clicked(GtkWidget *widget, GdkEventButton *event, gpointer column)
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
        GList *list = NULL, *llist = NULL;
        list = gtk_tree_selection_get_selected_rows (selection, &model);
        llist = g_list_first (list);

        gtk_tree_model_get_iter (model, &g_RomPropDialog.iter,(GtkTreePath *) llist->data);
        gtk_tree_model_get (model, &g_RomPropDialog.iter, 22, &g_RomPropDialog.entry, -1);

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);

        show_romPropDialog();
        }
}

static void setup_view (GtkWidget *view)
{
    GtkListStore *store = gtk_list_store_new (23, 
    G_TYPE_STRING, //Country 0
    G_TYPE_STRING, //Good Name 1
    GDK_TYPE_PIXBUF, //NULL for padding... 2
    G_TYPE_STRING, //User Comments 3
    G_TYPE_STRING, //File Name 4
    G_TYPE_STRING, //MD5 Hash 5
    G_TYPE_STRING, //CRC1 6
    G_TYPE_STRING, //CRC2 7
    G_TYPE_STRING, //Internal Name 8
    G_TYPE_STRING, //Save Type 9
    G_TYPE_STRING, //Players 10
    G_TYPE_STRING, //Size 11
    G_TYPE_STRING, //Compression 12
    G_TYPE_STRING, //Image Type 13
    G_TYPE_STRING, //CIC Chip 14
    G_TYPE_STRING, //Rumble 15
    GDK_TYPE_PIXBUF, //Pixbufs for Status stars. 16,17,18,19,20
    GDK_TYPE_PIXBUF, 
    GDK_TYPE_PIXBUF, 
    GDK_TYPE_PIXBUF, 
    GDK_TYPE_PIXBUF,  
    GDK_TYPE_PIXBUF, //Flag 21
    G_TYPE_POINTER, //Entry 22
    -1); 

    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    GtkTreeViewColumn   *column;
    int i;
    model = GTK_TREE_MODEL (store);
    char buffer1[32], buffer2[32];

    renderer = gtk_cell_renderer_pixbuf_new ();
    g_MainWindow.column[0] = gtk_tree_view_column_new();
    column = g_MainWindow.column[0];
    gtk_tree_view_column_set_title(column, tr(column_names[0])); 
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", 21);
    g_object_set(renderer, "xpad", 5, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 0);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[1] = gtk_tree_view_column_new_with_attributes (tr(column_names[1]), renderer, "text", 1, NULL);
    column = g_MainWindow.column[1];
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 1);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    g_MainWindow.column[2] = gtk_tree_view_column_new();
    column = g_MainWindow.column[2];
    gtk_tree_view_column_set_title(column, tr(column_names[2])); 
    renderer = gtk_cell_renderer_pixbuf_new ();
    g_object_set(renderer, "xpad", 2, NULL);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", 2);
    for ( i = 0; i < 5; ++i )
        {
        renderer = gtk_cell_renderer_pixbuf_new ();
        gtk_tree_view_column_pack_start(column, renderer, FALSE);
        gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", 16+i);
        }
    renderer = gtk_cell_renderer_pixbuf_new ();
    g_object_set(renderer, "xpad", 2, NULL);
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", 2);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 2);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    for ( i = 3; i < 16; ++i )
        {
        renderer = gtk_cell_renderer_text_new ();
        g_MainWindow.column[i] = gtk_tree_view_column_new_with_attributes (tr(column_names[i]), renderer, "text", i, NULL);
        column = g_MainWindow.column[i];
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_column_set_reorderable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, i);
        gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, i);
        gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);
        }

    renderer = gtk_cell_renderer_text_new ();
    g_MainWindow.column[16] = gtk_tree_view_column_new();
    column = g_MainWindow.column[16];
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_insert_column(GTK_TREE_VIEW (view), column, 2);
    gtk_signal_connect( GTK_OBJECT(column->button), "button-press-event", G_CALLBACK(callback_header_clicked), column);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model );
    g_object_unref (model);

    g_MainWindow.romHeaderMenu = gtk_menu_new();

    GtkWidget *item;

    for (i = 0; i < 16; ++i)
        {
        strncpy(buffer1,column_names[i],31);
        buffer1[31]='\0';
        item = gtk_check_menu_item_new_with_label(tr(buffer1)); 
        snprintf(buffer2, 31, "Column%sVisible", strnstrip(buffer1,31));
        if((g_MainWindow.columnVisible[i]=config_get_bool(buffer2,2))==2)
             {
             if(i<5) 
                 { g_MainWindow.columnVisible[i] = TRUE; }
             else
                 { g_MainWindow.columnVisible[i] = FALSE; }
             config_put_bool(buffer2,g_MainWindow.columnVisible[i]);
             }
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), g_MainWindow.columnVisible[i]); 
        gtk_tree_view_column_set_visible(g_MainWindow.column[i], g_MainWindow.columnVisible[i]);
        gtk_menu_append(GTK_MENU(g_MainWindow.romHeaderMenu), item);
        gtk_widget_show(item);
        g_signal_connect(item, "activate", G_CALLBACK(callback_column_visible), GUINT_TO_POINTER(i));
        }

    gtk_tree_view_column_set_visible(g_MainWindow.column[16], FALSE);
    for ( i = 0; i < 16; ++i )
        {
        if(g_MainWindow.columnVisible[i])
            { return; }
        }
    gtk_tree_view_column_set_visible(g_MainWindow.column[16], TRUE);
}

/*********************************************************************************************************
* gui functions
*/
//Create rom browser.
int create_romBrowser( void )
{
    GtkWidget *rightClickMenu;
    GtkWidget *separatorItem;

    GtkIconTheme *theme = gtk_icon_theme_get_default();
    if(gtk_icon_theme_has_icon(theme, "emblem-new"))
        { star = gtk_icon_theme_load_icon(theme, "emblem-new", 16, 0, NULL); }
    else
        { star = gdk_pixbuf_new_from_file( get_iconpath("16x16/star.png"), NULL); }

    australia = gdk_pixbuf_new_from_file( get_iconpath("australia.png"), NULL);
    europe = gdk_pixbuf_new_from_file( get_iconpath("europe.png"), NULL);
    france = gdk_pixbuf_new_from_file( get_iconpath("france.png"), NULL);
    germany = gdk_pixbuf_new_from_file( get_iconpath("germany.png"), NULL);
    italy = gdk_pixbuf_new_from_file( get_iconpath("italy.png"), NULL);
    japan = gdk_pixbuf_new_from_file( get_iconpath("japan.png"), NULL);
    spain = gdk_pixbuf_new_from_file( get_iconpath("spain.png"), NULL);
    usa = gdk_pixbuf_new_from_file( get_iconpath("usa.png"), NULL);
    japanusa = gdk_pixbuf_new_from_file( get_iconpath("japanusa.png"), NULL);
    n64cart = gdk_pixbuf_new_from_file( get_iconpath("n64cart.xpm"), NULL);

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
    gtk_tree_view_set_reorderable   (GTK_TREE_VIEW(g_MainWindow.romDisplay), FALSE );
 
    gtk_signal_connect( GTK_OBJECT(g_MainWindow.romDisplay), "row-activated", GTK_SIGNAL_FUNC(callback_rowSelected), (gpointer)NULL );
    gtk_signal_connect( GTK_OBJECT(g_MainWindow.romDisplay), "button-press-event", GTK_SIGNAL_FUNC(callback_rombrowser_context), (gpointer)rightClickMenu);

    return 0;
}
