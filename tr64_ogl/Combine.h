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

#ifndef _COMBINE_

#define _COMBINE_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "type_sizes.h"

#define CYCLE_MODE1 0
#define CYCLE_MODE2 1
#define CYCLE_COPY  2
#define CYCLE_FILL  3

extern _u8 cycle_mode;
extern _u32 cycle1;
extern _u32 cycle2;
extern int ModesUsedCnt;

extern void rdp_setcombine();
extern void DrawVisualTriangle(int vn[3]);
extern void BuildCombine(_u8 tcycmode);

#define UseT0               0x01
#define UseT0Iv             0x02
#define UseT0Alpha          0x04
#define UseT0AlphaIv        0x08
#define UseMT0Alpha         0x10
#define UseMT0AlphaIv       0x20

#define UseT1               0x040
#define UseT1Iv             0x080
#define UseT1Alpha          0x100
#define UseT1AlphaIv        0x200
#define UseMT1Alpha         0x400
#define UseMT1AlphaIv       0x800

#define UsePrim             0x01000
#define UsePrimIv           0x02000
#define UsePrimAlpha        0x04000
#define UsePrimAlphaIv      0x08000
#define UseMPrimAlpha       0x10000
#define UseMPrimAlphaIv     0x20000

#define UseShade            0x040000
#define UseShadeIv          0x080000
#define UseShadeAlpha       0x100000
#define UseShadeAlphaIv     0x200000
#define UseMShadeAlpha      0x400000
#define UseMShadeAlphaIv    0x800000

#define UseEnv              0x01000000
#define UseEnvIv            0x02000000
#define UseEnvAlpha         0x04000000
#define UseEnvAlphaIv       0x08000000
#define UseMEnvAlpha        0x10000000
#define UseMEnvAlphaIv      0x20000000

#define UseZeroAlpha        0x80000000
//#define UseTexMask  (UseTex | UseTexIv | UseTexAlpha | UseTexAlphaIv)
#define UseTex0Mask  (UseT0 | UseT0Iv | UseT0Alpha | UseT0AlphaIv | UseMT0Alpha | UseMT0AlphaIv)
#define UseTex1Mask  (UseT1 | UseT1Iv | UseT1Alpha | UseT1AlphaIv | UseMT1Alpha | UseMT1AlphaIv)


#define MaskShade   (~(UseShade | UseShadeIv | UseShadeAlpha | UseShadeAlphaIv | UseMShadeAlpha | UseMShadeAlphaIv))
#define UseAlphaIv  (UseEnvAlphaIv | UsePrimAlphaIv | UseShadeAlphaIv)
#define UseColorIv  (UseEnvIv | UsePrimIv | UseShadeIv | UseMPrimAlphaIv | UseMEnvAlphaIv | UseMShadeAlphaIv)
#define UseIvMask  (UsePrimIv  | UsePrimAlphaIv  | UseMPrimAlphaIv \
                  | UseShadeIv | UseShadeAlphaIv | UseMShadeAlphaIv \
                  | UseEnvIv   | UseEnvAlphaIv   | UseMEnvAlphaIv)

#define Use_CVG_Only ~(UsePrimAlpha | UsePrimAlphaIv | UseShadeAlpha | UseShadeAlphaIv | UseEnvAlpha | UseEnvAlphaIv)


#define UsePrimEnv      (UsePrim | UseEnv)
#define UsePrimShade    (UsePrim | UseShade)
#define UsePrimEnvShade (UsePrim | UseEnv | UseShade)
#define UseEnvShade     (UseEnv  | UseShade)

#define UsePrimEnvAlpha         (UsePrimAlpha | UseEnvAlpha)
#define UsePrimShadeAlpha       (UsePrimAlpha | UseShadeAlpha)
#define UsePrimEnvShadeAlpha    (UsePrimAlpha | UseEnvAlpha | UseShadeAlpha)
#define UseEnvShadeAlpha        (UseEnvAlpha | UseShadeAlpha)

typedef struct
{
    int TexMode;
    int TexColor;
    int t1;
    int s1;
    int t2;
    int s2;
} t_Combine;

typedef struct 
{
    _u32 cycle1C;
    _u32 cycle1A;

    _u32 cycle2C;
    _u32 cycle2A;

    _u32 cycle_mode;
    
    char* Color_a0;
    char* Color_b0;
    char* Color_c0;
    char* Color_d0;

    char* Alpha_a0;
    char* Alpha_b0;
    char* Alpha_c0;
    char* Alpha_d0;

    char* Color_a1;
    char* Color_b1;
    char* Color_c1;
    char* Color_d1;

    char* Alpha_a1;
    char* Alpha_b1;
    char* Alpha_c1;
    char* Alpha_d1;
} _ModesUsed;


#endif