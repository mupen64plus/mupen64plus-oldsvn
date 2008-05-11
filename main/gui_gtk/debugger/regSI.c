/*
 * debugger/regSI.c
 * 
 * 
 * Debugger for Mupen64 - davFr
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64 is copyrighted (C) 2002 Hacktarux
 * Mupen64 homepage: http://mupen64.emulation64.com
 *         email address: hacktarux@yahoo.fr
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

#include "regSI.h"


static GtkWidget *clRegSI;


// We keep a copy of values displayed to screen.
// Screen is only updated when values REALLY changed, and display need an
// update. It makes things go really faster :)
static uint32   gui_fantom_reg_SI[4];

static char *mnemonicSI[]=
{
    "SI_DRAM_ADDR_REG",     "SI_PIF_ADDR_RD64B_REG",
    "SI_PIF_ADDR_WR64B_REG",    "SI_STATUS_REG",
};

static unsigned int *regptrsSI[] = {
    &si_register.si_dram_addr,
    &si_register.si_pif_addr_rd64b,
    &si_register.si_pif_addr_wr64b,
    &si_register.si_stat
};

//]=-=-=-=-=-=-=-=[ Initialisation of Serial Interface Display ]=-=-=-=-=-=-=-[

void init_regSI()
{
    int i;

    frRegSI = gtk_frame_new("Serial Interface");

    //=== Creation of Registers Value Display ========/
    clRegSI = init_hwreg_clist(4, mnemonicSI);
    gtk_container_add(GTK_CONTAINER(frRegSI), clRegSI);
    gtk_clist_set_selection_mode(GTK_CLIST(clRegSI), GTK_SELECTION_SINGLE);
    
    //=== Fantom Registers Initialisation ============/
    for( i=0; i<4; i++)
    {
        gui_fantom_reg_SI[i] = 0x12345678;
        //Should be put to the least probable value.
    }
}




//]=-=-=-=-=-=-=-=-=-=[ Mise-a-jour Serial Interface Display ]=-=-=-=-=-=-=-=-=[

void update_regSI()
{
    char txt[24];
    int i;
    
    gtk_clist_freeze(GTK_CLIST(clRegSI));

    for (i=0; i<4; i++) {
        if (gui_fantom_reg_SI[i] != (uint32)(*regptrsSI[i])) {
            gui_fantom_reg_SI[i] = (uint32)(*regptrsSI[i]);
            sprintf(txt, "%.8lX", *regptrsSI[i]);
            gtk_clist_set_text(GTK_CLIST(clRegSI), i, 1, txt);
            gtk_clist_set_background(GTK_CLIST(clRegSI), i, &color_modif);
        } else {
            gtk_clist_set_background(GTK_CLIST(clRegSI), i, &color_ident);
        }
    }
    
    gtk_clist_thaw(GTK_CLIST(clRegSI));
}
