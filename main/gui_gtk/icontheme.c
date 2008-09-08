/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - icontheme.c                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
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

/* icontheme.c - Checks current gtk icon theme for application icons. */

#include <gtk/gtk.h>

#include "main_gtk.h"

#include "../main.h"

/* Does the current iconset have all the icons the application (plugin authors, 
 * add requests here) need? If icons could use themed icons, but fallbacks would 
 * be okay (non-standard themed icons), don't check here, and use a force flag of
 * TRUE in your set_icon() calls.
 */
gboolean check_icon_theme()
{
    if(gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-start")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-pause")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "media-playback-stop")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "view-fullscreen")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "preferences-system")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-warning")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-error")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "dialog-question")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "video-display")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "audio-card")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "input-gaming")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "window-close")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "document-save")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "document-save-as")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "document-revert")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "document-open")&&
       gtk_icon_theme_has_icon(g_MainWindow.iconTheme, "gtk-about"))
        return TRUE;
    else
        return FALSE;
}

/* Set image to a themed icon of size from gtk iconset or NULL unless using fallback. 
 * Force allows for the use of packaged icons with a gtk iconset.
 */
void set_icon(GtkWidget* image, const gchar* icon, int size, gboolean force)
{
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    if(pixbuf)
        g_object_unref(pixbuf);

    if(g_MainWindow.fallbackIcons&&!force)
        pixbuf = gtk_icon_theme_load_icon(g_MainWindow.iconTheme, icon, size,  0, NULL);
    else
        {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%dx%d/%s.png", size, size, icon);
        pixbuf = gdk_pixbuf_new_from_file(get_iconpath(buffer), NULL);
        }
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
}
