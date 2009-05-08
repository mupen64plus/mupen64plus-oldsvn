/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Copyright (C) 2009 icepir8                                            *
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

#ifndef __RDP_H
#define __RDP_H

#define CMD0 rdp_reg.cmd0
#define CMD1 rdp_reg.cmd1
#define ADDR (rdp_reg.pc[rdp_reg.pc_i] - 8)



//////////////////////////////////////////////////////////////////////////////
// UC4                                                                      //
//////////////////////////////////////////////////////////////////////////////
#define G_MW_MATRIX     0x00    /* NOTE: also used by movemem */
#define G_MW_NUMLIGHT   0x02
#define G_MW_CLIP       0x04
#define G_MW_SEGMENT    0x06
#define G_MW_FOG        0x08
#define G_MW_LIGHTCOL   0x0a
#define G_MW_FORCEMTX   0x0c
#define G_MW_PERSPNORM  0x0e


#define G_MV_MMTX       2   
#define G_MV_PMTX       6
#define G_MV_VIEWPORT   8
#define G_MV_LIGHT      10
#define G_MV_POINT      12
#define G_MV_MATRIX     14      /* NOTE: this is in moveword table */
#define G_MVO_LOOKATX   (0*24)
#define G_MVO_LOOKATY   (1*24)
#define G_MVO_L0        (2*24)
#define G_MVO_L1        (3*24)
#define G_MVO_L2        (4*24)
#define G_MVO_L3        (5*24)
#define G_MVO_L4        (6*24)
#define G_MVO_L5        (7*24)
#define G_MVO_L6        (8*24)
#define G_MVO_L7        (9*24)









typedef void (*rdp_instr)();

void rdp_go();
void rdp_reset();
void rdp_dis();

void rdp_step();
void rdp_dl_check();
void rdp_execute_dlist();

#endif

