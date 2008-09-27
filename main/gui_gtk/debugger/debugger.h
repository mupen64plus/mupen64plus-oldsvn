/**
 * Mupen64Plus main/gui_gtk/debugger/debugger.h
 *
 * Copyright (C) 2002 DavFr - robind@esiee.fr
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

#ifndef GUIGTK_DEBUGGER_H
#define GUIGTK_DEBUGGER_H

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "../../../debugger/debugger.h"

#include "breakpoints.h"
#include "desasm.h"
#include "registers.h"
#include "regTLB.h"
#include "memedit.h"
#include "varlist.h"

#define DEBUGGER_VERSION "0.0.3"


extern GdkColor    color_modif,    // Color of modified register.
                   color_ident;    // Unchanged register.

extern GtkWidget   *winRegisters;

extern PangoFontDescription *debugger_font_desc;

#endif //GTK_DEBUGGER_H

