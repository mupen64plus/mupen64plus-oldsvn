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

//#define DEBUG_R
#include <windows.h>
#include <math.h>
#include <gl/gl.h>
//#include <glext.h>

#include "WinGL.h"
#include "rdp_registers.h"
#include "texture.h"
#include "combine.h"

//** externals
extern int SetActiveTexture();
extern t_rdp_reg rdp_reg;
extern HWND hGraphics;
extern _u32 TexMode;
extern _u32 TexColor;
extern int ucode_version;
extern int Src_Alpha, Dst_Alpha;

extern float fScalex;
extern float fScaley;
extern float imgHeight;
extern float imgWidth;

extern int aTilew;
extern int aTileh;
extern float Multw;
extern float Multh;

char output[1024];
char render_output[1024];

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

 static char *Alpha[] = { "AC_COMBINED" ,           "AC_TEXEL0" ,
                          "AC_TEXEL1" ,             "AC_PRIMITIVE" , 
                          "AC_SHADE" ,              "AC_ENVIRONMENT" , 
                          "AC_PRIM_LOD_FRAC" ,      "AC_0"};

 //** global variables
extern BOOL refresh_matrix;
extern BOOL refresh_lights;

extern _u8 uc0_a0,  uc0_b0,  uc0_c0,  uc0_d0, 
       uc0_Aa0, uc0_Ab0, uc0_Ac0, uc0_Ad0,
       uc0_a1,  uc0_b1,  uc0_c1,  uc0_d1, 
       uc0_Aa1, uc0_Ab1, uc0_Ac1, uc0_Ad1;

extern _u8 cycle_mode;
extern _u32 cycle1;
extern _u32 cycle2;
extern int DList_C;

//** Prototypes
void Render_FlushVisualRenderBuffer(void);
void Render_load_modelview(float m[4][4]);
void Render_load_projection(float m[4][4]);
void Render_mul_modelview(float m[4][4]);
void Render_mul_projection(float m[4][4]);
void Render_push_load_modelview(float m[4][4]);
void Render_push_mul_modelview(float m[4][4]);
void Render_pop_modelview();
void Render_viewport();
void Render_lookat_x();
void Render_lookat_y();
void Render_light(int n);
void Render_line3d(int vn[]);
void Render_geometry_zbuffer(int on);
void Render_geometry_cullfront(int on);
void Render_geometry_cullback(int on);
void Render_geometry_cullfrontback(int on);
void Render_ClearVisual();
void Render_tri1(int vn[], _u8 flag);
void Render_tri2(int vn[], _u8 flag);

static unsigned char tBuff[1024][1024][4];
static unsigned char tBuff2[1024][1024][4];
BOOL NewTex = TRUE;

byte rbmpHdr[] = { 0x42, 0x4D, 0x36, 0x84,
                  0x03, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x36, 0x00,
                  0x00, 0x00, 0x28, 0x00,
                  0x00, 0x00, 0x40, 0x01,
                  0x00, 0x00, 0xF0, 0x00,
                  0x00, 0x00, 0x01, 0x00,
                  0x18, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x84,
                  0x03, 0x00, 0xC4, 0x0E,
                  0x00, 0x00, 0xC4, 0x0E,
                  0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00};

int scrshtcnt = 0;

void DumpScrn2Bmp(_u8 *bffr,_u32 width, _u32 height)
{
    t_tile *t_p = &rdp_reg.td[0];
    t_tile *lt_p = &rdp_reg.td[7];

    static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
    static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };
    static char *cm[]       = { "NOMIRROR/WARP(NOCLAMP)", "MIRROR", "CLAMP", "MIRROR&CLAMP" };

    int x,y;
    _u32 *tbffr = (_u32*)bffr;
    char bmpFileName[128] = "";
    char sMsg[256] = "";

    FILE *bmpFile;
    byte tline[1024 * 4];

    *(_u32*)&rbmpHdr[2] = (_u32) ((width * height * 3) + 0x036);

    *(_u32*)&rbmpHdr[0x12] = width;
    *(_u32*)&rbmpHdr[0x16] = height;

    *(_u32*)&rbmpHdr[0x22] = (_u32) ((width * height * 3));

    sprintf(bmpFileName,"Tiles\\ScrnSht%08i.bmp",scrshtcnt++);
    bmpFile = fopen(bmpFileName,"wb");

    fwrite(rbmpHdr, 0x36, 1, bmpFile);
    
//  for ( y = (int)height - 1;y >= 0; y--)
    for ( y = 0;y < (int)height; y++)
    {
        tbffr = (_u32*)bffr;
        tbffr += (y * width);
        for (x = 0; x < (int)width; x++)
        {
            tline[x*3+0] = ((_u8*)tbffr)[2];
            tline[x*3+1] = ((_u8*)tbffr)[1];
            tline[x*3+2] = ((_u8*)tbffr)[0];
            tbffr++;
        }

        fwrite(tline, ((width * 3) + 3) & 0x0fffffc, 1, bmpFile);
    }
    fclose(bmpFile);

}

void fastScaleImage(int srcwidth, int srcheight,unsigned char *SrcBuff, int dstwidth,int dstheight, unsigned char *DstBuff)
{
    int xPos,yPos;
    long sXpos = 0;
    long sYpos = 0;
    long dX = (long)((65536.0f * (float)(srcwidth)/(float)dstwidth));
    long dY = (long)((65536.0f * (float)(srcheight)/(float)dstheight));
    _u32 *tsrc = (_u32*)SrcBuff;
    _u32 *tdst = (_u32*)DstBuff;
    _u32 *src;
    _u32 *dst;

    for(yPos = 0; yPos < dstheight; yPos++)
    {
        dst = (_u32*)&(tdst[yPos * dstwidth]);
        src = (_u32*)&(tsrc[(sYpos >> 16) * srcwidth]);
        sXpos = 0;
        for(xPos = 0; xPos < dstwidth; xPos++)
        {
            dst[xPos]=src[sXpos >> 16];
            sXpos += dX;
        }
        sYpos += dY;
    }
}

void fastScaleImage2(int srcwidth, int srcheight,unsigned char *SrcBuff, int dstwidth,int dstheight, unsigned char *DstBuff)
{
    int xPos,yPos;
    long sXpos = 0;
    long sYpos = 0;
    long dX = (long)((65536.0f * (float)(srcwidth)/(float)dstwidth));
    long dY = (long)((65536.0f * (float)(srcheight)/(float)dstheight));
    _u32 *tsrc = (_u32*)SrcBuff;
    _u32 *tdst = (_u32*)DstBuff;
    _u32 *src;
    _u32 *dst;

    for(yPos = 0; yPos < dstheight; yPos++)
    {
        dst = (_u32*)&(tdst[(dstheight - yPos) * dstwidth - 1]);
        src = (_u32*)&(tsrc[(srcheight - (sYpos >> 16)) * srcwidth - 1]);
        sXpos = 0;
        for(xPos = dstwidth; xPos > 0; xPos--)
        {
            dst[xPos - 1]=src[dstwidth - (sXpos >> 16) - 1];
            sXpos += dX;
        }
        sYpos += dY;
    }
}

void fastRepeatScaleImage(int srcwidth, int srcheight,unsigned char *SrcBuff,int rptwidth, int rptheight, int dstwidth,int dstheight, unsigned char *DstBuff)
{
    int xPos,yPos;
    long sXpos = 0;
    long sYpos = 0;
    long dX = (long)((65536.0f * (float)(srcwidth)/(float)dstwidth));
    long dY = (long)((65536.0f * (float)(srcheight)/(float)dstheight));
    _u32 *tsrc = (_u32*)SrcBuff;
    _u32 *tdst = (_u32*)DstBuff;
    _u32 *src;
    _u32 *dst;

    for(yPos = 0; yPos < dstheight; yPos++)
    {
        dst = (_u32*)&(tdst[yPos * dstwidth]);
        src = (_u32*)&(tsrc[(sYpos >> 16) * srcwidth]);
        sXpos = 0;
        for(xPos = 0; xPos < dstwidth; xPos++)
        {
            dst[xPos]=src[sXpos >> 16];
            sXpos += dX;
        }
        sYpos += dY;
    }
}

extern unsigned char    *pRDRAM;
extern BOOL FrameBuffer;
extern BOOL FogEnabled;

//** Functions
void Render_FrameBuffer5(void)
{
    if (rdp_reg.colorimg_addr) // && FrameBuffer)
    {
        int realwidth = WinData.Width;
        int realheight = WinData.Height;
        int new_width = (int)imgWidth;
        int new_height = (int)imgHeight;

        _u16 *dst;
        _u8 *src;
        _u32 *src32;
        int cx,cy;

        glFlush();
        glReadBuffer(GL_BACK);
        glRasterPos2i(0,0);
        glReadPixels (0,0,realwidth, realheight,  GL_RGBA, GL_UNSIGNED_BYTE, tBuff); 
//      gluScaleImage_asm386optimized(GL_RGBA, GL_FALSE, 1, realwidth, realheight,(unsigned char*)tBuff,
//                          new_width, new_height, (unsigned char*)tBuff2);

        fastScaleImage(realwidth, realheight,(unsigned char*)tBuff,
                            new_width, new_height, (unsigned char*)tBuff2);

        for (cy = 0; cy < new_height; cy ++)
        {
            dst = (_u16*)&pRDRAM[(((_u32)new_width * cy * 2) + rdp_reg.colorimg_addr) & 0x07FFFFF];
//          src = (_u8*)((_u32)(tBuff2) + (((_u32)new_height - cy -1) * (_u32)new_width * 4));
            src = (_u8*)((_u32)(tBuff2) + ((cy) * (_u32)new_width * 4));
            src32 = (_u32*)(tBuff2);
            src = (_u8*)&src32[((new_height - cy -1) * new_width)];
            for (cx = 0; cx < new_width; cx +=2)
            {
                _u16 tclr = (((_u16)(src[cx * 4 + 4]) & 0xf8)<< 8) |
                            (((_u16)(src[cx * 4 + 5]) & 0xf8)<< 3) |
                            (((_u16)(src[cx * 4 + 6]) & 0xf8)>> 2) |
                            1;
                *(dst++) = tclr;
                dst[cx] = tclr;

                tclr = (((_u16)(src[cx * 4 + 0]) & 0xf8)<< 8) |
                       (((_u16)(src[cx * 4 + 1]) & 0xf8)<< 3) |
                       (((_u16)(src[cx * 4 + 2]) & 0xf8)>> 2) |
                       1;
                *(dst++) = tclr;
                dst[cx+1] = tclr;
            }
        }

    }
}

//** Functions
void Render_FrameBuffer(void)
{
    if (rdp_reg.colorimg_addr && FrameBuffer)
    {
        int realwidth = WinData.Width;
        int realheight = WinData.Height;
        int new_width = (int)imgWidth;
        int new_height = (int)imgHeight;

        _u16 *dst;
        _u8 *src;
        _u32 *src32;
        int cx,cy;

        glFlush();
        glReadBuffer(GL_BACK);
        glRasterPos2i(0,0);
        glReadPixels (0,0,realwidth, realheight,  GL_RGBA, GL_UNSIGNED_BYTE, tBuff); 
//      gluScaleImage_asm386optimized(GL_RGBA, GL_FALSE, 1, realwidth, realheight,(unsigned char*)tBuff,
//                          new_width, new_height, (unsigned char*)tBuff2);

        fastScaleImage(realwidth, realheight,(unsigned char*)tBuff,
                            new_width, new_height, (unsigned char*)tBuff2);

        for (cy = 0; cy < new_height; cy ++)
        {
            dst = (_u16*)&pRDRAM[(((_u32)new_width * cy * 2) + rdp_reg.colorimg_addr) & 0x07FFFFF];
//          src = (_u8*)((_u32)(tBuff2) + (((_u32)new_height - cy -1) * (_u32)new_width * 4));
            src = (_u8*)((_u32)(tBuff2) + ((cy) * (_u32)new_width * 4));
            src32 = (_u32*)(tBuff2);
            src = (_u8*)&src32[((new_height - cy -1) * new_width)];
            for (cx = 0; cx < new_width; cx +=2)
            {
                _u16 tclr = (((_u16)(src[cx * 4 + 4]) & 0xf8)<< 8) |
                            (((_u16)(src[cx * 4 + 5]) & 0xf8)<< 3) |
                            (((_u16)(src[cx * 4 + 6]) & 0xf8)>> 2) |
                            1;
                *(dst++) = tclr;
                dst[cx] = tclr;

                tclr = (((_u16)(src[cx * 4 + 0]) & 0xf8)<< 8) |
                       (((_u16)(src[cx * 4 + 1]) & 0xf8)<< 3) |
                       (((_u16)(src[cx * 4 + 2]) & 0xf8)>> 2) |
                       1;
                *(dst++) = tclr;
                dst[cx+1] = tclr;
            }
        }

    }
}

void Render_FlushVisualRenderBuffer(void)
{

    WGL_SwapBuffers();
}

void Render_ResetViewPort()
{
    rdp_reg.vp[0] = imgWidth / 2;
    rdp_reg.vp[1] = imgHeight / 2;
    rdp_reg.vp[4] = imgWidth / 2;
    rdp_reg.vp[5] = imgHeight / 2;
    Render_viewport();
}

void Render_ClearVisual()
{
        glDisable(GL_SCISSOR_TEST);
        glDrawBuffer(GL_BACK);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glDepthMask(GL_TRUE);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glClear(GL_DEPTH_BUFFER_BIT);
//        glEnable(GL_SCISSOR_TEST);
}

void Render_load_modelview(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf((GLfloat *)m);
}

void Render_load_projection(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((GLfloat *)m);
//        glMatrixMode(GL_MODELVIEW);
}

void Render_mul_modelview(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_MODELVIEW);
        glMultMatrixf((GLfloat *)m);
}

void Render_mul_projection(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_PROJECTION);
        glMultMatrixf((GLfloat *)m);
        glMatrixMode(GL_MODELVIEW);
}

void Render_push_load_modelview(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf((GLfloat *)m);
}

void Render_push_mul_modelview(float m[4][4])
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixf((GLfloat *)m);
}

void Render_pop_modelview()
{
        refresh_matrix = TRUE;
        glFlush();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
}

//extern int WGL_sizeX;
//extern int WGL_sizeY;
int VPulx, VPuly, VPlrx, VPlry, VPwidth, VPheight;

void Render_viewport()
{
    float fVPulx, fVPuly, fVPlrx, fVPlry;

    fVPulx = rdp_reg.vp[4] - rdp_reg.vp[0];
    fVPuly = rdp_reg.vp[5] - rdp_reg.vp[1];
//  fVPuly = 240.0f - fVPuly;
    fVPuly = imgHeight - fVPuly;

    fVPlrx = rdp_reg.vp[4] + rdp_reg.vp[0];
    fVPlry = rdp_reg.vp[5] + rdp_reg.vp[1];
//  fVPlry = 240.0f - fVPlry;
    fVPlry = imgHeight - fVPlry;
 
    VPwidth = (int)(rdp_reg.vp[0] * 2.0f * fScalex);
    VPheight = (int)(rdp_reg.vp[1] * 2.0f * fScaley);

    fVPulx *= fScalex;
    fVPuly *= fScaley;

    fVPlrx *= fScalex;
    fVPlry *= fScaley;
    
    VPulx = (int)fVPulx ;
    VPuly = (int)(fVPuly)-VPheight;

    VPlrx = (int)fVPlrx;
    VPlry = (int)fVPuly;
    
    glViewport(VPulx, VPuly, VPwidth, VPheight);
}

void Render_lookat_x()
{
}

void Render_lookat_y()
{
}


void Render_light(int n)
{
/*                rdp_reg.light[i].r;
                rdp_reg.light[i].g;
                rdp_reg.light[i].b;
                rdp_reg.light[i].a;
                rdp_reg.light[i].r_copy;
                rdp_reg.light[i].g_copy;
                rdp_reg.light[i].b_copy;
                rdp_reg.light[i].a_copy;
                rdp_reg.light[i].x;
                rdp_reg.light[i].y;
                rdp_reg.light[i].z;
                rdp_reg.light[i].w;

*/  
    refresh_lights = TRUE;
}

void Render_line3d(int vn[])
{
        int     i;
        float   v[4];

        glBegin(GL_LINES);

        for(i=0; i<2; i++)
        {
                v[0] = rdp_reg.vtx[vn[i]].x;   /* * perspective_normal; */
                v[1] = rdp_reg.vtx[vn[i]].y;   /* * perspective_normal; */
                v[2] = rdp_reg.vtx[vn[i]].z;   /* * perspective_normal; */
                v[3] = 1.0;   /* * perspective_normal; */

                glColor4ub(0x00, 0x99, 0x00, 0xff);
                glVertex3f(v[0],
                        v[1],
                        v[2]
                        );
        }
        glEnd();
}

void Render_geometry_zbuffer(int on)
{
        if(on)
                glEnable(GL_DEPTH_TEST);
        else
                glDisable(GL_DEPTH_TEST);
}

void Render_geometry_zwrite(int on)
{
        if(on)
                glDepthMask(GL_TRUE);
        else
                glDepthMask(GL_FALSE);
}

void Render_geometry_cullfront(int on)
{
        if(on)
        {
                glCullFace(GL_FRONT);
                glEnable(GL_CULL_FACE);
        }
        else
        {
                glDisable(GL_CULL_FACE);
        }
              //  glDisable(GL_CULL_FACE);
}

void Render_geometry_cullback(int on)
{
        if(on)
        {
                glCullFace(GL_BACK);
                glEnable(GL_CULL_FACE);
        }
        else
        {
                glDisable(GL_CULL_FACE);
        }
           //     glDisable(GL_CULL_FACE);
}

void Render_geometry_cullfrontback(int on)
{
        if(on)
        {
                glCullFace(GL_FRONT_AND_BACK);
                glEnable(GL_CULL_FACE);
        }
        else
        {
                glDisable(GL_CULL_FACE);
        }
         //       glDisable(GL_CULL_FACE);
}

extern _u8 checkup;
extern _u32 fPasses,fT1;
extern _u32 fS1c,fS2c;
extern _u32 fS1a,fS2a;

void BuildCombine(_u8 tcycmod);

//extern void MathSize(int tile_num);
void Render_TexRectangle(float xh, float yh, 
                         float xl, float yl,
                         int tile_num,
                         float s, float t, 
                         float dsdx, float dtdy)
{
    t_tile *tmpTile = &rdp_reg.td[tile_num];
    t_tile *tmpTile1 = &rdp_reg.td[tile_num+1];
    t_tile *savCurTile = rdp_reg.m_CurTile;

    float s0, t0, s1, t1;
    float ft0s0, ft0t0, ft0s1, ft0t1;

    float s10, t10, s11, t11;
    float ft1s0, ft1t0, ft1s1, ft1t1;

    int sHeight,sWidth;

    float color[4],colorIv[4];


    float txl = (float)xl;
    float tyl = (float)yl;
    float txh = (float)xh;
    float tyh = (float)yh;
    
    float tw = (float)(tmpTile->Width);
    float th = (float)(tmpTile->Height);

    float t1w = (float)(tmpTile1->Width);
    float t1h = (float)(tmpTile1->Height);

    int tex1 = 1;
    int mul1 = 0;

    int tex2 = 1;
    int mul2 = 0;

    int passes = 0;

    int twidth = rdp_reg.TextureImage.width;
    BOOL undefmode = FALSE;

    GLint hadDepthTest, hadBlending, hadAlphaTest;

    _u32 oldTile = rdp_reg.tile;

    if ((dsdx < 0.1) && (dsdx > -0.1)) dsdx = 1;
    if ((dtdy < 0.1) && (dtdy > -0.1)) dtdy = 1;

    s1 = (float)fabs((double)(xl - xh)) +1;
    t1 = (float)fabs((double)(yl - yh)) +1;

//  return;

    s1 *= dsdx;
    t1 *= dtdy;
    s11 = s1;
    t11 = t1;

    {
        _u32 tmpS = tmpTile->uls;
        _u32 tmpT = tmpTile->ult;

        s0 = s - tmpS;
        t0 = t - tmpT;
    }

    {
        _u32 tmpS = tmpTile1->uls;
        _u32 tmpT = tmpTile1->ult;

        s10 = s - tmpS;
        t10 = t - tmpT;
    }

    s1 += s0;
    t1 += t0;

    s11 += s10;
    t11 += t10;

    if(tmpTile->shifts<11)
        tw *= 1<<tmpTile->shifts;
    else
        tw /= 1<<(16-tmpTile->shifts);

    if(tmpTile->shiftt<11)
        th *= 1<<tmpTile->shiftt;
    else
        th /= 1<<(16-tmpTile->shiftt);

    if(tmpTile1->shifts<11)
        t1w *= 1<<tmpTile1->shifts;
    else
        t1w /= 1<<(16-tmpTile1->shifts);

    if(tmpTile1->shiftt<11)
        t1h *= 1<<tmpTile1->shiftt;
    else
        t1h /= 1<<(16-tmpTile1->shiftt);

    if (tw < 1) tw++;
    if (th < 1) th++;

    if (t1w < 1) t1w++;
    if (t1h < 1) t1h++;

    if (tmpTile->mirrort)
        th *= 2;

    if (tmpTile->mirrors)
        tw *= 2;

    if (tmpTile1->mirrort)
        t1h *= 2;

    if (tmpTile1->mirrors)
        t1w *= 2;

    ft0s0 = (float)s0 / tw;
    ft0t0 = (float)t0 / th;
    ft0s1 = (float)s1 / tw;
    ft0t1 = (float)t1 / th;

    {
        if (ft0s0 < ft0s1)
        {
            ft0s0 += 0.15f / tw;
            ft0s1 -= 0.15f / tw;
        }
        else
        {
            ft0s0 -= 0.15f / tw;
            ft0s1 += 0.15f / tw;
        }

        if (ft0t0 < ft0t1)
        {
            ft0t0 += 0.15f / th;
            ft0t1 -= 0.15f / th;
        }
        else
        {
            ft0t0 -= 0.15f / th;
            ft0t1 += 0.15f / th;
        }
    }

    ft1s0 = (float)s10 / t1w;
    ft1t0 = (float)t10 / t1h;
    ft1s1 = (float)s11 / t1w;
    ft1t1 = (float)t11 / t1h;

    {
        if (ft1s0 < ft1s1)
        {
            ft1s0 += 0.15f / t1w;
            ft1s1 -= 0.15f / t1w;
        }
        else
        {
            ft1s0 -= 0.15f / t1w;
            ft1s1 += 0.15f / t1w;
        }

        if (ft1t0 < ft1t1)
        {
            ft1t0 += 0.15f / t1h;
            ft1t1 -= 0.15f / t1h;
        }
        else
        {
            ft1t0 -= 0.15f / t1h;
            ft1t1 += 0.15f / t1h;
        }

    }

    if (txh > txl)
    {
        txh += 0.700f;
        txl -= 0.700f;
    }
    else
    {
        txh -= 0.700f;
        txl += 0.700f;
    }

    if (tyh > tyl)
    {
        tyh += 0.700f;
        tyl -= 0.700f;
    }
    else
    {
        tyh -= 0.700f;
        tyl += 0.700f;
    }

    tex1 = 1;
    mul1 = 0;
    TexMode = 0;
    TexColor = 0;

    if (cycle_mode > 1)
    {
        tex1 = 0;
        mul1 = UseT0 | UseT0Alpha;
        passes = 0;
    }
    else
    {
        fT1 = 0;
        fS1c = 0;
        fS2c = 0;
        fS1a = 0;
        fS2a = 0;

        BuildCombine(cycle_mode);
        if (checkup > 0) undefmode = TRUE;
        tex1 = 0;
        tex2 = 0;

        passes = fT1 >> 4;
        mul1 = fS1c | fS1a;
        mul2 = fS2c | fS2a;

        mul1 &= MaskShade;
        mul2 &= MaskShade;

        if ((mul1 & UseZeroAlpha) == UseZeroAlpha) return; // hack!!
    }

    sWidth = rdp_reg.colorimg_width;
    sHeight = (sWidth * 3) >> 2;

    glGetIntegerv(GL_DEPTH_TEST, &hadDepthTest); //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_BLEND, &hadBlending); //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_ALPHA_TEST, &hadAlphaTest); //** Should be glGetBooleanv, but 

    glPushAttrib(GL_ENABLE_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
//  glDisable(GL_ALPHA_TEST);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    Src_Alpha = GL_SRC_ALPHA;
    Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;

/*
    switch (rdp_reg.mode_l & 0x03)
    {
    case 0x00:
        glDisable(GL_ALPHA_TEST);
        break;
    case 0x01:
        glEnable(GL_ALPHA_TEST);
        break;
    case 0x02:
        glEnable(GL_ALPHA_TEST);
        break;
    case 0x03:
        glEnable(GL_ALPHA_TEST);
        break;
    }
    //glEnable(GL_BLEND);
*/
/*
    switch (rdp_reg.mode_l & 0xFFFF0000)
    {
      // Road, zelda
      // clr_fog * a_step + clr_in * 1ma
      // clr_in * a_in + clr_mem * 1ma
    case 0xc8100000:
      break;
      
      // Mace objects
    case 0x03820000:
      // Mario kart player select
      // clr_in * 0 + clr_in * 1
      //  - or just clr_in, no matter what alpha
    case 0x0f0a0000:
        Src_Alpha = GL_ONE;
        Dst_Alpha = GL_ZERO;
        glDisable(GL_ALPHA_TEST);
        break;
      
    default:
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
    }
/** /
    if (
        (rdp_reg.mode_l & 0x2000) 
              && !(rdp_reg.mode_l & 0x1000)
        )
    {
        //A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);
        Src_Alpha = GL_ONE;
        Dst_Alpha = GL_ZERO;
        glDisable(GL_ALPHA_TEST);
    }
/**/

    glBlendFunc(Src_Alpha, Dst_Alpha);
    glAlphaFunc(GL_GEQUAL,AlphaLevel);
    glEnable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
//  glOrtho(0, sWidth, sHeight, 0, -1, 1);
    glOrtho(0, sWidth, sHeight, 0, -0, 1023);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //if ((tex1 == 1) || (mul1 & UseTex0Mask) || (mul1 & UseTex1Mask))
    if ((mul1 & UseTex0Mask) || (mul1 & UseTex1Mask))
    {
        _u32 sTM = TexMode;
        
        TexMode = 0;

        if (mul1 & UseT0)
            TexMode |= Tex0;
        if (mul1 & UseT1)
            TexMode |= Tex1;

        if (mul1 & UseT0Iv)
            TexMode |= InvT0;
        if (mul1 & UseT1Iv)
            TexMode |= InvT1;

        if (mul1 & UseT0Alpha)
            TexMode |= AlphaT0;
        if (mul1 & UseT1Alpha)
            TexMode |= AlphaT1;

        if (mul1 & UseT0AlphaIv)
            TexMode |= InvT0Alpha;
        if (mul1 & UseT1AlphaIv)
            TexMode |= InvT1Alpha;

        if (mul1 & UseMT0Alpha)
            TexMode |= T0MultAlpha;
        if (mul1 & UseMT1Alpha)
            TexMode |= T1MultAlpha;

        if (mul1 & UseMT0AlphaIv) 
            TexMode |= InvT0MultAlpha;
        if (mul1 & UseMT1AlphaIv) 
            TexMode |= InvT1MultAlpha;

        rdp_reg.tile = tile_num;

        SetActiveTexture();

        rdp_reg.tile = oldTile;

        glEnable(GL_TEXTURE_2D);
        TexMode = sTM;
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;

        colorIv[0] = 1.0f;
        colorIv[1] = 1.0f;
        colorIv[2] = 1.0f;
        colorIv[3] = 1.0f;

        if ((mul1 & UsePrim) != 0)
        {
            color[0] *= rdp_reg.prim_r;
            color[1] *= rdp_reg.prim_g;
            color[2] *= rdp_reg.prim_b;
        }

        if ((mul1 & UseEnv) != 0)
        {
            color[0] *= rdp_reg.env_r;
            color[1] *= rdp_reg.env_g;
            color[2] *= rdp_reg.env_b;
        }

        if ((mul1 & UsePrimIv) != 0)
        {
            colorIv[0] *= rdp_reg.prim_r;
            colorIv[1] *= rdp_reg.prim_g;
            colorIv[2] *= rdp_reg.prim_b;
        }

        if ((mul1 & UseEnvIv) != 0)
        {
            colorIv[0] *= rdp_reg.env_r;
            colorIv[1] *= rdp_reg.env_g;
            colorIv[2] *= rdp_reg.env_b;
        }

        if ((mul1 & UsePrimAlpha) != 0)
        {
            color[3] *= rdp_reg.prim_a;
        }

        if ((mul1 & UseEnvAlpha) != 0)
        {
            color[3] *= rdp_reg.env_a;
        }

        if ((mul1 & UsePrimAlphaIv) != 0)
        {
            colorIv[3] *= rdp_reg.prim_a;
        }

        if ((mul1 & UseEnvAlphaIv) != 0)
        {
            colorIv[3] *= rdp_reg.env_a;
        }

        if ((mul1 & UseMPrimAlpha) != 0)
        {
            color[0] *= rdp_reg.prim_a;
            color[1] *= rdp_reg.prim_a;
            color[2] *= rdp_reg.prim_a;
        }

        if ((mul1 & UseMEnvAlpha) != 0)
        {
            color[0] *= rdp_reg.env_a;
            color[1] *= rdp_reg.env_a;
            color[2] *= rdp_reg.env_a;
        }

        if ((mul1 & UseMPrimAlphaIv) != 0)
        {
            colorIv[0] *= rdp_reg.prim_a;
            colorIv[1] *= rdp_reg.prim_a;
            colorIv[2] *= rdp_reg.prim_a;
        }

        if ((mul1 & UseMEnvAlphaIv) != 0)
        {
            colorIv[0] *= rdp_reg.env_a;
            colorIv[1] *= rdp_reg.env_a;
            colorIv[2] *= rdp_reg.env_a;
        }

//#define UseAlphaIv            (UseEnvAlphaIv | UsePrimAlphaIv | UseShadeAlphaIv)
//#define UseColorIv            (UseEnvIv | UsePrimIv | UseShadeIv | UseMPrimAlphaIv | UseMEnvAlphaIv)
        if (mul1 & UseColorIv)
        {
            color[0] *= 1.0f - colorIv[0];
            color[1] *= 1.0f - colorIv[1];
            color[2] *= 1.0f - colorIv[2];
        }

        if (mul1 & UseAlphaIv)
        {
            color[3] *= 1.0f - colorIv[3];
        }

        glColor4fv(color);

    glBegin(GL_QUADS);
        if ((tex1 == 1) || (mul1 & UseTex0Mask) || (mul1 & UseTex1Mask))
        {
            float ts1,ts0,tt1,tt0;

            if (aTilew)
            {
                ts1=ft1s1;
                ts0=ft1s0;
            }
            else
            {
                ts1=ft0s1;
                ts0=ft0s0;
            }

            if (aTileh)
            {
                tt1=ft1t1;
                tt0=ft1t0;
            }
            else
            {
                tt1=ft0t1;
                tt0=ft0t0;
            }

            ts0 /= Multw;
            tt0 /= Multh;
            ts1 /= Multw;
            tt1 /= Multh;

            glTexCoord2f(ts1, tt1);
            glVertex2f(txl, tyl);
        
            glTexCoord2f(ts0, tt1);
            glVertex2f(txh, tyl);
        
            glTexCoord2f(ts0, tt0);
            glVertex2f(txh, tyh);
        
            glTexCoord2f(ts1, tt0);
            glVertex2f(txl, tyh);
        }
        else
        {
            glVertex2f(txl, tyl);
            glVertex2f(txh, tyl);
            glVertex2f(txh, tyh);
            glVertex2f(txl, tyh);
        }
    glEnd();

    if (passes > 1)
    {

//  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendFunc(Src_Alpha, GL_ONE);

    if ((tex2 == 1) || (mul2 & UseTex0Mask) || (mul2 & UseTex1Mask))
    {
        _u32 sTM = TexMode;

        TexMode = 0;

        if (mul2 & UseT0)
            TexMode |= Tex0;
        if (mul2 & UseT1)
            TexMode |= Tex1;

        if (mul2 & UseT0Iv)
            TexMode |= InvT0;
        if (mul2 & UseT1Iv)
            TexMode |= InvT1;

        if (mul2 & UseT0Alpha)
            TexMode |= AlphaT0;
        if (mul2 & UseT1Alpha)
            TexMode |= AlphaT1;

        if (mul2 & UseT0AlphaIv)
            TexMode |= InvT0Alpha;
        if (mul2 & UseT1AlphaIv)
            TexMode |= InvT1Alpha;

        if (mul2 & UseMT0Alpha)
            TexMode |= T0MultAlpha;
        if (mul2 & UseMT1Alpha)
            TexMode |= T1MultAlpha;

        if (mul2 & UseMT0AlphaIv) 
            TexMode |= InvT0MultAlpha;
        if (mul2 & UseMT1AlphaIv) 
            TexMode |= InvT1MultAlpha;

        rdp_reg.tile = tile_num;

        SetActiveTexture();

        rdp_reg.tile = oldTile;

        glEnable(GL_TEXTURE_2D);
        TexMode = sTM;
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;

        colorIv[0] = 1.0f;
        colorIv[1] = 1.0f;
        colorIv[2] = 1.0f;
        colorIv[3] = 1.0f;

        if ((mul2 & UsePrim) != 0)
        {
            color[0] *= rdp_reg.prim_r;
            color[1] *= rdp_reg.prim_g;
            color[2] *= rdp_reg.prim_b;
        }

        if ((mul2 & UseEnv) != 0)
        {
            color[0] *= rdp_reg.env_r;
            color[1] *= rdp_reg.env_g;
            color[2] *= rdp_reg.env_b;
        }

        if ((mul2 & UsePrimIv) != 0)
        {
            colorIv[0] *= rdp_reg.prim_r;
            colorIv[1] *= rdp_reg.prim_g;
            colorIv[2] *= rdp_reg.prim_b;
        }

        if ((mul2 & UseEnvIv) != 0)
        {
            colorIv[0] *= rdp_reg.env_r;
            colorIv[1] *= rdp_reg.env_g;
            colorIv[2] *= rdp_reg.env_b;
        }

        if ((mul2 & UsePrimAlpha) != 0)
        {
            color[3] *= rdp_reg.prim_a;
        }

        if ((mul2 & UseEnvAlpha) != 0)
        {
            color[3] *= rdp_reg.env_a;
        }

        if ((mul2 & UsePrimAlphaIv) != 0)
        {
            color[3] *= rdp_reg.prim_1ma;
        }

        if ((mul2 & UseEnvAlphaIv) != 0)
        {
            color[3] *= rdp_reg.env_1ma;
        }

        if ((mul2 & UseMPrimAlpha) != 0)
        {
            color[0] *= rdp_reg.prim_a;
            color[1] *= rdp_reg.prim_a;
            color[2] *= rdp_reg.prim_a;
        }

        if ((mul2 & UseMEnvAlpha) != 0)
        {
            color[0] *= rdp_reg.env_a;
            color[1] *= rdp_reg.env_a;
            color[2] *= rdp_reg.env_a;
        }

        if ((mul2 & UseMPrimAlphaIv) != 0)
        {
            colorIv[0] *= rdp_reg.prim_a;
            colorIv[1] *= rdp_reg.prim_a;
            colorIv[2] *= rdp_reg.prim_a;
        }

        if ((mul2 & UseMEnvAlphaIv) != 0)
        {
            colorIv[0] *= rdp_reg.env_a;
            colorIv[1] *= rdp_reg.env_a;
            colorIv[2] *= rdp_reg.env_a;
        }

//#define UseAlphaIv            (UseEnvAlphaIv | UsePrimAlphaIv | UseShadeAlphaIv)
//#define UseColorIv            (UseEnvIv | UsePrimIv | UseShadeIv | UseMPrimAlphaIv | UseMEnvAlphaIv)
        if (mul2 & UseColorIv)
        {
            color[0] *= 1.0f - colorIv[0];
            color[1] *= 1.0f - colorIv[1];
            color[2] *= 1.0f - colorIv[2];
        }

        if (mul2 & UseAlphaIv)
        {
            color[3] *= 1.0f - colorIv[3];
        }

        glColor4fv(color);

    glBegin(GL_QUADS);
        if ((tex2 == 1) || (mul2 & UseTex0Mask) || (mul2 & UseTex1Mask))
        {
            float ts1,ts0,tt1,tt0;

            if (aTilew)
            {
                ts1=ft1s1;
                ts0=ft1s0;
            }
            else
            {
                ts1=ft0s1;
                ts0=ft0s0;
            }

            if (aTileh)
            {
                tt1=ft1t1;
                tt0=ft1t0;
            }
            else
            {
                tt1=ft0t1;
                tt0=ft0t0;
            }

            glTexCoord2f(ts1, tt1);
            glVertex2f(txl, tyl);
        
            glTexCoord2f(ts0, tt1);
            glVertex2f(txh, tyl);
        
            glTexCoord2f(ts0, tt0);
            glVertex2f(txh, tyh);
        
            glTexCoord2f(ts1, tt0);
            glVertex2f(txl, tyh);

/*          glTexCoord2f(ft0s1, ft0t1);
            glVertex2f(txl, tyl);
        
            glTexCoord2f(ft0s0, ft0t1);
            glVertex2f(txh, tyl);
        
            glTexCoord2f(ft0s0, ft0t0);
            glVertex2f(txh, tyh);
        
            glTexCoord2f(ft0s1, ft0t0);
            glVertex2f(txl, tyh);
*/      }
        else
        {
            glVertex2f(txl, tyl);
            glVertex2f(txh, tyl);
            glVertex2f(txh, tyh);
            glVertex2f(txl, tyh);
        }
    glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(Src_Alpha, Dst_Alpha);
    glAlphaFunc(GL_GEQUAL,AlphaLevel);

    if (hadBlending) glEnable(GL_BLEND);
    if (hadDepthTest) glEnable(GL_DEPTH_TEST);
    if (hadAlphaTest) glEnable(GL_ALPHA_TEST);

    rdp_reg.m_CurTile = savCurTile;
//  glCallList(kListEnd2D);

}

void Render_TexRectangleFastFB(float xh, float yh, 
                         float xl, float yl,
                         int tile_num,
                         float s, float t, 
                         float dsdx, float dtdy)
{
    float s0, t0, s1, t1;
    float ft0s0, ft0t0, ft0s1, ft0t1;

    int sHeight,sWidth;

    float color[4],colorIv[4];

    float txl = (float)xl;
    float tyl = (float)yl;
    float txh = (float)xh;
    float tyh = (float)yh;
    
    float tw = rdp_reg.framebuffer_width; // 512
    float th = tw * .75f; // 256;

    int tex1 = 1;
    int mul1 = 0;

    int tex2 = 1;
    int mul2 = 0;

    int passes = 0;

    int twidth = rdp_reg.framebuffer_width;
    int theight = (int)((float)rdp_reg.framebuffer_width * 0.75) - 1;
    int toffset = theight - ((rdp_reg.TextureImage.addr - rdp_reg.framebuffer_addr) / twidth) / 2;
    //int soffset = theight - (rdp_reg.TextureImage.addr - rdp_reg.framebuffer_addr)/ 2;

    BOOL undefmode = FALSE;

    GLint hadDepthTest, hadBlending, hadAlphaTest;

    if ((dsdx < 0.1) && (dsdx > -0.1)) dsdx = 1;
    if ((dtdy < 0.1) && (dtdy > -0.1)) dtdy = 1;

    s1 = (float)fabs((double)(xl - xh)) +1;
    t1 = (float)fabs((double)(yl - yh)) +1;

//  return;
    s0 = s;
    t0 = t + toffset;

    s1 *= dsdx;
    t1 *= dtdy;

    s1 += s0;
    t1 += t0;

//  tw = 512;
//  th = 256;

    ft0s0 = (float)s0 / tw;
    ft0t0 = (float)t0 / th;
    ft0s1 = (float)s1 / tw;
    ft0t1 = (float)t1 / th;

    {
        if (ft0s0 < ft0s1)
        {
            ft0s0 += 0.15f / tw;
            ft0s1 -= 0.15f / tw;
        }
        else
        {
            ft0s0 -= 0.15f / tw;
            ft0s1 += 0.15f / tw;
        }

        if (ft0t0 < ft0t1)
        {
            ft0t0 += 0.15f / th;
            ft0t1 -= 0.15f / th;
        }
        else
        {
            ft0t0 -= 0.15f / th;
            ft0t1 += 0.15f / th;
        }
    }

    if (txh > txl)
    {
        txh += 0.500;
        txl -= 0.500;
    }
    else
    {
        txh -= 0.500;
        txl += 0.500;
    }

    if (tyh > tyl)
    {
        tyh += 0.500;
        tyl -= 0.500;
    }
    else
    {
        tyh -= 0.500;
        tyl += 0.500;
    }
/*
    txl *= fScalex;
    tyl *= fScaley;
    txh *= fScalex;
    tyh *= fScaley;
*/
    tex1 = 1;
    mul1 = 0;
    TexMode = 0;
    TexColor = 0;

    if (cycle_mode > 1)
    {
        tex1 = 0;
        mul1 = UseT0 | UseT0Alpha;
        passes = 0;
    }
    else
    {
        fT1 = 0;
        fS1c = 0;
        fS2c = 0;
        fS1a = 0;
        fS2a = 0;

        BuildCombine(cycle_mode);
        if (checkup > 0) undefmode = TRUE;
        tex1 = 0;
        tex2 = 0;

        passes = fT1 >> 4;
        mul1 = fS1c | fS1a;
        mul2 = fS2c | fS2a;

        mul1 &= MaskShade;
        mul2 &= MaskShade;

        if ((mul1 & UseZeroAlpha) == UseZeroAlpha) return; // hack!!
    }

    sWidth = rdp_reg.colorimg_width;
    sHeight = (sWidth * 3) >> 2;

    glGetIntegerv(GL_DEPTH_TEST, &hadDepthTest);    //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_BLEND, &hadBlending);          //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_ALPHA_TEST, &hadAlphaTest);    //** Should be glGetBooleanv, but 

    glPushAttrib(GL_ENABLE_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_ALPHA_TEST);
    Src_Alpha = GL_SRC_ALPHA;
    Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;

    glBlendFunc(Src_Alpha, Dst_Alpha);
    glAlphaFunc(GL_GEQUAL,AlphaLevel);
    glEnable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    SetOrtho();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glViewport(VPulx, VPuly, VPwidth, VPheight);

    if ((mul1 & UseTex0Mask) || (mul1 & UseTex1Mask))
    {
        glBindTexture(GL_TEXTURE_2D, 1);
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;

    colorIv[0] = 1.0f;
    colorIv[1] = 1.0f;
    colorIv[2] = 1.0f;
    colorIv[3] = 1.0f;

    if ((mul1 & UsePrim) != 0)
    {
        color[0] *= rdp_reg.prim_r;
        color[1] *= rdp_reg.prim_g;
        color[2] *= rdp_reg.prim_b;
    }

    if ((mul1 & UseEnv) != 0)
    {
        color[0] *= rdp_reg.env_r;
        color[1] *= rdp_reg.env_g;
        color[2] *= rdp_reg.env_b;
    }

    if ((mul1 & UsePrimIv) != 0)
    {
        colorIv[0] *= rdp_reg.prim_r;
        colorIv[1] *= rdp_reg.prim_g;
        colorIv[2] *= rdp_reg.prim_b;
    }

    if ((mul1 & UseEnvIv) != 0)
    {
        colorIv[0] *= rdp_reg.env_r;
        colorIv[1] *= rdp_reg.env_g;
        colorIv[2] *= rdp_reg.env_b;
    }

    if ((mul1 & UsePrimAlpha) != 0)
    {
        color[3] *= rdp_reg.prim_a;
    }

    if ((mul1 & UseEnvAlpha) != 0)
    {
        color[3] *= rdp_reg.env_a;
    }

    if ((mul1 & UsePrimAlphaIv) != 0)
    {
        colorIv[3] *= rdp_reg.prim_a;
    }

    if ((mul1 & UseEnvAlphaIv) != 0)
    {
        colorIv[3] *= rdp_reg.env_a;
    }

    if ((mul1 & UseMPrimAlpha) != 0)
    {
        color[0] *= rdp_reg.prim_a;
        color[1] *= rdp_reg.prim_a;
        color[2] *= rdp_reg.prim_a;
    }

    if ((mul1 & UseMEnvAlpha) != 0)
    {
        color[0] *= rdp_reg.env_a;
        color[1] *= rdp_reg.env_a;
        color[2] *= rdp_reg.env_a;
    }

    if ((mul1 & UseMPrimAlphaIv) != 0)
    {
        colorIv[0] *= rdp_reg.prim_a;
        colorIv[1] *= rdp_reg.prim_a;
        colorIv[2] *= rdp_reg.prim_a;
    }

    if ((mul1 & UseMEnvAlphaIv) != 0)
    {
        colorIv[0] *= rdp_reg.env_a;
        colorIv[1] *= rdp_reg.env_a;
        colorIv[2] *= rdp_reg.env_a;
    }

    if (mul1 & UseColorIv)
    {
        color[0] *= 1.0f - colorIv[0];
        color[1] *= 1.0f - colorIv[1];
        color[2] *= 1.0f - colorIv[2];
    }

    if (mul1 & UseAlphaIv)
    {
        color[3] *= 1.0f - colorIv[3];
    }

    glColor4fv(color);

    glBegin(GL_QUADS);
    
    if ((tex1 == 1) || (mul1 & UseTex0Mask) || (mul1 & UseTex1Mask))
    {
        float ts1,ts0,tt1,tt0;

        ts1=ft0s1;
        ts0=ft0s0;

        tt1=ft0t1;
        tt0=ft0t0;

        glTexCoord2f(ts1, tt1);
        glVertex2f(txl, tyl);
        
        glTexCoord2f(ts0, tt1);
        glVertex2f(txh, tyl);
        
        glTexCoord2f(ts0, tt0);
        glVertex2f(txh, tyh);
        
        glTexCoord2f(ts1, tt0);
        glVertex2f(txl, tyh);
    }
    else
    {
        glVertex2f(txl, tyl);
        glVertex2f(txh, tyl);
        glVertex2f(txh, tyh);
        glVertex2f(txl, tyh);
    }
    glEnd();

    if (passes > 1)
    {

    glBlendFunc(Src_Alpha, GL_ONE);

    if ((tex2 == 1) || (mul2 & UseTex0Mask) || (mul2 & UseTex1Mask))
    {
        glBindTexture(GL_TEXTURE_2D, 1);

        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;

    colorIv[0] = 1.0f;
    colorIv[1] = 1.0f;
    colorIv[2] = 1.0f;
    colorIv[3] = 1.0f;

    if ((mul2 & UsePrim) != 0)
    {
        color[0] *= rdp_reg.prim_r;
        color[1] *= rdp_reg.prim_g;
        color[2] *= rdp_reg.prim_b;
    }

    if ((mul2 & UseEnv) != 0)
    {
        color[0] *= rdp_reg.env_r;
        color[1] *= rdp_reg.env_g;
        color[2] *= rdp_reg.env_b;
    }

    if ((mul2 & UsePrimIv) != 0)
    {
        colorIv[0] *= rdp_reg.prim_r;
        colorIv[1] *= rdp_reg.prim_g;
        colorIv[2] *= rdp_reg.prim_b;
    }

    if ((mul2 & UseEnvIv) != 0)
    {
        colorIv[0] *= rdp_reg.env_r;
        colorIv[1] *= rdp_reg.env_g;
        colorIv[2] *= rdp_reg.env_b;
    }

    if ((mul2 & UsePrimAlpha) != 0)
    {
        color[3] *= rdp_reg.prim_a;
    }

    if ((mul2 & UseEnvAlpha) != 0)
    {
        color[3] *= rdp_reg.env_a;
    }

    if ((mul2 & UsePrimAlphaIv) != 0)
    {
        color[3] *= rdp_reg.prim_1ma;
    }

    if ((mul2 & UseEnvAlphaIv) != 0)
    {
        color[3] *= rdp_reg.env_1ma;
    }

    if ((mul2 & UseMPrimAlpha) != 0)
    {
        color[0] *= rdp_reg.prim_a;
        color[1] *= rdp_reg.prim_a;
        color[2] *= rdp_reg.prim_a;
    }

    if ((mul2 & UseMEnvAlpha) != 0)
    {
        color[0] *= rdp_reg.env_a;
        color[1] *= rdp_reg.env_a;
        color[2] *= rdp_reg.env_a;
    }

    if ((mul2 & UseMPrimAlphaIv) != 0)
    {
        colorIv[0] *= rdp_reg.prim_a;
        colorIv[1] *= rdp_reg.prim_a;
        colorIv[2] *= rdp_reg.prim_a;
    }

    if ((mul2 & UseMEnvAlphaIv) != 0)
    {
        colorIv[0] *= rdp_reg.env_a;
        colorIv[1] *= rdp_reg.env_a;
        colorIv[2] *= rdp_reg.env_a;
    }

    if (mul2 & UseColorIv)
    {
        color[0] *= 1.0f - colorIv[0];
        color[1] *= 1.0f - colorIv[1];
        color[2] *= 1.0f - colorIv[2];
    }

    if (mul2 & UseAlphaIv)
    {
        color[3] *= 1.0f - colorIv[3];
    }

    glColor4fv(color);

    if ((tex2 == 1) || (mul2 & UseTex0Mask) || (mul2 & UseTex1Mask))
        {
            float ts1,ts0,tt1,tt0;

            ts1=ft0s1;
            ts0=ft0s0;

            tt1=ft0t1;
            tt0=ft0t0;

            glBegin(GL_QUADS);

            glTexCoord2f(ts1, tt1);
            glVertex2f(txl, tyl);
        
            glTexCoord2f(ts0, tt1);
            glVertex2f(txh, tyl);
        
            glTexCoord2f(ts0, tt0);
            glVertex2f(txh, tyh);
        
            glTexCoord2f(ts1, tt0);
            glVertex2f(txl, tyh);

        }
        else
        {
            glVertex2f(txl, tyl);
            glVertex2f(txh, tyl);
            glVertex2f(txh, tyh);
            glVertex2f(txl, tyh);
        }
    glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    glBlendFunc(Src_Alpha, Dst_Alpha);
    glAlphaFunc(GL_GEQUAL,AlphaLevel);

    if (hadBlending) glEnable(GL_BLEND);
    if (hadDepthTest) glEnable(GL_DEPTH_TEST);
    if (hadAlphaTest) glEnable(GL_ALPHA_TEST);

//  glCallList(kListEnd2D);

}


void Render_Fog(BOOL on)
{
    if (on)
    {
        static GLint fogMode;
        float fo = rdp_reg.fog_fo;
        float fm = rdp_reg.fog_fm;
        float rng = 128000.0f / fm;
        float min = 500.0f - ((fo * rng) / 256.0f);
        //float rng = 131072.0f / fm;
        //float min = 512.0f - ((fo * rng) / 256.0f);
        float max = rng + min;
        GLfloat fogColor[4] = {rdp_reg.fog_r, rdp_reg.fog_g, rdp_reg.fog_b, 1.0f};//rdp_reg.fog_a};

        glEnable(GL_FOG);

        if (fo == 0.0f)
        {
            min = 990;
            max = 1000;
        }

        //fogMode = GL_LINEAR;//GL_EXP;
        fogMode = GL_EXP;
        glFogi (GL_FOG_MODE, fogMode);
        glFogfv (GL_FOG_COLOR, fogColor);
        glFogf (GL_FOG_DENSITY, 0.350f);
        glHint (GL_FOG_HINT, GL_DONT_CARE);
        glFogf (GL_FOG_START, min);
        glFogf (GL_FOG_END, max);
    }
    else
    {
        glFlush();
        glDisable(GL_FOG);
    }
}

void Render_FillRectangle(int xh, int yh, 
                         int xl, int yl,
                         _u32 fillclr)
{
    GLint hadDepthTest, hadBlending, hadAlphaTest;

    int sHeight,sWidth;
    float color[4];
    float txl = (float)xl / 4.0f;
    float tyl = (float)yl / 4.0f;
    float txh = (float)xh / 4.0f;
    float tyh = (float)yh / 4.0f;

//** Set to 2d ...
    if (rdp_reg.depthimg_addr == rdp_reg.colorimg_addr) 
    {
        glDisable(GL_SCISSOR_TEST);
        glDrawBuffer(GL_BACK);
        glDepthMask(GL_TRUE);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        return;
    }

    //return;

    sWidth = rdp_reg.colorimg_width;
    sHeight = (sWidth * 3) >> 2;

    glGetIntegerv(GL_DEPTH_TEST, &hadDepthTest); //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_BLEND, &hadBlending);       //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_ALPHA_TEST, &hadAlphaTest); //** Should be glGetBooleanv, but 

    glPushAttrib(GL_ENABLE_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
//  glOrtho(0, sWidth, sHeight, 0, -1, 1);
    glOrtho(0, sWidth, sHeight, 0, -0, 1023);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if(!(rdp_reg.mode_h & 0x00200000))
    {
            /* if we are not in 1 or 2 cycle mode (not in copy or fill mode) */
            /* we must not draw the bottom and right edges                   */
                //color = rdp_reg.blendcolor;
        color[0] = rdp_reg.prim_r; //((((fillclr >> 11 ) & 0x001f)<<3)) / 255.0f;
        color[1] = rdp_reg.prim_g; //((((fillclr >>  6 ) & 0x001f)<<3)) / 255.0f;
        color[2] = rdp_reg.prim_b; //((((fillclr >>  1 ) & 0x001f)<<3)) / 255.0f;
        color[3] = rdp_reg.prim_a; //((fillclr & 0x01) ? 0xff : 0x00) / 255.0f;
    }
    else
    {
        color[0] = rdp_reg.fill_r; //((((fillclr >> 11 ) & 0x001f)<<3)) / 255.0f;
        color[1] = rdp_reg.fill_g; //((((fillclr >>  6 ) & 0x001f)<<3)) / 255.0f;
        color[2] = rdp_reg.fill_b; //((((fillclr >>  1 ) & 0x001f)<<3)) / 255.0f;
        color[3] = rdp_reg.fill_a; //((fillclr & 0x01) ? 0xff : 0x00) / 255.0f;
    }


    glColor4fv(color); 

    glBegin(GL_QUADS);
        {
            glVertex2f(txl, tyl);
            glVertex2f(txh, tyl);
            glVertex2f(txh, tyh);
            glVertex2f(txl, tyh);
        }
    glEnd();

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glBlendFunc(Src_Alpha, Dst_Alpha);
    glAlphaFunc(GL_GEQUAL,AlphaLevel);
    if (hadBlending) glEnable(GL_BLEND);
    if (hadDepthTest) glEnable(GL_DEPTH_TEST);
    if (hadAlphaTest) glEnable(GL_ALPHA_TEST);

//  glCallList(kListEnd2D);

}

void Render_tri1(int vn[], _u8 flag)
{
/*  int i;

    int width  = rdp_reg.td[rdp_reg.loadtile].lrs - rdp_reg.td[rdp_reg.loadtile].uls + 1; 
    int height = rdp_reg.td[rdp_reg.loadtile].lrt - rdp_reg.td[rdp_reg.loadtile].ult + 1; 
    width  >>= rdp_reg.td[rdp_reg.loadtile].shifts;
    height >>= rdp_reg.td[rdp_reg.loadtile].shiftt;


    if (rdp_reg.texture[rdp_reg.loadtile].scale_s == 0) rdp_reg.texture[rdp_reg.loadtile].scale_s = 1;
    if (rdp_reg.texture[rdp_reg.loadtile].scale_t == 0) rdp_reg.texture[rdp_reg.loadtile].scale_t = 1;

    glColor3f(1.0f, 1.0f, 1.0f);

    Load_ActiveTexture(rdp_reg.loadtile);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    for(i=0; i<3; i++)
    {
        t_vtx *vertex = &rdp_reg.vtx[vn[i]];
        float s = vertex->s * rdp_reg.texture[rdp_reg.loadtile].scale_s / (float)width;
        float t = vertex->t * rdp_reg.texture[rdp_reg.loadtile].scale_t / (float)height;
        glTexCoord2f(s, t);

        glVertex3f((float)vertex->x,
                   (float)vertex->y,
                   (float)vertex->z);
    }
    glEnd();*/
}

void Render_tri2(int vn[], _u8 flag)
{
    Render_tri1(vn, flag);
    Render_tri1(vn+3, flag);
}

void Render_DrawVisualRectangle(int ulx, int uly, int lrx, int lry)
{
        glCallList(rdp_reg.colorimg_width);
//            glBegin(GL_LINE_STRIP);
              glBegin(GL_TRIANGLE_STRIP);
                glColor3ub(0x3f, 0x3f, 0xcf);
                glVertex2f((float)ulx, (float)uly);
                glVertex2f((float)lrx, (float)uly);
                glVertex2f((float)lrx, (float)lry);
                glVertex2f((float)ulx, (float)lry);
                glVertex2f((float)ulx, (float)uly);
            glEnd();
        glCallList(kListEnd2D);

} /* void DrawVisualRectangle(int ulx, int uly, int lrx, int lry) */







void Render_triangle(int vn[])
{
        int     i;
        float   v[3];

        glDisable(GL_TEXTURE_2D);

        glBegin(GL_TRIANGLES);

        for(i=0; i<3; i++)
        {
                v[0] = rdp_reg.vtx[vn[i]].x;   // perspective_normal;
                v[1] = rdp_reg.vtx[vn[i]].y;   // perspective_normal;
                v[2] = rdp_reg.vtx[vn[i]].z;   // perspective_normal;

                glColor4ub( rdp_reg.vtx[vn[i]].r,
                            rdp_reg.vtx[vn[i]].g,
                            rdp_reg.vtx[vn[i]].b, 0xff);
//                glColor4ub( 255,255,255,255);

                glVertex3f( v[0],
                            v[1],
                            v[2]);
        }
        glEnd();
}

