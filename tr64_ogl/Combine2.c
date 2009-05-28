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

//#define DEBUGCC
#include <windows.h>
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Combine.h"
#include "rdp_registers.h"
#include "texture.h"
#include "rdp.h"
#include "debug.h"

//* externals
extern void MDrawVisualTriangle(int vn[3], int t1, int s1, int ss1);
HWND hGraphics;

//** globals
_u8 uc0_a0,  uc0_b0,  uc0_c0,  uc0_d0, 
      uc0_Aa0, uc0_Ab0, uc0_Ac0, uc0_Ad0,
      uc0_a1,  uc0_b1,  uc0_c1,  uc0_d1, 
      uc0_Aa1, uc0_Ab1, uc0_Ac1, uc0_Ad1;

_u8 cycle_mode;

_u32 cycle1;
_u32 cycle2;

_u8 Prev_cycle_mode;
_u32 Prev_cycle1 = 0;
_u32 Prev_cycle2 = 0;

_u32 cycle1C;
_u32 cycle1A;

_u32 cycle2C;
_u32 cycle2A;

BOOL UseT1Cyc1 = FALSE;
BOOL UseT1Cyc2 = FALSE;

_u8 checkup = 0;

t_Combine CombineArray[4];
int CombinePasses;

extern _u32 TexMode;
extern _u32 TexColor;

extern int  Vtidx[256];
extern float VtSc[256];
extern float VtTc[256];
extern int Vtcnt;
void FlushVisualTriangle(int vn[]);

  static char *Mode[] = { "CC_COMBINED" ,           "CC_TEXEL0" ,
                          "CC_TEXEL1" ,             "CC_PRIMITIVE" ,
                          "CC_SHADE" ,              "CC_ENVIRONMENT" ,
                          "CC_CENTER" ,             "CC_COMBINED_ALPHA" ,
                          "CC_TEXEL0_ALPHA" ,       "CC_TEXEL1_ALPHA" ,
                          "CC_PRIMITIVE_ALPHA" ,    "CC_SHADE_ALPHA" ,
                          "CC_ENV_ALPHA" ,          "CC_LOD_FRACTION" ,
                          "CC_PRIM_LOD_FRAC" ,      "CC_K5" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,          "CC_0" };

 static char *Alpha[] = { "AC_COMBINED" ,      "AC_TEXEL0" ,
                          "AC_TEXEL1" ,        "AC_PRIMITIVE" , 
                          "AC_SHADE" ,          "AC_ENVIRONMENT" , 
                          "AC_PRIM_LOD_FRAC" , "AC_0"};

void rdp_setcombine()
{
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETCOMBINE", ADDR, CMD0, CMD1);

    cycle1 = 0;
    cycle2 = 0;

    uc0_a0  = (_u8)((rdp_reg.cmd0 >> 20) & 0xF); 
    uc0_b0  = (_u8)((rdp_reg.cmd1 >> 28) & 0xF);
    uc0_c0  = (_u8)((rdp_reg.cmd0 >> 15) & 0x1F);
    uc0_d0  = (_u8)((rdp_reg.cmd1 >> 15) & 0x7);
    uc0_Aa0 = (_u8)((rdp_reg.cmd0 >> 12) & 0x7); 
    uc0_Ab0 = (_u8)((rdp_reg.cmd1 >> 12) & 0x7);
    uc0_Ac0 = (_u8)((rdp_reg.cmd0 >> 9)  & 0x7); 
    uc0_Ad0 = (_u8)((rdp_reg.cmd1 >> 9)  & 0x7);

    cycle1 = (uc0_a0<<0)  | (uc0_b0<<4)  | (uc0_c0<<8)  | (uc0_d0<<13)| 
                    (uc0_Aa0<<16)| (uc0_Ab0<<19)| (uc0_Ac0<<22)| (uc0_Ad0<<25);

    uc0_a1  = (_u8)((rdp_reg.cmd0 >> 5)  & 0xF);
    uc0_b1  = (_u8)((rdp_reg.cmd1 >> 24) & 0xF);
    uc0_c1  = (_u8)((rdp_reg.cmd0 >> 0)  & 0x1F);
    uc0_d1  = (_u8)((rdp_reg.cmd1 >> 6)  & 0x7);
    uc0_Aa1 = (_u8)((rdp_reg.cmd1 >> 21) & 0x7);
    uc0_Ab1 = (_u8)((rdp_reg.cmd1 >> 3)  & 0x7);
    uc0_Ac1 = (_u8)((rdp_reg.cmd1 >> 18) & 0x7);
    uc0_Ad1 = (_u8)((rdp_reg.cmd1 >> 0)  & 0x7);

    cycle2 = (uc0_a1<<0)  | (uc0_b1<<4)  | (uc0_c1<<8)  | (uc0_d1<<13)| 
                    (uc0_Aa1<<16)| (uc0_Ab1<<19)| (uc0_Ac1<<22)| (uc0_Ad1<<25);

    UseT1Cyc1 = (uc0_a0  == 2) || (uc0_b0  == 2) || (uc0_c0  == 2) || (uc0_d0  == 2)
             || (uc0_Aa0 == 2) || (uc0_Ab0 == 2) || (uc0_Ac0 == 2) || (uc0_Ad0 == 2);

    UseT1Cyc2 = (uc0_a1  == 2) || (uc0_b1  == 2) || (uc0_c1  == 2) || (uc0_d1  == 2)
             || (uc0_Aa1 == 2) || (uc0_Ab1 == 2) || (uc0_Ac1 == 2) || (uc0_Ad1 == 2);

    cycle1C = cycle1 & 0x0ffff;
    cycle1A = (cycle1 >> 16) & 0x0ffff;;

    cycle2C = cycle2 & 0x0ffff;
    cycle2A = (cycle2 >> 16) & 0x0ffff;;

    if ((uc0_c0>6) && (uc0_c0<12))
        uc0_c0 += 0;
    if ((uc0_c1>6) && (uc0_c1<12))
        uc0_c1 += 0;

    if ((UseT1Cyc1) || (UseT1Cyc2))
        cycle2C = cycle2 & 0x0ffff;

#ifdef LOG_ON

        LOG_TO_FILE("   mode0=$%06lx; mode1=$%08lx",
               cycle1,
               cycle2);
        LOG_TO_FILE("\ta0 = % 18s, b0 = % 18s, c0 = % 18s, d0 = % 18s",
               Mode[uc0_a0],Mode[uc0_b0],Mode[uc0_c0],Mode[uc0_d0]);
        LOG_TO_FILE("\tAa0 = % 17s, Ab0 = % 17s, Ac0 = % 17s, Ad0 = % 17s",
               Alpha[uc0_Aa0],Alpha[uc0_Ab0],Alpha[uc0_Ac0],Alpha[uc0_Ad0]);
        LOG_TO_FILE("\ta1 = % 18s, b1 = % 18s, c1 = % 18s, d1 = % 18s",
               Mode[uc0_a1],Mode[uc0_b1],Mode[uc0_c1],Mode[uc0_d1]);
        LOG_TO_FILE("\tAa1 = % 17s, Ab1 = % 17s, Ac1 = % 17s, Ad1 = % 17s\n",
               Alpha[uc0_Aa1],Alpha[uc0_Ab1],Alpha[uc0_Ac1],Alpha[uc0_Ad1]);

#endif
} /* static void rdp_setcombine() */

_u32 fPasses,fT1;
_u32 fS1c,fS2c;
_u32 fS1a,fS2a;
_u32 sCycleM,sCycle1,sCycle2;

_u32 Prev_fT1;
_u32 Prev_fS1c,Prev_fS2c;
_u32 Prev_fS1a,Prev_fS2a;

void BuildCombine(_u8 t_cycle_mode)
{
    if ((Prev_cycle1 == cycle1) && (Prev_cycle2 == cycle2) && (Prev_cycle_mode == t_cycle_mode))
    {
        fT1= Prev_fT1;
        fS1c = Prev_fS1c;
        fS2c = Prev_fS2c;
        fS1a = Prev_fS1a;
        fS2a = Prev_fS2a;
        return;
    }

    Prev_cycle1 = cycle1;
    Prev_cycle2 = cycle2;
    Prev_cycle_mode = t_cycle_mode;
    checkup = 0;
/*
        if (t_cycle_mode == CYCLE_MODE2)
        {
            switch(cycle2C)
            {
            case 0x2d12:
            case 0x3fff:
            case 0x7fff:
            case 0x9fff:
            case 0xbfff:
            case 0xdfff:
                cycle1C = cycle2C;
                break;
            }
            switch(cycle2A)
            {
            case 0x03ff:
            case 0x07ff:
            case 0x09ff:
            case 0x0bff:
            case 0x0dff:
                cycle1A = cycle2A;
                break;
            }
        }
*/
        switch(cycle1)
        {
        case 0x020a2d12: // t1,t0,plf,t0 -- 0,0,0,t0 // needs fixed
//          fT1 = 0x20;
//          fS1c = UseT0 | UseT0Alpha;
//          fS2c = UseT1 | UseT1Alpha;
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0738e4f0: // cmb,k5,cmb,ca -- cmb,0,shade,prim
                    fS1c = UseT0 | UseT0Alpha | UseShade | UseShadeAlpha;
                    fS2c = UseT0 | UseT0Alpha | UsePrimAlpha;
                    break;
                case 0x09ffe4f0: // cmb,k5,cmb,ca -- 0,0,shade,0
                    fS1c = UseT0 | UseShade | UseShadeAlpha;
                    fS2c = UseT1 | UseShade | UseShadeAlpha;
                    break;
                case 0x0f38e4f0: // cmb,k5,cmb,ca -- cmb,0,shade,0
                    fS1c |= UseShade | UseShadeAlpha;
                    fS2c |= UseShade | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02ca2a12: // t1,t0,prima,t0 -- t1,t0,prim,t0 // fix me
//          fT1 = 0x20;
//          fS1c = UseT0 | UseT0Alpha | UseMPrimAlphaIv;
//          fS2c = UseT1 | UseT1Alpha | UseMPrimAlpha;
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffa053: // prim,env,cmb,env -- cmb,0,shade,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha;
                    fS2c = UseEnv | UseT0Iv | UseT0Alpha;
                    break;
                case 0x0f38e4f0: // cmb,k5,cmb,ca -- cmb,0,shade,0
                    fS1c |= UseShade | UseShadeAlpha;
                    fS2c |= UseShade | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02ce2412: // t1,t0,shade,t0 -- plf,t0,prim,t0
            fT1 = 0x20;
            fS1c = UseT0 | UseT0Alpha | UseShadeIv;
            fS2c = UseT1 | UseT0Alpha | UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f380c05: // env,cmb,enva,cmb -- cmb,0,shade,0 // fixme
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShadeIv | UseMEnvAlphaIv | UseT0Alpha | UseShadeAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x034a2c12: // t1,t0,enva,t0 -- t1,t0,env,t0 // fix me
//          fT1 = 0x20;
//          fS1c = UseT0 | UseT0Alpha | UseMPrimAlphaIv;
//          fS2c = UseT1 | UseT1Alpha | UseMPrimAlpha;
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffa053: // prim,env,cmb,env -- cmb,0,shade,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha;
                    fS2c = UseEnv | UseT0Iv | UseT0Alpha;
                    break;
                case 0x0ef8a053: // prim,env,cmb,env -- cmb,0,shade,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha;
                    break;
                case 0x0f38e4f0: // cmb,k5,cmb,ca -- cmb,0,shade,0
                    fS1c |= UseShade | UseShadeAlpha;
                    fS2c |= UseShade | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03b22e32: // t1,prim,plf,t0 -- t1,plf,plf,t0
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0ef8a053: // prim,env,cmb,env -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv  | UseT0Iv | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff2c32: // t1,prim,enva,t0 -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0ef8a053: // prim,env,cmb,env -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv  | UseT0Iv | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff3fff: // k5,k5,0,t0 -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0ef8a053: // prim,env,cmb,env -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv  | UseT0Iv | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff4526: // cent,t1,env,t1 -- 0,0,0,t0
            fT1 = 0x20;
            fS1c = UseT0 | UseT0Alpha | UseEnv;
            fS2c = UseT1 | UseT0Alpha | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                case 0x0af8e3f0: // cmb,k5,prim,ca -- cmb,0,prim,env
                    fS1c |= UsePrim | UsePrimAlpha;
                    fS2c |= UsePrim | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff5fff: // k5,k5,0,t0 -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT1 | UseT0Alpha;
            fS2c = UseT1 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff6935: // env,prim,t1a,prim -- 0,0,0,t0
            fT1 = 0x20;
            fS1c = UseEnv | UseMT1Alpha;
            fS2c = UsePrim | UseMT1AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f781fff: // k5,k5,0,cmb -- cmb,0,env,0
                    fS1c |= UseEnvAlpha;
                    fS2c |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff9fff: // k5,k5,0,shade -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseShade | UseT0Alpha;
            fS2c = UseShade | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ffdfff: // k5,k5,0,cent -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ffe3f1: // t0,k5,prim,ca -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT0Alpha;
            fS2c = UseT0 | UsePrim | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffe4f0: // cmb,k5,shade,ca -- 0,0,0,cmb
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0x0ef80c05: // env,cmb,enva,cmb -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShade | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ffe3f2: // t1,k5,prim,ca -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT1 | UsePrim | UseT0Alpha;
            fS2c = UseT1 | UsePrim | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ffe4f1: // t0,k5,shade,ca -- 0,0,0,t0
            fT1 = 0x10;
            fS1c = UseT0 | UseShade | UseT0Alpha;
            fS2c = UseT0 | UseShade | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffe0f3: // cmb,k5,prim,ca -- 0,0,0,cmb
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0x0ef80c05: // env,cmb,enva,cmb -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShade | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha;
                    break;
                case 0x0f78e0f3: // cmb,k5,prim,ca -- cmb,0,env,0
                    fS1c |= UsePrim | UseEnvAlpha;
                    fS2c |= UsePrim | UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x05ffe3f1: // t0,k5,prim,0 -- 0,0,0,t1
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT1Alpha | UsePrimAlpha;
            fS2c = UseT0 | UsePrim | UseT1Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff3fff: // k5,k5,0,t0 -- 0,0,0,prim
            fT1 = 0x10;
            fS1c = UseT0 | UsePrimAlpha;
            fS2c = UseT0 | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff7fff: // k5,k5,0,prim -- 0,0,0,prim
            fT1 = 0x10;
            fS1c = UsePrim | UsePrimAlpha;
            fS2c = UsePrim | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff8531: // t0,prim,env,shade -- 0,0,0,prim
            fT1 = 0x20;
            fS1c = UseShade | UseMEnvAlphaIv | UsePrimAlpha;
            fS2c = UseT0 | UseMEnvAlpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f78e4f0: // cmb,k5,shade,ca -- cmb,0,env,0
                    fS1c |= UseShade | UseEnvAlpha;
                    fS2c |= UseShade | UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff8c45: // env,shade,enva,shade -- 0,0,0,prim
            fT1 = 0x20;
            fS1c = UseShade | UseMEnvAlphaIv | UsePrimAlpha;
            fS2c = UseEnv | UseMEnvAlpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0ef8e3f0: // cmb,k5,prim,ca -- cmb,0,prim,0
                    fT1 = 0x10;
                    fS1c |= UsePrim | UsePrimAlpha;
                    fS2c |= UsePrim | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff9fff: // k5,k5,0,shade -- 0,0,0,prim
            fT1 = 0x10;
            fS1c = UseShade | UsePrimAlpha;
            fS2c = UseShade | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ffe1f3: // prim,k5,t0,ca -- 0,0,0,prim
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UsePrimAlpha;
            fS2c = UseT0 | UsePrim | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ff3fff: // k5,k5,0,t0 -- 0,0,0,shade
            fT1 = 0x10;
            fS1c = UseT0 | UseShadeAlpha;
            fS2c = UseT0 | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ff6536: // cent,env,prim,env -- 0,0,0,shade
            fT1 = 0x20;
            fS1c = UseT0 | UsePrim | UseShadeAlpha;
            fS2c = UseEnv | UsePrimIv | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f78e4f0: // cmb,k5,shade,0 -- cmb,0,env,0
                    fS1c |= UseShade | UseEnvAlpha;
                    fS2c |= UseShade | UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ff8c45: // env,shade,enva,shade -- 0,0,0,shade
            fT1 = 0x20;
            fS1c = UseShade | UseMEnvAlphaIv | UseShadeAlpha;
            fS2c = UseEnv | UseMEnvAlpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0ef8e3f0: // cmb,k5,prim,ca -- cmb,0,prim,0
                    fT1 = 0x10;
                    fS1c |= UsePrim | UsePrimAlpha;
                    fS2c |= UsePrim | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ff9fff: // k5,k5,0,shade -- 0,0,0,shade
            fT1 = 0x10;
            fS1c = UseShade | UseShadeAlpha;
            fS2c = UseShade | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ffe4f1: // t0,k5,shade,0 -- 0,0,0,shade
            fT1 = 0x10;
            fS1c = UseT0 | UseShade | UseShadeAlpha;
            fS2c = UseT0 | UseShade | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0af9e3f1: // t0,k5,prim,0 -- t0,0,prim,env
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT0Alpha | UsePrimAlpha;
            fS2c = UseT0 | UsePrim | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0b29ab51: // t0,env,shadea,env -- t0,env,shade,env
            fT1 = 0x20;
            fS1c = UseT0 | UseT0Alpha | UseShadeAlpha;
            fS2c = UseEnv | UseEnvAlpha | UseShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffe4f0: // cmb,k5,cmb,ca -- cmb,0,shade,0
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0bffe5f1: // t0,k5,env,0 -- 0,0,0,env
            fT1 = 0x10;
            fS1c = UseT0 | UseEnv | UseEnvAlpha;
            fS2c = UseT0 | UseEnv | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0df9a153: // prim,env,t0,env -- t0,0,0,plf
            fT1 = 0x20;
            fS1c = UseT0   | UsePrim | UseT0Alpha;
            fS2c = UseT0Iv | UseEnv  | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ffe4f0: // cmb,0,shade,ca -- 0,0,0,cmb
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7ba853: // prim,env,t0a,env -- prim,0,t0,0
            fT1 = 0x20;
            fS1c = UsePrim | UseMT0Alpha | UseT0Alpha | UsePrimAlpha;
            fS2c = UseEnv | UseMT0AlphaIv | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7de1f5: // t0,k5,env,ca -- env,0,t0,0
            fT1 = 0x10;
            fS1c = UseT0 | UseEnv | UseT0Alpha | UseEnvAlpha;
            fS2c = UseT0 | UseEnv | UseT0Alpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0eb9e2f1: // t1,k5,t0,ca -- plf,0,t1,0 // ?????
            fT1 = 0x20;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT1 | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f38e4f0: // cmb,k5,shade,ca -- cmb,0,shade,0
                    fS1c = UseT0 | UseShade | UseShadeAlpha;
                    fS2c = UseT1 | UseShade | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ebe2d12: // t1,t0,plf,t0 -- plf,0,t1,0 // needs fixed
//          fT1 = 0x20;
//          fS1c = UseT0 | UseT0Alpha;
//          fS2c = UseT1 | UseT1Alpha;
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x09ffe4f0: // cmb,k5,cmb,ca -- 0,0,shade,0
                    fS1c = UseT0 | UseShade | UseShadeAlpha;
                    fS2c = UseT1 | UseShade | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef92c15: // env,t0,enva,t0 -- t0,0,prim,0
            fT1 = 0x20;
            fS1c = UseT0 | UseMEnvAlphaIv | UsePrimAlpha | UseT0Alpha;
            fS2c = UseEnv | UseMEnvAlpha | UsePrimAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef93fff: // k5,k5,0,t0 -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UseT0 | UsePrimAlpha | UseT0Alpha;
            fS2c = UseT0 | UsePrimAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef96b31: // t0,prim,shadea,prim -- t0,0,prim,0
            fT1 = 0x20;
            fS1c = UseT0 | UseMShadeAlpha | UseT0Alpha | UsePrimAlpha;
            fS2c = UsePrim | UseMShadeAlphaIv | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff0405: // env,cmb,shade,cmb -- 0,0,0,cmb // fixme
                    fS1c = UseShadeIv | UseT0 | UseMShadeAlpha | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseShade | UseEnv | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef97fff: // k5,k5,0,prim -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UsePrim | UsePrimAlpha | UseT0Alpha;
            fS2c = UsePrim | UsePrimAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef9a153: // prim,env,t0,env -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT0Alpha | UsePrimAlpha;
            fS2c = UseT0Iv | UseEnv | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef9e3f1: // t0,k5,prim,0 -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT0Alpha | UsePrimAlpha;
            fS2c = UseT0 | UsePrim | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                case 0x01ff0c05: // env,cmb,enva,cmb -- 0,0,0,cmb
                    fT1 = 0X20;
                    fS1c = UsePrim | UseT0 | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef9e3f2: // t1,k5,prim,0 -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UseT1 | UsePrim | UseT0Alpha | UsePrimAlpha;
            fS2c = UseT1 | UsePrim | UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef9ffff: // k5,k5,0,ca -- t0,0,prim,0
            fT1 = 0x10;
            fS1c = UsePrimAlpha | UseT0Alpha;
            fS2c = UsePrimAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efae3f1: // t0,k5,prim,ca -- t1,0,prim,0
            fT1 = 0x10;
            fS1c = UseT0 | UsePrim | UseT1Alpha | UsePrimAlpha;
            fS2c = UseT0 | UsePrim | UseT1Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShade | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efae3f2: // t1,k5,prim,ca -- t1,0,prim,0
            fT1 = 0x10;
            fS1c = UseT1 | UsePrim | UseT1Alpha | UsePrimAlpha;
            fS2c = UseT1 | UsePrim | UseT1Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShade | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efc7fff: // k5,k5,0,prim -- shade,0,prim,0
            fT1 = 0x10;
            fS1c = UsePrim | UsePrimAlpha;
            fS2c = UsePrim | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f392d12: // t1,t0,plf,t0 -- t0,0,shade,0 // needs work
//          fT1 = 0x20;
//          fS1c = UseT0 | UseT0Iv | UseT0Alpha | UseShadeAlpha;
//          fS2c = UseT1 | UseT0 | UseT0Alpha | UseShadeAlpha;
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha | UseShadeAlpha;
            fS2c = UseT0 | UseT0Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f78e4f0: // cmb,k5,shade,ca -- cmb,0,env,0
                    fS1c |= UseShade | UseEnvAlpha;
                    fS2c |= UseShade | UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f393fff: // k5,k5,0,t0 -- t0,0,shade,0
            fT1 = 0x10;
            fS1c = UseT0 | UseShadeAlpha | UseT0Alpha;
            fS2c = UseT0 | UseShadeAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f781fff: // k5,k5,0,cmb -- cmb,0,env,0
                    fS1c |= UseEnvAlpha;
                    fS2c |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f396531: // t0,prim,env,prim -- t0,0,shade,0
            fT1 = 0x20;
            fS1c = UseT0 | UseEnv | UseT0Alpha | UseShadeAlpha;
            fS2c = UsePrim | UseEnvIv | UseT0Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0f78e4f0: // cmb,k5,shade,0 -- cmb,0,env,0
                    fS1c |= UseShade | UseEnvAlpha;
                    fS2c |= UseShade | UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f397fff: // k5,k5,0,prim -- t0,0,shade,0
            fT1 = 0x10;
            fS1c = UsePrim | UseShadeAlpha | UseT0Alpha;
            fS2c = UsePrim | UseShadeAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f39e4f1: // t0,k5,shade,0 -- t0,0,0,shade
            fT1 = 0x10;
            fS1c = UseT0 | UseShade | UseT0Alpha | UseShadeAlpha;
            fS2c = UseT0 | UseShade | UseT0Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                    break;
                case 0x01ffe5f0: // cmb,k5,env,0 -- 0,0,0,cmb
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0x0ef80c05: // env,cmb,enva,cmb -- cmb,0,prim,0
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0 | UseMEnvAlphaIv | UseT0Alpha | UsePrimAlpha | UseShadeAlpha;
                    fS2c = UseEnv | UseMEnvAlpha | UseT0Alpha | UsePrimAlpha | UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f79e5f1: // t0,k5,env,0 -- t0,0,0,env
            fT1 = 0x10;
            fS1c = UseT0 | UseEnv | UseT0Alpha | UseEnvAlpha;
            fS2c = UseT0 | UseEnv | UseT0Alpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0fff2c31: // t0,prim,enva,t0 -- 0,0,0,plf
            fT1 = 0x10;
            fS1c = UseT0 | UseT0Alpha;
            fS2c = UseT0 | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x0dffa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseT0Alpha;
                    fS2c = UseEnv  | UseT0Iv | UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0fffffff: // k5,k5,0,ca -- 0,0,0,0 // not right 
            fT1 = 0x10;
            fS1c = UsePrimAlpha | UsePrimAlphaIv | UseEnvAlpha | UseEnvAlphaIv;
            fS2c = UsePrimAlpha | UsePrimAlphaIv | UseEnvAlpha | UseEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(cycle2)
                {
                case 0x01ff1fff:
                case 0x0fffffff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        default:
            {
                checkup = 4;
            }
        }

    if (checkup != 0)
    {       
#ifdef DEBUGCC
        char output[1024];
        sprintf(output,"Mode1C = %04x\tMode2C = %04x\tCycleMode = %i\n"
                       "Mode1A = %04x\tMode2A = %04x\n\n"
                       "a0  = %s  \tb0  = %s  \tc0  = %s  \td0  = %s\n\n"
                       "a1  = %s  \tb1  = %s  \tc1  = %s  \td1  = %s\n\n"
                       "Aa0 = %s  \tAb0 = %s  \tAc0 = %s  \tAd0 = %s\n\n"
                       "Aa1 = %s  \tAb1 = %s  \tAc1 = %s  \tAd1 = %s",
                cycle1C,cycle2C,t_cycle_mode,
                cycle1A,cycle2A,
                Mode[uc0_a0],   Mode[uc0_b0],   Mode[uc0_c0],   Mode[uc0_d0],
                Mode[uc0_a1],   Mode[uc0_b1],   Mode[uc0_c1],   Mode[uc0_d1],
                Alpha[uc0_Aa0], Alpha[uc0_Ab0], Alpha[uc0_Ac0], Alpha[uc0_Ad0],
                Alpha[uc0_Aa1], Alpha[uc0_Ab1], Alpha[uc0_Ac1], Alpha[uc0_Ad1]);
            MessageBox(hGraphics, output, "DisplayCC", MB_OK);
//          exit(1);
//      rdp_reset();
#else
        fT1 = 0x10;
        fS1c = UseT0;
        fS2c = UseT0;
        fS1a = UseT0Alpha;
        fS2a = UseT0Alpha;
#endif
    }

    fS1c &= ~UseTex1Mask;
    fS2c &= ~UseTex1Mask;
    fS1c &= ~UseTex1Mask;
    fS2a &= ~UseTex1Mask;

    Prev_fT1= fT1;
    Prev_fS1c = fS1c;
    Prev_fS2c = fS2c;
    Prev_fS1a = fS1a;
    Prev_fS2a = fS2a;

}

void SetFastCombine(int vn[3], _u32 t1, _u32 s1, _u32 s2)
{
    //fPasses
//  fT1 = t1;
//  fS1 = s1;
//  fS2 = s2;

    MDrawVisualTriangle(vn,t1,s1,s2);
}

void NewDrawVisualTriangle(int vn[3])
//void DrawVisualTriangle(int vn[3])
{
    int pass;

    for (pass = 0; pass < CombinePasses; pass++)
    {
        TexColor = CombineArray[pass].TexColor;// this is wrong, fix it!
        TexMode = CombineArray[pass].TexMode;
        MDrawVisualTriangle(vn,CombineArray[pass].t1,CombineArray[pass].s1,CombineArray[pass].s2);
    }

}

//void OldDrawVisualTriangle(int vn[3])
void DrawVisualTriangle(int vn[3])
{
    Vtidx[Vtcnt] = vn[0]; Vtcnt++;
    Vtidx[Vtcnt] = vn[1]; Vtcnt++;
    Vtidx[Vtcnt] = vn[2]; Vtcnt++;
}

//void OldDrawVisualTriangle(int vn[3])
void DrawVisualTriangle6(int vn[3],float Sc[3],float Tc[3])
{
    VtSc[Vtcnt] = Sc[0];
    VtTc[Vtcnt] = Tc[0];
    Vtidx[Vtcnt] = vn[0]; Vtcnt++;

    VtSc[Vtcnt] = Sc[1];
    VtTc[Vtcnt] = Tc[1];
    Vtidx[Vtcnt] = vn[1]; Vtcnt++;

    VtSc[Vtcnt] = Sc[2];
    VtTc[Vtcnt] = Tc[2];
    Vtidx[Vtcnt] = vn[2]; Vtcnt++;
}

extern int  ucode_version;

void FlushVisualTriangle(int vn[256])
{
    TexColor = 0;
    TexMode = 0;
    
    fT1 = 0;
    fS1c = 0;
    fS2c = 0;
    fS1a = 0;
    fS2a = 0;

    // handle color combine
    if ((cycle_mode == CYCLE_MODE1) || (cycle_mode == CYCLE_MODE2))
    {
        BuildCombine(cycle_mode);
        if (checkup == 0) SetFastCombine(vn,fT1,fS1c | fS1a,fS2c | fS2a);
    }
    else if(cycle_mode == CYCLE_COPY)
    {
        checkup = 2;
        SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
    }
    else if(cycle_mode == CYCLE_FILL)
    {
//      checkup = 3;
        BuildCombine(cycle_mode & 1);
        if (checkup == 0) SetFastCombine(vn,fT1,fS1c | fS1a,fS2c | fS2a);
    }
}

