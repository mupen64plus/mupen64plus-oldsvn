/**
 * Mupen64 - varlist.h
 * Copyright (C) 2008 HyperHacker (at gmail, dot com)
 *
 * If you want to contribute to this part of the project please
 * contact DavFr or Hacktarux first.
 * 
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence.
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

#ifndef GUIGTK_VARLIST_H
#define GUIGTK_VARLIST_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "debugger.h"
#include "ui_clist_edit.h"

int varlist_opened, varlist_auto_update;

GtkWidget *winVarList;

void update_varlist();
void init_varlist();

#endif  // VARLIST_H
