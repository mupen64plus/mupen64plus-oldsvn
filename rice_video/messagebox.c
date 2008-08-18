/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - messagebox.c                                            *
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h> 
#include <sys/stat.h>

#include <gtk/gtk.h>

#include "messagebox.h"
#include "support.h"

static char l_IconDir[PATH_MAX] = {'\0'};

int isfile(char *path)
{
    struct stat sbuf;
    return (stat(path, &sbuf) == 0) && S_ISREG(sbuf.st_mode);
}

static void get_icondir()
{
    char buf[2048];
    //Try to find mupen64plus' icon directory.
    int n = readlink("/proc/self/exe", buf, PATH_MAX);
    if (n > 0)
        {
        buf[n] = '\0';
        dirname(buf);
        strncpy(l_IconDir, buf, PATH_MAX);
        strncat(buf, "/icons/32x32/mupen64plus.png", PATH_MAX - strlen(buf));
        }
    //If it's not in the executable's directory, try a couple of default locations
    if (buf[0] == '\0' || !isfile(buf))
        {
        strcpy(l_IconDir, "/usr/local/share/mupen64plus");
        strcpy(buf, l_IconDir);
        strcat(buf, "/config/mupen64plus.conf");
        if(!isfile(buf))
            {
            strcpy(l_IconDir, "/usr/share/mupen64plus");
            strcpy(buf, l_IconDir);
            strcat(buf, "/icons/32x32/mupen64plus.png");
            // if install dir is not in the default locations, try the same dir as the binary
            if (!isfile(buf))
                {
                //try cwd as last resort
                getcwd(l_IconDir, PATH_MAX);
                }
            }
        }

    strcat(l_IconDir, "/icons/");
}

char* get_iconpath(char* iconfile)
{
    static char path[PATH_MAX];
    strncpy(path, l_IconDir, PATH_MAX-strlen(iconfile));
    strcat(path, iconfile);
    return path;
}

//Check for Gtk icons here.
gboolean check_icon_theme()
{
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    if(gtk_icon_theme_has_icon(theme, "document-open")&&
       gtk_icon_theme_has_icon(theme, "media-playback-start")&&
       gtk_icon_theme_has_icon(theme, "media-playback-pause")&&
       gtk_icon_theme_has_icon(theme, "media-playback-stop")&&
       gtk_icon_theme_has_icon(theme, "view-fullscreen")&&
       gtk_icon_theme_has_icon(theme, "preferences-system")&& 
       gtk_icon_theme_has_icon(theme, "video-display")&& 
       gtk_icon_theme_has_icon(theme, "audio-card")&& 
       gtk_icon_theme_has_icon(theme, "input-gaming")&&
       gtk_icon_theme_has_icon(theme, "dialog-warning")&&
       gtk_icon_theme_has_icon(theme, "dialog-error")&&
       gtk_icon_theme_has_icon(theme, "dialog-question"))
        return TRUE;
    else
        return FALSE;
}

static gint delete_question_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
   return TRUE; // undeleteable
}

static void
button1_clicked( GtkWidget *widget, gpointer data )
{
    int *ret = (int *)data;

    *ret = 1;
}

static void
button2_clicked( GtkWidget *widget, gpointer data )
{
    int *ret = (int *)data;

    *ret = 2;
}

static void
button3_clicked( GtkWidget *widget, gpointer data )
{
    int *ret = (int *)data;

    *ret = 3;
}

int
messagebox( const char *title, int flags, const char *fmt, ... )
{
    va_list ap;
    char buf[2048];
    int ret = 0;

    GtkWidget *dialog;
    GtkWidget *hbox;
    GtkWidget *icon = NULL;
    GtkWidget *label;
    GtkWidget *button1, *button2 = NULL, *button3 = NULL;

    va_start( ap, fmt );
    vsnprintf( buf, 2048, fmt, ap );
    va_end( ap );

    // check flags
    switch( flags & 0x000000FF )
    {
    case MB_ABORTRETRYIGNORE:
        button1 = gtk_button_new_with_label( "Abort" );
        button2 = gtk_button_new_with_label( "Retry" );
        button3 = gtk_button_new_with_label( "Ignore" );
        break;

    case MB_CANCELTRYCONTINUE:
        button1 = gtk_button_new_with_label( "Cancel" );
        button2 = gtk_button_new_with_label( "Retry" );
        button3 = gtk_button_new_with_label( "Continue" );
        break;

    case MB_OKCANCEL:
        button1 = gtk_button_new_with_label( "Ok" );
        button2 = gtk_button_new_with_label( "Cancel" );
        break;

    case MB_RETRYCANCEL:
        button1 = gtk_button_new_with_label( "Retry" );
        button2 = gtk_button_new_with_label( "Cancel" );
        break;

    case MB_YESNO:
        button1 = gtk_button_new_with_label( "Yes" );
        button2 = gtk_button_new_with_label( "No" );
        break;

    case MB_YESNOCANCEL:
        button1 = gtk_button_new_with_label( "Yes" );
        button2 = gtk_button_new_with_label( "No" );
        button3 = gtk_button_new_with_label( "Cancel" );
        break;

    case MB_OK:
    default:
        button1 = gtk_button_new_with_label( "Ok" );
    }

    dialog = gtk_dialog_new();
    gtk_container_set_border_width( GTK_CONTAINER(dialog), 10 );
    gtk_window_set_title( GTK_WINDOW(dialog), title );
    gtk_window_set_policy( GTK_WINDOW(dialog), 0, 0, 0 );
    gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                GTK_SIGNAL_FUNC(delete_question_event), (gpointer)NULL );

    hbox = gtk_hbox_new( FALSE, 5 );
    gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 0 );
    gtk_widget_show( hbox );

    // icon
    get_icondir();
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    GdkPixbuf *pixbuf;

    switch( flags & 0x00000F00 )
    {
    case MB_ICONWARNING:
        if(check_icon_theme())
            {
            pixbuf = gtk_icon_theme_load_icon(theme, "dialog-warning", 32,  0, NULL);
            icon = gtk_image_new_from_pixbuf(pixbuf); 
            }
        else
            icon = gtk_image_new_from_file(get_iconpath("32x32/dialog-warning.png"));
        break;

    case MB_ICONQUESTION:
    case MB_ICONINFORMATION:
       if(check_icon_theme())
            {
            pixbuf = gtk_icon_theme_load_icon(theme, "dialog-question", 32,  0, NULL);
            icon = gtk_image_new_from_pixbuf(pixbuf); 
            }
        else
            icon = gtk_image_new_from_file(get_iconpath("32x32/dialog-question.png"));
        break;

    case MB_ICONERROR:
        if(check_icon_theme())
            {
            pixbuf = gtk_icon_theme_load_icon(theme, "dialog-error", 32,  0, NULL);
            icon = gtk_image_new_from_pixbuf(pixbuf); 
            }
        else
            icon = gtk_image_new_from_file(get_iconpath("32x32/dialog-error.png"));
        break;
    }

    if( icon )
    {
        gtk_box_pack_start( GTK_BOX(hbox), icon, FALSE, FALSE, 0 );
        gtk_widget_show( icon );
    }

    label = gtk_label_new( buf );
    gtk_box_pack_start( GTK_BOX(hbox), label, TRUE, TRUE, 0 );
    gtk_widget_show( label );

    if( button1 )
    {
        gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->action_area), button1, TRUE, TRUE, 0 );
        gtk_widget_show( button1 );
        gtk_signal_connect( GTK_OBJECT(button1), "clicked",
                    GTK_SIGNAL_FUNC(button1_clicked), (gpointer)&ret );

    }
    if( button2 )
    {
        gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->action_area), button2, TRUE, TRUE, 0 );
        gtk_widget_show( button2 );
        gtk_signal_connect( GTK_OBJECT(button2), "clicked",
                    GTK_SIGNAL_FUNC(button2_clicked), (gpointer)&ret );
    }
    if( button3 )
    {
        gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->action_area), button3, TRUE, TRUE, 0 );
        gtk_widget_show( button3 );
        gtk_signal_connect( GTK_OBJECT(button3), "clicked",
                    GTK_SIGNAL_FUNC(button3_clicked), (gpointer)&ret );
    }

    gtk_widget_show( dialog );

    while( !ret )
        if( gtk_main_iteration() )
            break;

    gtk_widget_destroy( dialog );

    return ret;
}
