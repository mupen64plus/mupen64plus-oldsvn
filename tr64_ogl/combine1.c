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
#include "tr_windows.h"
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "combine.h"
#include "rdp_registers.h"
#include "texture.h"
#include "rdp.h"
//#include "debug.h"

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

#define maxCombineModes 40
_ModesUsed ModesUsed[maxCombineModes];
int ModesUsedCnt = 0;

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

    rdp_reg.combine_h = CMD0 & 0x00ffffff;
    rdp_reg.combine_l = CMD1;

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

_u32 tcycle1C = cycle1C;
_u32 tcycle1A = cycle1A;

_u32 tcycle2C = cycle2C;
_u32 tcycle2A = cycle2A;

    if ((Prev_cycle1 == cycle1) && (Prev_cycle2 == cycle2) && (Prev_cycle_mode == t_cycle_mode))
    {
        fT1= Prev_fT1;
        fS1c = Prev_fS1c;
        fS2c = Prev_fS2c;
        fS1a = Prev_fS1a;
        fS2a = Prev_fS2a;
        return;
    }

    {
        int idx;
        BOOL found = FALSE;
        for(idx = 0; idx < ModesUsedCnt; idx++)
        {
            if(
                (ModesUsed[idx].cycle1C == cycle1C) &&
                (ModesUsed[idx].cycle1A == cycle1A) &&
                (ModesUsed[idx].cycle2C == cycle2C) &&
                (ModesUsed[idx].cycle2A == cycle2A)
                )
            {
                found = TRUE;
                break;
            }
        }
        
        if(!found && ModesUsedCnt < maxCombineModes)
        {
            ModesUsed[ModesUsedCnt].cycle1C = cycle1C;
            ModesUsed[ModesUsedCnt].cycle1A = cycle1A;
            ModesUsed[ModesUsedCnt].cycle2C = cycle2C;
            ModesUsed[ModesUsedCnt].cycle2A = cycle2A;
            ModesUsed[ModesUsedCnt].cycle_mode = t_cycle_mode;
            
            ModesUsed[ModesUsedCnt].Color_a0 = Mode[uc0_a0];
            ModesUsed[ModesUsedCnt].Color_b0 = Mode[uc0_b0];
            ModesUsed[ModesUsedCnt].Color_c0 = Mode[uc0_c0];
            ModesUsed[ModesUsedCnt].Color_d0 = Mode[uc0_d0];
            
            ModesUsed[ModesUsedCnt].Alpha_a0 = Alpha[uc0_Aa0];
            ModesUsed[ModesUsedCnt].Alpha_b0 = Alpha[uc0_Ab0];
            ModesUsed[ModesUsedCnt].Alpha_c0 = Alpha[uc0_Ac0];
            ModesUsed[ModesUsedCnt].Alpha_d0 = Alpha[uc0_Ad0];
            
            ModesUsed[ModesUsedCnt].Color_a1 = Mode[uc0_a1];
            ModesUsed[ModesUsedCnt].Color_b1 = Mode[uc0_b1];
            ModesUsed[ModesUsedCnt].Color_c1 = Mode[uc0_c1];
            ModesUsed[ModesUsedCnt].Color_d1 = Mode[uc0_d1];
            
            ModesUsed[ModesUsedCnt].Alpha_a1 = Alpha[uc0_Aa1];
            ModesUsed[ModesUsedCnt].Alpha_b1 = Alpha[uc0_Ab1];
            ModesUsed[ModesUsedCnt].Alpha_c1 = Alpha[uc0_Ac1];
            ModesUsed[ModesUsedCnt].Alpha_d1 = Alpha[uc0_Ad1];

            ModesUsedCnt++;
        }
    }

    Prev_cycle1 = cycle1;
    Prev_cycle2 = cycle2;
    Prev_cycle_mode = t_cycle_mode;
    checkup = 0;

        if (t_cycle_mode == CYCLE_MODE2)
        {
            switch(tcycle2C)
            {
            case 0x2d12:
            case 0x3fff:
            case 0x7fff:
            case 0x9fff:
            case 0xbfff:
            case 0xdfff:
                tcycle1C = tcycle2C;
                break;
            }
            switch(tcycle2A)
            {
            case 0x03ff:
            case 0x07ff:
            case 0x09ff:
            case 0x0bff:
            case 0x0dff:
                tcycle1A = tcycle2A;
                break;
            }
        }
        switch(tcycle1C)
        {
        case 0x2132: // t1,prim,t0,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2132:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x22f6: // cent,k5,t1,t0
            fT1 = 0x10;
            fS1c = UseT0;// | UseT1;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x22f6:
                    break;
                case 0x6035: // env,prim,cmb,prim
                    fT1 = 0x20;
                    fS1c = UseEnv  | UseT0;
                    fS2c = UsePrim | UseT0Iv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2312: // t1,t0,prim,t0
            fT1 = 0x20;
            fS1c = UseT1 | UsePrim;
            fS2c = UseT0 | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2312:
                    break;
                case 0x0c05: // env,cmb,enva,cmb
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UseT0  | UseMEnvAlphaIv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2315: // env,t0,prim,t0
            fT1 = 0x20;
            fS1c = UseEnv | UsePrim;
            fS2c = UseT0 | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2312:
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2316: // cent,t0,prim,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2316:
                    break;
                case 0x0c05:
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UseT0  | UseMEnvAlphaIv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x23f6: // cent,k5,prim,t0
            fT1 = 0x10;//0x20;
            fS1c = UseT0;
            fS2c = UseT0;// | UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x23f6:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2412: // t1,t0,shade,t0
            fT1 = 0x20;
            fS1c = UseT1 | UseShade;
            fS2c = UseT0 | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2413:
                    break;
                case 0x0c05:
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UseT0  | UseMEnvAlphaIv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2413: // prim,t0,shade,t0
            fT1 = 0x20;
            fS1c = UsePrim | UseShade;
            fS2c = UseT0   | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2413:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2416: // cent,t0,shade,t0
            fT1 = 0x10;
            fS1c = UseT0;// | UseShade;
            fS2c = UseT0;// | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2416:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2512: // t1,t0,env,t0
            fT1 = 0x20;
            fS1c = UseT1 | UseEnv;
            fS2c = UseT0 | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2512:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2516: // cent,t0,env,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2516:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x28f5: // env,k5,t0alpha,t0
            fT1 = 0x20;
            fS1c = UseEnv | UseMT0Alpha;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x28f5:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2a12: // t1,t0,prima,t0
            fT1 = 0x20;
            fS1c = UseT1 | UseMPrimAlpha;
            fS2c = UseT0 | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2a12:
                    break;
                case 0x6035:// env,prim,cmb,prim
                    fT1 = 0x20;
                    fS1c = UseEnv  | UseT0;
                    fS2c = UsePrim | UseT0Iv;
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;//   | UseMPrimAlphaIv;
                    fS2c = UseEnv  | UseT0Iv;// | UseMPrimAlpha;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2a13: // prim,t0,prima,t0
            fT1 = 0x20;
            fS1c = UsePrim | UseMPrimAlpha;
            fS2c = UseT0 | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2a13:
                    break;
                case 0x6035:// env,prim,cmb,prim
                    fT1 = 0x20;
                    fS1c = UseEnv  | UseT0;
                    fS2c = UsePrim | UseT0Iv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;//   | UseMPrimAlphaIv;
                    fS2c = UseEnv  | UseT0Iv;// | UseMPrimAlpha;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2b12: // t1,t0,shadea,t0
            fT1 = 0x20;
            fS1c = UseT1 | UseMShadeAlpha;
            fS2c = UseT0 | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2b12:
                    break;
                case 0x8043:
                    fT1 = 0x20;
                    fS1c = UsePrim  | UseT0;//   | UseMPrimAlphaIv;
                    fS2c = UseShade | UseT0Iv;// | UseMPrimAlpha;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;//   | UseMPrimAlphaIv;
                    fS2c = UseEnv  | UseT0Iv;// | UseMPrimAlpha;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2b14: //shade,t0,shada,t0
            fT1 = 0x20;
            fS1c = UseShade | UseMShadeAlpha;
            fS2c = UseT0    | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2b14:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2c12: // t1,t0,enva,t0
            fT1 = 0x20;
            fS1c = UseT1 | UseMEnvAlpha;
            fS2c = UseT0 | UseMEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2c12:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0; //   | UseMEnvAlphaIv;
                    fS2c = UseEnv  | UseT0Iv; // | UseMEnvAlpha;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2c15: // env,t0,enva,t0
            fT1 = 0x20;
            fS1c = UseEnv | UseMEnvAlpha;
            fS2c = UseT0  | UseMEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2c15:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;                  
                case 0xe4f0:
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2c31: // t0,prim.enva,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2c31:
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2c32: // t1,prim,enva,t0
            fT1 = 0x10;
            fS1c = UseT0; //UseT1 | UseMEnvAlpha;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2c32:
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2d11:// t0,t0,lodfrc,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2d11:
                    break;
                case 0xa054: // shade,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv | UseT0Iv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f1: // t0,k5,shade,ca
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2d12:// t1,t0,lodfrc,t0 ???fixme
            fT1 = 0x10;//0x20;
            fS1c = UseT0;
            fS2c = UseT0;//UseT1 | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2d12:
                    break;
                case 0x6054: //// fix me shade, env,cmb,prim
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnvIv | UsePrim | UseT0Iv;
                    break;
                case 0x6450: //// fixme cmb, env, shade. prim
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseShadeIv | UseEnvIv | UsePrim;
                    break;
                case 0xa054: // shade,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv | UseT0Iv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f1: // t0,k5,shade,ca
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2e12: // t1,t0,plf,t0 ??? fixme
            fT1 = 0x20;
            fS1c = UseT0 | UseT0Iv;
            fS2c = UseT1 | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2e12:
                    break;
                case 0x6034: // shade,prim,cmb,prim
                    fT1 = 0x20;
                    fS1c |= UseShade | UseT0;
                    fS2c |= UsePrim  | UseT0Iv;
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2e13: // prim,t0,plf,t0 // needs fixed
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UsePrim | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2e13:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2e31: // t0,prim,plf,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2e31:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2e32: // t1,prim,plf,t0 ??? fixme
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2e32:
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xa054:
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv   | UseT0Iv;
                    break;
                case 0xa0f3:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2ef2:
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2ef2:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xa054:
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv   | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x2ef6: // cent,k5,plf,t0 // fixme
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x2ef6:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xa054:
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv   | UseT0Iv;
                    break;
                case 0xa3f0: // cmb,k5,prim,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv;
                    break;
                case 0xa4f0: // cmb,k5,shade,env
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0;
                    fS2c = UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x3ff2: // t1,k5,0,t0 // fixme
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x3ff2:
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x3fff: // k5,k5,k5,t0
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x3fff:
                    break;
                case 0x0503: // prim,cmb,env,cmb
                    fT1 = 0x20;
                    fS1c = UsePrim | UseEnv;
                    fS2c = UseT0 | UseEnvIv;
                    break;
                case 0x05f6: // cent,k5,env,cmb
                    fT1 = 0x20;
                    fS1c = UseT0;
                    fS2c = UseEnv;// UseT0 | UseEnv;
                    break;
                case 0x0a53: // prim,env,prima,cmb // can't do yet.
                    fT1 = 0x20;
                    fS1c = UsePrim | UseMPrimAlpha;
                    fS2c = UseT0;
                    break;
                case 0x6054: //// fix me shade, env,cmb,prim // can't do yet.
                    //fT1 = 0x20;
                    fS1c = UseT0 | UseShade;
                    fS2c = UseT0Iv | UsePrim; // | UseEnvIv 
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0: // cmb,k5,prim,cmba
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4526: // cent,t1,env,t1
            fT1 = 0x10;
            fS1c = UseT1 | UseEnvIv;
            fS2c = UseT1 | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4526:
                    break;
                case 0x0306: // cent,cmb,prim,cmb
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4a21: // t0,t1,prima,t1
            fT1 = 0x20;
            fS1c = UseT0 | UseMPrimAlpha;
            fS2c = UseT1 | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4a21:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4b21: // t0,t1,shadea,t1
            fT1 = 0x20;
            fS1c = UseT0 | UseMShadeAlpha;
            fS2c = UseT1 | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4a21:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4c21: // t0,t1,enva,t1
            fT1 = 0x20;
            fS1c = UseT0 | UseMEnvAlpha;
            fS2c = UseT1 | UseMEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4c12:
                    break;
                case 0x6a30: // cmb,prim,prima,prim
                    fT1 = 0x20;
                    fS1c = UseT0   | UseMPrimAlpha;
                    fS2c = UsePrim | UseMPrimAlphaIv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4cf1:
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4cf1:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x4e31: // t0,prim,plf,t1
            fT1 = 0x10;
            fS1c = UseT1; // UseT0;
            fS2c = UseT1; // UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4e31:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x5edf: // 
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x4e31:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x5fff: // 0,k5,0,t1
            fT1 = 0x10;
            fS1c = UseT1;
            fS2c = UseT1;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x5fff:
                    break;
                case 0x8045: // env,shade,cmb,shade
                    fT1 = 0x20;
                    fS1c = UseEnv   | UseT1;
                    fS2c = UseShade | UseT1Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6134: // shade,prim,t0,prim
            fT1 = 0x20;
            fS1c = UseShade | UseT0;
            fS2c = UsePrim  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6134:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6135: // env,prim,t0,prim
            fT1 = 0x20;
            fS1c = UseEnv  | UseT0;
            fS2c = UsePrim | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6135:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6156: // cent,env,t0,prim
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UsePrim | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6156:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x61f6: // cent,k5,t0,prim
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x61f6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6235: // env,prim,t1,prim
            fT1 = 0x20;
            fS1c = UseEnv  | UseT1;
            fS2c = UsePrim | UseT1Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6235:
                case 0xe2f0:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6236: // cent,prim,t1,prim
            fT1 = 0x10;// 0x20;
            fS1c = UsePrim | UseT1Iv;//UseT0   | UseT1;
            fS2c = UsePrim | UseT1Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6236:
                case 0xe2f0:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6435: // env,prim,shade,prim
            fT1 = 0x20;
            fS1c = UseEnv  | UseShade;
            fS2c = UsePrim | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6435:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6436: // cent,prim,shade,prim
            fT1 = 0x10;//0x20
            fS1c = UsePrim | UseShadeIv;// UseT0   | UseShade;
            fS2c = UsePrim | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6436:
                    break;
                case 0xe0f1:
                    fS1c |= UseT0;
                    fS2c |= UseT0;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6451: // t0,env,shade,prim // can't do yet.
            fT1 = 0x20;
            fS1c = UseT0 | UseShade;
            fS2c = UseEnvIv | UsePrim | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6451:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6531: // t0,prim,env,prim
            fT1 = 0x20;
            fS1c = UseT0   | UseEnv;
            fS2c = UsePrim | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6531:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6536: //cent,prim,env,prim
            fT1 = 0x20;
            fS1c = UseEnv;
            fS2c = UsePrim | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6536:
                    break;
                case 0xe4f0: // cmb,k5,shade,ca
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x65f1: // t0,k5,env,prim
            fT1 = 0x20;
            fS1c = UseT0   | UseEnv;
            fS2c = UsePrim | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1ff0:
                case 0x1fff:
                case 0x65f1:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6935: // env,prim,t1a,prim
            fT1 = 0x20;
            fS1c = UseEnv  | UseMT1Alpha;
            fS2c = UsePrim | UseMT1AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6935:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6a31: // t0,prim,prima,prim
            fT1 = 0x20;
            fS1c = UseT0   | UseMPrimAlpha;
            fS2c = UsePrim | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6a31:
                    break;
                case 0x0405: // env,cmb,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseEnv | UseShade;
                    fS2c = UseT0 | UseShadeIv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6a54: // shade,env,prima,prim 
            fT1 = 0x20;
            fS1c = UseShade | UseMPrimAlpha;
            fS2c = UsePrim | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6a54:
                    break;
                case 0x0405: // env,cmb,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseEnv | UseShade;
                    fS2c = UseT0 | UseShadeIv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6b31: // t0,prim,shadea,prim
            fT1 = 0x20;
            fS1c = UseT0   | UseMShadeAlpha;
            fS2c = UsePrim | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6b31:
                    break;
                case 0x0405: // env,cmb,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseEnv | UseShade;
                    fS2c = UseT0 | UseShadeIv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6c31: // t0,prim,enva,prim
            fT1 = 0x20;
            fS1c = UseT0   | UseMEnvAlpha;
            fS2c = UsePrim | UseMEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6c31:
                    break;
                case 0x0405: // env,cmb,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseEnv | UseShade;
                    fS2c = UseT0 | UseShadeIv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x6e12:
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x6e12:
                    break;
                case 0x6034:
                    fT1 = 0x20;
                    fS1c |= UseShade | UseT0;
                    fS2c |= UsePrim  | UseT0Iv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x7fff: // k5,k5,0,prim
            fT1 = 0x10;
            fS1c = UsePrim;
            fS2c = UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x7fff:
                    break;
                case 0x05f6: // cent,k5,env,cmb
                    //fT1 = 0x20;
                    fS1c = UsePrim | UseEnvIv;
                    fS2c = UsePrim | UseEnvIv;//UseT0 | UseEnv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8143: // prim,shade,t0,shade
            fT1 = 0x20;
            fS1c = UsePrim  | UseT0;
            fS2c = UseShade | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8143:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8145: // env,shade,t0,shade
            fT1 = 0x20;
            fS1c = UseEnv   | UseT0;
            fS2c = UseShade | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8145:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x81f3:// prim,k5,t0,shade
            fT1 = 0x20;
            fS1c = UsePrim  | UseT0;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8143:
                    break;
                case 0xaef0:// cmb,k5,plf,env
                    fS1c = UseT0 | UseShade;
                    fS2c = UseEnv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8341: // t0,shade,prim,shade
            fT1 = 0x20;
            fS1c = UseT0    | UsePrim;
            fS2c = UseShade | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8341:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8531: // t0,prim.env,shade
            fT1 = 0x20;
            fS1c = UseT0   | UseEnv;
            fS2c = UsePrim | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8531:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8545:
            fT1 = 0x20;
            fS1c = UseEnv   | UseEnv;
            fS2c = UseShade | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8545:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8546://cent,shade,env,shade
            fT1 = 0x10;//0x20;
            fS1c = UseShade | UseEnvIv;//UseT0    | UseEnv;
            fS2c = UseShade | UseEnvIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8546:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8841: // t0,shade,t0a,shade
            fT1 = 0x20;
            fS1c = UseShade | UseMT0AlphaIv;
            fS2c = UseT0    | UseMT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8841:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8a43: // prim,shade,prima,shade
            fT1 = 0x20;
            fS1c = UsePrim  | UseMPrimAlpha;
            fS2c = UseShade | UseMPrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8a43:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8c21: // t0,t1,enva shade // needs fixed
            fT1 = 0x20;
            fS1c = UseT0 | UseMEnvAlpha;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8c21:
                    break;
                case 0xe350: // cmb,env,prim,ca
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;                  
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;                  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8c45:// env,shade,enva,shade
            fT1 = 0x20;
            fS1c = UseEnv  | UseMEnvAlpha;
            fS2c = UseShade | UseMEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8a43:
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x8e31: // t0,prim,plf,shade
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8e31:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x9f31:
            fT1 = 0x10;
            fS1c = UseShade;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x8e31:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;  
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x9ff6: // cent,k5,0,shade
            fT1 = 0x10;
            fS1c = UseShade;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x9ff6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x9fff: // k5,k5,0,shade
            fT1 = 0x10;
            fS1c = UseShade;
            fS2c = UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0x9fff:
                    break;
                case 0x0503: // prim,cmb,env,cmb
                    fT1 = 0x20;
                    fS1c = UseShade | UseEnvIv;
                    fS2c = UsePrim | UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa151: // t0,env,t0,env
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UseEnv  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa151:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa153: // prim,env,t0,env
            fT1 = 0x20;
            fS1c = UsePrim | UseT0;
            fS2c = UseEnv  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa153:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa154: // shade,env,t0,env
            fT1 = 0x20;
            fS1c = UseShade | UseT0;
            fS2c = UseEnv  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa154:
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa156: // cent,env,t0,env
            fT1 = 0x10;
            fS1c = UseEnv  | UseT0Iv;
            fS2c = UseEnv  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa154:
                    break;
                case 0x2a10:
                    fT1 = 0x20;
                    fS1c = UseMPrimAlphaIv | UseT0;
                    fS2c = UseEnv | UseMPrimAlpha | UseT0Iv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa251:
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa251:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa253: // prim,env,t1,env
            fT1 = 0x20;
            fS1c = UsePrim | UseT1;
            fS2c = UseEnv | UseT1Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa253:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa351: // t0,env,prim,env
            fT1 = 0x20;
            fS1c = UseT0 | UsePrim;
            fS2c = UseEnv | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa351:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa353: // prim,env,prim,env
            fT1 = 0x20;
            fS1c = UsePrim;
            fS2c = UseEnv | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa356:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa356: // cent,env,prim,env
            fT1 = 0x10;//0x20;
            fS1c = UseEnv | UsePrimIv;//UseT0 | UsePrim;
            fS2c = UseEnv | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa356:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa3f4: // shad,k5,prim,env
            fT1 = 0x20;
            fS1c = UseShade | UsePrim;
            fS2c = UseEnv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa3f4:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa3f1: // t0,k5,prim,env
            fT1 = 0x20;
            fS1c = UseT0 | UsePrim;
            fS2c = UseEnv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa3f1:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa451: // t0,env,shade,env
            fT1 = 0x20;
            fS1c = UseT0 | UseShade;
            fS2c = UseEnv | UseShadeIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa451:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa851: // t0,env,t0a,env
            fT1 = 0x20;
            fS1c = UseT0  | UseMT0Alpha;
            fS2c = UseEnv | UseMT0AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa851:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xa853: // prim,env,t0a,env
            fT1 = 0x20;
            fS1c = UsePrim | UseMT0Alpha;
            fS2c = UseEnv  | UseMT0AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xa853:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xab51: // t0,env,shadea,env
            fT1 = 0x20;
            fS1c = UseT0  | UseMShadeAlpha;
            fS2c = UseEnv | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xab51:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xab53: // prim,env,shadea,env
            fT1 = 0x20;
            fS1c = UsePrim | UseMShadeAlpha;
            fS2c = UseEnv  | UseMShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xab53:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xae53:
            fT1 = 0x20;
            fS1c = UsePrim | UseT0;
            fS2c = UseEnv  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xae53:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xaef1: // t0,k5,plf,env
            fT1 = 0x20;
            fS1c = UseT0;
            fS2c = UseEnv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xaef1:
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xbfff:
            fT1 = 0x10;
            fS1c = UseEnv;
            fS2c = UseEnv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xbfff:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xdfff: // fix me!!!
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xdfff:
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe132:
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe132:
                    break;
                case 0x64f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe136: // cent,prim,t0,ca
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe136:
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe154: // shade,env,t0,ca // can't do correctly yet.
            fT1 = 0x10;
            fS1c = UseShade | UseT0;
            fS2c = UseShade | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe154:
                    break;
                case 0x64f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f1: // t0,k5,t0,ca
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f1:
                    break;
                case 0x64f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f2: // t1,k5,t0,ca
            fT1 = 0x10;
            fS1c = UseT0 | UseT1;
            fS2c = UseT0 | UseT1;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f2:
                    break;
                case 0x64f0: // cm,k5,shade,prim
                    fT1 = 0x20;
                    fS1c |= UseShade;
                    fS2c = UsePrim;
                    break;
                case 0x8530: // cm,prim,env,shade
                    //fT1 = 0x20;
                    //fS1c |= UseShade;
                    //fS2c = UsePrim;
                    break;
                case 0xa3f0: // cmb,k5,prim,env
                    fT1 = 0x20;
                    fS1c |= UsePrim;
                    fS2c = UseEnv;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xeaf0:
                    fS1c |= UseMPrimAlpha;
                    fS2c |= UseMPrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f3: // prim,k5,t0,ca
            fT1 = 0x10;
            fS1c = UsePrim | UseT0;
            fS2c = UsePrim | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f3:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f4: // shade,k5,t0,ca
            fT1 = 0x10;
            fS1c = UseShade | UseT0;
            fS2c = UseShade | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f4:
                    break;
                case 0x0703:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseMPrimAlpha;
                    fS2c = UseShade | UseT0 | UseMPrimAlphaIv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f5: // env,k5,t0,ca
            fT1 = 0x10;
            fS1c = UseEnv | UseT0;
            fS2c = UseEnv | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f5:
                    break;
                case 0x0703:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseMPrimAlpha;
                    fS2c = UseShade | UseT0 | UseMPrimAlphaIv;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe1f6: // cent,k5,t0,cmba
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe1f6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe2f1: // t0,k5,t1,cmba
            fT1 = 0x10;
            fS1c = UseT1 | UseT0;
            fS2c = UseT1 | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe2f1:
                    break;
                case 0x0703:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseMPrimAlpha;
                    fS2c = UseShade | UseT0 | UseMPrimAlphaIv;
                    break;
                case 0x2710:
                    fS1c = UseT0;
                    fS2c = UseT0;
                    break;
                case 0x6730: // cmb,prim,cmba,prim // needs work.
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe3f0: // cmb,k5,prim,cmba
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f1: // t0,k5,prim,cmba
            fT1 = 0x10;
            fS1c = UsePrim | UseT0;
            fS2c = UsePrim | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f1:
                    break;
                case 0x0c05:
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UsePrim | UseT0 | UseMEnvAlphaIv;
                    break;
                case 0xa053:
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0;
                    fS2c = UseEnv  | UseT0Iv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f4:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f2: //t1,k5,prim,cmba
            fT1 = 0x10;
            fS1c = UseT1 | UsePrim;
            fS2c = UseT1 | UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f2:
                    break;
                case 0x64f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f3: // prim,k5,prim,ca
            fT1 = 0x10;
            fS1c = UsePrim;
            fS2c = UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f3:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f4: // shade,k5,prim,ca
            fT1 = 0x10;
            fS1c = UsePrim | UseShade;
            fS2c = UsePrim | UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f4:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f6: // cent,k5,prim,ca
            fT1 = 0x10;
            fS1c = UsePrim;// | UseT0;
            fS2c = UsePrim;// | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe3f7: // ca,k5,prim,ca
            fT1 = 0x10;
            fS1c = UsePrim;
            fS2c = UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe3f7:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe451: //t0,env,shade,ca fixme
            fT1 = 0x10;
            fS1c = UseShade | UseT0;
            fS2c = UseShade | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f1:
                    break;
                case 0x0106: // cent,cmb,t0,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseT0Iv;
                    fS2c = UseT0;
                    break;
                case 0x04f2: // t1,0,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseShade | UseT1;
                    fS2c = UseT0;
                    break;
                case 0x0506: // cent,cmb,env,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseEnvIv;
                    fS2c = UseT0  | UseEnv;
                    break;
                case 0x05f6: // cent,k5,env,cmb ????
                    fT1 = 0x20;
                    fS1c = UseT0 | UseShade;
                    fS2c = UseT0 | UseEnv;
                    break;
                case 0x0b06: // cent,cmb,shadea,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseMShadeAlphaIv;
                    fS2c = UseT0  | UseShadeAlpha;
                    break;
                case 0x0c05:
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UseT0  | UseShade | UseMEnvAlphaIv;
                    break;
                case 0x6450: //// fixme cmb, env, shade. prim
                    fS1c = UseShade | UseT0;
                    fS2c = UseShade | UseT0;
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseShade;
                    fS2c = UseEnv  | UseT0Iv | UseShadeIv;
                    break;
                case 0xa3f0: // cmb,k5,prim,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseShade | UseT0;
                    fS2c = UseEnv;
                    break;
                case 0xac50: // cmb,env,enva,env
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0 | UseMEnvAlpha;
                    fS2c = UseEnv | UseMEnvAlphaIv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xecf0: // cmb,k5,enva,ca
                    fS1c |= UseMEnvAlpha;
                    fS2c |= UseMEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe4f1: //t0,k5,shade,ca
            fT1 = 0x10;
            fS1c = UseShade | UseT0;
            fS2c = UseShade | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f1:
                    break;
                case 0x0106: // cent,cmb,t0,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseT0Iv;
                    fS2c = UseT0;
                    break;
                case 0x04f2: // t1,0,shade,cmb
                    fT1 = 0x20;
                    fS1c = UseShade | UseT1;
                    fS2c = UseT0;
                    break;
                case 0x0506: // cent,cmb,env,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseEnvIv;
                    fS2c = UseT0  | UseEnv;
                    break;
                case 0x05f6: // cent,k5,env,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseT0 | UseShade;
                    fS2c = UseT0 | UseEnv;
                    break;
                case 0x0b06: // cent,cmb,shadea,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseT0 | UseMShadeAlphaIv;
                    fS2c = UseT0  | UseShadeAlpha;
                    break;
                case 0x0c05:
                    fT1 = 0x20;
                    fS1c = UseEnv | UseMEnvAlpha;
                    fS2c = UseT0  | UseShade | UseMEnvAlphaIv;
                    break;
                case 0x6450: //// fixme cmb, env, shade. prim
                    fS1c = UseShade | UseT0;
                    fS2c = UseShade | UseT0;
                    break;
                case 0xa053: // prim,env,cmb,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseT0 | UseShade;
                    fS2c = UseEnv  | UseT0Iv | UseShadeIv;
                    break;
                case 0xa3f0: // cmb,k5,prim,env
                    fT1 = 0x20;
                    fS1c = UsePrim | UseShade | UseT0;
                    fS2c = UseEnv;
                    break;
                case 0xac50: // cmb,env,enva,env
                    fT1 = 0x20;
                    fS1c = UseShade | UseT0 | UseMEnvAlpha;
                    fS2c = UseEnv | UseMEnvAlphaIv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xecf0: // cmb,k5,enva,ca
                    fS1c |= UseMEnvAlpha;
                    fS2c |= UseMEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe4f3: // prim,k5,shade,ca
            fT1 = 0x10;
            fS1c = UseShade | UsePrim;
            fS2c = UseShade | UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f3:
                    break;
                case 0x0205: // env,cmb,t1,cmb
                    fT1 = 0x20;
                    fS1c = UseT1 | UseEnv;
                    fS2c = UsePrim | UseShade | UseT1Iv;
                    break;
                case 0x0b06: // cent,cmb,shadea,cmb
                    fT1 = 0x10;
                    fS1c = UseShade | UsePrim | UseMShadeAlphaIv;//UseT0 | UseMShadeAlpha;
                    fS2c = UseShade | UsePrim | UseMShadeAlphaIv;
                    break;
                case 0x05f6: // cent,k5,env,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UsePrim | UseShade | UseEnvIv;
                    fS2c = UseT0 | UseEnv;
                    break;
                case 0x0c01:
                    fT1 = 0x20;
                    fS1c = UseT0 | UseMEnvAlpha;
                    fS2c = UsePrim | UseShade | UseMEnvAlphaIv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe4f4: // shade,k5,shade,ca
            fT1 = 0x10;
            fS1c = UseShade;// | UseShade;
            fS2c = UseShade;// | UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f4:
                    break;
                case 0x0a03:
                    fT1 = 0x20;
                    fS1c = UseShade | UseMPrimAlpha;
                    fS2c = UseShade | UseMPrimAlphaIv;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe4f5: // env,k5,shade,ca
            fT1 = 0x10;
            fS1c = UseShade | UseEnv;
            fS2c = UseShade | UseEnv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f4:
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe4f6: // cent,k5,shade,ca
            fT1 = 0x10;
            fS1c = UseShadeIv;// | UseT0;
            fS2c = UseShadeIv;// | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe4f6:
                    break;
                case 0x0a03:
                    fT1 = 0x20;
                    fS1c = UseShade | UseMPrimAlpha;
                    fS2c = UseShade | UseMPrimAlphaIv;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe5f1: // t0,k5,env,ca
            fT1 = 0x10;
            fS1c = UseEnv | UseT0;
            fS2c = UseEnv | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe5f1:
                    break;
                case 0x0c06: // cent,cmb,enva,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseEnv | UseT0 | UseMEnvAlphaIv;
                    fS2c = UseT0  | UseEnvAlpha;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe5f3: // prim,k5,env,ca
            fT1 = 0x10;
            fS1c = UseEnv | UsePrim;
            fS2c = UseEnv | UsePrim;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe5f3:
                    break;
                case 0x0c01:
                    fT1 = 0x20;
                    fS1c = UseT0 | UseMEnvAlpha;
                    fS2c = UsePrim | UseShade | UseMEnvAlphaIv;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe5f0:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe5f4: // shade,k5,env,cmba
            fT1 = 0x10;
            fS1c = UseEnv | UseShade;
            fS2c = UseEnv | UseShade;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe5f1:
                    break;
                case 0x0c06: // cent,cmb,enva,cmb
                    fT1 = 0x10;//0x20;
                    fS1c = UseShade | UseEnv | UseMEnvAlphaIv;
                    fS2c = UseT0  | UseEnvAlpha;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe5f6: // cent,k5,env,cmba
            fT1 = 0x10;
            fS1c = UseEnv;// | UseT0;
            fS2c = UseEnv;// | UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe5f6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xe6f6: // cent,k5,cent,cmba ?????
            fT1 = 0x10;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xe6f6:
                    break;
                case 0xe4f0:
                    fS1c |= UseShade;
                    fS2c |= UseShade;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xeaf1: // t0,k5,prima,ca
            fT1 = 0x10;
            fS1c = UseT0 | UseMPrimAlpha;
            fS2c = UseT0 | UseMPrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xeaf1:
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xeaff: // k5,k5,prima,ca // don't know about this one.
            fT1 = 0x10;
            fS1c = UseMPrimAlpha;
            fS2c = UseMPrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xeaff:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xebf1:
            fT1 = 0x10;
            //fS1c = UseShadeAlpha | UseT0;
            //fS2c = UseShadeAlpha | UseT0;
            fS1c = UseT0;
            fS2c = UseT0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xebf1:
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0xffff:
            fT1 = 0x10;
            if(tcycle1A == 0x0fff)
                fS1c = UseZeroAlpha;
            else
                fS1c = UsePrim;
            fS2c = UseZeroAlpha;
            //break;

            //fS1c = UseMT0AlphaIv | UseMPrimAlpha;//UseZeroAlpha;
            //fS2c = UseMT0AlphaIv | UseMPrimAlpha;//UseZeroAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2C)
                {
                case 0x1fff:
                case 0xffff:
                    break;
                case 0x6f54: //shade,env,k5,prim
                    fT1 = 0x20;
                    fS1c = UsePrim;
                    fS2c = UseShade;
                    break;
                case 0xe0f3:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
                    break;
                case 0xe0f5:
                    fS1c |= UseEnv;
                    fS2c |= UseEnv;
                    break;
                case 0xe3f0:
                    fS1c |= UsePrim;
                    fS2c |= UsePrim;
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

        // handle alpha combine
        switch(tcycle1A)
        {
        case 0x0209: // t0,t0,Cmb,t0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x020a:
                    break;
                case 0x01e7: // 0,shade,0,cmb
                    break;
                case 0x0738: //need fixed
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x09ff:
                    fS1a = UseShadeAlpha;
                    fS2a = UseShadeAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f39: // t0,0,shade,0
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x020a: // t1,t0,Cmb,t0
            fS1a = UseT0Alpha;// | UseT1Alpha;
            fS2a = UseT0Alpha;// | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x020a:
                    break;
                case 0x01e7: // 0,shade,0,cmb
                    break;
                case 0x0738: //need fixed
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x09ff:
                    fS1a = UseShadeAlpha;
                    fS2a = UseShadeAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f39: // t0,0,shade,0
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0f7c:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02bd:// env,0,t1,t0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x02bd:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02ca: // t1,t0,prim,t0
            fS1a = UseT0Alpha;// | UsePrimAlphaIv;
            fS2a = UseT0Alpha;// UseT1Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x02ca:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02ce:// 1,t0,prim,t0
            fS1a = UseT0AlphaIv | UsePrimAlphaIv;
            fS2a = UseT0AlphaIv | UsePrimAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x02ce:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x02cf:// 0,t0,prim,t0
            fS1a = UseT0Alpha | UsePrimIv;
            fS2a = UseT0Alpha | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x02cf:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x030a:// t1,t0,shade,t0 // needs fixed
            fS1a = UseT1Alpha;// | UsePrimIv;
            fS2a = UseT1Alpha;// | UsePrimIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x02cf:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x030f: // 0,T0,Shade,T0
            fS1a = UseT0Alpha | UseShadeAlphaIv;
            fS2a = UseT0Alpha | UseShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x030f:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x034a: // t1,t0,env,t0
            fS1a = UseT0Alpha;// UseT0Alpha | UseEnvAlphaIv;
            fS2a = UseT0Alpha;// UseT1Alpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x034a:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0372: // t1,1,env,t0 // needs fixed
            //fT1 = 0x20;
            fS1a = UseT0Alpha;// UseT1Alpha * UseEnvAlpha;
            fS2a = UseT0Alpha;// UseT0Alpha * UseEnvAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0372:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x037a: // T1,0,Env,T0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0372:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0389: // t0,t0,1(plf),t0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0389:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x038a: // t1,t0,1(plf),t0 // t1 only? 
            //fT1 = 0x20;
            fS1a = UseT0Alpha;// UseT1Alpha | UseT0Alpha;
            fS2a = UseT0Alpha;// UseT0Alpha | UseT0AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x038a:
                    break;
                case 0x0178:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0eb8:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03a2: // T1,Shade,1,T0 // needs fixed
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03b2:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03b2: //  t1,1,1(plf),t0
            fS1a = UseT0Alpha;// UseT0Alpha | UseT0AlphaIv;
            fS2a = UseT0Alpha;// UseT1Alpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03b2:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0ef9:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03b3: // Prim,1,1(plf),T0 // needs fixed
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03b2:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ba:// t1,0,1(plf),t0 // needs fixed
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03ba:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0eb8:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ca: // t1,t0,0,t0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03ca:
                    break;
                case 0x0eb8:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x03ff: // 0,0,0,t0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x03ff:
                    break;
                case 0x0af8: // cmb,0,prim,env
                    //fS1a |= UsePrimAlpha;
                    //fS2a |= UsePrimAlpha;
                    break;
                case 0x0e3d:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0ef8: // cmb,0,prim,0
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x04d1: // t0,t1,prim,t1
            fS1a = UseT1Alpha; // UseT0Alpha;
            fS2a = UseT1Alpha; // UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x04d1:
                case 0x03ff:
                    break;
                case 0x0af8: // cmb,0,prim,env
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0571: // T0,1(plf),Env,T1 // needs fixed
            fS1a = UseT1Alpha;
            fS2a = UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x05b1:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x05b1: // t0,1,1,t1
            fS1a = UseT1Alpha; // UseT0Alpha;
            fS2a = UseT1Alpha; // UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x05b1:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x05ff: // 0,0,0,t1
            fS1a = UseT1Alpha;
            fS2a = UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x05ff:
                    break;
                case 0x0af8: // cmb,0,prim,env
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x065d: // env,t0,prim,t0
            fS1a = UseT0AlphaIv | UsePrimAlpha;
            fS2a = UseT0AlphaIv | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x05ff:
                    break;
                case 0x0af8: // cmb,0,prim,env
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x065f: // 0,t0,prim,t0
            fS1a = UseT0AlphaIv | UsePrimAlpha;
            fS2a = UseT0AlphaIv | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x05ff:
                    break;
                case 0x0af8: // cmb,0,prim,env
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x071a: // t1,shade,prim,shade ??? fixme
//          fS1a = UseShadeAlpha | UsePrimAlphaIv;
//          fS2a = UseShadeAlpha | UsePrimAlphaIv;
//          fS1a = UsePrimAlpha | UseT1Alpha;
//          fS2a = UsePrimAlpha | UseT1Alpha;
            fS1a = UseT1Alpha;
            fS2a = UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x071a:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x071d: // env,prim,shade,prim
            fS1a = UseEnvAlpha; //UseShadeAlpha | UseEnvAlpha;
            fS2a = UseEnvAlpha; //UseShadeAlpha | UseEnvAlpha;
//          fS1a = UsePrim; //UseShadeAlpha | UseEnvAlpha;
//          fS2a = UsePrim; //UseShadeAlpha | UseEnvAlpha;
//          fS1a = UsePrim | UseShadeAlpha;
//          fS2a = UsePrim | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x071d:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0779: // t0,0,env,prim
            fS1a = UsePrimAlpha | UseT0Alpha;
            fS2a = UsePrimAlpha | UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x078e:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x078e: // 1,T0,1(plf),Prim // fixme
            fS1a = UsePrimAlpha | UseT0AlphaIv;
            fS2a = UsePrimAlpha | UseT0AlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x078e:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07fb: //  prim,0,env,0
            fS1a = UsePrimAlpha | UseEnvAlpha;
            fS2a = UsePrimAlpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x07fb:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07fe: //  plf,0,0,prim
            fS1a = UsePrimAlpha;
            fS2a = UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x07fe:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x07ff: // 0,0,0,prim
            fS1a = UsePrimAlpha;
            fS2a = UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x07ff:
                    break;
                case 0x0eb8:
                    fS1a |= UseT1Alpha;
                    fS2a |= UseT1Alpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0863: // prim,shade,t0,shade
            fS1a = UsePrimAlpha;
            fS2a = UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0863:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha   | UseT0Alpha;
                    fS2a |= UseShadeAlpha | UseT0AlphaIv;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x09ff: // 0,0,0,shade
            fS1a = UseShadeAlpha;
            fS2a = UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x09ff:
                    break;
                case 0x07ff:
                    fS1a = UsePrimAlpha;
                    fS2a = UsePrimAlpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0f7c:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0a6b: // prim,env,t0,env
            fS1a = UsePrimAlpha | UseT0;
            fS2a = UseEnvAlpha  | UseT0Iv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0a6b:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0abe: // 1,0,t1,env
            fS1a = UseT1Alpha;
            fS2a = UseEnvAlpha;
            fT1 = 0x20;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0a6b:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ae9: // t0,env,prim,env // fixme
            fS1a = UseT0Alpha;// | UsePrimAlpha;
            fS2a = UseT0Alpha;// | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0ae9:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0af9: // t0,0,prim,env
            fS1a = UseT0Alpha | UsePrimAlpha;
            fS2a = UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0af9:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0b29: // t0,env,shade,env
            fS1a = UseT0Alpha;// | UseShadeAlpha;
            fS2a = UseT0Alpha;// | UseShadeAlpha;//UseEnvAlpha | UseShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0b29:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0b3e: // 1(plf),0,shade,env
            fS1a = UseEnvAlpha | UseShadeAlpha;
            fS2a = UseEnvAlpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0b3e:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0bff: // 0,0,0,env
            fS1a = UseEnvAlpha;
            fS2a = UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0bff:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0db6: // 1,1,1,1 (plf,plf,plf,plf)
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0db6:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0df9: // t0,0,0,1(plf)
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0df9:
                    break;
                case 0x0ef8: // cm.0,prim,0
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0dff: // 0,0,0,1
            fS1a = 0;//UseT0Alpha;
            fS2a = 0;//UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0dff:
                    break;
                case 0x0ef8: // cm.0,prim,0
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e63: // prim,shade,t0,0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e63:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e66: // shade,env,t0,0
            fS1a = UseT0Alpha | UseShadeAlphaIv;
            fS2a = UseT0Alpha | UseShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e66:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e6c: //  shade,env,t0,0
            fS1a = UseT0Alpha | UseShadeAlphaIv;
            fS2a = UseT0Alpha | UseShadeAlphaIv;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e6c:
                    break;
                case 0x00fe: // 1(plf),0,prim,cmb
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e79: // T0,0,T0,0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e79:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7a: // t1,0,t0,0
            fS1a = UseT0Alpha | UseT1Alpha;
            fS2a = UseT0Alpha | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e7a:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7b: // prim,0,t0,0
            fS1a = UseT0Alpha | UsePrimAlpha;
            fS2a = UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e7b:
                    break;
                case 0x01ba:
                    break;
                case 0x0e38:
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7c: //shade,0,t0,0
            fS1a = UseT0Alpha | UseShadeAlpha;
            fS2a = UseT0Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e7c:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7d: // env,0,t0,0
            fS1a = UseT0Alpha | UseEnvAlpha;
            fS2a = UseT0Alpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e7d:
                    break;
                case 0x01ba:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0e7e: // 1,0,t0,0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0e7e:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0eb9: // t0,0,t1,0
            fS1a = UseT0Alpha | UseT1Alpha;
            fS2a = UseT0Alpha | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0eb9:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ebb: // Prim,0,T1,0
            fS1a = UsePrimAlpha | UseT1Alpha;
            fS2a = UsePrimAlpha | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0eb9:
                    break;
                case 0x01ba:
                    break;
                case 0x0dff:
                    fS1a = 0;//UseT0Alpha;
                    fS2a = 0;//UseT0Alpha;
                    //fS1a |= UsePrimAlpha;
                    //fS2a |= UsePrimAlpha;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ebe: // 1,0,t1,0
            fS1a = UseT0Alpha | UseT1Alpha;
            fS2a = UseT0Alpha | UseT1Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0ebe:
                    break;
                case 0x01ba:
                    break;
                case 0x0dff:
                    fS1a = 0;//UseT0Alpha;
                    fS2a = 0;//UseT0Alpha;
                    //fS1a |= UsePrimAlpha;
                    //fS2a |= UsePrimAlpha;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ece: // plf,t0,prim,0
            fS1a = UseT0Alpha | UsePrimAlpha;
            fS2a = UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0ebe:
                    break;
                case 0x01ba:
                    break;
                case 0x0dff:
                    fS1a = 0;//UseT0Alpha;
                    fS2a = 0;//UseT0Alpha;
                    //fS1a |= UsePrimAlpha;
                    //fS2a |= UsePrimAlpha;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ee1: // t0,shade,prim,0 // needs fixed
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0ee1:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0ef9: // t0,0,prim,0
            fS1a = UseT0Alpha | UsePrimAlpha;
            fS2a = UseT0Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0ef9:
                    break;
                case 0x01ba: // t1,0,plf,cmb
                    fT1 = 0x20;
                    fS2a = UseT1Alpha | UseT0Alpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efa: // t1,0,prim,0
            fS1a = UseT1Alpha | UsePrimAlpha;
            fS2a = UseT1Alpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0efa:
                    break;
                case 0x01ba:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efb: // prim,0,prim,0
            fS1a = UsePrimAlpha;
            fS2a = UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0efa:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0efc: // shade,0,prim,0
            fS1a = UseShadeAlpha | UsePrimAlpha;
            fS2a = UseShadeAlpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0efc:
                    break;
                case 0x01ba:
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f0e: // 1,t0,shade,0
            fS1a = UseT0AlphaIv | UseShadeAlpha;
            fS2a = UseT0AlphaIv | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f0e:
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f39: // t0,0,shade,0
            fS1a = UseT0Alpha | UseShadeAlpha;
            fS2a = UseT0Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f39:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f3a:// t1,0,shade,0
            fS1a = UseT1Alpha | UseShadeAlpha;
            fS2a = UseT1Alpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f3a:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f3b:
            fS1a = UsePrimAlpha | UseShadeAlpha;
            fS2a = UsePrimAlpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f3b:
                    break;
                case 0x0085: // enva,cmba,t1a,cmba ??? fixme
                    //fS1a |= UseT1AlphaIv;
                    //fS2a |= UseT1AlphaIv;
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f3d: // env,0,shade,0
            fS1a = UseEnvAlpha | UseShadeAlpha;
            fS2a = UseEnvAlpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f3d:
                    break;
                case 0x0085: // enva,cmba,t1a,cmba
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f3e: // 1,0,shade,0
            fS1a = UseShadeAlpha;
            fS2a = UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f3e:
                    break;
                case 0x0085: // enva,cmba,t1a,cmba
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f79: // t0,0,env,0
            fS1a = UseT0Alpha | UseEnvAlpha;
            fS2a = UseT0Alpha | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f79:
                    break;
                case 0x01ba:
                    fT1 = 0x20l;
                    fS2a = UseT0Alpha | UseT1Alpha;
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f7a: // t1,0,env,0
            fS1a = UseT1Alpha | UseEnvAlpha;
            fS2a = UseT1Alpha | UseEnvAlpha;
//          fS1a = UseT0 | UseEnvAlpha;
//          fS2a = UseT0 | UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f7a:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f7b: // prim,0,env,0
            fS1a = UseEnvAlpha | UsePrimAlpha;
            fS2a = UseEnvAlpha | UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f7b:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f7c:
            fS1a = UseEnvAlpha | UseShadeAlpha;
            fS2a = UseEnvAlpha | UseShadeAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f7c:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f7e: // plf,0,env,0
            fS1a = UseEnvAlpha;
            fS2a = UseEnvAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f7e:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f38:
                    fS1a |= UseShadeAlpha;
                    fS2a |= UseShadeAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0f9e: // ?????
            fS1a = UsePrimAlpha;
            fS2a = UsePrimAlpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0f9e:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0fb9: // t0,0,1(plf),0
            fS1a = UseT0Alpha;
            fS2a = UseT0Alpha;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0fb9:
                    break;
                case 0x01ba:
                    break;
                case 0x0ef8:
                    fS1a |= UsePrimAlpha;
                    fS2a |= UsePrimAlpha;
                    break;
                case 0x0f78:
                    fS1a |= UseEnvAlpha;
                    fS2a |= UseEnvAlpha;
                    break;
                case 0x0fb8:
                    fS1a |= UseT0Alpha;
                    fS2a |= UseT0Alpha;
                    break;
                default:
                    checkup = 4;
                    break;
                }
            }
            break;
        case 0x0fff:
            fS1a = 0;
            fS2a = 0;
            if (t_cycle_mode == CYCLE_MODE2)
            {
                switch(tcycle2A)
                {
                case 0x01ff:
                case 0x0fff:
                    break;
                case 0x0dff:
                    fS1a = 0;//UseT0Alpha;
                    fS2a = 0;//UseT0Alpha;
                    //fS1a |= UseT0Alpha;
                    //fS2a |= UseT0Alpha;
                    break;
                case 0x0f7a:
                    fS1a = UseT1Alpha | UseEnvAlpha;
                    fS2a = UseT1Alpha | UseEnvAlpha;
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
        int xrt = 0;
#ifdef DEBUGCC
        char output[1024];
        if(t_cycle_mode == 0)
        {
        sprintf(output,"Mode1C = %04x\tCycleMode = %i\n"
                       "Mode1A = %04x\n\n"
                       "a0  = %s  \tb0  = %s  \tc0  = %s  \td0  = %s\n\n"
                       "Aa0 = %s  \tAb0 = %s  \tAc0 = %s  \tAd0 = %s",
                cycle1C,t_cycle_mode,
                cycle1A,
                Mode[uc0_a0],   Mode[uc0_b0],   Mode[uc0_c0],   Mode[uc0_d0],
                Alpha[uc0_Aa0], Alpha[uc0_Ab0], Alpha[uc0_Ac0], Alpha[uc0_Ad0]);
        }
        else
        {
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
        }

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

    Prev_fT1= fT1;
    Prev_fS1c = fS1c;
    Prev_fS2c = fS2c;
    Prev_fS1a = fS1a;
    Prev_fS2a = fS2a;

}

void SetFastCombine(int vn[3], _u32 t1, _u32 s1, _u32 s2)
{
    if ((s1 & UseZeroAlpha) == UseZeroAlpha) return; // Hack!!
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

void FlushVisualTriangle(int vn[])
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
//  else if(cycle_mode)
//  {
//      MDrawVisualTriangle(vn,0x10,UseShade,0);
//  }
//
//  if (checkup != 0)
//  {       
//      MDrawVisualTriangle(vn,0x10,UseShade,0);
//  }
}


//#undef DEBUGCC
