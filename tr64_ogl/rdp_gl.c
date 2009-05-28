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

#include <windows.h>
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

#include "rdp_registers.h"
#include "rdp_gl.h"
#include "combine.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wingl.h"
#include "3dmath.h"
#include "lighting.h"
#include "Texture.h"
#include "debug.h"
//** Globals
extern char output[1024];
extern _u32 TexMode;
extern ucode_version;
extern float UC6_Matrices[3][4][4];

extern WINDATA_T WinData;

extern float imgHeight;
extern float imgWidth;

extern BOOL FogEnabled;

//#define OPENGL_LIGHTS

//** display list defines for 2d stuff 
#define kListEnd2D          1
#define kListEndLights      1
#define kListBeginLights    9
#define kListBegin2D160   160
#define kListBegin2D320   320
#define kListBegin2D640   640
#define kListBegin2D800   800
#define kListBegin2D960   960
#define kListBegin2D1024 1024
#define kListBegin2D1200 1200

#define BLEND_NOOP              0x0000

#define BLEND_NOOP5             0xcc48  // Fog * 0 + Mem * 1
#define BLEND_NOOP4             0xcc08  // Fog * 0 + In * 1
#define BLEND_FOG_ASHADE        0xc800
#define BLEND_FOG_3             0xc000  // Fog * AIn + In * 1-A
#define BLEND_FOG_MEM           0xc440  // Fog * AFog + Mem * 1-A
#define BLEND_FOG_APRIM         0xc400  // Fog * AFog + In * 1-A

#define BLEND_BLENDCOLOR        0x8c88
#define BLEND_BI_AFOG           0x8400  // Bl * AFog + In * 1-A
#define BLEND_BI_AIN            0x8040  // Bl * AIn + Mem * 1-A

#define BLEND_MEM               0x4c40  // Mem*0 + Mem*(1-0)?!
#define BLEND_FOG_MEM_3         0x44c0  // Mem * AFog + Fog * 1-A

#define BLEND_NOOP3             0x0c48  // In * 0 + Mem * 1
#define BLEND_PASS              0x0c08  // In * 0 + In * 1
#define BLEND_FOG_MEM_IN_MEM    0x0440  // In * AFog + Mem * 1-A
#define BLEND_FOG_MEM_FOG_MEM   0x04c0  // In * AFog + Fog * 1-A
#define BLEND_OPA               0x0044  //  In * AIn + Mem * AMem
#define BLEND_XLU               0x0040
#define BLEND_MEM_ALPHA_IN      0x4044  //  Mem * AIn + Mem * AMem

// RSP_SETOTHERMODE_H gSetCycleType 
#define CYCLE_TYPE_1        0
#define CYCLE_TYPE_2        1
#define CYCLE_TYPE_COPY     2
#define CYCLE_TYPE_FILL     3

//** for Lighting
extern BOOL refresh_lights;// = TRUE;
extern BOOL refresh_matrix;// = TRUE;
extern MATRIX4 view_matrix, invers;
extern VECTOR Light_Vector[8];




//** all Functions
extern void MathTextureScales();

int Vtidx[512];
float VtSc[512];
float VtTc[512];

int Vtcnt = 0;

GLint Src_Alpha = GL_SRC_ALPHA;
GLint Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;

float fScalex = 2.0f;
float fScaley = 2.0f;

void MDrawVisualTriangle(int vn[3], _u32 t1, _u32 s1, _u32 s2);

/******************************************************************************\
*                                                                              *
*   3d RenderStuff for RDP (OPENGL - helper routines)                          *
*                                                                              *
\******************************************************************************/
extern unsigned long FrameCounter ;
extern _u8 cycle_mode;

static GLint idx[3] = {0,1,2};

static float ClrArry[16*256];
static float VtxArry[16*256];
static float TexArry[16*256];
static float NrmArry[16*256];

int ArrayOk = 0;

void InitArrays(void)
{
//  glVertexPointer(3,GL_FLOAT,0,VtxArry);
    glVertexPointer(4,GL_FLOAT,0,VtxArry);
    glColorPointer(4,GL_FLOAT,0,ClrArry);
    glNormalPointer(GL_FLOAT,0,NrmArry);
    glTexCoordPointer(2,GL_FLOAT,0,TexArry);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    ArrayOk = 1;
}

int aTilew = 0;
int aTileh = 0;
float Multw = 1;
float Multh = 1;

void Render_Fog2(int vn[3])
{
    float color[4];
    //float FogAlpha;
    float rng,min,max,zDepth,zOffset;
    int fVtxCnt = 0;
    int i;
    int vsel = 0;

    rng = 131072.0f / rdp_reg.fog_fm;
    min = 512.0f - (rng * rdp_reg.fog_fo /256);
    max = min + rng;

    zDepth = rdp_reg.vp[2] * 8;
    zOffset = (rdp_reg.vp[6] * 4) - (rdp_reg.vp[2] * 4);

    color[0] = rdp_reg.fog_r;
    color[1] = rdp_reg.fog_g;
    color[2] = rdp_reg.fog_b;
    color[3] = rdp_reg.fog_a;

    for(i=0; i<Vtcnt; i += 3)
    {
        t_vtx *vertex[3] = {&rdp_reg.vtx[vn[i]],&rdp_reg.vtx[vn[i+1]],&rdp_reg.vtx[vn[i+2]]};
            {
                float FogAlpha;
                float Distance = (float)sqrt(vertex[0]->x * vertex[0]->x + vertex[0]->y *vertex[0]->y + vertex[0]->z * vertex[0]->z);

                FogAlpha = (Distance - min) / (rng);
                if(FogAlpha > 1.0f) FogAlpha = 1.0f;
                if(FogAlpha < 0.0f) FogAlpha = 0.0f;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                Distance = (float)sqrt(vertex[1]->x * vertex[1]->x + vertex[1]->y *vertex[1]->y + vertex[1]->z * vertex[1]->z);
                
                FogAlpha = (vertex[1]->z - min) / (rng);
                if(FogAlpha > 1.0f) FogAlpha = 1.0f;
                if(FogAlpha < 0.0f) FogAlpha = 0.0f;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                Distance = (float)sqrt(vertex[2]->x * vertex[2]->x + vertex[2]->y *vertex[2]->y + vertex[2]->z * vertex[2]->z);

                FogAlpha = (Distance - min) / (rng);
                if(FogAlpha > 1.0f) FogAlpha = 1.0f;
                if(FogAlpha < 0.0f) FogAlpha = 0.0f;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;
            }
    }

    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_TEXTURE_2D);
    
    glDepthFunc(GL_EQUAL);
    glDrawArrays(GL_TRIANGLES,0,fVtxCnt);
    glDepthFunc(GL_LEQUAL);
}

void Render_Fog1(int vn[3])
{
    float color[4];
    //float FogAlpha,
    float rng,min,max,zDepth,zOffset;
    int fVtxCnt = 0;
    int i;
    int vsel = 0;

    rng = 131072.0f / rdp_reg.fog_fm;
    min = 512.0f - (rng * rdp_reg.fog_fo /256);
    max = min + rng;

    zDepth = rdp_reg.vp[2] * 8;
    zOffset = (rdp_reg.vp[6] * 4) - (rdp_reg.vp[2] * 4);

    color[0] = rdp_reg.fog_r;
    color[1] = rdp_reg.fog_g;
    color[2] = rdp_reg.fog_b;
    color[3] = rdp_reg.fog_a;

    for(i=0; i<Vtcnt; i += 3)
    {
        t_vtx *vertex[3] = {&rdp_reg.vtx[vn[i]],&rdp_reg.vtx[vn[i+1]],&rdp_reg.vtx[vn[i+2]]};
        _u32 FogSpan = 0;

        vsel = 0;

        if (vertex[0]->w == min)
            FogSpan |= 0x01;
        if (vertex[0]->w > min)
            FogSpan |= 0x02;
        if (vertex[0]->w == max)
            FogSpan |= 0x04;
        if (vertex[0]->w > max)
            FogSpan |= 0x08;

        if (vertex[1]->w == min)
            FogSpan |= 0x10;
        if (vertex[1]->w > min)
            FogSpan |= 0x20;
        if (vertex[1]->w == max)
            FogSpan |= 0x40;
        if (vertex[1]->w > max)
            FogSpan |= 0x80;

        if (vertex[2]->w == min)
            FogSpan |= 0x100;
        if (vertex[2]->w > min)
            FogSpan |= 0x200;
        if (vertex[2]->w == max)
            FogSpan |= 0x400;
        if (vertex[2]->w > max)
            FogSpan |= 0x800;

        switch (FogSpan)
        {
        case 0x0000:// all vertexes <= min.
        case 0x0001:
        case 0x0010:
        case 0x0011:
        case 0x0100:
        case 0x0101:
        case 0x0110:
        case 0x0111:
            break;
        case 0x0112: // all vertexs between min and max.
        case 0x0121:
        case 0x0211:

        case 0x0221:
        case 0x0212:
        case 0x0122:

        case 0x0222:

        case 0x0226:
        case 0x0262:
        case 0x0622:

        case 0x0662:
        case 0x0626:
        case 0x0266:

        case 0x0116:
        case 0x0161:
        case 0x0611:

        case 0x0661:
        case 0x0616:
        case 0x0166:
            {
                float FogAlpha;

                FogAlpha = (vertex[0]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[0]->x;
                VtxArry[fVtxCnt*4+1] = vertex[0]->y;
                VtxArry[fVtxCnt*4+2] = vertex[0]->z;
                VtxArry[fVtxCnt*4+3] = vertex[0]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                FogAlpha = (vertex[1]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                FogAlpha = (vertex[2]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;
            }
            break;
        case 0x0200: // 1 vetrex between min and max and rest <= min
        case 0x0201:
        case 0x0210:
            vsel++;
        case 0x0020:
        case 0x0021:
        case 0x0120:
            vsel++;
        case 0x0002:
        case 0x0012:
        case 0x0102:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (min - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vsel]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vsel]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vsel]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vsel]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vsel]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                FogAlpha = 0.0f;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;
            }
            break;
        case 0x0022: // 1 vetrex <= min rest between min and max.
            vsel++;
        case 0x0202:
            vsel++;
        case 0x0220:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (min - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vi2]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vi2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                FogAlpha = (vertex[vi1]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;
            }
            break;
        case 0x0a22:// 1 vertex > max rest between min and max.
            vsel++;
        case 0x02a2:
            vsel++;
        case 0x022a:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (max - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vi2]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vi2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                FogAlpha = (vertex[vi1]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vsel]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vsel]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vsel]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vsel]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;
            }
            break;
        case 0x026a: // 1 vertex between min and max, rest >= max
        case 0x02a6:
        case 0x02aa:
            vsel++;
        case 0x062a:
        case 0x0a26:
        case 0x0a2a:
            vsel++;
        case 0x06a2:
        case 0x0a62:
        case 0x0aa2:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (max - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vsel]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = vertex[vsel]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vsel]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vsel]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vsel]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                FogAlpha = 1.0f;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;


                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;
            }
            break;
        case 0x0666: // all vertexs >= max
        case 0x066a:
        case 0x06a6:
        case 0x06aa:
        case 0x0a66:
        case 0x0a6a:
        case 0x0aa6:
        case 0x0aaa:
            VtxArry[fVtxCnt*4+0] = vertex[0]->x;
            VtxArry[fVtxCnt*4+1] = vertex[0]->y;
            VtxArry[fVtxCnt*4+2] = vertex[0]->z;
            VtxArry[fVtxCnt*4+3] = vertex[0]->w;

            ClrArry[fVtxCnt*4+0] = color[0];
            ClrArry[fVtxCnt*4+1] = color[1];
            ClrArry[fVtxCnt*4+2] = color[2];
            ClrArry[fVtxCnt*4+3] = 1.0f;

            fVtxCnt++;

            VtxArry[fVtxCnt*4+0] = vertex[1]->x;
            VtxArry[fVtxCnt*4+1] = vertex[1]->y;
            VtxArry[fVtxCnt*4+2] = vertex[1]->z;
            VtxArry[fVtxCnt*4+3] = vertex[1]->w;

            ClrArry[fVtxCnt*4+0] = color[0];
            ClrArry[fVtxCnt*4+1] = color[1];
            ClrArry[fVtxCnt*4+2] = color[2];
            ClrArry[fVtxCnt*4+3] = 1.0f;

            fVtxCnt++;

            VtxArry[fVtxCnt*4+0] = vertex[2]->x;
            VtxArry[fVtxCnt*4+1] = vertex[2]->y;
            VtxArry[fVtxCnt*4+2] = vertex[2]->z;
            VtxArry[fVtxCnt*4+3] = vertex[2]->w;

            ClrArry[fVtxCnt*4+0] = color[0];
            ClrArry[fVtxCnt*4+1] = color[1];
            ClrArry[fVtxCnt*4+2] = color[2];
            ClrArry[fVtxCnt*4+3] = 1.0f;

            fVtxCnt++;

            break;
        case 0x0a00:// 1 vertex > max rest < min.
            vsel++;
        case 0x00a0:
            vsel++;
        case 0x000a:
            {
                //float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                float tx3,ty3,tz3,vs3;
                float tx4,ty4,tz4,vs4;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (max - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);
                vs3 = (min - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs4 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                tx3 = (vertex[vi1]->x - vertex[vsel]->x) * vs3
                    + vertex[vsel]->x;
                ty3 = (vertex[vi1]->y - vertex[vsel]->y) * vs3
                    + vertex[vsel]->y;
                tz3 = (vertex[vi1]->z - vertex[vsel]->z) * vs3
                    + vertex[vsel]->z;

                tx4 = (vertex[vi2]->x - vertex[vsel]->x) * vs4
                    + vertex[vsel]->x;
                ty4 = (vertex[vi2]->y - vertex[vsel]->y) * vs4
                    + vertex[vsel]->y;
                tz4 = (vertex[vi2]->z - vertex[vsel]->z) * vs4
                    + vertex[vsel]->z;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vsel]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vsel]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vsel]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vsel]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;
            }
            break;
        case 0x00aa:// 1 vertex < min rest > max.
            vsel++;
        case 0x0a0a:
            vsel++;
        case 0x0aa0:
            {
                //float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                float tx3,ty3,tz3,vs3;
                float tx4,ty4,tz4,vs4;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (min - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);
                vs3 = (max - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs4 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                tx3 = (vertex[vi1]->x - vertex[vsel]->x) * vs3
                    + vertex[vsel]->x;
                ty3 = (vertex[vi1]->y - vertex[vsel]->y) * vs3
                    + vertex[vsel]->y;
                tz3 = (vertex[vi1]->z - vertex[vsel]->z) * vs3
                    + vertex[vsel]->z;

                tx4 = (vertex[vi2]->x - vertex[vsel]->x) * vs4
                    + vertex[vsel]->x;
                ty4 = (vertex[vi2]->y - vertex[vsel]->y) * vs4
                    + vertex[vsel]->y;
                tz4 = (vertex[vi2]->z - vertex[vsel]->z) * vs4
                    + vertex[vsel]->z;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;
            }
            break;
        case 0x02a0:// 1 vertex < min 1 > max other between min and max.
            vsel++;
        case 0x0a02:
            vsel++;
        case 0x002a:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                float tx3,ty3,tz3,vs3;
                float tx4,ty4,tz4,vs4;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (min - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);
                vs3 = (max - vertex[vi1]->w) / (vertex[vi2]->w - vertex[vi1]->w);
                vs4 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                tx3 = (vertex[vi2]->x - vertex[vi1]->x) * vs3
                    + vertex[vi1]->x;
                ty3 = (vertex[vi2]->y - vertex[vi1]->y) * vs3
                    + vertex[vi1]->y;
                tz3 = (vertex[vi2]->z - vertex[vi1]->z) * vs3
                    + vertex[vi1]->z;

                tx4 = (vertex[vi2]->x - vertex[vsel]->x) * vs4
                    + vertex[vsel]->x;
                ty4 = (vertex[vi2]->y - vertex[vsel]->y) * vs4
                    + vertex[vsel]->y;
                tz4 = (vertex[vi2]->z - vertex[vsel]->z) * vs4
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vi1]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi2]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi2]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi2]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi2]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;
            }
            break;
        case 0x020a:// 1 vertex < min 1 > max other between min and max.
            vsel++;
        case 0x00a2:
            vsel++;
        case 0x0a20:
            {
                float FogAlpha;

                float tx1,ty1,tz1,vs1;
                float tx2,ty2,tz2,vs2;
                float tx3,ty3,tz3,vs3;
                float tx4,ty4,tz4,vs4;
                int vi1,vi2;

                vi1 = vsel + 1;
                if (vi1 > 2) vi1 -= 3;

                vi2 = vsel + 2;
                if (vi2 > 2) vi2 -= 3;

                vs1 = (max - vertex[vsel]->w) / (vertex[vi1]->w - vertex[vsel]->w);
                vs2 = (max - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);
                vs3 = (min - vertex[vi1]->w) / (vertex[vi2]->w - vertex[vi1]->w);
                vs4 = (min - vertex[vsel]->w) / (vertex[vi2]->w - vertex[vsel]->w);

                tx1 = (vertex[vi1]->x - vertex[vsel]->x) * vs1
                    + vertex[vsel]->x;
                ty1 = (vertex[vi1]->y - vertex[vsel]->y) * vs1
                    + vertex[vsel]->y;
                tz1 = (vertex[vi1]->z - vertex[vsel]->z) * vs1
                    + vertex[vsel]->z;

                tx2 = (vertex[vi2]->x - vertex[vsel]->x) * vs2
                    + vertex[vsel]->x;
                ty2 = (vertex[vi2]->y - vertex[vsel]->y) * vs2
                    + vertex[vsel]->y;
                tz2 = (vertex[vi2]->z - vertex[vsel]->z) * vs2
                    + vertex[vsel]->z;

                tx3 = (vertex[vi2]->x - vertex[vi1]->x) * vs3
                    + vertex[vi1]->x;
                ty3 = (vertex[vi2]->y - vertex[vi1]->y) * vs3
                    + vertex[vi1]->y;
                tz3 = (vertex[vi2]->z - vertex[vi1]->z) * vs3
                    + vertex[vi1]->z;

                tx4 = (vertex[vi2]->x - vertex[vsel]->x) * vs4
                    + vertex[vsel]->x;
                ty4 = (vertex[vi2]->y - vertex[vsel]->y) * vs4
                    + vertex[vsel]->y;
                tz4 = (vertex[vi2]->z - vertex[vsel]->z) * vs4
                    + vertex[vsel]->z;

                FogAlpha = (vertex[vi1]->w - min) / (rng);

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vi1]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vi1]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vi1]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vi1]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = FogAlpha;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx3;
                VtxArry[fVtxCnt*4+1] = ty3;
                VtxArry[fVtxCnt*4+2] = tz3;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx4;
                VtxArry[fVtxCnt*4+1] = ty4;
                VtxArry[fVtxCnt*4+2] = tz4;
                VtxArry[fVtxCnt*4+3] = min;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 0.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx2;
                VtxArry[fVtxCnt*4+1] = ty2;
                VtxArry[fVtxCnt*4+2] = tz2;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = vertex[vsel]->x;
                VtxArry[fVtxCnt*4+1] = vertex[vsel]->y;
                VtxArry[fVtxCnt*4+2] = vertex[vsel]->z;
                VtxArry[fVtxCnt*4+3] = vertex[vsel]->w;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;

                VtxArry[fVtxCnt*4+0] = tx1;
                VtxArry[fVtxCnt*4+1] = ty1;
                VtxArry[fVtxCnt*4+2] = tz1;
                VtxArry[fVtxCnt*4+3] = max;

                ClrArry[fVtxCnt*4+0] = color[0];
                ClrArry[fVtxCnt*4+1] = color[1];
                ClrArry[fVtxCnt*4+2] = color[2];
                ClrArry[fVtxCnt*4+3] = 1.0f;

                fVtxCnt++;
            }
            break;
        default:
            fVtxCnt += 0;
        }
                
            //FogAlpha = (VtxArry[i*4+3] - min) / (rng);
            //ClrArry[i*4+3] = FogAlpha - 0.75f;
    }

    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_TEXTURE_2D);
    
//  glDepthFunc(GL_EQUAL);
    glDrawArrays(GL_TRIANGLES,0,fVtxCnt);
//  glDepthFunc(GL_LEQUAL);
}

void BlendFunc(GLint sFactor,GLint dFactor)
{
    Src_Alpha = sFactor;
    Dst_Alpha = dFactor;
    glBlendFunc(sFactor,dFactor);
}

void SetAlphaTestEnable(BOOL AlphaTest)
{
    if(AlphaTest)
        glEnable(GL_ALPHA_TEST);
    else
        glDisable(GL_ALPHA_TEST);
}

void CalcBlend()
{
    _u32 blendmode_1 = (_u32)((rdp_reg.mode_l >> 16) & 0xcccc );
    _u32 blendmode_2 = (_u32)((rdp_reg.mode_l >> 16) & 0x3333 );
    _u32 cycletype = (rdp_reg.mode_h >> 20) & 0x03;//gRDP.otherMode.cycle_type;

    switch( cycletype )
    {
    case CYCLE_TYPE_FILL:
        //BlendFunc(GL_ONE, GL_ZERO);
        //glEnable(GL_BLEND);
        glDisable(GL_BLEND);
        break;
    case CYCLE_TYPE_COPY:
        //glDisable(GL_BLEND);
        BlendFunc(GL_ONE, GL_ZERO);
        glEnable(GL_BLEND);
        break;
    case CYCLE_TYPE_2:
        if(((rdp_reg.mode_l >> 14) & 0x01) && ((rdp_reg.mode_l >> 14) & 0x01))
        {
            BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            break;
        }

        /*
        if( ((rdp_reg.mode_l >> 13) & 0x01) && ((rdp_reg.mode_l >> 12) & 0x01)==0 )
        {
            BlendFunc(GL_ONE, GL_ZERO);
            glEnable(GL_BLEND);
            break;
        }
        */

        switch(blendmode_1 + blendmode_2)
        {
        case BLEND_PASS+(BLEND_PASS>>2):    // In * 0 + In * 1
        case BLEND_FOG_APRIM+(BLEND_PASS>>2):
            BlendFunc(GL_ONE, GL_ZERO);
            if( ((rdp_reg.mode_l >> 13) & 0x01) )
            {
                glEnable(GL_BLEND);
            }
            else
            {
                glDisable(GL_BLEND);
            }

            SetAlphaTestEnable(((rdp_reg.mode_l >> 14) & 0x03) ==1 ? TRUE : FALSE);
            break;
        case BLEND_PASS+(BLEND_OPA>>2):
            // 0x0c19
            // Cycle1:  In * 0 + In * 1
            // Cycle2:  In * AIn + Mem * AMem
            if( ((rdp_reg.mode_l >> 12) & 0x01) && ((rdp_reg.mode_l >> 13) & 0x01) )
            {
                BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
            }
            else
            {
                BlendFunc(GL_ONE, GL_ZERO);
                glEnable(GL_BLEND);
            }
            break;
        case BLEND_PASS + (BLEND_XLU>>2):
            // 0x0c18
            // Cycle1:  In * 0 + In * 1
            // Cycle2:  In * AIn + Mem * 1-A
        case BLEND_FOG_ASHADE + (BLEND_XLU>>2):
            //Cycle1:   Fog * AShade + In * 1-A
            //Cycle2:   In * AIn + Mem * 1-A    
        case BLEND_FOG_APRIM + (BLEND_XLU>>2):
            //Cycle1:   Fog * AFog + In * 1-A
            //Cycle2:   In * AIn + Mem * 1-A    
        //case BLEND_FOG_MEM_FOG_MEM + (BLEND_OPA>>2):
            //Cycle1:   In * AFog + Fog * 1-A
            //Cycle2:   In * AIn + Mem * AMem   
        case BLEND_FOG_MEM_FOG_MEM + (BLEND_PASS>>2):
            //Cycle1:   In * AFog + Fog * 1-A
            //Cycle2:   In * 0 + In * 1
        case BLEND_XLU + (BLEND_XLU>>2):
            //Cycle1:   Fog * AFog + In * 1-A
            //Cycle2:   In * AIn + Mem * 1-A    
        case BLEND_BI_AFOG + (BLEND_XLU>>2):
            //Cycle1:   Bl * AFog + In * 1-A
            //Cycle2:   In * AIn + Mem * 1-A    
        case BLEND_XLU + (BLEND_FOG_MEM_IN_MEM>>2):
            //Cycle1:   In * AIn + Mem * 1-A
            //Cycle2:   In * AFog + Mem * 1-A   
        case BLEND_PASS + (BLEND_FOG_MEM_IN_MEM>>2):
            //Cycle1:   In * 0 + In * 1
            //Cycle2:   In * AFog + Mem * 1-A   
            BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            break;
        case BLEND_FOG_MEM_FOG_MEM + (BLEND_OPA>>2):
            //Cycle1:   In * AFog + Fog * 1-A
            //Cycle2:   In * AIn + Mem * AMem   
            BlendFunc(GL_ONE, GL_ZERO);
            glEnable(GL_BLEND);
            break;

        case BLEND_FOG_APRIM + (BLEND_OPA>>2):
            // For Golden Eye
            //Cycle1:   Fog * AFog + In * 1-A
            //Cycle2:   In * AIn + Mem * AMem   
        case BLEND_FOG_ASHADE + (BLEND_OPA>>2):
            //Cycle1:   Fog * AShade + In * 1-A
            //Cycle2:   In * AIn + Mem * AMem   
        case BLEND_BI_AFOG + (BLEND_OPA>>2):
            //Cycle1:   Bl * AFog + In * 1-A
            //Cycle2:   In * AIn + Mem * 1-AMem 
        case BLEND_FOG_ASHADE + (BLEND_NOOP>>2):
            //Cycle1:   Fog * AShade + In * 1-A
            //Cycle2:   In * AIn + In * 1-A
        case BLEND_NOOP + (BLEND_OPA>>2):
            //Cycle1:   In * AIn + In * 1-A
            //Cycle2:   In * AIn + Mem * AMem
        case BLEND_NOOP4 + (BLEND_NOOP>>2):
            //Cycle1:   Fog * AIn + In * 1-A
            //Cycle2:   In * 0 + In * 1
        case BLEND_FOG_ASHADE+(BLEND_PASS>>2):
            //Cycle1:   Fog * AShade + In * 1-A
            //Cycle2:   In * 0 + In * 1
        case BLEND_FOG_3+(BLEND_PASS>>2):
            BlendFunc(GL_ONE, GL_ZERO);
            glEnable(GL_BLEND);
            break;
        case BLEND_FOG_ASHADE+0x0301:
            // c800 - Cycle1:   Fog * AShade + In * 1-A
            // 0301 - Cycle2:   In * 0 + In * AMem
            BlendFunc(GL_SRC_ALPHA, GL_ZERO);
            glEnable(GL_BLEND);
            break;
        case 0x0c08+0x1111:
            // 0c08 - Cycle1:   In * 0 + In * 1
            // 1111 - Cycle2:   Mem * AFog + Mem * AMem
            BlendFunc(GL_ZERO, GL_DST_ALPHA);
            glEnable(GL_BLEND);
            break;
        default:
            if( blendmode_2 == (BLEND_PASS>>2) )
            {
                BlendFunc(GL_ONE, GL_ZERO);
            }
            else
            {
                BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            glEnable(GL_BLEND);
            break;
        }
        break;
    default:    // 1/2 Cycle or Copy
        if( ((rdp_reg.mode_l >> 14) & 0x01) && ((rdp_reg.mode_l >> 4) & 0x01) && blendmode_1 != BLEND_FOG_ASHADE )
        {
            BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            break;
        }
        //if( ((rdp_reg.mode_l >> 14) & 0x01) && options.enableHackForGames == HACK_FOR_COMMANDCONQUER )
        //{
        //  BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //  glEnable(GL_BLEND);
        //  break;
        //}

        switch ( blendmode_1 )
        //switch ( blendmode_2<<2 )
        {
        case BLEND_XLU: // IN * A_IN + MEM * (1-A_IN)
        case BLEND_BI_AIN:  // Bl * AIn + Mem * 1-A
        case BLEND_FOG_MEM: // c440 - Cycle1:   Fog * AFog + Mem * 1-A
        case BLEND_FOG_MEM_IN_MEM:  // c440 - Cycle1:   In * AFog + Mem * 1-A
        case BLEND_BLENDCOLOR:  //Bl * 0 + Bl * 1
        case 0x00c0:    //In * AIn + Fog * 1-A
            BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            break;
        case BLEND_MEM_ALPHA_IN:    //  Mem * AIn + Mem * AMem
            BlendFunc(GL_ZERO, GL_DST_ALPHA);
            glEnable(GL_BLEND);
            break;
        case BLEND_PASS:    // IN * 0 + IN * 1
            BlendFunc(GL_ONE, GL_ZERO);
            if( ((rdp_reg.mode_l >> 13) & 0x01) )
            {
                glEnable(GL_BLEND);
            }
            else
            {
                glDisable(GL_BLEND);
            }
            break;
        case BLEND_OPA:     // IN * A_IN + MEM * A_MEM
            //if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
            //{
            //  BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //}
            //else
            {
                BlendFunc(GL_ONE, GL_ZERO);
            }
            glEnable(GL_BLEND);
            break;
        case BLEND_NOOP:        // IN * A_IN + IN * (1 - A_IN)
        case BLEND_FOG_ASHADE:  // Fog * AShade + In * 1-A
        case BLEND_FOG_MEM_3:   // Mem * AFog + Fog * 1-A
        case BLEND_BI_AFOG:     // Bl * AFog + In * 1-A
            BlendFunc(GL_ONE, GL_ZERO);
            glEnable(GL_BLEND);
            break;
        case BLEND_FOG_APRIM:   // Fog * AFog + In * 1-A
            BlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ZERO);
            glEnable(GL_BLEND);
            break;
        case BLEND_NOOP3:       // In * 0 + Mem * 1
        case BLEND_NOOP5:       // Fog * 0 + Mem * 1
            BlendFunc(GL_ZERO, GL_ONE);
            glEnable(GL_BLEND);
            break;
        case BLEND_MEM:     // Mem * 0 + Mem * 1-A
            // WaveRace
            BlendFunc(GL_ZERO, GL_ONE);
            glEnable(GL_BLEND);
            break;
        default:
            BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            SetAlphaTestEnable(TRUE);
            break;
        }
    }
}

void MDrawVisualTriangle(int vn[3], _u32 t1, _u32 s1, _u32 s2)
{
    float color[4] = {1.0f,1.0f,1.0f,1.0f};
    float colorIv[4] = {1.0f,1.0f,1.0f,1.0f};
    float calc_color[4] = {1.0f,1.0f,1.0f,1.0f};
    float calc_colorIv[4] = {1.0f,1.0f,1.0f,1.0f};

    int i,ci;
    int passes = t1 >> 4;
    int aTile = rdp_reg.tile;
//** Lighting       

    LOG_TO_FILE("\tFLUSH TRIS pass 1");

    //Render_geometry_cullfrontback(0);

    ci = 0;
    rdp_reg.loadtile = 0;
    //rdp_reg.m_CurTile = &rdp_reg.td[rdp_reg.tile];
 
    Src_Alpha = GL_SRC_ALPHA;
    Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
    glEnable(GL_ALPHA_TEST);
///////*
//////  switch (rdp_reg.mode_l & 0x03)
//////    {
//////    case 0x00:
//////      glDisable(GL_ALPHA_TEST);
//////      break;
//////    case 0x01:
//////      glEnable(GL_ALPHA_TEST);
//////      break;
//////    case 0x02:
//////      glEnable(GL_ALPHA_TEST);
//////      break;
//////    case 0x03:
//////      glEnable(GL_ALPHA_TEST);
//////      break;
//////  }
//////    //glEnable(GL_BLEND);
///////**/ 
//////  switch (rdp_reg.mode_l & 0xFFFF0000)
//////    {
//////      // Road, zelda
//////      // clr_fog * a_step + clr_in * 1ma
//////      // clr_in * a_in + clr_mem * 1ma
//////    case 0xc8100000:
//////      break;
//////      
//////      // Mace objects
//////    case 0x03820000:
//////      // Mario kart player select
//////      // clr_in * 0 + clr_in * 1
//////      //  - or just clr_in, no matter what alpha
////////    case 0x0f1a0000:
////////        Src_Alpha = Src_Alpha;
//////    case 0x0f0a0000:
//////      Src_Alpha = GL_ONE;
//////      Dst_Alpha = GL_ZERO;
//////        glDisable(GL_ALPHA_TEST);
//////      break;
//////      
//////    default:
//////      Src_Alpha = GL_SRC_ALPHA;
//////      Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
//////    }
///////**/
//////  if (
//////      (rdp_reg.mode_l & 0x2000) 
//////            && !(rdp_reg.mode_l & 0x1000)
//////      )
//////  {
//////      //A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);
//////      Src_Alpha = GL_ONE;
//////      Dst_Alpha = GL_ZERO;
//////        glDisable(GL_ALPHA_TEST);
//////  }
///////**/
///////*
//////    {
//////        _u32 x;
//////        int b1,b2;
//////
//////      b1=GL_SRC_ALPHA;
//////      b2=GL_ONE_MINUS_SRC_ALPHA;
//////
//////        x= (rdp_reg.mode_l>>16) & 0x0f0f;
//////
//////        if (cycle_mode == 0)
//////          x>>=8;
//////
//////      switch (x)
//////      {
//////      case 0x0000: // decal?
//////            //b1=GL_ONE;
//////            //b2=GL_ZERO;
//////          break;
//////      case 0x0801:
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////          break;
//////      case 0x0c08:
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////          break;
//////      case 0x0c09: // decal?
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////          break;
//////      case 0x0f0a:
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////          //glDisable(GL_ALPHA_TEST);
//////          break;
//////      default:
//////          x = x;
//////      }
//////*/
///////*        if(cycle_mode == 1) x>>=2;
//////
//////        x&=0x3333;
//////
//////        if(!x)
//////      {
//////            b1=GL_SRC_ALPHA;
//////            b2=GL_ONE_MINUS_SRC_ALPHA;
//////      }
//////      else if (x==0x0302)
//////        {
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////        }
//////        else if(x==0x0011)
//////        {
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////        }
//////        else if(x==0x0010)
//////        {
//////            b1=GL_SRC_ALPHA;
//////            b2=GL_ONE_MINUS_SRC_ALPHA;
//////        }
//////        else if(x==0x1310)
//////        {
//////            b1=GL_ZERO;
//////            b2=GL_ONE; //X_INVOTHERALPHA;
//////        }
//////        else
//////        {
//////            b1=GL_ONE;
//////            b2=GL_ZERO;
//////        }
//////
//////      Src_Alpha = b1;
//////      Dst_Alpha = b2;
//////  }
//////*/
    CalcBlend();

//  glBlendFunc(Src_Alpha, Dst_Alpha);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (ArrayOk == 0)
        InitArrays();
#ifdef OLD_LIGHTING
    if ((rdp_reg.geometrymode & 0x00020000) != 0)
    {
#ifndef OPENGL_LIGHTS
        if (refresh_matrix)
        {
            MATRIX4 tmp_mtx;
            get_matrix(tmp_mtx);
            //glGetFloatv(GL_MODELVIEW_MATRIX, tmp_mtx);
            if (!compare_matrix4(view_matrix, tmp_mtx))
            {
                memcpy(view_matrix, tmp_mtx, sizeof(MATRIX4));
                m3_inverse( invers, view_matrix);               
                calculate_light_vectors();
            }
            refresh_matrix = FALSE;
        }
#endif //OPENGL_LIGHTS
        if (refresh_lights) calculate_light_vectors();
    }

//** Lighting - end
#ifdef OPENGL_LIGHTS
    //(rdp_reg.useLights != 0)
    if (((s1 & UseShade) != 0) && ((rdp_reg.geometrymode & 0x00020000) != 0)) 
    {
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }
#endif //OPENGL_LIGHTS

#endif //OLD_LIGHTING

    if (((t1 & 1) == 1) || (s1 & UseTex0Mask) || (s1 & UseTex1Mask))
    {
        _u32 sTM = TexMode;

        TexMode = 0;
        aTile = rdp_reg.tile;

        if (s1 & UseT1Iv)
        {
            LOG_TO_FILE("\t UseT1Iv");
            TexMode |= InvT1;
        }
        if (s1 & UseT1)
        {
            LOG_TO_FILE("\t UseT1");
            TexMode |= Tex1;
        }
        if (s1 & UseT1Alpha)
        {
            LOG_TO_FILE("\t UseT1Alpha");
            TexMode |= AlphaT1;
        }
        if (s1 & UseT1AlphaIv)
        {
            LOG_TO_FILE("\t UseT1AlphaIv");
            TexMode |= InvT1Alpha;
        }
        if (s1 & UseMT1Alpha)
        {
            LOG_TO_FILE("\t UseMT1Alpha");
            TexMode |= T1MultAlpha;
        }
        if (s1 & UseMT1AlphaIv) 
        {
            LOG_TO_FILE("\t UseMT1AlphaIv");
            TexMode |= InvT1MultAlpha;
        }

        if (s1 & UseT0)
        {
            LOG_TO_FILE("\t UseT0");
            TexMode |= Tex0;
        }
        if (s1 & UseT0Iv)
        {
            LOG_TO_FILE("\t UseT0Iv");
            TexMode |= InvT0;
        }
        if (s1 & UseT0Alpha)
        {
            LOG_TO_FILE("\t UseT0Alpha");
            TexMode |= AlphaT0;
        }
        if (s1 & UseT0AlphaIv)
        {
            LOG_TO_FILE("\t UseT0AlphaIv");
            TexMode |= InvT0Alpha;
        }
        if (s1 & UseMT0Alpha)
        {
            LOG_TO_FILE("\t UseMT0Alpha");
            TexMode |= T0MultAlpha;
        }
        if (s1 & UseMT0AlphaIv) 
        {
            LOG_TO_FILE("\t UseMT0AlphaIv");
            TexMode |= InvT0MultAlpha;
        }

        SetActiveTexture();
        TexMode = sTM;
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else 
    {
        glDisable(GL_TEXTURE_2D);
    }

    if(rdp_reg.mode_l & 0x04000)
    {
        LOG_TO_FILE("\t Force blend");
        //calc_color[0] *= rdp_reg.blend_r; //(((rdp_reg.primcolor >> 24) & 0xff) / 255.0f);
        //calc_color[1] *= rdp_reg.blend_g; //(((rdp_reg.primcolor >> 16) & 0xff) / 255.0f);
        //calc_color[2] *= rdp_reg.blend_b; //(((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
        //calc_color[3] *= rdp_reg.blend_a; //(((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s1 & UsePrim) != 0)
    {
        LOG_TO_FILE("\t UsePrim");
        calc_color[0] *= rdp_reg.prim_r; //(((rdp_reg.primcolor >> 24) & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.prim_g; //(((rdp_reg.primcolor >> 16) & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.prim_b; //(((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s1 & UseEnv) != 0)
    {
        LOG_TO_FILE("\t UseEnv");
        calc_color[0] *= rdp_reg.env_r; //(((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.env_g; //(((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.env_b; //(((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s1 & UsePrimIv) != 0)
    {
        LOG_TO_FILE("\t UsePrimIv");
/*
        calc_color[0] *= rdp_reg.prim_1mr;
        calc_color[1] *= rdp_reg.prim_1mg;
        calc_color[2] *= rdp_reg.prim_1mb;
*/
        calc_colorIv[0] *= rdp_reg.prim_r; //(((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.prim_g; //(((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.prim_b; //(((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s1 & UseEnvIv) != 0)
    {
        LOG_TO_FILE("\t UseEnvIv");
/*
        calc_color[0] *= rdp_reg.env_1mr;
        calc_color[1] *= rdp_reg.env_1mg;
        calc_color[2] *= rdp_reg.env_1mb;
*/
        calc_colorIv[0] *= rdp_reg.env_r; //(((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.env_g; //(((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.env_b; //(((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s1 & UsePrimAlpha) != 0)
    {
        LOG_TO_FILE("\t UsePrimAlpha");
        calc_color[3] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseEnvAlpha) != 0)
    {
        LOG_TO_FILE("\t UseEnvAlpha");
        calc_color[3] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s1 & UsePrimAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UsePrimAlphaIv");
//      calc_color[3] *= rdp_reg.prim_1ma; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_colorIv[3] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseEnvAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseEnvAlphaIv");
//      calc_color[3] *= rdp_reg.env_1ma; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_colorIv[3] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseMPrimAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMPrimAlpha");
        calc_color[0] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseMEnvAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMEnvAlpha");
        calc_color[0] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseMPrimAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMPrimAlphaIv");
/*
        calc_color[0] *= rdp_reg.prim_1ma;
        calc_color[1] *= rdp_reg.prim_1ma;
        calc_color[2] *= rdp_reg.prim_1ma;
*/
        calc_colorIv[0] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseMEnvAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMEnvAlphaIv");
/*
        calc_color[1] *= rdp_reg.env_1ma;
        calc_color[2] *= rdp_reg.env_1ma;
        calc_color[3] *= rdp_reg.env_1ma;
*/
        calc_colorIv[0] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s1 & UseShade) != 0)
    {
        LOG_TO_FILE("\t UseShade");
    }

    if ((s1 & UseShadeIv) != 0)
    {
        LOG_TO_FILE("\t UseShadeIv");
    }

    if ((s1 & UseShadeAlpha) != 0)
    {
        LOG_TO_FILE("\t UseShadeAlpha");
    }

    if ((s1 & UseShadeAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseShadeAlphaIv");
    }

    if ((s1 & UseMShadeAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMShadeAlpha");
    }

    if ((s1 & UseMShadeAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMShadeAlphaIv");
    }

    LOG_TO_FILE("");

    for(i=0; i<Vtcnt; i++)
    {
        t_vtx *vertex = &rdp_reg.vtx[vn[i]];

        color[0] = calc_color[0];
        color[1] = calc_color[1];
        color[2] = calc_color[2];
        color[3] = calc_color[3];

        colorIv[0] = calc_colorIv[0];
        colorIv[1] = calc_colorIv[1];
        colorIv[2] = calc_colorIv[2];
        colorIv[3] = calc_colorIv[3];

//      rdp_reg.geometrymode |= 0x00020000;

        if ((s1 & UseShade) != 0)
        {
            //if ((rdp_reg.geometrymode & 0x00020000) == 0) 
            //{
            //  color[0] *= ((vertex->r) / 255.0f);
            //  color[1] *= ((vertex->g) / 255.0f);
            //  color[2] *= ((vertex->b) / 255.0f);
            //}
            //else
            { 
                float lcolor[4] = {0,0,0,0};
#ifndef OPENGL_LIGHTS
                //////math_lighting((t_vtx_tn*)(char*)vertex, lcolor);
                ////math_lightingN((t_vtx*)(char*)vertex, lcolor);
                ////color[0] *= lcolor[0];
                ////color[1] *= lcolor[1];
                ////color[2] *= lcolor[2];
                color[0] *= vertex->lcolor[0];
                color[1] *= vertex->lcolor[1];
                color[2] *= vertex->lcolor[2];
#else
                NrmArry[i*3+0] = (((sBYTE)vertex->r) / 127.0f);
                NrmArry[i*3+1] = (((sBYTE)vertex->g) / 127.0f);
                NrmArry[i*3+2] = (((sBYTE)vertex->b) / 127.0f);
//              NormalizeVector(NrmArry);
#endif
            }
        }

        if ((s1 & UseShadeIv) != 0)
        {
            colorIv[0] *= vertex->lcolor[0];
            colorIv[1] *= vertex->lcolor[1];
            colorIv[2] *= vertex->lcolor[2];
        }

        if (((s1 & UseShadeAlpha) != 0) && ((rdp_reg.geometrymode & 0x00020000) == 0))
        {
            color[3] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if (((s1 & UseShadeAlphaIv) != 0) && ((rdp_reg.geometrymode & 0x00020000) == 0))
        {
            colorIv[3] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if ((s1 & UseMShadeAlpha) != 0)
        {
            color[0] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            color[1] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            color[2] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if ((s1 & UseMShadeAlphaIv) != 0)
        {
            colorIv[0] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            colorIv[1] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            colorIv[2] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if (s1 & UseColorIv)
        {
            color[0] *= 1.0f - colorIv[0];
            color[1] *= 1.0f - colorIv[1];
            color[2] *= 1.0f - colorIv[2];
        }

        if (s1 & UseAlphaIv)
        {
            color[3] *= 1.0f - colorIv[3];
        }

        ClrArry[i*4+0] = color[0];
        ClrArry[i*4+1] = color[1];
        ClrArry[i*4+2] = color[2];
        ClrArry[i*4+3] = color[3];

        VtxArry[i*4+0] = vertex->x;
        VtxArry[i*4+1] = vertex->y;
        VtxArry[i*4+2] = vertex->z;
        VtxArry[i*4+3] = vertex->w;

        if (((t1 & 1) == 1) || (s1 & UseTex0Mask) || (s1 & UseTex1Mask))
        {

        if (0x00080000 & rdp_reg.geometrymode)
            i=i+0;
        
        //(vertex->genliner)
        if (0x00040000 & rdp_reg.geometrymode)//(vertex->genliner)
        {
            if (ucode_version != 5)
            {
                TexArry[i*2+0] = vertex->s;
                TexArry[i*2+1] = vertex->t;
            }
            else
            {
                TexArry[i*2+0] = VtSc[i];
                TexArry[i*2+1] = VtTc[i];
            }
        }
        else
        {
            float tS,tT;
            //float ts,tt;
            
            if (ucode_version != 6)
            {
                tS = vertex->s * rdp_reg.texture[aTile + aTilew].scale_s;
                tT = vertex->t * rdp_reg.texture[aTile + aTileh].scale_t;

                {
                    _u32 tmpS = rdp_reg.td[aTile + aTilew].uls;
                    _u32 tmpT = rdp_reg.td[aTile + aTileh].ult;

                    tS -= (float)(tmpS);
                    tT -= (float)(tmpT);
                }

            }
            else
            {
                tS = VtSc[i] * rdp_reg.texture[aTile + aTilew].scale_s;
                tT = VtTc[i] * rdp_reg.texture[aTile + aTileh].scale_t;
            }

            TexArry[i*2+0] = tS * rdp_reg.texture[aTile + aTilew].scale_w / Multw;
            TexArry[i*2+1] = tT * rdp_reg.texture[aTile + aTileh].scale_h / Multh;
        }
        }

    }

    glDrawArrays(GL_TRIANGLES,0,Vtcnt);

    if (passes > 0x1) 
    {

    LOG_TO_FILE("\tFLUSH TRIS pass 2");

    glBlendFunc(Src_Alpha, GL_ONE);
    if (((t1 & 2) == 2) || (s2 & UseTex0Mask) || (s2 & UseTex1Mask))
    {
        _u32 sTM = TexMode;

        TexMode = 0;
        aTile = rdp_reg.tile;

        if (s2 & UseT1Iv)
        {
            LOG_TO_FILE("\t UseT1Iv");
            TexMode |= InvT1;
        }
        if (s2 & UseT1)
        {
            LOG_TO_FILE("\t UseT1");
            TexMode |= Tex1;
        }
        if (s2 & UseT1Alpha)
        {
            LOG_TO_FILE("\t UseT1Alpha");
            TexMode |= AlphaT1;
        }
        if (s2 & UseT1AlphaIv)
        {
            LOG_TO_FILE("\t UseT1AlphaIv");
            TexMode |= InvT1Alpha;
        }
        if (s2 & UseMT1Alpha)
        {
            LOG_TO_FILE("\t UseMT1Alpha");
            TexMode |= T1MultAlpha;
        }
        if (s2 & UseMT1AlphaIv) 
        {
            LOG_TO_FILE("\t UseMT1AlphaIv");
            TexMode |= InvT1MultAlpha;
        }

        if (s2 & UseT0)
        {
            LOG_TO_FILE("\t UseT0");
            TexMode |= Tex0;
        }
        if (s2 & UseT0Iv)
        {
            LOG_TO_FILE("\t UseT0Iv");
            TexMode |= InvT0;
        }
        if (s2 & UseT0Alpha)
        {
            LOG_TO_FILE("\t UseT0Alpha");
            TexMode |= AlphaT0;
        }
        if (s2 & UseT0AlphaIv)
        {
            LOG_TO_FILE("\t UseT0AlphaIv");
            TexMode |= InvT0Alpha;
        }
        if (s2 & UseMT0Alpha)
        {
            LOG_TO_FILE("\t UseMT0Alpha");
            TexMode |= T0MultAlpha;
        }
        if (s2 & UseMT0AlphaIv) 
        {
            LOG_TO_FILE("\t UseMT0AlphaIv");
            TexMode |= InvT0MultAlpha;
        }

        SetActiveTexture();
        TexMode = sTM;
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else 
    {
        glDisable(GL_TEXTURE_2D);
    }

    calc_color[0] = 1.0f;
    calc_color[1] = 1.0f;
    calc_color[2] = 1.0f;
    calc_color[3] = 1.0f;

    calc_colorIv[0] = 1.0f;
    calc_colorIv[1] = 1.0f;
    calc_colorIv[2] = 1.0f;
    calc_colorIv[3] = 1.0f;

    if ((s2 & UsePrim) != 0)
    {
        LOG_TO_FILE("\t UsePrim");
        calc_color[0] *= rdp_reg.prim_r; //(((rdp_reg.primcolor >> 24) & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.prim_g; //(((rdp_reg.primcolor >> 16) & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.prim_b; //(((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s2 & UseEnv) != 0)
    {
        LOG_TO_FILE("\t UseEnv");
        calc_color[0] *= rdp_reg.env_r; //(((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.env_g; //(((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.env_b; //(((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s2 & UsePrimIv) != 0)
    {
        LOG_TO_FILE("\t UsePrimIv");
        calc_colorIv[0] *= rdp_reg.prim_r; //(((rdp_reg.primcolor >> 24) & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.prim_g; //(((rdp_reg.primcolor >> 16) & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.prim_b; //(((rdp_reg.primcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s2 & UseEnvIv) != 0)
    {
        LOG_TO_FILE("\t UseEnvIv");
        calc_colorIv[0] *= rdp_reg.env_r; //(((rdp_reg.envcolor >> 24) & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.env_g; //(((rdp_reg.envcolor >> 16) & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.env_b; //(((rdp_reg.envcolor >>  8) & 0xff) / 255.0f);
    }

    if ((s2 & UsePrimAlpha) != 0)
    {
        LOG_TO_FILE("\t UsePrimAlpha");
        calc_color[3] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseEnvAlpha) != 0)
    {
        LOG_TO_FILE("\t UseEnvAlpha");
        calc_color[3] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s2 & UsePrimAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UsePrimAlphaIv");
        calc_colorIv[3] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseEnvAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseEnvAlphaIv");
        calc_colorIv[3] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseMPrimAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMPrimAlpha");
        calc_color[0] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseMEnvAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMEnvAlpha");
        calc_color[0] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_color[1] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_color[2] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseMPrimAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMPrimAlphaIv");
        calc_colorIv[0] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.prim_a; //((rdp_reg.primcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseMEnvAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMEnvAlphaIv");
        calc_colorIv[0] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_colorIv[1] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
        calc_colorIv[2] *= rdp_reg.env_a; //((rdp_reg.envcolor & 0xff) / 255.0f);
    }

    if ((s2 & UseShade) != 0)
    {
        LOG_TO_FILE("\t UseShade");
    }

    if ((s2 & UseShadeIv) != 0)
    {
        LOG_TO_FILE("\t UseShadeIv");
    }

    if ((s2 & UseShadeAlpha) != 0)
    {
        LOG_TO_FILE("\t UseShadeAlpha");
    }

    if ((s2 & UseShadeAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseShadeAlphaIv");
    }

    if ((s2 & UseMShadeAlpha) != 0)
    {
        LOG_TO_FILE("\t UseMShadeAlpha");
    }

    if ((s2 & UseMShadeAlphaIv) != 0)
    {
        LOG_TO_FILE("\t UseMShadeAlphaIv");
    }
    
    LOG_TO_FILE("");

    for(i=0; i<Vtcnt; i++)
    {
        t_vtx *vertex = &rdp_reg.vtx[vn[i]];

        color[0] = calc_color[0];
        color[1] = calc_color[1];
        color[2] = calc_color[2];
        color[3] = calc_color[3];

        colorIv[0] = calc_colorIv[0];
        colorIv[1] = calc_colorIv[1];
        colorIv[2] = calc_colorIv[2];
        colorIv[3] = calc_colorIv[3];

        if ((s2 & UseShade) != 0)
        {
            color[0] *= vertex->lcolor[0];
            color[1] *= vertex->lcolor[1];
            color[2] *= vertex->lcolor[2];
        }

        if ((s2 & UseShadeIv) != 0)
        {
            colorIv[0] *= vertex->lcolor[0];
            colorIv[1] *= vertex->lcolor[1];
            colorIv[2] *= vertex->lcolor[2];
        }

        if (((s2 & UseShadeAlpha) != 0) && ((rdp_reg.geometrymode & 0x00020000) == 0))
        {
            color[3] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if (((s2 & UseShadeAlphaIv) != 0) && ((rdp_reg.geometrymode & 0x00020000) == 0))
        {
            colorIv[3] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if ((s2 & UseMShadeAlpha) != 0)
        {
            color[0] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            color[1] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            color[2] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if ((s2 & UseMShadeAlphaIv) != 0)
        {
            colorIv[0] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            colorIv[1] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
            colorIv[2] *= vertex->lcolor[3];//((vertex->a) / 255.0f);
        }

        if (s2 & UseColorIv)
        {
            color[0] *= 1.0f - colorIv[0];
            color[1] *= 1.0f - colorIv[1];
            color[2] *= 1.0f - colorIv[2];
        }

        if (s2 & UseAlphaIv)
        {
            color[3] *= 1.0f - colorIv[3];
        }

        ClrArry[i*4+0] = color[0];
        ClrArry[i*4+1] = color[1];
        ClrArry[i*4+2] = color[2];
        ClrArry[i*4+3] = color[3];

        if (((t1 & 1) == 1) || (s1 & UseTex0Mask) || (s1 & UseTex1Mask))
        {

        if (0x00080000 & rdp_reg.geometrymode)
            i=i+0;
        
        if (0x00040000 & rdp_reg.geometrymode)//(vertex->genliner)
        {
            if (ucode_version != 5)
            {
                TexArry[i*2+0] = vertex->s;
                TexArry[i*2+1] = vertex->t;
            }
            else
            {
                TexArry[i*2+0] = VtSc[i];
                TexArry[i*2+1] = VtTc[i];
            }
        }
        else
        {
            float tS,tT;
            //float ts,tt;
            
            if (ucode_version != 6)
            {
                tS = vertex->s * rdp_reg.texture[aTile + aTilew].scale_s;
                tT = vertex->t * rdp_reg.texture[aTile + aTileh].scale_t;

                {
                    _u32 tmpS = rdp_reg.td[aTile + aTilew].uls;
                    _u32 tmpT = rdp_reg.td[aTile + aTileh].ult;
                    tS -= (float)(tmpS);
                    tT -= (float)(tmpT);
                }

            }
            else
            {
                tS = VtSc[i] * rdp_reg.texture[aTile + aTilew].scale_s;
                tT = VtTc[i] * rdp_reg.texture[aTile + aTileh].scale_t;
            }

            TexArry[i*2+0] = tS * rdp_reg.texture[aTile + aTilew].scale_w / Multw;
            TexArry[i*2+1] = tT * rdp_reg.texture[aTile + aTileh].scale_h / Multh;
        }
        }

    }

    glDrawArrays(GL_TRIANGLES,0,Vtcnt);
    }

    if ((rdp_reg.geometrymode & 0x00010000) && FogEnabled)
        Render_Fog2(vn);

    //glBlendFunc(Src_Alpha, Dst_Alpha);
}
// new june 16, will clean up code later.
extern void MathTextureScales();
extern HWND hGraphics;
extern int triUseprim; // new june 14
void DrawTexRectangleT(float u1, float u2, float v1, float v2, int tile,
                       int xh, int xl, int yh, int yl)
{
    glCallList(rdp_reg.colorimg_width);

    SetActiveTexture();
    glColor3f(1,1,1);       //are in modulate-mode...
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glTexCoord2f(u2,  v1); glVertex2f((float)xl, (float)yl);
        glTexCoord2f(u1,  v1); glVertex2f((float)xh, (float)yl);
        glTexCoord2f(u1,  v2); glVertex2f((float)xh, (float)yh);
        glTexCoord2f(u2,  v2); glVertex2f((float)xl, (float)yh);
    glEnd();

    glCallList(kListEnd2D);
}

void DrawTexRectangle(float xh, float yh, 
                      float xl, float yl,
                      int tile,
                      float s, float t, 
                      float dsdx, float dtdy)
{
/*  int depth_test, w, h;
    float sw,sh;
    t_tile *tmpTile = &rdp_reg.td[tile];

//  sw = tmpTile->Width;
//  sh = tmpTile->Height;
    s *= rdp_reg.texture[tile].scale_s;
    t *= rdp_reg.texture[tile].scale_t;

//** Set to 2d ...
    glCallList(rdp_reg.colorimg_width);

    SetActiveTexture();
    glColor3f(1,1,1);       //are in modulate-mode...
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);

        glTexCoord2f(1, 1); glVertex2f(xl, yl);
        glTexCoord2f(s, 1); glVertex2f(xh, yl);
        glTexCoord2f(s, t); glVertex2f(xh, yh);
        glTexCoord2f(1, t); glVertex2f(xl, yh);

/*      glTexCoord2f(dsdx*(s/w + sw/w), dtdy*(t/h + sh/h)); glVertex2f(xl, yl);
        glTexCoord2f(dsdx*(s/w +    0), dtdy*(t/h + sh/h)); glVertex2f(xh, yl);
        glTexCoord2f(dsdx*(s/w +    0), dtdy*(t/h +    0)); glVertex2f(xh, yh);
        glTexCoord2f(dsdx*(s/w + sw/w), dtdy*(t/h +    0)); glVertex2f(xl, yh);

    glEnd();
//**
    glCallList(kListEnd2D);
*/
    t_tile *tmpTile = &rdp_reg.td[tile];
    float s0, t0, s1, t1;

    //rdp_reg.m_CurTile = &rdp_reg.td[tile];

//  MathSize(tile_num);
    s0 = s / tmpTile->Width;
    t0 = t / tmpTile->Height;
    s1 = (s + tmpTile->Width  * dsdx) / tmpTile->Width;
    t1 = (t + tmpTile->Height * dtdy) / tmpTile->Height;

//** Set to 2d ...
    SetActiveTexture();

    glEnable(GL_TEXTURE_2D);

    glCallList(rdp_reg.colorimg_width);
    glColor3f(1,1,1);       //are in modulate-mode...

    glBegin(GL_QUADS);
        glTexCoord2f(s1, t1); glVertex2f(xl, yl);
        glTexCoord2f(s0, t1); glVertex2f(xh, yl);
        glTexCoord2f(s0, t0); glVertex2f(xh, yh);
        glTexCoord2f(s1, t0); glVertex2f(xl, yh);
    glEnd();
//**
//  glCallList(kListEnd2D);


}

   _u32 swidth = 320;
   _u32 sheight = 240;
   _u32 sulx = 0;
   _u32 stuly = 0;

void ResizeVisualClipRectangle()
{
/*
   _u32 tsulx    = (_u32)(sulx * fScalex);
   _u32 tstuly   = WinData.Height - (_u32)(stuly * fScaley);
   _u32 tswidth  = (_u32)(swidth * fScalex);
   _u32 tsheight = (_u32)(sheight * fScaley);
*/
   _u32 tsulx    = (_u32)(sulx * fScalex);
   _u32 tstuly   = (_u32)((imgHeight - stuly) * fScaley);
   _u32 tswidth  = (_u32)(swidth * fScalex);
   _u32 tsheight = (_u32)(sheight * fScaley);

   glScissor(tsulx ,tstuly,tswidth,tsheight);
   glEnable(GL_SCISSOR_TEST);
}

void SetVisualClipRectangle(_u32 ulx, _u32 uly, _u32 lrx, _u32 lry)
{
   _u32 width = lrx - ulx;
   _u32 height = lry - uly;
   _u32 tuly = (_u32)(imgHeight - lry);

   swidth = width;
   sheight = height;
   sulx = ulx;
//   stuly = tuly;
   stuly = lry;

   ResizeVisualClipRectangle();
//   glScissor(ulx * fScalex ,tuly * fScaley ,width * fScalex ,height * fScaley);
//   glEnable(GL_SCISSOR_TEST);

}

void SetVisualRenderBuffer(void)
{

}
static _u32 CurVisualColor;

void SetVisualColor(_u32 Color)
{
    CurVisualColor = Color;
}

