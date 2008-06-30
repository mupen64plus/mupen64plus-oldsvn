/***************************************************************************
 romproperties.h - Handles rom properties dialog
----------------------------------------------------------------------------
Began                : Wed Nov 13 2002
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

#ifndef __ROMPROPERTIES_H__
#define __ROMPROPERTIES_H__

#include <gtk/gtk.h>

#include "../romcache.h"

typedef struct
{
    GtkWidget *dialog;
    // entries
    GtkWidget *filenameEntry;
    GtkWidget *goodnameEntry;
    GtkWidget *flag;
    GtkWidget *countryEntry;
    GtkWidget *status[5];
    GtkWidget *fullpathEntry;
    GtkWidget *crc1Entry;
    GtkWidget *crc2Entry;
    GtkWidget *md5Entry;
    GtkWidget *internalnameEntry;
    GtkWidget *sizeEntry;
    GtkWidget *savetypeEntry;
    GtkWidget *playersEntry;
    GtkWidget *compressiontypeEntry;
    GtkWidget *imagetypeEntry;
    GtkWidget *cicchipEntry;
    GtkWidget *rumbleEntry;
    GtkWidget *commentsEntry;
    GtkTreeIter iter;
    cache_entry *entry;
} SRomPropertiesDialog;

extern SRomPropertiesDialog g_RomPropDialog;

int create_romPropDialog();
void show_romPropDialog();

#endif // __ROMPROPERTIES_H__
