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

_u32 cycle1C;
_u32 cycle1A;

_u32 cycle2C;
_u32 cycle2A;

BOOL UseT1Cyc1 = FALSE;
BOOL UseT1Cyc2 = FALSE;

t_Combine CombineArray[4];
int CombinePasses;

extern _u32 TexMode;
extern _u32 TexColor;

extern int  Vtidx[256];
extern float VtSc[256];
extern float VtTc[256];
extern int Vtcnt;
void FlushVisualTriangle(int vn[]);

  static char *Mode[] = { "CC_COMBINED" ,             "CC_TEXEL0" ,
                          "CC_TEXEL1" ,       "CC_PRIMITIVE" ,
                          "CC_SHADE" ,            "CC_ENVIRONMENT" ,
                          "CC_CENTER" ,       "CC_COMBINED_ALPHA" ,
                          "CC_TEXEL0_ALPHA" ,    "CC_TEXEL1_ALPHA" ,
                          "CC_PRIMITIVE_ALPHA" , "CC_SHADE_ALPHA" ,
                          "CC_ENV_ALPHA" ,        "CC_LOD_FRACTION" ,
                          "CC_PRIM_LOD_FRAC" ,   "CC_K5" ,
                          "CC_UNDEFINED" ,        "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,        "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,        "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,       "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,       "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,       "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,       "CC_UNDEFINED" ,
                          "CC_UNDEFINED" ,       "CC_0" };

 static char *Alpha[] = { "AC_COMBINED" ,      "AC_TEXEL0" ,
                          "AC_TEXEL1" ,        "AC_PRIMITIVE" , 
                          "AC_SHADE" ,          "AC_ENVIRONMENT" , 
                          "AC_PRIM_LOD_FRAC" , "AC_0"};

void rdp_setcombine()
{
    LOG_TO_FILE("%08X: %08X %08X CMD RDP_SETCOMBINE \n", ADDR, CMD0, CMD1);

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

#ifdef LOG_ON

/*      PRINT_RDP_INFO("SETCOMBINE - ignored")
        PRINT_RDP_MNEMONIC("    mode0=$%06lx; mode1=$%08lx\n",
               uc0_cycle1,
               uc0_cycle2);
        PRINT_RDP_WARNING("ignored")
        PRINT_RDP_MNEMONIC("a0  = %s, b0  = %s, c0  = %s, d0  = %s\n",
               Mode[uc0_a0],Mode[uc0_b0],Mode[uc0_c0],Mode[uc0_d0]);
        PRINT_RDP_MNEMONIC("Aa0 = %s, Ab0 = %s, Ac0 = %s, Ad0 = %s\n",
               Alpha[uc0_Aa0],Alpha[uc0_Ab0],Alpha[uc0_Ac0],Alpha[uc0_Ad0]);
        PRINT_RDP_MNEMONIC("a1  = %s, b1  = %s, c1  = %s, d1  = %s\n",
               Mode[uc0_a1],Mode[uc0_b1],Mode[uc0_c1],Mode[uc0_d1]);
        PRINT_RDP_MNEMONIC("Aa1 = %s, Ab1 = %s, Ac1 = %s, Ad1 = %s\n",
               Alpha[uc0_Aa1],Alpha[uc0_Ab1],Alpha[uc0_Ac1],Alpha[uc0_Ad1]);
*/
#endif
} /* static void rdp_setcombine() */

_u32 fPasses,fT1,fS1,fS2;
_u32 sCycleM,sCycle1,sCycle2;

void SetFastCombine(int vn[3], _u32 t1, _u32 s1, _u32 s2)
{
    //fPasses
    fT1 = t1;
    fS1 = s1;
    fS2 = s2;

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
    _u8 checkup = 0;
    TexColor = 0;
    TexMode = 0;

    if ((cycle_mode == CYCLE_MODE1) || (cycle_mode == CYCLE_MODE2))
    {
        switch(cycle1)
        {
/*
        case 0x00000000:
            break;
        // Ultra1
        // ????? t1,t1,lodfrac,t0,t1,t0,comb,t0
        case 0x020a2d12: if (cycle_mode == CYCLE_MODE2)
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                 break;
                            case 0x0738e4f0:
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                 break;
                            case 0x09ffe4f0:
                                 SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                                 break;
                            case 0x0ef8e4f0:
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                 break;
                            case 0x0f38e4f0:
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                 break;
                            default:
                                 checkup = 1;
                                 break;
                            }
                         }
                         TexMode = Tex;
                         SetFastCombine(vn,0x10,UseT0,0);
                         break;
        case 0x02bd3fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha,UseT0 | UseT0Alpha | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         SetFastCombine(vn,0x20,UseT0 | UseT0Alpha,UseT0 | UseT0Alpha | UseEnvAlpha);
                         break;
        //????? not right should be
        // t1,t0,prim,t0,t1,t0,prim,t0
        // env,comb,enva,comb,0,0,0,comb
        case 0x02ca2312: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff0c05:
                                SetFastCombine(vn,0x10,UseEnv | UseEnvAlpha | UseT0Alpha,0);
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlphaIv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x02be22f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            //TexMode = Tex;
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha,UsePrim | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0dff6035:
                                SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha,UsePrim | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         //TexMode = Tex;
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        // t1,t0,prima,t0,t1,t0,prim,t0
        case 0x02ca2a12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            //TexMode = Tex;
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            case 0x0f786035:
                                SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha | UseEnvAlpha,UsePrim | UseT0 | UseT0Alpha | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x02ca2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            //TexMode = Tex;
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            case 0x0f78a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseEnvAlpha, UseEnv | UseEnvAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         //TexMode = Tex;
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x02ce2412: if (cycle_mode == CYCLE_MODE2)  
                         {
                            //TexMode = Tex;
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            case 0x0f380c05:
                                SetFastCombine(vn,0x10,UseEnv | UseEnvAlpha | UseT0Alpha | UseShadeAlpha, 0);
                                SetFastCombine(vn,0x10,UseT0 | UseEnvAlphaIv | UseT0Alpha | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         //TexMode = Tex;
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x02cee3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            //TexMode = Tex;
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         //TexMode = Tex;
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                         break;
        case 0x030fe1f3: if (cycle_mode == CYCLE_MODE2)  // is wrong?
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UseT0 | UseT0Alpha | UseShadeAlphaIv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlphaIv,0);
                         break;
        case 0x034a2c12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01bae4f0:
                                //TexMode = Prim_Sub_Env_Mult_Tex_Add_Env;
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e4f0:
                                // ????SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha | UsePrimAlpha | UseShade,0);
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x034a2c32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShadeAlpha,UseT0Iv | UseT0Alpha | UseEnv | UseShadeAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? t1,t0,plf,t0,t1,plf,plf,t0
        case 0x034a2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff6034:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShade,UseT0Iv | UseT0Alpha | UsePrim);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x034a8c21: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe350://??? need work
                                SetFastCombine(vn,0x20,UseT0 | UsePrim | UseT0Alpha,UseShade | UsePrim | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha,UseShade | UseT0Alpha);
                         }
                         break;
        case 0x03722c12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrimAlpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnv | UsePrimAlpha);
                                break;
                            case 0x0f38a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrim, UseEnv | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03722c32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrimAlpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnv | UsePrimAlpha);
                                break;
                            case 0x0f38a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrim, UseEnv | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x037a2c32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShadeAlpha,UseT0Iv | UseT0Alpha | UseEnv | UseShadeAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03892e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x0f38e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0389e1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x0f38e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038a2132: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038a2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0af8a053: // not right needs + envalpha
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x0f38e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038a2e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038a2ef2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x0f38e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            case 0x0f78e5f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? k5,k50,0,prim,t1,t0,plf,t0
        case 0x038a7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UsePrim,0);
                         }
                         break;
        case 0x038aa153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e0f4:
                                SetFastCombine(vn,0x2,UsePrim | UseT0 | UseT0Alpha | UseShade | UsePrimAlpha, UseEnv | UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha, UseEnv | UseT0Iv | UseT0Alpha);
                         }
                         break;
        case 0x038ae132: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0eb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef864f0:
                                SetFastCombine(vn,0x20,UseT0Alpha | UsePrimAlpha | UsePrim,UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038ae1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0eb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef864f0:
                                SetFastCombine(vn,0x20,UseT0Alpha | UsePrimAlpha | UsePrim,UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x038ae2f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrimAlpha | UsePrim,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03b22e12: if (cycle_mode == CYCLE_MODE2)
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? t1,prim,plf,t0,t1,plf,plf,t0
        case 0x03b22e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Alpha | UseT0,
                                                       UseEnv  | UseT0Alpha | UseT0Iv);
                                break;
                            case 0x0ef8a053:
                                //SetFastCombine(vn,0x23,UsePrim | UsePrimAlpha,UsePrim | UsePrimAlpha | UseT0Iv | UseT0AlphaIv);
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0Alpha | UseT0,
                                                       UseEnv  | UsePrimAlpha | UseT0Alpha | UseT0Iv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03b22ef7: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Alpha | UseT0,
                                                       UseEnv  | UseT0Alpha | UseT0Iv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03b92e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Alpha | UseT0,
                                                       UseEnv  | UseT0Alpha | UseT0Iv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ba2e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShadeAlpha,
                                                       UseEnv  | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ff23f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ff2c15: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseEnv | UseEnvAlpha | UseT0Alpha,0);
                             SetFastCombine(vn,0x10,UseT0 | UseEnvAlphaIv | UseT0Alpha,0);
                         }
                         break;
        case 0x03ff2c31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Iv | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             //TexMode = Tex_Sub_Prim_Mult_EnvAlpha_Add_Tex;
                             //TexColor = rdp_reg.primcolor;
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseEnvAlpha,UseT0 | UseT0Alpha);
                         }
                         break;
        case 0x03ff2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x23,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ff2e31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ff2e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? k5,k5,0,t0,0,0,0,t0
        case 0x03ff3fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff05f6:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseEnv,UseT0 | UseT0Alpha);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            case 0x01ffe0f3:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        //????? k5,k5,0,prim,0,0,0,t0
        case 0x03ff7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UsePrim,0);
                         }
                         break;
        //????? prim,shade,t0,shade,0,0,0,t0
        case 0x03ff8143: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseT0Iv | UseT0Alpha | UseShade);
                         }
                         break;
        // not right.
        //????? t0,shade,prim,shade,0,0,0,to
        case 0x03ff8341: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseShade | UseT0Alpha | UsePrimIv);
                         }
                         break;
        //????? k5,k5,0,shade,0,0,0,t0
        case 0x03ff9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade,0);
                                break;
                            case 0x03ff9fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim | UsePrimAlpha ,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0Alpha | UseShade,0);
                         break;
        case 0x03ffa153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Iv | UseT0Alpha, UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x01ffe0f4:
                                SetFastCombine(vn,0x23,UsePrim | UseShade | UseT0 | UseT0Alpha,UseEnv | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x20,UsePrim | UseT0Iv | UseT0Alpha,UseEnv | UsePrim | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x23,UsePrim | UseShade | UseT0 | UseT0Alpha,UseEnv | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseShade | UseT0Iv | UseT0Alpha, UseEnv | UsePrimAlpha | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x20,UsePrim | UseT0Iv | UseT0Alpha, UseEnv | UseT0Iv | UseT0Alpha);
                         break;
        //????? k5,k5,0,env,0,0,0,t0
        case 0x03ffbfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseEnv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0Alpha | UseEnv,0);
                         break;
        case 0x03ffdfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x03ffe1f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x03ffe1f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? x t1,k5,t0,ca,0,0,0,t0 not done right doesn't use t1
        case 0x03ffe1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x03ffe0f4:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? x prim,k5,t0,ca,0,0,0,t0 not done right doesn't use t1
        case 0x03ffe1f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x03ffe1f3:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                         }
                         break;
        //????? shade,k5,t0,ca,0,0,0,t0
        case 0x03ffe1f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x03ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         }
                         break;
        //????? t0,k5,prim,ca
        //      0,0,0,t0
        //      comb,k5,shade,ca
        //      0,0,0,ca
        case 0x03ffe3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade,0);
                                break;
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimShade | UseShadeAlpha,0);
                                break;
                            case 0x0fb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade,0);
                                break;
                            case 0x0fb81fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                         }
                         break;
        //????? t0,k5,shade,ca,0,0,0,t0
        case 0x03ffe4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {       
                                // ???? cent,k5,env,comb,0,0,0,comb
                            case 0x01ff05f6:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseShade,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                                // ???? comb,k5,prim,env,0,0,0,comb
                            case 0x01ffa3f0:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UseShade,UseT0Alpha | UseEnv);
                                break;
                            case 0x01ffe0f3:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade,0);
                                break;
                            case 0x01ffe0f5:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseShade,0);
                                break;
                                // ???? comb,k5,prim,env,0,0,0,comb
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade,0);
                                break;
                            case 0x01ffe5f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseShade,0);
                                break;
                            case 0x03ffe4f1:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef80c05:
                                SetFastCombine(vn,0x10,UseEnv | UsePrimAlpha | UseT0Alpha | UseEnvAlpha, 0);
                                SetFastCombine(vn,0x10,UseT0 | UseShade | UsePrimAlpha | UseT0Alpha | UseEnvAlphaIv, 0);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            case 0x0f78e0f3:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseEnvAlpha | UseShade,0);
                                break;
                            case 0x0f78e0f5:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseEnvAlpha | UseShade,0);
                                break;
                            case 0x0f78e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseEnvAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         }
                         break;
        case 0x03ffe4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UseShade,0);
                         }
                         break;
        //????? cent,k5,shade,ca,0,0,0,t0
        case 0x03ffe4f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         }
                         break;
        //????? x t0,k5,env,ca,0,0,0,t0
        case 0x03ffe5f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv,0);
                         }
                         break;
        case 0x03ffecf7: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                SetFastCombine(vn,0x10,UseT0AlphaIv | UseEnv | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UseEnv,0);
                         }
                         break;
        //????? k5,k5,0,ca,0,0,0,t0
        case 0x03ffffff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha,0);
                         }
                         break;
        case 0x04d14a21: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x0f78a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha | UseEnvAlpha,UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha | UseEnvAlpha);
                                break;
                            case 0x0f78e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha | UsePrim | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         }
                         break;
        case 0x05714cf1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha | UseEnvAlpha,UseT0Iv | UseT0Alpha | UseEnv | UsePrimAlpha | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                         }
                         break;
        case 0x05b14e31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseT0Iv | UseT0Alpha | UseEnv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                         }
                         break;
        case 0x065f7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UseT0AlphaIv | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         { // ??? needs work
                             SetFastCombine(vn,0x10,UsePrim | UseT0AlphaIv | UsePrimAlpha,0);
                         }
                         break;
        case 0x06bb2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         { // ??? needs work
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         }
                         break;
        case 0x071d6435: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseEnv | UseShade | UseShadeAlpha | UseEnvAlpha,UsePrim | UseShadeIv | UseShadeAlpha | UseEnvAlpha);
                         }
                         break;
        case 0x07ff2316: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha,0);
                         }
                         break;
        case 0x07ff2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38a053:
                                //TexMode = Tex;
                                //SetFastCombine(vn,0x23,UsePrim | UsePrimAlpha | UseShadeAlpha,UseEnv | UsePrimAlpha | UseShadeAlpha | UseT0Iv | UseT0AlphaIv);
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrim, UseEnv | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha,0);
                         }
                         break;
        case 0x07ff3fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha,0);
                         }
        //????? k5,k5,0,shade,0,0,0,prim
        case 0x07ff8143: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseT0 | UsePrim | UsePrimAlpha,UseT0Iv| UseShade | UsePrimAlpha);
                         }
        case 0x07ff84f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x20,UseT0 | UseShade | UsePrim | UsePrimAlpha,UseShade | UsePrim | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseShade | UsePrimAlpha,UseShade | UsePrimAlpha);
                         }
                         break;
        case 0x07ff8531: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x20,UseShade | UsePrimAlpha,UseShade | UseEnv | UsePrimAlpha | UseT0 | UseT0Alpha);
                                break;
                            case 0x0f78e4f0: // not right
                                SetFastCombine(vn,0x2,UseT0 | UseShade | UseEnv | UseEnvAlpha | UsePrimAlpha,UsePrim | UseShade | UseEnvAlpha | UsePrimAlpha | UseT0Iv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnv | UsePrimAlpha,0);
                         }
                         break;
        case 0x07ff8c45: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseEnv | UsePrim | UsePrimAlpha | UseEnvAlpha,0);
                                SetFastCombine(vn,0x10,UseShade | UsePrim | UsePrimAlpha | UseEnvAlphaIv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseEnv | UsePrimAlpha | UseEnvAlpha,0);
                             SetFastCombine(vn,0x10,UseShade | UsePrimAlpha | UseEnvAlphaIv,0);
                         }
                         break;
        case 0x07ff8cf1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe350: // ???? needs work
                                SetFastCombine(vn,0x20,UseShade | UsePrim | UsePrimAlpha,UseT0 | UseEnvAlpha | UsePrim | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseShade | UsePrimAlpha,UseT0 | UseEnvAlpha | UsePrimAlpha);
                         }
                         break;
        //????? k5,k5,0,shade,0,0,0,prim
        case 0x07ff9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                 SetFastCombine(vn,0,UseShade | UsePrimAlpha,0);
                         }
                         break;
        case 0x07ffe1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff8530:
                                SetFastCombine(vn,0x20,UseShade | UsePrimAlpha,UseEnv | UseT0 | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha,0);
                         break;
        //????? t0,k5,prim,ca,0,0,0,t0
        case 0x07ffe3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            case 0x0f78e4f0:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x0fb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha,0);
                         break;
        case 0x07ffe3f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UsePrim | UseShade | UsePrimAlpha,0);
                         break;
        //????? t0,k5,prim,ca,0,0,0,t0
        case 0x07ffe4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha | UseShade,0);
                                break;
                            case 0x0e78e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            case 0x0f381fff:
                                SetFastCombine(vn,0x10,UseT0| UsePrimAlpha | UseShadeAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UseT0 | UsePrimAlpha | UseShade,0);
                         break;
        case 0x07ffe4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseShade,0);
                         break;
        case 0x07ffe4f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UsePrimAlpha | UseShade,0);
                         break;
        case 0x07ff7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x07ff7fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha ,0);
                                break;
                            case 0x0fb81fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0fb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha ,0);
                         break;
        //????? Mario's face
        // not right should be pri,shade,t0,shade,pri,shade,t0,shade
        case 0x08638143: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UsePrimAlpha,UseShade | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                         }
                         break;
        case 0x08857b99: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseShadeAlpha,UsePrim | UseEnvAlpha);
                         }
                         break;
        // cent,lf,undef,env,t1,t0,0,shade
        case 0x09cab1d6: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         { // ???? needs work
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha,0);
                         }
                         break;
        case 0x09ff2413: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UseShade | UseShadeAlpha,UseT0 | UseShadeIv | UseShadeAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseShade | UseShadeAlpha,UseT0 | UseShadeIv | UseShadeAlpha);
                         }
                         break;
        //????? super mario background in intro.
        // k5,k5,0,t0,0,0,0,shade
        case 0x09ff3fff: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UseShadeAlpha,0);
                                break;
                            case 0x09ff1fff:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShadeAlpha,0);
                         }
                         break;

        case 0x09ff6451: if (cycle_mode == CYCLE_MODE2)  
                         {
                         switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseShade | UseShadeAlpha,UsePrim | UseShadeAlpha);
                         }
                         break;
        // cent,prim,env,prim,0,0,0,Shade
        case 0x09ff6536: if (cycle_mode == CYCLE_MODE2)  
                         {
                         switch(cycle2)
                            {
                            case 0x0f78e4f0:
                                SetFastCombine(vn,0x20,UseT0   | UseShade | UseShadeAlpha | UseEnvAlpha | UseEnv,
                                                       UsePrim | UseShade | UseShadeAlpha | UseEnvAlpha | UseEnvIv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0   | UseShadeAlpha | UseEnv,
                                                    UsePrim | UseShadeAlpha | UseEnvIv);
                         }
                         break;
        case 0x09ff8841: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0,0,2);
                         {
                                 SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha,0);
                                 SetFastCombine(vn,0x10,UseT0AlphaIv | UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x09ff8a43: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseShadeAlpha,UseShade | UsePrimAlphaIv | UseShadeAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseShadeAlpha,UseShade | UsePrimAlphaIv | UseShadeAlpha);
                         }
                         break;
        case 0x09ff8c45: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseEnv | UseShadeAlpha | UseEnvAlpha | UsePrim | UsePrimAlpha,0);
                                SetFastCombine(vn,0x10,UseShade | UseShadeAlpha | UseEnvAlphaIv | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0,0,2);
                         {
                             SetFastCombine(vn,0x10,UseEnv | UseShadeAlpha | UseEnvAlpha,0);
                             SetFastCombine(vn,0x10,UseShade | UseShadeAlpha | UseEnvAlphaIv,0);
                         }
                         break;
        // das rote 64-teil oben rechts x
        // k5,k5,0,shade,0,0,0,shade
        case 0x09ff9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseShade | UseShadeAlpha,0);
                                break;
                            case 0x09ff9fff:
                                SetFastCombine(vn,0x10,UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                 SetFastCombine(vn,0x10,UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x09ffbfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseEnv | UseShadeAlpha,0);
                                break;
                            case 0x09ffbfff:
                                SetFastCombine(vn,0x10,UseEnv | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                 SetFastCombine(vn,0x10,UseEnv | UseShadeAlpha,0);
                         }
                         break;
        // ????? shade,k5,prim,comba,0,0,0,shade
        case 0x09ffe3f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0,UsePrimShade | UseShadeAlpha,0);
                         }
                         break;
        //ie: Mario64 - logo rand... :) 
        // t0,k5,shade,comba,0,0,0,shade
        case 0x09ffe4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                             TexMode = Tex;
                             switch(cycle2)
                             {
                             case 0x01ff1fff:
                                 SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                                 break;
                             case 0x01ffe3f0:
                                     SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UseShadeAlpha,0);
                                 break;
                             case 0x07ffe3f0:
                                     SetFastCombine(vn,0x10,UseT0 | UseShade | UsePrim | UsePrimAlpha,0);
                                 break;
                             case 0x09ffe3f0:
                                     SetFastCombine(vn,0x10,UseT0 | UseShade | UsePrim | UseShadeAlpha,0);
                                 break;
                             case 0x0f38e4f0:
                                     SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                 break;
                             default:
                                 checkup = 1;
                                 break;
                             }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                         }
                         break;
        // ????? prim,k5,shade,comba
        //       0,0,0,shade
        //       k5,k5,0,comb
        //       0,0,0,comb
        case 0x09ffe4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UseShade | UseShadeAlpha,0);
                                break;
                            case 0x09ff0c01:
                                SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha | UseShadeAlpha,0);
                                SetFastCombine(vn,0x10,UsePrim | UseShade | UseEnvAlphaIv | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrim | UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x09ffe4f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x0b29ab51:if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x1ffe4f0:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,UseEnv | UseShade | UseShadeAlphaIv | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | 0,13);
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShadeAlpha,UseEnv | UseShadeAlphaIv | UseEnvAlpha);
                         }
                         break;
        // ????? t1,t0,env,t0,0,0,0,env,k5,k5,0,comb,0,0,0,comb
        case 0x0bff2512:if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x1ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | 0,13);
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha,0);
                         }
                         break;
        // Mario-Schrift (beim Logo): alphablending fehlt irgendwie ... x
        // k5,k5,0,t0,0,0,0,env
        case 0x0bff3fff:if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             TexMode = Tex;
                             SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bff4a21:if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseEnvAlpha,0);
                         }
                         break;
        // k5,k5,0,prim,0,0,0,env
        case 0x0bff7fff:if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            // ???? cent,k5,env,comb,0,0,0,comb
                            case 0x01ff05f6:
                                TexMode = Tex;
                                SetFastCombine(vn,0x20,UseT0 | UseEnv | UseEnvAlpha,UsePrim | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UsePrim | UseEnvAlpha,0);
                         }
                         break;
        //????? env,shade,env,shade,0,0,0,env
        case 0x0bff8545: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | 0,9);
                         {
                             SetFastCombine(vn,0x20,UseEnv | UseEnvAlpha,UseShade | UseEnvIv | UseEnvAlpha);
                         }
                         break;
        //????? t0,shade,t0,shade,0,0,0,env
        case 0x0bff8841: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | 0,9);
                         {
                             SetFastCombine(vn,0,UseShade | UseT0AlphaIv | UseEnvAlpha,0);
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                             //SetFastCombine(vn,0x20,UseShade | UseT0AlphaIv | UseEnvAlpha,UseT0 | UseT0Alpha | UseEnvAlpha);
                         }
                         break;
        //????? k5,k5,0,shade,0,0,0,env
        case 0x0bff9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0bff9fff:
                                SetFastCombine(vn,0,UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0,UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffa153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseShade | UseEnvAlpha,UseEnv | UseT0Iv | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseT0 | UseEnvAlpha,UseEnv | UseT0Iv | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffa851: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseShade | UseT0 | UseT0Alpha | UseEnvAlpha,0);
                                SetFastCombine(vn,0x10,UseShade | UseEnv | UseT0AlphaIv | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                             SetFastCombine(vn,0x10,UseEnv | UseT0AlphaIv | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffe154: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffe1f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UseT0 | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x0bff1fff:
                                SetFastCombine(vn,0,UseT0 | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0,UseT0 | UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffe3f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UsePrim | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0,UsePrim | UseShade | UseEnvAlpha,0);
                         }
                         break;
        //????? t0,k5,shade,comba,0,0,0,env
        case 0x0bffe4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe0f3:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x01ffe5f0:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UseEnv | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | 0,9);
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseEnvAlpha,0);
                         }
                         break;
        //????? prim,k5,shade,ca,0,0,0.env
        case 0x0bffe4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff05f6:
                                SetFastCombine(vn,0x20,UseT0 | UseEnv | UseEnvAlpha,UsePrim | UseShade | UseEnvAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0,UsePrim | UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffe5f1: if (cycle_mode == CYCLE_MODE2)
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseEnv | UsePrimShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0bffbfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UseEnv | UseEnvAlpha ,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0,UseEnv | UseShade | UseEnvAlpha ,0);
                                break;
                            case 0x0bffbfff:
                                SetFastCombine(vn,0,UseEnv | UseEnvAlpha ,0);
                                break;
                            default:
                                checkup = 1;
                                break;

                            }
                         }
                         else
                             SetFastCombine(vn,0,UseEnv | UseEnvAlpha ,0);
                         break;
        case 0x0db62e31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0db6a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0db62e32: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0db6a053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0db6a153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0db6e4f0:
                                SetFastCombine(vn,0x23,UsePrim | UseShade | UseT0 | UseT0Alpha,UseEnv | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                         }
                         break;
        case 0x0dff28f5: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0bffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0dff2e31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0dffa053:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? t1,t0,enva,t0,0,0,0,primlodfrac
        case 0x0dff2c12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x01ffe5f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? k5,k5,0,t0,0,0,0,primlodfrac
        case 0x0dff3fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0dff4c21: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0dff6135: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnv);
                         }
                         break;
        //????? k5,k5,0,prim,0,0,0,primlodfrac
        case 0x0dff7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                //SetFastCombine(vn,0x10,UseT0Alpha | UsePrim,0);
                                SetFastCombine(vn,0x10,UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             //SetFastCombine(vn,0x10,UseT0Alpha | UsePrim,0);
                             SetFastCombine(vn,0x10,UsePrim,0);
                         }
                         break;
        //????? k5,k5,0,shade,0,0,0,primlodfrac
        case 0x0dff9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade,0);
                         }
                         break;
        case 0x0dffa153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShade,UseEnv | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0dffe4f0:
                                SetFastCombine(vn,0x23,UsePrim | UseT0 | UseT0Alpha | UseShade,UseEnv | UseShade | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                         }
                         break;
        // not done right should be 
        // t0,env,prim,env,0,0,0,prim_lod_frac
        case 0x0dffa351: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {

                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim,UseEnv | UseT0Alpha | UsePrimIv);
                         }
                         break;
        //????? k5,k5,0,center,0,0,0,primlodfrac
        case 0x0dffdfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0dffe1f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? t0,k5,prim,comba,0,0,0,primlodfrac
        case 0x0dffe3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             //TexMode = AlphaT0;
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim,0);
                         }
                         break;
        //????? shade,k5,prim,comba,0,0,0,primlodfrac
        case 0x0dffe3f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                TexMode = AlphaT0;
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             TexMode = AlphaT0;
                             SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim,0);
                         }
                         break;
        //????? t0,k5,shade,comba,0,0,0,primlodfrac
        case 0x0dffe4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShade | UsePrim,UseT0Iv | UseT0Alpha | UseShade | UseEnv);
                                break;
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim,0);
                                break;
                            case 0x01ffe5f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             //TexMode = AlphaT0;
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         }
                         break;
        case 0x0dffe4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim,0);
                                break;
                            case 0x0f781fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             TexMode = AlphaT0;
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim,0);
                         }
                         break;
        case 0x0dffe4f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x1ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         }
                         break;
        //????? t0,k5,env,comba,0,0,0,primlodfrac
        case 0x0dffe5f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {                           
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv,0);
                         }
                         break;
        case 0x0e5ee136: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe4f0:
                                TexMode = Tex;
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0e66a153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha);
                         }
                         break;
        case 0x0e799fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UseShade,0);
                         }
                         break;
        case 0x0e79e1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0e7a2e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8a053:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0e7ae1f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? center,env,t0,prim,prim,0,t0,0
        case 0x0e7b613f: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrim | UseT0Iv | UseT0Alpha | UsePrimAlpha,0);
                         }
                         break;
        case 0x0e7b6156: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0e7b6156:
                                //TexMode = Tex_Sub_Env_Mult_Tex_Add_Prim;
                                //TexColor = rdp_reg.primcolor ^ rdp_reg.envcolor;
                                //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrimAlpha,UsePrim | UseT0Iv | UseT0Alpha | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             //TexMode = Prim_Sub_Tex | Tex;
                             //TexColor = rdp_reg.primcolor;
                             //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                             //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrimAlpha,UsePrim | UseT0Iv | UseT0Alpha | UsePrimAlpha);
                         }
                         break;
        //????? prim,env,t0,env,prim,0,t0,0
        case 0x0e7ba153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,UseEnv | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha | UseShade,UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha | UseShade);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,UseEnv | UseT0Iv | UseT0Alpha | UsePrimAlpha);
                         }
                         break;
        case 0x0e7be3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0e7be1f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,0);
                                break;
                            case 0x0f381fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0e7be3f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseT0Alpha,0);
                         }
                         break;
        //????? k5,k5,0,prim,prim,0,t0,0
        case 0x0e7b7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         }
                         break;
        case 0x0e7ca153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShadeAlpha,UseEnv | UseT0Iv | UseT0Alpha | UseShadeAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShadeAlpha,UseEnv | UseT0Iv | UseT0Alpha | UseShadeAlpha);
                         break;
        case 0x0e7ce1f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                         break;
        //????? t0,k5,prim,comba,shade,0,t0,0
        case 0x0e7ce3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                         break;
        //????? t0,k5,shade,comba,env,0,t0,0
        case 0x0e7d3fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                         break;
        case 0x0e7de4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnvAlpha,0);
                         break;
        case 0x0e7e4a21: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         break;
        case 0x0e7ee4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade,0);
                         break;
        case 0x0eb92e12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x0eb93fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0eb93fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                                break;
                            case 0x0ef83fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x0eb95fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f388045: // ?????? strange mode
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0Alpha,0);
                         break;
        case 0x0eb9e2f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         break;
        case 0x0eb9ffff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f38e1f2:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha,0);
                         }
                         break;
        case 0x0ebe2d12: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x09ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseShadeAlpha | UseShade,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha,0);
                         }
                         break;
        case 0x0ef92c15: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x22,UseEnv | UseT0Alpha | UseEnvAlpha | UsePrimAlpha,0);
                             SetFastCombine(vn,0x22,UseT0 | UseT0Alpha | UseEnvAlphaIv | UsePrimAlpha,0);
                         break;
        //????? t0,prim,plf,t0,t0,0,prim,0
        case 0x0ef92e31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa053:
                                //SetFastCombine(vn,0,UseEnv | UsePrimAlpha,0);
                                //SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseT0Iv | UseT0Alpha | UseEnv | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         break;
        case 0x0ef92ef6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffa4f0:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,UseEnv | UseT0Alpha | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         break;
        //????? k5,k5,0,prim,t0,0,prim,0
        case 0x0ef93fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrimAlpha,0);
                         break;
        case 0x0ef96135: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha | UsePrimAlpha,UsePrim | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha | UsePrimAlpha,UsePrim | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                         }
                         break;
        case 0x0ef96b31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff0405:
                                //SetFastCombine(vn,0x23,UseEnv | UseShadeAlpha | UsePrimAlpha | UseShadeAlpha,UsePrim | UsePrimAlpha| UseShade | UseShadeAlphaIv);
                                SetFastCombine(vn,0x20,UseEnv | UseShade | UseT0Alpha | UsePrimAlpha,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrimAlpha | UseShadeIv);
                                SetFastCombine(vn,0x20,UseEnv | UseShade | UseT0Alpha | UsePrimAlpha,UsePrim | UseShadeAlphaIv | UseT0Alpha | UsePrimAlpha | UseShadeIv);
                                break;
                            case 0x01ff0c05:
                                //SetFastCombine(vn,0x23,UseEnv | UsePrimAlpha,UsePrim | UsePrimAlpha | UseEnvIv);
                                SetFastCombine(vn,0x20,UseEnv | UseEnvAlpha | UseT0Alpha | UsePrimAlpha,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrimAlpha | UseEnvAlphaIv);
                                SetFastCombine(vn,0x20,UseEnv | UseEnvAlpha | UseT0Alpha | UsePrimAlpha,UsePrim | UseShadeAlphaIv | UseT0Alpha | UsePrimAlpha | UseEnvAlphaIv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha | UsePrimAlpha,0);
                             SetFastCombine(vn,0x10,UsePrim | UseShadeAlphaIv | UseT0Alpha | UsePrimAlpha,0);
                         }
                         break;
        //????? k5,k5,0,prim,t0,0,prim,0
        case 0x0ef97fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         break;
        case 0x0ef98143: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseT0Iv | UseT0Alpha | UseShade | UsePrimAlpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,UseT0Iv | UseT0Alpha | UseShade | UsePrimAlpha);
                         break;
        //????? k5,k5,0,shade,t0,0,prim,0
        case 0x0ef99fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                             switch(cycle2)
                             {
                             default:
                                 checkup = 1;
                                 break;
                             }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UseShade | UsePrimAlpha,0);
                         }
                         break;
        //????? prim,env,t0,env,t0,0,prim,0
        case 0x0ef9a153: if (cycle_mode == CYCLE_MODE2)  
                         {
                             switch(cycle2)
                             {
                             case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0Alpha | UseT0,
                                                        UseEnv | UsePrimAlpha | UseT0Alpha | UseT0Iv);
                                 break;
                             case 0x01ffe0f4:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,UseEnv | UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                 break;
                             case 0x01ffe4f0:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,UseEnv | UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                 break;
                             case 0x0fb8e0f4:
                                SetFastCombine(vn,0x20,UsePrim | UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,UseEnv | UseShade | UsePrimAlpha | UseT0Iv | UseT0Alpha);
                                 break;
                             default:
                                 checkup = 1;
                                 break;
                             }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UsePrim | UsePrimAlpha | UseT0Alpha | UseT0,
                                                    UseEnv  | UsePrimAlpha | UseT0Alpha | UseT0Iv);
                         }
                         break;
        //????? t0,env,prim,env,to,0,prim,0
        case 0x0ef9a351: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha, UseEnv | UsePrimAlpha | UseT0Alpha | UsePrimIv);
                         }
                         break;
        case 0x0ef9a3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha, UseEnv | UsePrimAlpha | UseT0Alpha);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha, UseEnv | UsePrimAlpha | UseT0Alpha);
                         }
                         break;
        case 0x0ef9dfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrimAlpha | UseT0 | UseT0Alpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UsePrimAlpha | UseT0 | UseT0Alpha,0);
                         }
                         break;
        //????? shade,k5,t0,ca,to,0,prim,0
        case 0x0ef9e1f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
            else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         break;
        //????? shade,k5,t0,ca,to,0,prim,0
        case 0x0ef9e1f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha | UsePrim,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                         break;
        //????? t0,k5,prim,ca,to,0,prim,0
        case 0x0ef9e3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01bae4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ff0c05:
                                SetFastCombine(vn,0x10,UseEnv | UsePrimAlpha | UseEnvAlpha,0);
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha | UseEnvAlphaIv,0);
                                break;
                            case 0x01ffe0f4:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0ef9e3f1:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x0fb8e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         }
                         break;
        case 0x0ef9e3f2: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01bae4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe3f2:
                                SetFastCombine(vn,0x10,UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         break;
        case 0x0ef9e3f6: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha,0);
                         break;
        case 0x0ef9e3f7: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UsePrim | UsePrimAlpha,0);
                         }
                         break;
        //????? t0,k5,shade,ca,to,0,prim,0
        case 0x0ef9e4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01bae3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                                break;
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha,0);
                         break;
        //????? shade,k5,prim,ca,shade,0,prim,0
        case 0x0efce3f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha | UseShade | UseShadeAlpha,0);
                         break;
        // prim,k5,0,shade,shade,0,prim,0
        case 0x0efc9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UsePrimAlpha | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else SetFastCombine(vn,0,UsePrimAlpha | UseShade | UseShadeAlpha,0);
                         break;
        // t1,t0,lf,t0,t0,0,Shade,0
        case 0x0f392d12: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            case 0x0f78e4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha,0);
                         }
                         break;
        // k5,k5,0,t0,t0,0,Shade,0
        case 0x0f393fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0f781fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShadeAlpha,0);
                         }
                         break;
        // cent,prim,env,prim,0,0,0,Shade
        case 0x0f396531: if (cycle_mode == CYCLE_MODE2)  
                         {                      
                            switch(cycle2)
                            {
                            case 0x0f78e4f0:
                                SetFastCombine(vn,0x20,UseT0   | UseT0Alpha | UseShade | UseShadeAlpha | UseEnvAlpha | UseEnv,
                                                       UsePrim | UseT0Alpha | UseShade | UseShadeAlpha | UseEnvAlpha | UseEnvIv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0   | UseShadeAlpha | UseT0Alpha | UseEnv,
                                                    UsePrim | UseShadeAlpha | UseT0Alpha | UseEnvIv);
                         }
                         break;
        case 0x0f398145: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }  
                         else 
                         {
                             SetFastCombine(vn,0x20,UseEnv | UseT0 | UseT0Alpha | UseShadeAlpha,UseShade | UseShadeAlpha | UseT0Iv | UseT0Alpha);
                         }
                         break;
        //????? t0,shade,prim,shade,to,0,shade,0
        case 0x0f398341: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }  
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UsePrim | UseT0Alpha | UseShadeAlpha,UsePrimIv | UseShade | UseT0Alpha | UseShadeAlpha);
                         }
                         break;
        //????? k5,k5,0,shade,t0,0,shade,0
        case 0x0f399fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x0f39a153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,UseT0Iv | UseT0Alpha | UseEnv | UseShadeAlpha);
                         }
                         break;
        case 0x0f3ae4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseShade | UseShadeAlpha,0);
                         }
                         break;
        case 0x0f3be3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             TexMode=Tex;
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UsePrimAlpha | UseShadeAlpha,0);
                         }
                         break;
        //????? prim,k5,shade,ca,prim,0,shade,0
        case 0x0f3be4f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0850205:// needs t1
                                SetFastCombine(vn,0x10,UsePrim | UseShade | UsePrimAlpha | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             //TexMode=Tex;
                             SetFastCombine(vn,0x10,UsePrim | UseShade | UsePrimAlpha | UseShadeAlpha,0);
                         }
                         break;
        case 0x0f39e3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                                break;
                            case 0x01ffe4f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShadeAlpha,0);
                         }
                         break;
        //????? t0,k5,shade,ca,t0,0,shade,0
        case 0x0f39e4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                                break;
                            case 0x01bae3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha | UsePrim,0);
                                break;
                            case 0x01ffe3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha | UsePrim,0);
                                break;
                            case 0x0ef80c05:
                                SetFastCombine(vn,0x10,UseEnv | UseT0Alpha | UsePrimAlpha | UseShadeAlpha | UseEnvAlpha,0);
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UsePrimAlpha | UseShadeAlpha | UseEnvAlphaIv,0);
                                break;
                            case 0x0ef81fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha | UsePrimAlpha,0);
                                break;
                            case 0x0ef8e3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha | UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                         }
                         break;
        //????? t0,k5,env,comba,t0,0,shade,0
        case 0x0f39e5f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseShadeAlpha,0);
                         }
                         break;
        // not right
        //????? t0,k5,env,comba,t0,0,shade,0
        case 0x0f39ffff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                TexMode = AlphaT0;
                                SetFastCombine(vn,0x10,UseT0Alpha | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UseShadeAlpha,0);
                         }
                         break;
        case 0x0f3ee4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha |  UseShade | UseShadeAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseShadeAlpha,0);
                         }
                         break;
        //????? k5,k5,0,t0,t0,0,env,0
        case 0x0f793fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                                break;
                            case 0x0fb81fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                             SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnvAlpha,0);
                         break;
        //????? k5,k5,0,shade,prim,0,env,0
        case 0x0f7b9fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UseShade | UsePrimAlpha | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                            SetFastCombine(vn,0,UseShade | UsePrimAlpha | UseEnvAlpha,0);
                         }
                         break;
        //????? k5,k5,0,env,t0,0,env,0
        case 0x0f79a153: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseEnvAlpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnvAlpha | UseEnv);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x20,UseT0 | UseT0Alpha | UseEnvAlpha | UsePrim,UseT0Iv | UseT0Alpha | UseEnvAlpha | UseEnv);
                         }
                         break;
        case 0x0f79bfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0Alpha | UseEnvAlpha | UseEnv,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                             SetFastCombine(vn,0x10,UseT0Alpha | UseEnvAlpha | UseEnv,0);
                         }
                         break;
        case 0x0f79e1f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x01bae3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                            SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0f79e3f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                            SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseEnvAlpha,0);
                         }
                         break;
        //????? t0,k5,shade,ca,t0,0,env,0,k5,k5,0,comb
        case 0x0f79e4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnvAlpha,0);
                                break;
                            case 0x01bae3f0:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UsePrim | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                            SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseShade | UseEnvAlpha,0);
                         }
                         break;
        //????? t0,k5,env,ca,t0,0,env,0
        case 0x0f79e5f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                            SetFastCombine(vn,0x10,UseT0 | UseT0Alpha | UseEnv | UseEnvAlpha,0);
                         break;
        //????? t0,k5,shade,ca,t1,0,env,0
        case 0x0f7ae4f1: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0x10,UseT0 | UseShade | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                            SetFastCombine(vn,0x10,UseT0 | UseShade | UseEnvAlpha,0);
                         }
                         break;
        case 0x0f7be5f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x01ff1fff:
                                SetFastCombine(vn,0,UsePrim | UseEnv | UsePrimAlpha | UseEnvAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else 
                         {
                            SetFastCombine(vn,0,UsePrim | UseEnv | UsePrimAlpha | UseEnvAlpha,0);
                         }
                         break;
        //????? shade,k5,env,comba,shade,0,env,0
        case 0x0f7ce5f4: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                SetFastCombine(vn,0,UseEnvShade | UseEnvShadeAlpha,0);
                         }
                         break;
        case 0x0fff7fff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                SetFastCombine(vn,0,UsePrim,0);
                         }
                         break;
        case 0x0fff81f3: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0fffaef0:
                                SetFastCombine(vn,0x20,UseT0 | UsePrim,UseShade);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                             SetFastCombine(vn,0x20,UseT0 | UsePrim,UseShade);
                         }
                         break;
                         //????? t0,prim,env,t0,0,0,0,0
        case 0x0fff2c31: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x0dffa053: //needs fixed
                                SetFastCombine(vn,0x20,UseEnv | UseT0Iv,UsePrim | UseT0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                SetFastCombine(vn,0x10,UseT0,0);
                                SetFastCombine(vn,0x10,UsePrim | UseEnvAlphaIv,0);
                         }
                         break;
        case 0x0fffbfff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                SetFastCombine(vn,0x10,UseEnv,0);
                         }
                         break;
        case 0x0fffffff: if (cycle_mode == CYCLE_MODE2)  
                         {
                            switch(cycle2)
                            {
                            case 0x07ff7fff:
                                SetFastCombine(vn,0x10,UsePrim | UsePrimAlpha,0);
                                break;
                            default:
                                checkup = 1;
                                break;
                            }
                         }
                         else
                         {
                                SetFastCombine(vn,0x10,UseT0,0);
                         }
                         break;
*/
        default:
            {
                checkup = 4;
            }
        }
    }
    else if(cycle_mode == CYCLE_COPY)
    {
        checkup = 2;
        MDrawVisualTriangle(vn,1,0,0);
    }
    else if(cycle_mode == CYCLE_FILL)
    {
        checkup = 3;
        MDrawVisualTriangle(vn,0,UsePrim,0);
    }
    else if(cycle_mode)
    {
        MDrawVisualTriangle(vn,1,0,0);
    }

    if (checkup != 0)
    {       
#ifdef DEBUGCC
        char *output[1024];
        sprintf(output,"Mode1 = %8x Mode2 = %8x CycleMode = %i\na0  = %s, b0  = %s, c0  = %s, d0  = %s\nAa0 = %s, Ab0 = %s, Ac0 = %s, Ad0 = %s\na1  = %s, b1  = %s, c1  = %s, d1  = %s\nAa1 = %s, Ab1 = %s, Ac1 = %s, Ad1 = %s\n",
                cycle1,cycle2,cycle_mode,
                Mode[uc0_a0],   Mode[uc0_b0],   Mode[uc0_c0],   Mode[uc0_d0],
                Alpha[uc0_Aa0], Alpha[uc0_Ab0], Alpha[uc0_Ac0], Alpha[uc0_Ad0],
                Mode[uc0_a1],   Mode[uc0_b1],   Mode[uc0_c1],   Mode[uc0_d1],
                Alpha[uc0_Aa1], Alpha[uc0_Ab1], Alpha[uc0_Ac1], Alpha[uc0_Ad1]);
            MessageBox(hGraphics, output, "DisplayCC", MB_OK);
        exit(1);
#else
        MDrawVisualTriangle(vn,1,0,0);
#endif
    }
#ifdef LOG_ON
        PRINT_RDP_MNEMONIC("%i 0x%x\n",checkup, cycle1);
#endif

}


//#undef DEBUGCC