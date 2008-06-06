/*
 * Mupen64Plus main/gui_gtk/debugger/regRI.c
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

#include "regRI.h"


static GtkWidget *clRegRI;


// We keep a copy of values displayed to screen.
// Screen is only updated when values REALLY changed, and display need an
// update. It makes things go really faster :)
static uint32   gui_fantom_reg_RI[5];

static char *mnemonicRI[]=
{
    "RI_MODE_REG",      "RI_CONFIG_REG",
    "RI_CURRENT_LOAD_REG",  "RI_SELECT_REG",
    "RI_REFRESH_REG",
};

static unsigned int *regptrsRI[] = {
    &ri_register.ri_mode,
    &ri_register.ri_config,
    &ri_register.ri_current_load,
    &ri_register.ri_select,
    &ri_register.ri_refresh
};

//]=-=-=-=-=-=-=-=[ Initialisation of RDRAM Interface Display ]=-=-=-=-=-=-=-=[

void init_regRI()
{
    int i;

    frRegRI = gtk_frame_new("RDRAM Interface");

    //=== Creation of Registers Value Display ==========/
    clRegRI = init_hwreg_clist(5, mnemonicRI);
    gtk_container_add(GTK_CONTAINER(frRegRI), clRegRI);
    gtk_clist_set_selection_mode(GTK_CLIST(clRegRI), GTK_SELECTION_SINGLE);

    //=== Fantom Registers Initialisation ============/
    for( i=0; i<5; i++)
    {
        gui_fantom_reg_RI[i] = 0x12345678;
    }
}




//]=-=-=-=-=-=-=-=-=-=[ Mise-a-jour RDRAM Interface Display ]=-=-=-=-=-=-=-=-=-[

void update_regRI()
{
    char txt[24];
    int i;
    
    gtk_clist_freeze(GTK_CLIST(clRegRI));

    for (i=0; i<5; i++) {
        if (gui_fantom_reg_RI[i] != (uint32)(*regptrsRI[i])) {
            gui_fantom_reg_RI[i] = (uint32)(*regptrsRI[i]);
            sprintf(txt, "%.8lX", *regptrsRI[i]);
            gtk_clist_set_text(GTK_CLIST(clRegRI), i, 1, txt);
            gtk_clist_set_background(GTK_CLIST(clRegRI), i, &color_modif);
        } else {
            gtk_clist_set_background(GTK_CLIST(clRegRI), i, &color_ident);
        }
    }
    
    gtk_clist_thaw(GTK_CLIST(clRegRI));
}
