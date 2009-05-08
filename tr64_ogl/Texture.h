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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "combine.h"

typedef void (*NCopyTexture)(t_tile *tp_t, _u8 *i_src, _u8 *i_dst, _s32 sx, _s32 ex, _s32 sy, _s32 ey);

typedef struct
{
    _u16 old_bytes[3];
    _u16 *addr;
    _u16 *addr2;
    _u16 *addr3;
} t_old_texture;

typedef struct
{
    _u32    TexID;
    _u32    TexMode;
    _u32    TexColor;
    _u32    offset;
    _u32    NextID;
    _u32    Palette;
    int     Size;
    int     aTilew;
    int     aTileh;
    float   Multw;
    float   Multh;
} t_index_texture;

/* added by hacktarux:
   linked structure that will contain all textures that have the 16 highest bits in CRC equal
 */
typedef struct _text_info text_info;
struct _text_info
{
    t_index_texture info;
    text_info *next;
};

extern int SetActiveTexture();

// Texture Modes
/*
#define Tex0                                0x0001
#define AlphaT0                             0x0002
#define T0MultAlpha                         0x0004
#define InvT0                               0x0010
#define InvT0Alpha                          0x0020
#define InvT0MultAlpha                      0x0040

#define Tex1                                0x0100
#define AlphaT1                             0x0200
#define T1MultAlpha                         0x0400
#define InvT1                               0x1000
#define InvT1Alpha                          0x2000
#define InvT1MultAlpha                      0x4000
*/

#define Tex0                                UseT0
#define AlphaT0                             UseT0Alpha
#define T0MultAlpha                         UseMT0Alpha
#define InvT0                               UseT0Iv
#define InvT0Alpha                          UseT0AlphaIv
#define InvT0MultAlpha                      UseMT0AlphaIv

#define Tex1                                UseT1
#define AlphaT1                             UseT1Alpha
#define T1MultAlpha                         UseMT1Alpha
#define InvT1                               UseT1Iv
#define InvT1Alpha                          UseT1AlphaIv
#define InvT1MultAlpha                      UseMT1AlphaIv

#define LoadTile0 (AlphaT0 | Tex0 | T0MultAlpha | InvT0 | InvT0Alpha | InvT0MultAlpha)
#define LoadTile1 (AlphaT1 | Tex1 | T1MultAlpha | InvT1 | InvT1Alpha | InvT1MultAlpha)
#define LoadTileMask (LoadTile0 | LoadTile1)

#define T0_TEX_MASK (Tex0 | InvT0)
#define T1_TEX_MASK (Tex1 | InvT1)
#define T0_ALPHA_MASK (AlphaT0 | T0MultAlpha | InvT0Alpha | InvT0MultAlpha)
#define T1_ALPHA_MASK (AlphaT1 | T1MultAlpha | InvT1Alpha | InvT1MultAlpha)
#define TEX_MASK (Tex0 | InvT0 | Tex1 | InvT1 | T0MultAlpha | InvT0MultAlpha | T1MultAlpha | InvT1MultAlpha)
#define TEX_ALPHA_MASK  (AlphaT0 | InvT0Alpha | AlphaT1 | InvT1Alpha)
#endif
