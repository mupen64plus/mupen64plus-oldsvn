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

/**
 * TrueReality - N64/rdp_registers.h
 * Copyright (C) 1998, 1999 Niki W. Waibel
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
 * Information about me (the author):
 *   Niki W. Waibel, Reichenau 20, 6890 Lustenau, Austria - EUROPE
 *   niki.waibel@gmx.net
**/





/* every 'export' in this file resides in 'rdp.c' */
/* this file could also be named as 'rdp_export.h' */




#ifndef __RDP_REGISTERS_H
#define __RDP_REGISTERS_H

#include "type_sizes.h"

// maximum shift value +1 // was using 11
#define MAXSHIFTS                   10
#define MAXSHIFTT                   10

#define DEFAULT                     0x00
#define USE_COLOR4_PRIM             0x01
#define USE_COLOR4_VERTEX_PRIM      0x02
#define USE_COLOR3_VERTEX           0x04
#define USE_TEXTURE                 0x10
#define USE_ALPHA                   0x20
#define USE_FREE                    0x40

#define RDPTD_LOAD_NOT_SET          -1
#define RDPTD_LOADBLOCK             1
#define RDPTD_LOADTILE              2
#define RDP_STACK_SIZE              0x10
//**  structure vertex
typedef struct
{
    float   x, y, z, w;
    _u16   flags;
    float   s, t;
    _u8    r, g, b, a;
    float   n1,n2,n3;
    float   PDn1,PDn2,PDn3;
    float   lcolor[4];
    BOOL genliner;
}
t_vtx;                           

typedef struct
{
    float    x, y, z, w;
    _u16   flags;
    float    s, t;
    _s8    nx, ny, nz;
    _u8  a;
    float   n1,n2,n3;
    float   PDn1,PDn2,PDn3;
    float   lcolor[4];
    BOOL genliner;
}
t_vtx_tn;

typedef struct
{
    float   sn1, cs1;
    float   sn2, cs2;
    float    r, g, b, a;
}
t_lightsw;                           

typedef struct
{
    float r, g, b, a;
    float r_copy, g_copy, b_copy, a_copy;
    float x, y, z, w;
    float mx, my, mz, mw;
} t_light;

//** first by Jeff
typedef struct
{
    _u8     fmt;            // frame format (3 bits)
    _u8     size;           // bits per pixel (2 bits)
    _s32    width;          // width of frame in pixels (12 bits)
    _u32    addr;           // RDRAM pointer to Image
    _u8     *LoadAddr;
    _u32    vaddr;
} t_ImageInfo;

typedef struct
{
//** SETTILE 
    _u8     format;
    _u8     size;
    _u32    line;
    _u32    tmem;
    _u8     palette;
    _u8     clampt;
    _u8     mirrort;
    _u8     maskt;
    _u8     shiftt;
    _u8     clamps;
    _u8     mirrors;
    _u8     masks;
    _u8     shifts;

    _u32    Width;
    _u32    Height;
    
    _u8     Texture_on;
    float   SScale;
    float   TScale;

    int     set_by;
    _u32    addr;
    _u32    offset;
//** LOADBLOCK / LOADTILE
    _u16    uls; /* Upper Left Texture X-Coordinate (10.2 bits) */
    _u16    ult; /* Upper Left Texture Y-Coordinate (10.2 bits) */
    _u16    lrs; /* Lower Right Texture X-Coordinate (10.2 bits) */
    _u16    lrt; /* Lower Right Texture Y-Coordinate (10.2 bits) */
    _u16    dxt; /* Increment dxt per scanline */

//**  SETTILESIZE
    float   sts_uls; /* Upper Left Texture X-Coordinate (10.2 bits) */
    float   sts_ult; /* Upper Left Texture Y-Coordinate (10.2 bits) */
    float   sts_lrs; /* Lower Right Texture X-Coordinate (10.2 bits) */
    float   sts_lrt; /* Lower Right Texture Y-Coordinate (10.2 bits) */

//  _u8     *texture;
    _u32    CRC; // CRC of texture
    _u32    lCRC; // CRC of texture
    _u32    tCRC[16]; // CRC of texture
    BOOL    WdSwpd;
    _u32    Pitch;
    _u16    LineWidth;
    BOOL    reload;
    _u16    Number;
    _u8     texture[512*512*4]; // Tile Texture
    _u32    Palette[256];
} t_tile;


typedef struct tLoadCache
{
    _u32 Addr;
    _u32 Offset;
    _u32 End;
    _u32 Pitch;
    _u32 LineWidth;
    _u16 Swapped;   
    _u16 StepAdj;
    _u16 LoadedBy;
} LoadCache;

typedef struct
{
    _u32    ulx;
    _u32    uly;
    _u32    lrx;
    _u32    lry;
    _u32    interlace_mode;
} t_scissor;

typedef struct 
{
        //** general 

        _u32 pc[RDP_STACK_SIZE];                            /* DL stack is 10 */
        _u32 pc_i;                              /* PC index (current DL depth) */

        _u32  cmd0, cmd1;
        _u32  start, stop, curr;

        //** color register 
        _u32  keycolor;     //** memory interface: 32bit 
        float key_r,key_g,key_b,key_a;
        float key_1mr,key_1mg,key_1mb,key_1ma;
 
        _u32  fillcolor;     //** memory interface: 32bit 
        float fill_r,fill_g,fill_b,fill_a;
        float fill_1mr,fill_1mg,fill_1mb,fill_1ma;
 
        _u32  primcolor;     //** color combiner: rgba 
        float prim_r,prim_g,prim_b,prim_a;
        float prim_1mr,prim_1mg,prim_1mb,prim_1ma;

        _u32  envcolor;      //** color combiner: rgba 
        float env_r,env_g,env_b,env_a;
        float env_1mr,env_1mg,env_1mb,env_1ma;

        _u32  blendcolor;    //** blender: rgba 
        float blend_r,blend_g,blend_b,blend_a;
        float blend_1mr,blend_1mg,blend_1mb,blend_1ma;

        _u32  fogcolor;      //** fog: rgba 
        float fog_r,fog_g,fog_b,fog_a;
        float fog_1mr,fog_1mg,fog_1mb,fog_1ma;

        float fog_fo;
        float fog_fm;
 
        _u32  primdepth;     //** depth
        float  fprimdepth;   //** depth float

        //** This is the frame buffer location to draw into 
        //** Used by rdp_setcolorimage() 
        _u8  colorimg_fmt;          //** Frame Format (3 bits)
        _u8  colorimg_size;         //** Bits per pixel (2 bits) 
        _u16 colorimg_width;        //** width of frame in pixels (12 bits) 
        _u32 colorimg_addr;         //** RDRAM pointer to Image (32 bits)
        _u32 colorimg_addr2;        // check value 

        _u32  depthimg_addr;         //** RDRAM pointer to Image (32 bits) 

        _u8     framebuffer_fmt;          //** Frame Format (3 bits)
        _u8     framebuffer_size;         //** Bits per pixel (2 bits) 
        _u16    framebuffer_width;        //** width of frame in pixels (12 bits) 
        _u32    framebuffer_addr;         //** RDRAM pointer to Image (32 bits) 

        //** This is the frame buffer location to draw into 
        //** Used by rdp_settextureimage() 
        t_ImageInfo TextureImage;
//** FiRES: muesste theoretisch fuer jedes der geladenen Texturen indiv.
        float m_TScale;             
        float m_SScale;
        t_tile *m_CurTile;

        //** This is the location of an 8 bit TLUT 
        //** Used by rdp_loadtlut() 
        _u8  tlut_8_fmt;   //** TLUT Format (3 bits) 
        _u8  tlut_8_size;  //** Bits per pixel (2 bits) 
        _u32  tlut_8_addr;  //** RDRAM pointer to TLUT (32 bits) 

        //** This is the location of an 4 bit TLUT 
        //** Used by rdp_loadtlut() 
        _u8  tlut_4_fmt[16];   //** TLUT Format (3 bits) 
        _u8  tlut_4_size[16];  //** Bits per pixel (2 bits) 
        _u32  tlut_4_addr[16];  //** RDRAM pointer to TLUT (32 bits) 

        t_scissor scissor;


        t_light light[16], lookat_x, lookat_y;


        /**
        *
        * This might be part of some sort of High Level Emulation (HLE).
        * HLE was first hacked out by Epsilon and RealityMan - the authors of UltraHLE.
        *
        **/

#define INVALID_TASK    -1
#define GFX_TASK        1
#define AUDIO_TASK      2

        int task;
        int size;   /* length of task */



        _u32 geometrymode_textures;
        _u32 geometrymode;
        _u32 mode_h;
        _u32 mode_l;
        _u32 combine_h;
        _u32 combine_l;
        _u32 lights;
        _u32 ambient_light;
        _u32 _segment[16];                      
        _u32 segment[16];                       /* segment pointer */

        float vp[8];                            /* viewport (scale, translate) */

        /*
        float perspective_normal;
        */
        float perspnorm;

        float mtx_proj[4][4];                   /* projection matrix */

        float mtx_model[10][4][4];              /* 10 modelview matrices */

        t_vtx vtx[256];                         /* 32 vertices */
        int mtxidx[256];

        _u32 ColorInfoAdd;                      /*  RGBA/Normals for vertexes perfect dark; */
        BOOL ColorMode;

        struct
        {
                _s32   nx;
                _s32   ny;
                _s32   px;
                _s32   py;
        }
        clip;                                   /* clipping stuff */


        int halt;
        int useLights;
        int graphics_changes;
//      t_tile *m_NexTile;
//      int useT1;
        _u32  cmd2, cmd3;

        int     tile;
        int     loadtile;

        struct
        {
                _u32 mipmap_level;
                _u32 on;               /* 1: on 0: off */
                float scale_t;
                float scale_s;
                float scale_w;
                float scale_h;
                float offset_t;
                float offset_s;
        }
        texture[8];

        t_tile  td[8];

} t_rdp_reg; /* struct rsp_reg */


typedef struct
{
    _u16    imageW;         // The width of BG image(u10.2)
    _u16    imageX;         // The x-coordinate of the upper-left position of BG image (u10.5)
    _u16    frameW;         // The width of the transfer frame (u10.2)
    _u16    frameX;         // The upper-left position of the transfer frame(s10.2)
    _u16    imageH;         // The height of BG image (u10.2)
    _u16    imageY;         // The y-coordinate of the upper-left position of BG image (u10.5)
    _u16    frameH;         // The height of the transfer frame (u10.2)
    _u16    frameY;         // The upper-left position of the transfer frame (s10.2)
    _u32    imagePtr;       // The texture address of the upper-left position of BG image
    _u8     imageSiz;       // The size of BG image G_IM_SIZ_* 
    _u8     imageFmt;       // The format of BG image G_IM_FMT_*
    _u16    imageLoad;      // Which to use, LoadBlock and LoadTile 
    _u16    imageFlip;      // Image horizontal flip. Flip using G_BG_FLAG_FLIPS.*/
    _u16    imagePal;       // The pallete number                        
} t_Fullscreen;


extern t_rdp_reg rdp_reg;

extern _u32 PaletteCRC[17];
extern _u32 InvPaletteCRC[17];
void UpdatePalette();

#endif
