/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main_gtk.h                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
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

/* main_gtk.c - Handles the main window and 'glues' it with other windows */

#ifndef __MAIN_GTK_H__
#define __MAIN_GTK_H__

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget* window;
    GtkWidget* toplevelVBox;  /* Topevel vbox containing menubar, toolbar, filter, rombrowser, and statusbar. */

    GtkWidget* dialogErrorImage;
    GtkWidget* dialogQuestionImage;

    GtkWidget* menuBar;
    GtkWidget* openMenuImage;

    GtkWidget* openRomMenuImage;
    GtkWidget* closeRomMenuImage;
    GtkWidget* quitMenuImage;

    GtkWidget* playMenuImage;
    GtkWidget* pauseMenuImage;
    GtkWidget* stopMenuImage;
    GtkWidget* saveStateMenuImage;
    GtkWidget* saveStateAsMenuImage;
    GtkWidget* loadStateMenuImage;
    GtkWidget* loadStateAsMenuImage;

    GtkWidget* configureMenuImage;
    GtkWidget* graphicsMenuImage;
    GtkWidget* audioMenuImage;
    GtkWidget* inputMenuImage;
    GtkWidget* rspMenuImage;
    GtkWidget* fullscreenMenuImage;

    GtkWidget* aboutMenuImage;

    GtkWidget* toolBar;
    GtkWidget* openButtonImage;
    GtkWidget* playButtonImage;
    GtkWidget* pauseButtonImage;
    GtkWidget* stopButtonImage;
    GtkWidget* saveStateButtonImage;
    GtkWidget* loadStateButtonImage;
    GtkWidget* fullscreenButtonImage;
    GtkWidget* configureButtonImage;

    GtkWidget* filterBar; /* Container object for filter, toggle visability. */
    GtkWidget* filter; /* Entry for filter text. */
    GtkAccelGroup* accelGroup;
    GtkAccelGroup* accelUnsafe; /* GtkAccelGroup for keys without Metas. Prevents GtkEntry widgets. */
    gboolean accelUnsafeActive; /* From getting keypresses, so must be deactivated. */

    GtkWidget* romScrolledWindow;
    /* Make two TreeViews, a visable manually filtered one for the Display, and a
    Non-visable FullList from which we can filter. */
    GtkWidget* romDisplay;
    GtkWidget* romFullList;
    GtkTreeViewColumn* column[17]; /* Column pointers in rombrowser. */
    char column_names[16][2][128]; /* Localized names and English config system strings. */
    int romSortColumn;
    GtkSortType romSortType;
    GtkWidget* romHeaderMenu; /* Context menu for rombrowser to control visible columns. */
    GtkWidget* playRomItem;
    GtkWidget* romPropertiesItem;

    GtkWidget* playRombrowserImage;
    GtkWidget* propertiesRombrowserImage;
    GtkWidget* refreshRombrowserImage;

    GtkWidget* statusBar; /* Statusbar message stack. */
    GtkWidget* statusBarHBox; /* Container object for statusbar, toggle visability. */
} SMainWindow;

extern SMainWindow g_MainWindow;
extern GdkPixbuf *australia, *europe, *france, *germany, *italy, *japan, *spain, *usa, *japanusa, *n64cart, *star, *staroff;

/* public helper functions. */
void set_icon(GtkWidget* image, const gchar* icon, int size, gboolean force);

/* The functons which all GUIs must implement. */
void gui_init(int* argc, char*** argv);
void gui_display(void);
void gui_main_loop(void);
int gui_message(unsigned char messagetype, const char *format, ...);
void update_rombrowser(unsigned int roms, unsigned short clear);

/* View defines */
#define TOOLBAR 1
#define FILTER 2
#define STATUSBAR 3

#endif /* __MAIN_GTK_H__ */
