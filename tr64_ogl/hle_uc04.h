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

#include <gl/gl.h>
//#include <glext.h>
#include "3dmath.h"

void rsp_uc04_vertex();
void rsp_uc04_modifyvertex();
void rsp_uc04_culldl();
void rsp_uc04_branchz();
void rsp_uc04_tri1();
void rsp_uc04_tri2();
void rsp_uc04_quad();
void rsp_uc04_fastcombine();
void rsp_uc04_rdphalf_cont();
void rsp_uc04_rdphalf_2();
void rsp_uc04_line3d();
void rsp_uc04_texture();
void rsp_uc04_popmatrix();
void rsp_uc04_setgeometrymode();
void rsp_uc04_matrix();
void rsp_uc04_moveword();
void rsp_uc04_movemem();
void rsp_uc04_displaylist();
void rsp_uc04_enddl();
void rsp_uc04_rdphalf_1();
void rsp_uc04_setothermode_l();
void rsp_uc04_setothermode_h();

void rsp_uc04_special_1()
{
    DebugBox("special_1");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_SPECIAL_1\n",
                 ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_special_2()
{
    DebugBox("special_2");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_SPECIAL_2\n",
                 ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_special_3()
{
    DebugBox("special_3");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_SPECIAL_3\n",
                 ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_dma_io()
{
    DebugBox("dma_io");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_DMA_IO\n",
                 ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_load_ucode()
{
    DebugBox("load_ucode");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_LOAD_UCODE\n",
                 ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_noop()
{
    DebugBox("noop");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_NOOP\n",
                 ADDR, CMD0, CMD1);
#endif
}

extern float fScalex;
extern float fScaley;

//////////////////////////////////////////////////////////////////////////////
// structure is tricky for changing the endians                             //
//////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    _s16 y;
    _s16 x;
    _u16 flags;
    _s16 z;

    _s16 t;
    _s16 s;

    _u8  a;
    _u8  b;
    _u8  g;
    _u8  r;
} t_vtx_uc4;

_u32 bzaddr;

//////////////////////////////////////////////////////////////////////////////
// LoadVertex                                                               //
//////////////////////////////////////////////////////////////////////////////
void rsp_uc04_vertex()
{
    _u32    a = segoffset2addr(rdp_reg.cmd1);
    int     v0, i, n;

    n = (rdp_reg.cmd0 >> 12) & 0xff;
    v0 = ((rdp_reg.cmd0 >> 1) & 0x7f) - n;

    if (refresh_matrix) update_cmbmatrix();

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_LOADVTX  vertex %i..%i\n",
                 ADDR, CMD0, CMD1,  v0, v0+n-1);
#endif

    for(i = 0; i < n; i++)
    {
//      t_vtx_uc4 *vtx = (t_vtx_uc4 *)&pRDRAM[a+(i*sizeof(t_vtx_uc4))];
        _u32 ad = a + (i * sizeof(t_vtx_uc4));

        rdp_reg.vtx[v0].x     = (float)(_s16)doReadMemHalfWord(ad+0);
        rdp_reg.vtx[v0].y     = (float)(_s16)doReadMemHalfWord(ad+2);
        rdp_reg.vtx[v0].z     = (float)(_s16)doReadMemHalfWord(ad+4);
        rdp_reg.vtx[v0].flags = (_u16)doReadMemHalfWord(ad+6);
        rdp_reg.vtx[v0].s     = (float)(_s16)doReadMemHalfWord(ad+8);
        rdp_reg.vtx[v0].t     = (float)(_s16)doReadMemHalfWord(ad+10);
        rdp_reg.vtx[v0].r     = (_u8)doReadMemByte(ad+12);
        rdp_reg.vtx[v0].g     = (_u8)doReadMemByte(ad+13);
        rdp_reg.vtx[v0].b     = (_u8)doReadMemByte(ad+14);
        rdp_reg.vtx[v0].a     = (_u8)doReadMemByte(ad+15);

        if( (USE_MM_HACK) && rdp_reg.combine_h == 0x00262a60 && rdp_reg.combine_l == 0x150c937f && rdp_reg.tile == 0 )
        {
            // Hack for Zelda Sun
            t_tile *t0 = &rdp_reg.td[0];
            t_tile *t1 = &rdp_reg.td[1];

            if( t0->format == 4 && t0->size == 1 && t0->Width == 64 &&
                t1->format == 4 && t1->size == 1 && t1->Width == 64 &&
                t0->Height == t1->Height )
            {
                rdp_reg.vtx[v0].s /= 2;
                rdp_reg.vtx[v0].t /= 2;
            }
        }

        {
            t_vtx *vertex = &rdp_reg.vtx[v0];
            float tmpvtx[4];

            tmpvtx[0] = vertex->x;
            tmpvtx[1] = vertex->y;
            tmpvtx[2] = vertex->z;
            tmpvtx[3] = 1.0f;

//          transform_vector(&tmpvtx[0],vertex->x,vertex->y,vertex->z);
//          project_vector2(&tmpvtx[0]);
            cmbtrans_vector(&tmpvtx[0]);

//          if (
//              (tmpvtx[2] < 0.001f)
//              && (tmpvtx[2] >= -0.001f)
//              )
//              tmpvtx[2] = tmpvtx[3];

            vertex->x = tmpvtx[0];
            vertex->y = tmpvtx[1];
            vertex->z = tmpvtx[2];
            vertex->w = tmpvtx[3];

            tmpvtx[0] = (float)(_s8)vertex->r;
            tmpvtx[1] = (float)(_s8)vertex->g;
            tmpvtx[2] = (float)(_s8)vertex->b;

            //transform_normal(&tmpvtx[0]);
            NormalizeVector(&tmpvtx[0]);

            vertex->n1 = tmpvtx[0];
            vertex->n2 = tmpvtx[1];
            vertex->n3 = tmpvtx[2];

            if (0x00020000 & rdp_reg.geometrymode)
            {
                if (USE_MM_HACK)
                    math_lightingMM((t_vtx*)(char*)vertex, vertex->lcolor);
                else
                    math_lightingN((t_vtx*)(char*)vertex, vertex->lcolor);
            }
            else
            {
                vertex->lcolor[0] = (float)vertex->r / 255.0f;
                vertex->lcolor[1] = (float)vertex->g / 255.0f;
                vertex->lcolor[2] = (float)vertex->b / 255.0f;
                vertex->lcolor[3] = (float)vertex->a / 255.0f;
            }

            if (0x00040000 & rdp_reg.geometrymode)
            {
                // G_TEXTURE_GEN enable the automatic generation of the texture
                // coordinates s and t.  A spherical mapping is used, based on the normal.

                float fLenght;

                tmpvtx[0] = (float)(_s8)vertex->r;
                tmpvtx[1] = (float)(_s8)vertex->g;
                tmpvtx[2] = (float)(_s8)vertex->b;

                cmbtrans_normal(&tmpvtx[0]);

                vertex->n1 = tmpvtx[0];
                vertex->n2 = tmpvtx[1];
                vertex->n3 = tmpvtx[2];
                
                fLenght = VectorLength(tmpvtx);

                vertex->s = (0.5f + (0.5f * vertex->n1 / fLenght));
                vertex->t = (0.5f - (0.5f * vertex->n2 / fLenght));
//              vertex->s = (0.5f + (0.5f * vertex->n1));
//              vertex->t = (0.5f - (0.5f * vertex->n2));
            }
#ifdef LOG_ON
        LOG_TO_FILE("\tvtx[%02i]: xyzw  -> %12.5f, %12.5f, %12.5f, %12.5f\n"
                    "\ts=%9.4f, t=%9.4f\n\tcolor   [%02X,%02X,%02X]\n"
                    "\tnormals [%12.5f,%12.f,%12.5f]\n",
                    v0,
                    rdp_reg.vtx[v0].x, rdp_reg.vtx[v0].y, rdp_reg.vtx[v0].z, rdp_reg.vtx[v0].w,
                    rdp_reg.vtx[v0].s, rdp_reg.vtx[v0].t,
                    rdp_reg.vtx[v0].r, rdp_reg.vtx[v0].g, rdp_reg.vtx[v0].b,
                    rdp_reg.vtx[v0].n1, rdp_reg.vtx[v0].n2, rdp_reg.vtx[v0].n3);
#endif

        }

        v0++;
    }
}

static _u32 tfbuffer[512*256];
extern byte bmpHdr[];
extern int bmpcnt;
extern _u32 *Color16_32;

void DumpBmp(_u8 *bffr,_u32 width, _u32 height)
{
    int x,y;
    _u32 *tbffr = (_u32*)bffr;
    char bmpFileName[128] = "";
    FILE *bmpFile;
    byte tline[1024 * 4];

    char EmuPath[128] = "";
    GetEmuDir(EmuPath);

    *(_u32*)&bmpHdr[2] = (_u32) ((width * height * 3) + 0x036);

    *(_u32*)&bmpHdr[0x12] = width;
    *(_u32*)&bmpHdr[0x16] = height;

    *(_u32*)&bmpHdr[0x22] = (_u32) ((width * height * 3));

    sprintf(bmpFileName,"%sTiles\\Dump%04i.bmp",EmuPath,bmpcnt++);
    bmpFile = fopen(bmpFileName,"wb");

    fwrite(bmpHdr, 0x36, 1, bmpFile);
    
    for ( y = height - 1;y >= 0; y--)
    {
        tbffr = (_u32*)bffr;
        tbffr += (y * width);
        for (x = 0; x < (int)width; x++)
        {
            {
                tline[x*3+0] = ((_u8*)tbffr)[2];
                tline[x*3+1] = ((_u8*)tbffr)[1];
                tline[x*3+2] = ((_u8*)tbffr)[0];
                tbffr++;
            }
        }

        fwrite(tline, width * 3, 1, bmpFile);
    }
    fclose(bmpFile);
}

typedef struct DRAWIMAGE_t {
    float frameX;
    float frameY;
    float frameW;
    float frameH;
    WORD imageX;
    WORD imageY;
    WORD imageW;
    WORD imageH;
    DWORD imagePtr;
    BYTE imageFmt;
    BYTE imageSiz;
    WORD imagePal;
} DRAWIMAGE;

void rsp_uc04_fullscreen()
{
    unsigned long i, j;
    unsigned int width = 256;
    unsigned int height =256;
    unsigned int mode = 2;

    _u32 *add = (_u32*)(&pRDRAM[segoffset2addr(rdp_reg.cmd1)]);
    t_Fullscreen *fsadd = (t_Fullscreen*)(&pRDRAM[segoffset2addr(rdp_reg.cmd1)]);

    _u32 Scrwidth = fsadd->imageW >> 2;
    _u32 Scrheight = fsadd->imageH >> 2;

    _u32 Dstwidth = fsadd->frameW >> 2;
    _u32 Dstheight = fsadd->frameH >> 2;

    _u32 fladd = segoffset2addr(fsadd->imagePtr);

    _u16 src;
    _u16 *base_src_ptr;

//  _u32 *lsrc;
    _u32 *lbase_src_ptr;

    _u32 *dest;
    _u32 tmpTex = 0x0800001;

    float txl = 0.0f;
    float tyl = 0.0f;
    float txh = 319.0f * fScalex;
    float tyh = 239.0f * fScaley;
    float color[4]={1.0f,1.0f,1.0f,1.0f};

    GLint hadDepthTest, hadBlending, hadAlphaTest;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_FULLSCREEN",ADDR, CMD0, CMD1);
    LOG_TO_FILE("\tAddress = %08x",segoffset2addr(rdp_reg.cmd1));
    LOG_TO_FILE("\tImageAddress = %08x",fladd);
    LOG_TO_FILE("\tSrcWidth = %i\tSrcHeight = %i",Scrwidth,Scrheight);
    LOG_TO_FILE("\tDstWidth = %i\tDstHeight = %i\n",Dstwidth,Dstheight);
#endif

/**/
    glGetIntegerv(GL_DEPTH_TEST, &hadDepthTest); //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_BLEND, &hadBlending); //** Should be glGetBooleanv, but 
    glGetIntegerv(GL_ALPHA_TEST, &hadAlphaTest); //** Should be glGetBooleanv, but 

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

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glEnable(GL_TEXTURE_2D);
/**/
    base_src_ptr  = (_u16*)(&pRDRAM[fladd]);
    lbase_src_ptr = (_u32*)(&pRDRAM[fladd]);

    for (i = 0; i<256*512; i++) tfbuffer[i] = 0;

        for(i=0; i<Scrheight; i++)
        {
            _u16 *tsrc = &(base_src_ptr[i * Scrwidth]);
            dest = (_u32*)&tfbuffer[i * 512];
            for(j=0; j<Scrwidth; j++)
            {               
                src = tsrc[(j ^ 0x01)];
                *dest = Color16_32[src];
                dest++;
            }
        }

#ifdef DUMP_TILE
        DumpBmp((_u8*)tfbuffer,512,256);
#endif

/**/
        glDeleteTextures(1, &tmpTex);
        glBindTexture(GL_TEXTURE_2D, tmpTex);
        
        glEnable(GL_TEXTURE_2D);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 256, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, (_u8*)tfbuffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glColor4fv(color);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);
            glVertex2f(txl, tyl);

            glTexCoord2f(319.0f/512.0f,0.0f);
            glVertex2f(txh, tyl);

            glTexCoord2f(319.0f/512.0f,239.0f/256.0f);
            glVertex2f(txh, tyh);

            glTexCoord2f(0,239.0f/256.0f);
            glVertex2f(txl, tyh);
        glEnd();

        //} // end for ulx uly

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    if (hadBlending) glEnable(GL_BLEND);
    if (hadDepthTest) glEnable(GL_DEPTH_TEST);
    if (hadAlphaTest) glEnable(GL_ALPHA_TEST);
/**/
}

// Modify vertex offsets
#define F3DEX2_POINT_RGBA               0x10
#define F3DEX2_POINT_ST                 0x14
#define F3DEX2_POINT_XYSCREEN           0x18
#define F3DEX2_POINT_ZSCREEN            0x1c

void rsp_uc04_modifyvertex()
{
    WORD    vtx = (WORD)((CMD0 & 0xFFFF) >> 1);
    BYTE    where = (BYTE)((CMD0 >> 16) & 0xFF);
    
    switch (where)
    {
        case F3DEX2_POINT_RGBA:
            rdp_reg.vtx[vtx].r = (_u8)((CMD1 >> 24) & 0xFF);
            rdp_reg.vtx[vtx].g = (_u8)((CMD1 >> 16) & 0xFF);
            rdp_reg.vtx[vtx].b = (_u8)((CMD1 >>  8) & 0xFF);
            rdp_reg.vtx[vtx].a = (_u8)((CMD1      ) & 0xFF);
            break;
        case F3DEX2_POINT_ST:
            rdp_reg.vtx[vtx].s = (SHORT)((CMD1 >> 16) & 0xFFFF);// * 0.03125f;
            rdp_reg.vtx[vtx].t = (SHORT)(CMD1 & 0xFFFF);// * 0.03125f;
            break;
        case F3DEX2_POINT_XYSCREEN:
            break;
        case F3DEX2_POINT_ZSCREEN:
            break;
    }

    //rdp_reg.vtx[vtx].changed = TRUE;
    //DebugBox("ModifyVertex\n");
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_MODIFYVTX \n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_culldl()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_CULLDL\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_branchz()
{

    rdp_reg.pc_i++;
    if(rdp_reg.pc_i > RDP_STACK_SIZE - 1)
    {
        return;
    }

    rdp_reg.pc[rdp_reg.pc_i] = bzaddr;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_BRANCHZ\n", ADDR, CMD0, CMD1);
#endif
}


void rsp_uc04_tri1()
{
    int     vn[3];

    vn[0] = ((CMD0 >> 16) & 0xff) / 2;
    vn[1] = ((CMD0 >>  8) & 0xff) / 2;
    vn[2] = ( CMD0        & 0xff) / 2;

    DrawVisualTriangle(vn);

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_TRI1 (%i,%i,%i)\n", 
                 ADDR, CMD0, CMD1, vn[0], vn[1], vn[2]);
#endif
} // void rsp_uc04_tri1()


void rsp_uc04_tri2()
{
    int     vn[6];
    
    vn[0] = ((CMD0 >> 16) & 0xff) / 2;
    vn[1] = ((CMD0 >>  8) & 0xff) / 2;
    vn[2] = ( CMD0        & 0xff) / 2;

    vn[3] = ((CMD1 >> 16) & 0xff) / 2;
    vn[4] = ((CMD1 >>  8) & 0xff) / 2;
    vn[5] = ( CMD1        & 0xff) / 2;

    DrawVisualTriangle(vn);
    DrawVisualTriangle(vn+3);

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_TRI2 (%i,%i,%i) (%i,%i,%i)\n", 
                 ADDR, CMD0, CMD1, vn[0], vn[1], vn[2], vn[3], vn[4], vn[5]);
#endif
} // void rsp_uc04_tri2()

void rsp_uc04_quad()
{
    int     vn[6];


    DebugBox("12");

    vn[0] = ((rdp_reg.cmd0 >> 16) & 0xff) / 2;
    vn[1] = ((rdp_reg.cmd0 >>  8) & 0xff) / 2;
    vn[2] = ( rdp_reg.cmd0        & 0xff) / 2;

    vn[3] = ((rdp_reg.cmd1 >> 16) & 0xff) / 2;
    vn[4] = ((rdp_reg.cmd1 >>  8) & 0xff) / 2;
    vn[5] = ( rdp_reg.cmd1        & 0xff) / 2;

    DrawVisualTriangle(vn);
    DrawVisualTriangle(vn+3);

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_QUAD (%i,%i,%i) (%i,%i,%i)\n", 
                 ADDR, CMD0, CMD1, vn[0], vn[1], vn[2], vn[3], vn[4], vn[5]);
#endif
} // static void rsp_uc04_quad()


void rsp_uc04_fastcombine()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_FAST_COMBINE",ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_rdphalf_cont()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_RDP_HALF_1\n",ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_rdphalf_2()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_RDP_HALF_2\n",ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_line3d()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_LINE_3D\n",ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_texture()
{
        int idx;
        int tile = (rdp_reg.cmd0 >> 8)  & 0x07;             //** tile t0
        int tile1 = (tile + 1)  & 0x07;                     //** tile t1
        _u32 mipmap_level = (rdp_reg.cmd0 >> 11) & 0x07;    //** mipmap_level   - not used yet
        _u32 on = (rdp_reg.cmd0 & 0xff);            //** 1: on - 0:off

        float s = (float)((rdp_reg.cmd1 >> 16) & 0xffff);
        float t = (float)((rdp_reg.cmd1      ) & 0xffff);
        float SScale;
        float TScale;

        t_tile *tmp_tile = &rdp_reg.td[tile];
        t_tile *tmp_tile1 = &rdp_reg.td[tile1];
        tmp_tile->Texture_on = (_u8)on;

        rdp_reg.tile = tile;

        //if (s<=1)
        //  SScale=1.0f;
        //else
            SScale=(float)s/65535.f;

        //if (t<=1)
        //  TScale=1.0f;
        //else
            TScale=(float)t/65535.f;

        TScale/=32.f;
        SScale/=32.f;
        
        if( (((rdp_reg.cmd1)>>16)&0xFFFF) == 0xFFFF )
        {
            SScale = 1/32.0f;
        }
        else if( (((rdp_reg.cmd1)>>16)&0xFFFF) == 0x8000 )
        {
            SScale = 1/64.0f;
        }
        if( (((rdp_reg.cmd1)    )&0xFFFF) == 0xFFFF )
        {
            TScale = 1/32.0f;
        }
        else if( (((rdp_reg.cmd1)    )&0xFFFF) == 0x8000 )
        {
            TScale = 1/64.0f;
        }

        for (idx=0;idx<7;idx++)
        {
            rdp_reg.tile = idx;
            tmp_tile1 = &rdp_reg.td[idx];
            tmp_tile1->SScale=SScale;
            tmp_tile1->TScale=TScale;
            rdp_reg.m_CurTile = tmp_tile1;
            MathTextureScales();
        }

        rdp_reg.tile = tile;
        rdp_reg.m_CurTile = tmp_tile;
//      MathTextureScales();

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_TEXTURE",ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tTile = %i, Mipmap = %i, enambled = %s\n",tile, mipmap_level, (on)?"on":"off");
#endif
}

void rsp_uc04_popmatrix()
{
//  _u32 param = (CMD1 / 64);
    _u32 param = (((((CMD0 >> 19) & 0x1f) + 1) * 8)/ 64);


    switch(CMD1)
    {
    case 0x40:
        pop_matrix();
        break;
    }

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_POPMATRIX\n",ADDR, CMD0, CMD1);
#endif
}

void rsp_uc04_setgeometrymode()
{
    BOOL mode = FALSE;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_SETGEOMETRYMODE",ADDR, CMD0, CMD1);
        LOG_TO_FILE(
                "\tClear:"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s",
                (!(rdp_reg.cmd0 & 0x00000001)) ? "\tzbuffer\n" : "",
                (!(rdp_reg.cmd0 & 0x00000002)) ? "\ttexture\n" : "",
                (!(rdp_reg.cmd0 & 0x00000004)) ? "\tshade\n" : "",
                (!(rdp_reg.cmd0 & 0x00000200)) ? "\tshade smooth\n" : "",
                (!(rdp_reg.cmd0 & 0x00001000)) ? "\tcull front\n" : "",
                (!(rdp_reg.cmd0 & 0x00002000)) ? "\tcull back\n" : "",
                (!(rdp_reg.cmd0 & 0x00010000)) ? "\tfog\n" : "",
                (!(rdp_reg.cmd0 & 0x00020000)) ? "\tlightning\n" : "",
                (!(rdp_reg.cmd0 & 0x00040000)) ? "\ttexture gen\n" : "",
                (!(rdp_reg.cmd0 & 0x00080000)) ? "\ttexture gen lin\n" : "",
                (!(rdp_reg.cmd0 & 0x00100000)) ? "\tlod\n" : ""
                );
        LOG_TO_FILE(
                "\tSet:\n"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s",
                (rdp_reg.cmd1 & 0x00000001) ? "\t        zbuffer\n" : "",
                (rdp_reg.cmd1 & 0x00000002) ? "\t        texture\n" : "",
                (rdp_reg.cmd1 & 0x00000004) ? "\t        shade\n" : "",
                (rdp_reg.cmd1 & 0x00000200) ? "\t        shade smooth\n" : "",
                (rdp_reg.cmd1 & 0x00001000) ? "\t        cull front\n" : "",
                (rdp_reg.cmd1 & 0x00002000) ? "\t        cull back\n" : "",
                (rdp_reg.cmd1 & 0x00010000) ? "\t        fog\n" : "",
                (rdp_reg.cmd1 & 0x00020000) ? "\t        lightning\n" : "",
                (rdp_reg.cmd1 & 0x00040000) ? "\t        texture gen\n" : "",
                (rdp_reg.cmd1 & 0x00080000) ? "\t        texture gen lin\n" : "",
                (rdp_reg.cmd1 & 0x00100000) ? "\t        lod\n" : ""
                );
#endif
        rdp_reg.geometrymode &= (rdp_reg.cmd0 & 0x0ffffff);
        rdp_reg.geometrymode |= (rdp_reg.cmd1 & 0x0ffffff);

        if(rdp_reg.cmd1 & 0x00000002)
                rdp_reg.geometrymode_textures = 1;

        switch(rdp_reg.geometrymode & 0x00000600)
        {
        case 0x0200:
            Render_geometry_cullfront(1);
            break;
        case 0x0400:
            Render_geometry_cullback(1);
            break;
        case 0x0600:
            Render_geometry_cullfrontback(1);
            break;
        default:
            Render_geometry_cullfrontback(0);
            break;
        } //** switch(rdp_reg.geometrymode & 0x00003000) 

        mode = (rdp_reg.geometrymode & 0x00080000) ? FALSE : TRUE;
        
        if(mode) // 
            glShadeModel(GL_SMOOTH);
        else
            glShadeModel(GL_FLAT);

        mode = (rdp_reg.geometrymode & 0x00010000) ? FALSE : TRUE;
        
        //Render_Fog(mode);

//      if((rdp_reg.geometrymode & 0x00020000) != 0)
//              rdp_reg.useLights = 1;

}

void rsp_uc04_matrix()
{

  static char *mtxop[] = {
                "modelview  mul  push",
                "modelview  mul  nopush",
                "modelview  load push",
                "modelview  load nopush",
                "projection mul  push",
                "projection mul  nopush",
                "projection load push",
                "projection load nopush"};

        _u32   a = segoffset2addr(rdp_reg.cmd1);
        _u8   command =(_u8)(CMD0 & 0xff);
        float   m[4][4];
//        int     i, j;


        refresh_lights = TRUE;
        hleGetMatrix((float*)&m[0,0], &pRDRAM[a]);
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_LOADMTX  at %08X\n", ADDR, CMD0, CMD1, a);
        LOG_TO_FILE(
                "\t%s\n"
                "\t\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n",
                mtxop[command],
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
                );
#endif
        switch(command)
        {

            case 0: // modelview  mul  push   
                push_mult_matrix((GLfloat *)m);
                break;

            case 1: // modelview  mul  nopush 
                mult_matrix((GLfloat *)m);
                break;

            case 2: // modelview  load push   
                push_load_matrix((GLfloat *)m);
                break;

            case 3: // modelview  load nopush 
                load_matrix((GLfloat *)m);
                break;

            case 4: // projection mul  push   
                DebugBox("strange Matrix-CMD");
                //glMultMatrixf((GLfloat *)m);
                mult_prj_matrix((GLfloat *)m);
                break;

            case 5: // projection mul  nopush 
                //glMultMatrixf((GLfloat *)m);
                mult_prj_matrix((GLfloat *)m);
                break;

            case 6: // projection load push   
                DebugBox("strange Matrix-CMD");
                //glLoadMatrixf((GLfloat *)m);
                load_prj_matrix((GLfloat *)m);
                break;

            case 7: // projection load nopush 
                //glLoadMatrixf((GLfloat *)m);
                load_prj_matrix((GLfloat *)m);
                break;

            default:
                break;

        } /* switch(command) */

}

void rsp_uc04_moveword()
{
        int i;
        _u16 offset= (_u16)(CMD0 & 0xffff);
        _u8 index=   (_u8)(CMD0>>16)&0xff;
        _u64 data=rdp_reg.cmd1;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_MOVEWORD", ADDR, CMD0, CMD1);
#endif
        switch(index)
        {
            case G_MW_MATRIX:
                {
                    // do matrix pre-mult so it's re-updated next time
                    if (refresh_matrix)
                        update_cmbmatrix();
                    
                    if (CMD0 & 0x20)  // fractional part
                    {
                        float fpart;
                        int index_x = (CMD0 & 0x1F) >> 1;
                        int index_y = index_x >> 2;
                        index_x &= 3;

                        fpart = (CMD1>>16)/65536.0f;
                        cmb_mtrx[index_y][index_x] = (float)(int)cmb_mtrx[index_y][index_x];
                        cmb_mtrx[index_y][index_x] += fpart;

                        fpart = (CMD1&0xFFFF)/65536.0f;
                        cmb_mtrx[index_y][index_x+1] = (float)(int)cmb_mtrx[index_y][index_x+1];
                        cmb_mtrx[index_y][index_x+1] += fpart;
                    }
                    else
                    {
                        float fpart;
                        int index_x = (CMD0 & 0x1F) >> 1;
                        int index_y = index_x >> 2;
                        index_x &= 3;

                        fpart = (float)fabs(cmb_mtrx[index_y][index_x] - (int)cmb_mtrx[index_y][index_x]);
                        cmb_mtrx[index_y][index_x] = (short)(CMD1>>16);

                        fpart = (float)fabs(cmb_mtrx[index_y][index_x+1] - (int)cmb_mtrx[index_y][index_x+1]);
                        cmb_mtrx[index_y][index_x+1] = (short)(CMD1&0xFFFF);
                    }
                }
                break;

            case G_MW_NUMLIGHT:
                //rdp_reg.lights = (_u32)((data & 0xff) / 16 - 1);
                rdp_reg.lights = (_u32)((data & 0xff) / 24);
                if ((rdp_reg.lights < 0)||(rdp_reg.lights > 15)) rdp_reg.lights = 0;
                rdp_reg.ambient_light =  rdp_reg.lights;
                break;

            case G_MW_CLIP:
                switch((rdp_reg.cmd0 >> 8) & 0xffff)
                {
                    case 0x0004:
                        rdp_reg.clip.nx = rdp_reg.cmd1;
#ifdef LOG_ON
                        LOG_TO_FILE("\tClip nx=%08i", CMD1);
#endif
                        break;

                    case 0x000c:
                        rdp_reg.clip.ny = rdp_reg.cmd1;
#ifdef LOG_ON
                        LOG_TO_FILE("\tClip ny=%08i", CMD1);
#endif
                        break;

                    case 0x0014:
                        rdp_reg.clip.px = rdp_reg.cmd1;
#ifdef LOG_ON
                        LOG_TO_FILE("\tClip px=%08i", CMD1);
#endif
                        break;

                    case 0x001c:
                        rdp_reg.clip.py = rdp_reg.cmd1;
#ifdef LOG_ON
                        LOG_TO_FILE("\tClip py=%08i", CMD1);
#endif
                        break;

                    default:
                        break;
                } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */               
                break;


            case G_MW_SEGMENT:
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD SEGMENT: $%08lx -> seg#%d\n", CMD1, offset / 4);
#endif
                rdp_reg.segment[offset >> 2] = CMD1 & 0xffffff;
                break;

            case G_MW_FOG:
                {
                    float fo,fm,min,max,rng;
                    
                    fm = (float)(_s16)((CMD1 & 0xffff0000)>> 16);
                    fo = (float)(_s16)((CMD1 & 0xffff));

                    rng = 128000.0f / fm;
                    min = 500.0f - ((fo * rng) / 256.0f);
                    max = rng + min;

                    rdp_reg.fog_fo = fo;
                    rdp_reg.fog_fm = fm;

#ifdef LOG_ON
                    LOG_TO_FILE("\tFog min = %f, max = %f",min,max);
#endif
                }
                break;

            case G_MW_LIGHTCOL:
                i = (rdp_reg.cmd0 & 0x0000e000) >> 13;

#ifdef LOG_ON
                LOG_TO_FILE("\tLight = %i, Color = %08X", i, CMD1);
#endif
                if(rdp_reg.cmd0 & 0x00000400)
                {
                        rdp_reg.light[i].r         = ((float)((rdp_reg.cmd1 >> 24) & 0xff))/255.0f;
                        rdp_reg.light[i].g         = ((float)((rdp_reg.cmd1 >> 16) & 0xff))/255.0f;
                        rdp_reg.light[i].b         = ((float)((rdp_reg.cmd1 >>  8) & 0xff))/255.0f;
                        rdp_reg.light[i].a         = 1.0f;
                }
                else
                {
                        rdp_reg.light[i].r_copy    = ((float)((rdp_reg.cmd1 >> 24) & 0xff))/255.0f;
                        rdp_reg.light[i].g_copy    = ((float)((rdp_reg.cmd1 >> 16) & 0xff))/255.0f;
                        rdp_reg.light[i].b_copy    = ((float)((rdp_reg.cmd1 >>  8) & 0xff))/255.0f;
                        rdp_reg.light[i].a_copy    = 1.0f;
                }

               break;

            case G_MW_FORCEMTX:
#ifdef LOG_ON
                LOG_TO_FILE("\tG_MW_FORCEMTX\n");
#endif
                break;

            case G_MW_PERSPNORM:
                rdp_reg.perspnorm = (float)(rdp_reg.cmd1 & 0xFFFF) / 65535.0f;
#ifdef LOG_ON
                LOG_TO_FILE("\tG_MW_PERSPNORM = %3.5f\n",rdp_reg.perspnorm);
#endif
                break;

            default: 
                ;

        } /* switch(rdp_reg.cmd0 & 0xff) */
}

void rsp_uc04_movemem()
{
        _u32 param;
//        int   i;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_MOVEMEM",ADDR, CMD0, CMD1);
#endif
        param = (CMD0 & 0xff);
        switch(param)
        {
            case G_MV_MMTX:
#ifdef LOG_ON
                LOG_TO_FILE("\tMV_MMTX\n");
#endif
                break;

            case G_MV_PMTX:
#ifdef LOG_ON
                LOG_TO_FILE("\tMV_PMTX\n");
#endif
                break;

            case G_MV_VIEWPORT:
                {
                    int a = segoffset2addr(rdp_reg.cmd1) & 0x007fffff;
                    a >>= 1;

                    rdp_reg.vp[0] = ((float)((_s16 *)pRDRAM)[(a+0)^1]) / 4.0f;
                    rdp_reg.vp[1] = ((float)((_s16 *)pRDRAM)[(a+1)^1]) / 4.0f;
                    rdp_reg.vp[2] = ((float)((_s16 *)pRDRAM)[(a+2)^1]) / 4.0f;
                    rdp_reg.vp[3] = ((float)((_s16 *)pRDRAM)[(a+3)^1]) / 4.0f;
                    rdp_reg.vp[4] = ((float)((_s16 *)pRDRAM)[(a+4)^1]) / 4.0f;
                    rdp_reg.vp[5] = ((float)((_s16 *)pRDRAM)[(a+5)^1]) / 4.0f;
                    rdp_reg.vp[6] = ((float)((_s16 *)pRDRAM)[(a+6)^1]) / 4.0f;
                    rdp_reg.vp[7] = ((float)((_s16 *)pRDRAM)[(a+7)^1]) / 4.0f;

                    Render_viewport();

#ifdef LOG_ON
                    LOG_TO_FILE("\tViewPort");
                    LOG_TO_FILE("\t{%f,%f,%f,%f",rdp_reg.vp[0],rdp_reg.vp[1],rdp_reg.vp[2],rdp_reg.vp[3]);
                    LOG_TO_FILE("\t %f,%f,%f,%f}\n",rdp_reg.vp[4],rdp_reg.vp[5],rdp_reg.vp[6],rdp_reg.vp[7]);
#endif
                }
                break;

            case G_MV_LIGHT:
                {
                    int a = segoffset2addr(rdp_reg.cmd1) & 0x007fffff;
                    int i = ((CMD0 >> 5) & 0x3ff);
                    if (i < G_MVO_L0) break;
                    i = (i - G_MVO_L0) / 24;
                    if (1 > 15) break;
                    rdp_reg.light[i].r         = ((float)((_u8 *)pRDRAM)[(a+ 0)^3])/255.0f;
                    rdp_reg.light[i].g         = ((float)((_u8 *)pRDRAM)[(a+ 1)^3])/255.0f;
                    rdp_reg.light[i].b         = ((float)((_u8 *)pRDRAM)[(a+ 2)^3])/255.0f;
                    rdp_reg.light[i].a         = 1.0f;
                    rdp_reg.light[i].r_copy    = ((float)((_u8 *)pRDRAM)[(a+ 4)^3])/255.0f;
                    rdp_reg.light[i].g_copy    = ((float)((_u8 *)pRDRAM)[(a+ 5)^3])/255.0f;
                    rdp_reg.light[i].b_copy    = ((float)((_u8 *)pRDRAM)[(a+ 6)^3])/255.0f;
                    rdp_reg.light[i].a_copy    = 1.0f;
                    
                    if (((((_u8 *)pRDRAM)[(a+ 0)]) == 0x08) && ((((_u8 *)pRDRAM)[(a+ 4)]) == 0xff) && USE_MM_HACK)
                    {
                        rdp_reg.light[i].x         = (float)(int)doReadMemHalfWord(a + 8);//((float)((_s8 *)pRDRAM)[(a+ 8)^3])/127.0f;doReadMemHalfWord();
                        rdp_reg.light[i].y         = (float)(int)doReadMemHalfWord(a + 10);//((float)((_s8 *)pRDRAM)[(a+ 9)^3])/127.0f;
                        rdp_reg.light[i].z         = (float)(int)doReadMemHalfWord(a + 12);//((float)((_s8 *)pRDRAM)[(a+10)^3])/127.0f;
                        rdp_reg.light[i].w         = (float)(int)doReadMemHalfWord(a + 14);//1.0f;
                    }
                    else
                    {
                        rdp_reg.light[i].x         = ((float)((_s8 *)pRDRAM)[(a+ 8)^3])/127.0f;
                        rdp_reg.light[i].y         = ((float)((_s8 *)pRDRAM)[(a+ 9)^3])/127.0f;
                        rdp_reg.light[i].z         = ((float)((_s8 *)pRDRAM)[(a+10)^3])/127.0f;
                        rdp_reg.light[i].w         = 1.0f;
                    }

                    refresh_lights = TRUE;

#ifdef LOG_ON
                    LOG_TO_FILE("\tLight[%i]",i);
                    LOG_TO_FILE("\tRed = %f, Green = %f, Blue = %f, Alpha = %f",
                        rdp_reg.light[i].r,
                        rdp_reg.light[i].g,
                        rdp_reg.light[i].b,
                        rdp_reg.light[i].a);
                    LOG_TO_FILE("\tx = %f, y = %f, z = %f\n",
                        rdp_reg.light[i].x,
                        rdp_reg.light[i].y,
                        rdp_reg.light[i].z);
#endif
                }
                break;

            case G_MV_POINT:
#ifdef LOG_ON
                LOG_TO_FILE("\tG_MV_POINT\n");
#endif
                break;

            case G_MV_MATRIX: 
#ifdef LOG_ON
                LOG_TO_FILE("\tMV_MATRIX\n");
#endif
                {
                    _u32   a = segoffset2addr(CMD1);

                    refresh_matrix = FALSE;
                    hleGetMatrix((float*)&cmb_mtrx[0,0], &pRDRAM[a]);
                }
                break;

            case G_MVO_LOOKATX:
                break;

            case G_MVO_LOOKATY:
                break;

            case G_MVO_L0:
            case G_MVO_L1:
            case G_MVO_L2:
            case G_MVO_L3:
            case G_MVO_L4:
            case G_MVO_L5:
            case G_MVO_L6:
            case G_MVO_L7:
                break;


            default:
                //DebugBox("unknow MoveMem %04x", param);
                break;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */
}

void rsp_uc04_displaylist()
{
        _u32 addr = segoffset2addr(CMD1);
        _u8  push = (_u8)(CMD0 >> 16) & 0xff; 

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_DISPLAYLIST",ADDR, CMD0, CMD1);
        LOG_TO_FILE("\tAddress = %08x %s\n",addr, (push)?", Branch":", Push");
#endif
        switch(push)
        {
            case 0:   // push: we do a call of the dl 
                rdp_reg.pc_i++;
                if(rdp_reg.pc_i > RDP_STACK_SIZE-1)
                {
                    DebugBox("DList Stack overflow");
                    return;
                }
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            case 1:   // branch 
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            default:
                DebugBox("Unknow DList command");
                break;

        } // switch(push) 

}

void rsp_uc04_enddl()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_END_DISPLAYLIST\n",ADDR, CMD0, CMD1);
#endif
        if(rdp_reg.pc_i < 0)
        {
            DebugBox("EndDL - Display Stack underrun");
            rdp_reg.halt = 1;
            return;
        }

        if(rdp_reg.pc_i == 0)
        {
            rdp_reg.halt = 1;
        }

        rdp_reg.pc_i--;
}


void rsp_uc04_enddisplaylist()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_END_DISPLAYLIST?\n",ADDR, CMD0, CMD1);
#endif
    
    rdp_reg.halt = 1;
    return;
}


void rsp_uc04_rdphalf_1()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_END_RDP_HALF_1\n",ADDR, CMD0, CMD1);
#endif
    bzaddr = segoffset2addr(rdp_reg.cmd1);
}

void rsp_uc04_setothermode_l()
{
        static char *ac[] = { "none", "threshold", "?", "diter" };
        static char *zs[] = { "pixel", "prim" };
        static char *a1[] =
                {
                        "        bl_1ma (1)",
                        "        bl_a_mem (1)",
                        "        bl_1 (1)",
                        "        bl_0 (1)"
                };
        static char *b1[] =
                {
                        "        bl_clr_in (1)",
                        "        bl_clr_mem (1)",
                        "        bl_clr_bl (1)",
                        "        bl_clr_fog (1)"
                };
        static char *c1[] =
                {
                        "        bl_a_in (1)",
                        "        bl_a_fog (1)",
                        "        bl_a_shade (1)",
                        "        bl_0 (1)"
                };
        static char *d1[] =
                {
                        "        bl_1ma (1)",
                        "        bl_a_mem (1)",
                        "        bl_1 (1)",
                        "        bl_0 (1)" 
                };
        static char *a2[] =
                {
                        "        bl_1ma (2)",
                        "        bl_a_mem (2)",
                        "        bl_1 (2)",
                        "        bl_0 (2)"
                };
        static char *b2[] =
                {
                        "        bl_clr_in (2)",
                        "        bl_clr_mem (2)",
                        "        bl_clr_bl (2)",
                        "        bl_clr_fog (2)"
                };
        static char *c2[] =
                {
                        "        bl_a_in (2)",
                        "        bl_a_fog (2)",
                        "        bl_a_shade (2)",
                        "        bl_0 (2)"
                };
        static char *d2[] =
                {
                        "        bl_1ma (2)",
                        "        bl_a_mem (2)",
                        "        bl_1 (2)",
                        "        bl_0 (2)" 
                };

        int len = ((rdp_reg.cmd0) & 0xff) + 1;
        int sft = 32 - ((rdp_reg.cmd0 >> 8) & 0xff) - len;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC4_SETOTHERMODE_L ", ADDR, CMD0, CMD1);
#endif
        switch (sft) //((rdp_reg.cmd0 >> 8) & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                LOG_TO_FILE("\tALPHACOMPARE: %s\n", ac[(rdp_reg.cmd1>>0x00) & 0x3]);
#endif

                rdp_reg.mode_l &= ~0x00000003;
                rdp_reg.cmd1   &=  0x00000003;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
                break;

            case 0x02:
#ifdef LOG_ON
                LOG_TO_FILE("\tZSRCSEL: %s\n", zs[(rdp_reg.cmd1>>0x02) & 0x1]);
#endif

                rdp_reg.mode_l &= ~0x00000004;
                rdp_reg.cmd1   &=  0x00000004;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
                break;

            case 0x03:
#ifdef LOG_ON
                LOG_TO_FILE("\tRENDERMODE: ");
                LOG_TO_FILE(""
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n",
                        (rdp_reg.cmd1 & 0x00000008) ? "\tanti alias on\n" : "\tanti alias off\n",
                        (rdp_reg.cmd1 & 0x00000010) ? "\tz_cmp on\n" : "\tz_cmp off\n",
                        (rdp_reg.cmd1 & 0x00000020) ? "\tz_upd on\n" : "\tz_upd off\n",
                        (rdp_reg.cmd1 & 0x00000040) ? "\tim_rd on\n" : "\tim_rd off\n",
                        (rdp_reg.cmd1 & 0x00000080) ? "\tclr_on_cvg on\n" : "\tclr_on_cvg off\n",
                        (rdp_reg.cmd1 & 0x00000100) ? "\tcvg_dst_warp on\n" : "\tcvg_dst_warp off\n",
                        (rdp_reg.cmd1 & 0x00000200) ? "\tcvg_dst_full on\n" : "\tcvg_dst_full off\n",
                        (rdp_reg.cmd1 & 0x00000400) ? "\tz_inter on\n" : "\tz_inter off\n",
                        (rdp_reg.cmd1 & 0x00000800) ? "\tz_xlu on\n" : "\tz_xlu off\n",
                        (rdp_reg.cmd1 & 0x00001000) ? "\tcvg_x_alpha on\n" : "\tcvg_x_alpha off\n",
                        (rdp_reg.cmd1 & 0x00002000) ? "\talpha_cvg_sel on\n" : "\talpha_cvg_sel off\n",
                        (rdp_reg.cmd1 & 0x00004000) ? "\tforce_bl on\n" : "\tforce_bl off\n",
                        (rdp_reg.cmd1 & 0x00008000) ? "\ttex_edge? on\n" : "\ttex_edge? off\n",
                        a2[(rdp_reg.cmd1>>16) & 0x3],
                        a1[(rdp_reg.cmd1>>18) & 0x3],
                        b2[(rdp_reg.cmd1>>20) & 0x3],
                        b1[(rdp_reg.cmd1>>22) & 0x3],
                        c2[(rdp_reg.cmd1>>24) & 0x3],
                        c1[(rdp_reg.cmd1>>26) & 0x3],
                        d2[(rdp_reg.cmd1>>28) & 0x3],
                        d1[(rdp_reg.cmd1>>30) & 0x3]
                        );
#endif
               rdp_reg.mode_l &= ~0xfffffff8;
                rdp_reg.cmd1   &=  0xfffffff8;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
                break;

            case 0x10:
                rdp_reg.mode_l &= ~0xffff0000;
                rdp_reg.cmd1   &=  0xffff0000;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tBLENDER: ");
                LOG_TO_FILE(""
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n"
                        "%s\n",
                        a2[(rdp_reg.cmd1>>16) & 0x3],
                        a1[(rdp_reg.cmd1>>18) & 0x3],
                        b2[(rdp_reg.cmd1>>20) & 0x3],
                        b1[(rdp_reg.cmd1>>22) & 0x3],
                        c2[(rdp_reg.cmd1>>24) & 0x3],
                        c1[(rdp_reg.cmd1>>26) & 0x3],
                        d2[(rdp_reg.cmd1>>28) & 0x3],
                        d1[(rdp_reg.cmd1>>30) & 0x3]
                        );
#endif
                break;

            default:
                //PRINT_RDP("SETOTHERMODE_L ?\n");
                ;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */

        if(rdp_reg.mode_l & 0x00000010)
                Render_geometry_zbuffer(1);
        else
                Render_geometry_zbuffer(0);

        if ((rdp_reg.mode_l & 0x0c00) != 0xc00) glDepthRange(-1,1.);
        else  glDepthRange(-1,0.9995);

        if (rdp_reg.mode_l & 0x00000020)
            Render_geometry_zwrite(1);
        else
            Render_geometry_zwrite(0);
/*
    switch (rdp_reg.mode_l & 0x00000003)
    {
    case 0:
        Src_Alpha = GL_ONE;
        Dst_Alpha = GL_ZERO;
        break;
    case 1:
    case 2:
    case 3:
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
        break;
    }
*/
/*
//  if ((rdp_reg.mode_l & 0x0f0f0000) != 0x50000)
    if ((rdp_reg.mode_l & 0x0f000000) != 0x0)
    {
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
    }
    else
    {
//      Src_Alpha = GL_ONE;
//      Dst_Alpha = GL_ZERO;
    }
*/
}

void rsp_uc04_setothermode_h()
{
        static char *ad[] = { "pattern", "notpattern", "noise", "disable" };
        static char *rd[] = { "magicsq", "bayer", "noise", "?" };
        static char *ck[] = { "none", "key" };
        static char *tc[] = { "conv", "?", "?", "?", "?", "filtconv", "filt", "?" };
        static char *tf[] = { "point", "?", "bilerp", "average" };
        static char *tt[] = { "none", "?", "rgba16", "ia16" };
        static char *tl[] = { "tile", "lod" };
        static char *td[] = { "clamp", "sharpen", "detail", "?" };
        static char *tp[] = { "none", "persp" };
        static char *ct[] = { "1cycle", "2cycle", "copy", "fill" };
        static char *cd[] = { "disable(hw>1)", "enable(hw>1)", "disable(hw1)", "enable(hw1)" };
        static char *pm[] = { "nprimitive", "1primitive" };

        int len = (rdp_reg.cmd0 & 0xff) +1;
        int sft = 32 - ((rdp_reg.cmd0 >> 8) & 0xff) - len;
//        switch((rdp_reg.cmd0 >> 8) & 0xff)

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC4_SETOTHERMODE_H\n",ADDR, CMD0, CMD1);
#endif
        switch(sft)
        {
            case 0x00:
#ifdef LOG_ON
                LOG_TO_FILE("\tBLENDMASK - ignored\n");
#endif
                break;

            case 0x04:
                rdp_reg.mode_h &= ~0x00000030;
                rdp_reg.cmd1   &=  0x00000030;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tALPHADITHER: %s\n", ad[(rdp_reg.cmd1>>0x04) & 0x3]);
#endif
                break;

            case 0x06:
                rdp_reg.mode_h &= ~0x000000c0;
                rdp_reg.cmd1   &=  0x000000c0;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tRGBDITHER: %s\n", rd[(rdp_reg.cmd1>>0x06) & 0x3]);
#endif
                break;

            case 0x08:
                rdp_reg.mode_h &= ~0x00000100;
                rdp_reg.cmd1   &=  0x00000100;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tCOMBINEKEY: %s\n", ck[(rdp_reg.cmd1>>0x08) & 0x1]);
#endif
                break;

            case 0x09:
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTURECONVERT: %s\n", tc[(rdp_reg.cmd1>>0x09) & 0x7]);
#endif
                rdp_reg.mode_h &= ~0x00000e00;
                rdp_reg.cmd1   &=  0x00000e00;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x0c:
                rdp_reg.mode_h &= ~0x00003000;
                rdp_reg.cmd1   &=  0x00003000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREFILTER: %s\n", tf[(rdp_reg.cmd1>>0x0c) & 0x3]);
#endif
                break;

            case 0x0e:
                rdp_reg.mode_h &= ~0x0000c000;
                rdp_reg.cmd1   &=  0x0000c000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTURELUT: %s\n", tt[(rdp_reg.cmd1>>0x0e) & 0x3]);
#endif
                break;

            case 0x10:
                rdp_reg.mode_h &= ~0x00010000;
                rdp_reg.cmd1   &=  0x00010000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTURELOD: %s\n", tl[(rdp_reg.cmd1>>0x10) & 0x1]);
#endif
                break;

            case 0x11:
                rdp_reg.mode_h &= ~0x00060000;
                rdp_reg.cmd1   &=  0x00060000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREDETAIL: %s\n", td[(rdp_reg.cmd1>>0x11) & 0x3]);
#endif
                break;

            case 0x13:
                rdp_reg.mode_h &= ~0x00080000;
                rdp_reg.cmd1   &=  0x00080000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREPERSP: %s\n", tp[(rdp_reg.cmd1>>0x13) & 0x1]);
#endif
                break;

            case 0x14:
                cycle_mode = (_u8)((rdp_reg.cmd1>>0x14) & 0x3);
                rdp_reg.mode_h &= ~0x00300000;
                rdp_reg.cmd1   &=  0x00300000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tCYCLETYPE: %s\n", ct[(rdp_reg.cmd1>>0x14) & 0x3]);
#endif
                break;

            case 0x16:
                rdp_reg.mode_h &= ~0x00400000;
                rdp_reg.cmd1   &=  0x00400000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tCOLORDITHER: %s\n", cd[(rdp_reg.cmd1>>0x16) & 0x1]);
#endif
                break;

            case 0x17:
                rdp_reg.mode_h &= ~0x00800000;
                rdp_reg.cmd1   &=  0x00800000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
#ifdef LOG_ON
                LOG_TO_FILE("\tPIPELINEMODE: %s\n", pm[(rdp_reg.cmd1>>0x17) & 0x1]);
#endif
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNDEFINED\n");
#endif
                break;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */
}


