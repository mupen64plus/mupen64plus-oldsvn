/*
 * Mupen64Plus main/gui_gtk/debugger/registers.h
 * 
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

#ifndef GUIGTK_REGISTERS_H
#define GUIGTK_REGISTERS_H

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>


#include "regGPR.h"
#include "regSpecial.h"
#include "regCop0.h"
#include "regCop1.h"
#include "regAI.h"
#include "regPI.h"
#include "regRI.h"
#include "regSI.h"
#include "regVI.h"


int registers_opened;

void init_registers();
void update_registers();
GtkCList * init_hwreg_clist(int n, char **regnames);

#endif //GUIGTK_REGISTERS_H
