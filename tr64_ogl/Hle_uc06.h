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

//////////////////////////////////////////////////////////////////////////////
// Diddy Kong                                                               //
//////////////////////////////////////////////////////////////////////////////

void rsp_uc06_matrix();
void rsp_uc06_movemem();
void rsp_uc06_vertex();
void rsp_uc06_displaylist();
void rsp_uc06_sprite2d();

void rsp_uc06_tri4();
void rsp_uc06_rdphalf_cont();
void rsp_uc06_rdphalf_2();
void rsp_uc06_rdphalf_1();
void rsp_uc06_line3d();
void rsp_uc06_cleargeometrymode();
void rsp_uc06_setgeometrymode();
void rsp_uc06_enddl();
void rsp_uc06_setothermode_l();
void rsp_uc06_setothermode_h();
void rsp_uc06_texture();
void rsp_uc06_moveword();
void rsp_uc06_popmatrix();
void rsp_uc06_culldl();
void rsp_uc06_tri1();
void rsp_uc06_dlinmem();

_u8     cmatrix = 0;
_u8     lmatrix = 0;
_u8     matrix, n;
_u32    v0 = 0;
BOOL    PopMatrix = FALSE;
BOOL    doDraw = TRUE;
BOOL    ClearV0 = TRUE;
BOOL    UseJFG = FALSE;
_u32    Mtrx_Base;
_u32    Vrtx_Base;

//////////////////////////////////////////////////////////////////////////////
// LoadVertex                                                               //
//////////////////////////////////////////////////////////////////////////////
#define SIZE_VERTEX_UC06 10

//  0-15 = ????
// 16-17 = Matrix Selection ?
// 19-23 = Num Vertices + 1
void rsp_uc06_vertex()
{
    _u32    a = segoffset2addr(CMD1);
    _u32    i, _len;
    t_vtx   *basevertex = &rdp_reg.vtx[0];



    float   tm[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

    _len = CMD0 & 0x0fff;

    n = (_u8)((CMD0 >> 19) & 0x1F) + 1;

    if (a < 0x2000)
    {
        UseJFG = TRUE;
        a = (CMD1  + Vrtx_Base) & 0x0ffffff;
    }

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_LOADVTX  vertex %i..%i m:%x cm:%i",
                 ADDR, CMD0, CMD1,  v0, n-1, matrix,cmatrix);
#endif

    load_matrix((float*)UC6_Matrices[cmatrix]);

    if (cmatrix == 2)
    {
        if (ClearV0)
            v0 = lmatrix;
    }
    else
        v0 = 0;

    if (UseJFG)
    {
        n = (_u8)((CMD0 >> 19) & 0x1F);
        v0 = ((CMD0 >> 9) & 0x1F);
    }

    ClearV0 = FALSE;

    for(i = 0; i < n; i++)
    {
        _u32 ad = a + (i * SIZE_VERTEX_UC06);

        rdp_reg.vtx[v0].x     = (float)(_s16)doReadMemHalfWord(ad+0);
        rdp_reg.vtx[v0].y     = (float)(_s16)doReadMemHalfWord(ad+2);
        rdp_reg.vtx[v0].z     = (float)(_s16)doReadMemHalfWord(ad+4);
        rdp_reg.vtx[v0].s     = 1.0f; 
        rdp_reg.vtx[v0].t     = 1.0f; 
        rdp_reg.vtx[v0].r     = doReadMemByte(ad+6);
        rdp_reg.vtx[v0].g     = doReadMemByte(ad+7);
        rdp_reg.vtx[v0].b     = doReadMemByte(ad+8);
        rdp_reg.vtx[v0].a     = doReadMemByte(ad+9);

#ifdef LOG_ON
        LOG_TO_FILE("   vtx[%02i]: [xyz %04X %04X %04X] -> %12.5f %12.5f %12.5f [%02X,%02X,%02X,%02X]",
              v0, 
              doReadMemHalfWord(ad+0),
              doReadMemHalfWord(ad+2),
              doReadMemHalfWord(ad+4),
              rdp_reg.vtx[v0].x, rdp_reg.vtx[v0].y, rdp_reg.vtx[v0].z,
              rdp_reg.vtx[v0].r, rdp_reg.vtx[v0].g, rdp_reg.vtx[v0].b, rdp_reg.vtx[v0].a);
#endif
        {
            t_vtx *vertex = &rdp_reg.vtx[v0];
            float tmpvtx[4];

            tmpvtx[0] = vertex->x;
            tmpvtx[1] = vertex->y;
            tmpvtx[2] = vertex->z;
            tmpvtx[3] = 1.0f;

            transform_vector2(&tmpvtx[0],vertex->x,vertex->y,vertex->z,vertex->w);

            if (lmatrix)
            {
                tmpvtx[0] += basevertex->x;
                tmpvtx[1] += basevertex->y;
                tmpvtx[2] += basevertex->z;
                tmpvtx[3] += basevertex->w;
            }

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
                math_lightingN((t_vtx*)(char*)vertex, vertex->lcolor);
            }
            else
            {
                vertex->lcolor[0] = (float)vertex->r / 255.0f;
                vertex->lcolor[1] = (float)vertex->g / 255.0f;
                vertex->lcolor[2] = (float)vertex->b / 255.0f;
                vertex->lcolor[3] = (float)vertex->a / 255.0f;
            }
        }

        v0++;
    }
#ifdef LOG_ON
    LOG_TO_FILE("");
#endif
}

extern void DrawVisualTriangle6(int vn[3],float Sc[3],float Tc[3]);

#define SIZE_DMA_TRI 16
//  0-15 = Tribuffer Size
// 16-19 = ???? everytime 1 ???
// 20-23 = Num Tris - 2
void rsp_uc06_tri4()
{
    int vn[3];
    _u8 flag = (_u8)((CMD0 & 0xFF0000) >> 16);
    _u32 i;
    float vtsc[3],vttc[3]; 
    _u32 a = segoffset2addr(rdp_reg.cmd1);
    _u32 num = (CMD0 & 0xFFF0) / SIZE_DMA_TRI;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_DMATRI num:%i at:%08X\n", ADDR, CMD0, CMD1, num, a);
#endif

    flag =  doReadMemByte(a);

    if ((flag & 0x40) == 0)
        Render_geometry_cullback(1);
/*  {
        switch(rdp_reg.geometrymode & 0x00003000)
        {
        case 0x1000:
            Render_geometry_cullfront(1);
            break;
        case 0x2000:
            Render_geometry_cullback(1);
            break;
        case 0x3000:
            Render_geometry_cullfrontback(1);
            break;
        default:
            Render_geometry_cullfrontback(0);
            break;
        }
    }
*/  else
        Render_geometry_cullfrontback(0);

    for (i=0; i<num; i++)
    {
        _u32 ad = a + (i * SIZE_DMA_TRI); 
        _s16 temp;

        temp =  doReadMemByte(ad);
        if (!temp)
            temp = temp;
        vn[0] = doReadMemByte(ad+1);
        vn[1] = doReadMemByte(ad+2);
        vn[2] = doReadMemByte(ad+3);

        vtsc[0] = (float)((_s16)doReadMemHalfWord(ad+4));
        vttc[0] = (float)((_s16)doReadMemHalfWord(ad+6));
        vtsc[1] = (float)((_s16)doReadMemHalfWord(ad+8));
        vttc[1] = (float)((_s16)doReadMemHalfWord(ad+10));
        vtsc[2] = (float)((_s16)doReadMemHalfWord(ad+12));
        vttc[2] = (float)((_s16)doReadMemHalfWord(ad+14));

        DrawVisualTriangle6(vn,vtsc,vttc);

#ifdef LOG_ON
        LOG_TO_FILE("\t(Vertex %i: s =%12.5f t =%12.5f, flags = %02x)\n\t(Vertex %i: s =%12.5f t =%12.5f)\n\t(Vertex %i: s =%12.5f t =%12.5f)\n", vn[0], vtsc[0], vttc[0],temp, vn[1], vtsc[1], vttc[1], vn[2], vtsc[2], vttc[2]);
#endif
    }

    ClearV0 = TRUE;

#ifdef LOG_ON
    LOG_TO_FILE("");
#endif
} // void rsp_uc06_tri4()


//seems to be okay
void rsp_uc06_texture()
{
        int tile = (rdp_reg.cmd0 >> 8)  & 0x07;             //** tile t0
        int tile1 = (tile + 1)  & 0x07;                     //** tile t1
        _u32 mipmap_level = (rdp_reg.cmd0 >> 11) & 0x07;    //** mipmap_level   - not used yet
        _u32 on = (rdp_reg.cmd0 & 0xff);            //** 1: on - 0:off

        float s = (float)((rdp_reg.cmd1 >> 16) & 0xffff);
        float t = (float)((rdp_reg.cmd1      ) & 0xffff);

        t_tile *tmp_tile = &rdp_reg.td[tile];
        t_tile *tmp_tile1 = &rdp_reg.td[tile1];
        tmp_tile->Texture_on = (_u8)on;

        rdp_reg.tile = tile;

        if (s<=1)
        {
            tmp_tile->SScale=1.0f;
            tmp_tile1->SScale=1.0f;
        }
        else
        {
            tmp_tile->SScale=(float)s/65535.f;
            tmp_tile1->SScale=(float)s/65535.f;
        }

        if (t<=1)
        {
            tmp_tile->TScale=1.0f;
            tmp_tile1->TScale=1.0f;
        }
        else
        {
            tmp_tile->TScale=(float)t/65535.f;
            tmp_tile1->TScale=(float)t/65535.f;
        }

        tmp_tile->TScale/=32.f;
        tmp_tile->SScale/=32.f;

        rdp_reg.m_CurTile = tmp_tile1;
        MathTextureScales();

        rdp_reg.m_CurTile = tmp_tile;
        MathTextureScales();

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_TEXTURE", ADDR, CMD0, CMD1);
    LOG_TO_FILE("\tTile = %i, S Scale = %f, T Scale = %f\n", tile, tmp_tile->SScale, tmp_tile->TScale);
#endif
}

void rsp_uc06_popmatrix()
{
    pop_matrix();
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_POPMATRIX \n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_matrix()
{

        _u32   a = segoffset2addr(rdp_reg.cmd1);
        _u8   command =(_u8)((CMD0 >> 22) & 0x3);

        float   m[4][4];
        float   tm[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,100}};

        if (a < 0x2000)
        {
            UseJFG = TRUE;
            a = (CMD1 + Mtrx_Base) & 0x0ffffff;
        }

        hleGetMatrix((float*)&m[0,0], &pRDRAM[a]);

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC6_LOADMTX  at %08X matrix:%i\n", ADDR, CMD0, CMD1, a, command);
#endif

        cmatrix = command;
        command = 0;

        if (UseJFG)
        {
            cmatrix = (_u8)((CMD0 >> 16) & 0x03);
            
            if (cmatrix)
                command = (_u8)((CMD0 >> 16) & 0x80);
        }

        memcpy(UC6_Matrices[cmatrix], &m[0,0], sizeof(m));

        if (command)
            JFG_MultMtrx(UC6_Matrices[cmatrix], UC6_Matrices[0]);

#ifdef LOG_ON
        LOG_TO_FILE(
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\n",
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
                );
#endif
}

void rsp_uc06_displaylist()
{
        _u32 addr = segoffset2addr(CMD1);
        _u8  push = (_u8)(CMD0 >> 16) & 0xff; 

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_DISPLAYLIST ->%08x , push = %02x\n", ADDR, CMD0, CMD1, addr , push);
#endif

        switch(push)
        {
            case 0:   // push: we do a call of the dl 
                rdp_reg.pc_i++;
                if(rdp_reg.pc_i > (RDP_STACK_SIZE-1))
                {
                    return;
                }
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            case 1:   // branch 
                rdp_reg.pc[rdp_reg.pc_i] = addr;    
                break;

            default:
                break;
        } // switch(push) 
}

void rsp_uc06_dlinmem()
{
        _u32 addr = CMD1, ta;
        _u8  push = (_u8)((CMD0 & 0xf0) >> 4); 
        _s8 limit = (_s8)(CMD0 >> 16) & 0xff;
        _u32 resaddr = rdp_reg.pc[rdp_reg.pc_i];

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_DL_INMEM", ADDR, CMD0, CMD1);
    LOG_TO_FILE("\tAddress = %08x, limit = %i \n", addr, limit);
#endif
        do
        {
            rdp_reg.pc[rdp_reg.pc_i] = addr;
            ta = addr >> 2;
            rdp_reg.cmd0 = ((_u32 *)pRDRAM)[ta + 0];
            rdp_reg.cmd1 = ((_u32 *)pRDRAM)[ta + 1];
            rdp_reg.cmd2 = ((_u32 *)pRDRAM)[ta + 3];
            rdp_reg.cmd3 = ((_u32 *)pRDRAM)[ta + 5];

#ifdef LOG_ON
            LOG_TO_FILE("  ");
#endif
            exec_gfx();

            limit--;
            addr += 8;
        } while(limit > 0);

        rdp_reg.pc[rdp_reg.pc_i] = resaddr;

#ifdef LOG_ON
        LOG_TO_FILE("****UC6_DL_INMEM-ends *****\n");
#endif
}


void rsp_uc06_enddl()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_ENDDISPLAYLIST \n", ADDR, CMD0, CMD1);
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



//////////////////////////////////////////////////////////////////////////////
// UCode 6 functions
//////////////////////////////////////////////////////////////////////////////



void rsp_uc06_setgeometrymode()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_SETGEOMETRYMODE", ADDR, CMD0, CMD1);
    LOG_TO_FILE(
                "\t+$%08lx:\n"
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
                rdp_reg.cmd1,
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
        rdp_reg.geometrymode |= rdp_reg.cmd1;

        if(rdp_reg.geometrymode & 0x00000002)
                rdp_reg.geometrymode_textures = 1;

//      if (rdp_reg.geometrymode & 0x00003000) Render_geometry_cullfrontback(0);
/*        switch(rdp_reg.geometrymode & 0x00003000)
        {
        case 0x1000:
            Render_geometry_cullfront(1);
            break;
        case 0x2000:
            Render_geometry_cullback(1);
            break;
        case 0x3000:
            Render_geometry_cullfrontback(1);
            break;
        default:
            Render_geometry_cullfrontback(0);
            break;
        } //** switch(rdp_reg.geometrymode & 0x00003000) 
*/

//      if((rdp_reg.geometrymode & 0x00020000) != 0)
//              rdp_reg.useLights = 1;

}

static void rsp_uc06_moveword()
{
//        int i;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC6_MOVEWORD", 
                ADDR, CMD0, CMD1);
#endif
        switch(rdp_reg.cmd0 & 0xff)
        {
            case 0x00:
                break;

            case 0x02:
                rdp_reg.lights = (rdp_reg.cmd1 & 0x07);
                rdp_reg.ambient_light =  rdp_reg.lights;

                lmatrix = (_u8)(CMD1 & 1);

#ifdef LOG_ON
                LOG_TO_FILE("\tNumber Lights = %03i", rdp_reg.lights);
#endif
                //calculate_light_vectors();
                break;

            case 0x04:
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

            case 0x06:
                {
                _u8 offset = (_u8)((CMD0 >> 10) & 0xf);
#ifdef LOG_ON
                LOG_TO_FILE("\tSegment[%i]=%08X", offset, CMD1);
#endif
                rdp_reg.segment[offset] = CMD1;
                }
                break;

            case 0x08:
                {
                    float fo,fm,min,max,rng;
                    //(_SHIFTL(fm,16,16) | _SHIFTL(fo,0,16)))

                    //(_SHIFTL((128000/((max)-(min))),16,16) |
                    //_SHIFTL(((500-(min))*256/((max)-(min))),0,16)))

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

            case 0x0a:
                cmatrix = (_u8)(CMD1 >> 6);

#ifdef LOG_ON
                LOG_TO_FILE("\tcMatrix = %i, Color = %08X", cmatrix, CMD1);
#endif
                break;

            case 0x0c:
#ifdef LOG_ON
                LOG_TO_FILE("\t****** 0x0c ******");
#endif
                break;

            case 0x0e:
                rdp_reg.perspnorm = (float)(rdp_reg.cmd1 & 0xFFFF) / 65535.0f;
#ifdef LOG_ON
                LOG_TO_FILE("\tPerspect normal = %f", rdp_reg.perspnorm);
#endif
                break;

            default:
                break;
        } /* switch(rdp_reg.cmd0 & 0xff) */
#ifdef LOG_ON
    LOG_TO_FILE("");
#endif
} /* static void rsp_uc06_moveword() */



void rsp_uc06_setothermode_l()
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

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC6_SETOTHERMODE_L", 
                ADDR, CMD0, CMD1);
#endif
        switch((rdp_reg.cmd0 >> 8) & 0xff)
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
                LOG_TO_FILE("\tRENDERMODE: $%08lx:\n", rdp_reg.cmd1 & 0xfffffff8,
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
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n"
                        "\t%s\n",
                        rdp_reg.cmd1 & 0xfffffff8,
                        (rdp_reg.cmd1 & 0x00000008) ? "\t        anti alias\n" : "",
                        (rdp_reg.cmd1 & 0x00000010) ? "\t        z_cmp\n" : "",
                        (rdp_reg.cmd1 & 0x00000020) ? "\t        z_upd\n" : "",
                        (rdp_reg.cmd1 & 0x00000040) ? "\t        im_rd\n" : "",
                        (rdp_reg.cmd1 & 0x00000080) ? "\t        clr_on_cvg\n" : "",
                        (rdp_reg.cmd1 & 0x00000100) ? "\t        cvg_dst_warp\n" : "",
                        (rdp_reg.cmd1 & 0x00000200) ? "\t        cvg_dst_full\n" : "",
                        (rdp_reg.cmd1 & 0x00000400) ? "\t        z_inter\n" : "",
                        (rdp_reg.cmd1 & 0x00000800) ? "\t        z_xlu\n" : "",
                        (rdp_reg.cmd1 & 0x00001000) ? "\t        cvg_x_alpha\n" : "",
                        (rdp_reg.cmd1 & 0x00002000) ? "\t        alpha_cvg_sel\n" : "",
                        (rdp_reg.cmd1 & 0x00004000) ? "\t        force_bl\n" : "",
                        (rdp_reg.cmd1 & 0x00008000) ? "\t        tex_edge?\n" : "",
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

            case 0x16:
#ifdef LOG_ON
                LOG_TO_FILE("\tBLENDER\n");
#endif
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN?\n");
#endif
                break;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */

//  LOG_TO_FILE("%08X: %08X %08X CMD UC6_SETOTHERMODE_L NC\n", ADDR, CMD0, CMD1);

        if (rdp_reg.mode_l & 0x00000010)
            Render_geometry_zbuffer(1);
        else
            Render_geometry_zbuffer(0);

        if (rdp_reg.mode_l & 0x00000020)
            Render_geometry_zwrite(1);
        else
            Render_geometry_zwrite(0);

//  switch (rdp_reg.mode_l & 0x00000003)
//  {
//  case 0:
//  case 2:
//      Src_Alpha = GL_ONE;
//      Dst_Alpha = GL_ZERO;
//      break;
//  case 1:
//  case 3:
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
//      break;
//  }
}

void rsp_uc06_setothermode_h()
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


#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC6_SETOTHERMODE_H", ADDR, CMD0, CMD1);
#endif

        switch((rdp_reg.cmd0 >> 8) & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                LOG_TO_FILE("\tBLENDMASK - ignored\n");
#endif
                break;

            case 0x04:
#ifdef LOG_ON
                LOG_TO_FILE("\tALPHADITHER: %s\n", ad[(rdp_reg.cmd1>>0x04) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00000030;
                rdp_reg.cmd1   &=  0x00000030;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x06:
#ifdef LOG_ON
                LOG_TO_FILE("\tRGBDITHER: %s\n", rd[(rdp_reg.cmd1>>0x06) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x000000c0;
                rdp_reg.cmd1   &=  0x000000c0;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x08:
#ifdef LOG_ON
                LOG_TO_FILE("\tCOMBINEKEY: %s\n", ck[(rdp_reg.cmd1>>0x08) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00000100;
                rdp_reg.cmd1   &=  0x00000100;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
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
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREFILTER: %s\n", tf[(rdp_reg.cmd1>>0x0c) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00003000;
                rdp_reg.cmd1   &=  0x00003000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x0e:
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTURELUT: %s\n", tt[(rdp_reg.cmd1>>0x0e) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x0000c000;
                rdp_reg.cmd1   &=  0x0000c000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x10:
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTURELOD: %s\n", tl[(rdp_reg.cmd1>>0x10) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00010000;
                rdp_reg.cmd1   &=  0x00010000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x11:
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREDETAIL: %s\n", td[(rdp_reg.cmd1>>0x11) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00060000;
                rdp_reg.cmd1   &=  0x00060000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x13:
#ifdef LOG_ON
                LOG_TO_FILE("\tTEXTUREPERSP: %s\n", tp[(rdp_reg.cmd1>>0x13) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00080000;
                rdp_reg.cmd1   &=  0x00080000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x14:
#ifdef LOG_ON
                LOG_TO_FILE("\tCYCLETYPE: %s\n", ct[(rdp_reg.cmd1>>0x14) & 0x3]);
#endif
                cycle_mode = (_u8)((rdp_reg.cmd1>>0x14) & 0x3);
                rdp_reg.mode_h &= ~0x00300000;
                rdp_reg.cmd1   &=  0x00300000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x16:
#ifdef LOG_ON
                LOG_TO_FILE("\tCOLORDITHER: %s\n", cd[(rdp_reg.cmd1>>0x16) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00400000;
                rdp_reg.cmd1   &=  0x00400000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x17:
#ifdef LOG_ON
                LOG_TO_FILE("\tPIPELINEMODE: %s\n", pm[(rdp_reg.cmd1>>0x17) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00800000;
                rdp_reg.cmd1   &=  0x00800000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN\n");
#endif
                break;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */

}

static void rsp_uc06_movemem()
{

        _u32 a;
        int   i = ((rdp_reg.cmd0 >> 16) & 0xff);

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_MOVEMEM", ADDR, CMD0, CMD1);
#endif
    switch((rdp_reg.cmd0 >> 16) & 0xff)
        {
            case 0x80:
                a = segoffset2addr(rdp_reg.cmd1) >> 1;

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
                break;

            case 0x82:
                a = segoffset2addr(rdp_reg.cmd1);

                rdp_reg.lookat_y.r         = ((float)((_u8 *)pRDRAM)[(a+ 0)^3])/255.0f;
                rdp_reg.lookat_y.g         = ((float)((_u8 *)pRDRAM)[(a+ 1)^3])/255.0f;
                rdp_reg.lookat_y.b         = ((float)((_u8 *)pRDRAM)[(a+ 2)^3])/255.0f;
                rdp_reg.lookat_y.a         = 1.0f;
                rdp_reg.lookat_y.r_copy    = ((float)((_u8 *)pRDRAM)[(a+ 4)^3])/255.0f;
                rdp_reg.lookat_y.g_copy    = ((float)((_u8 *)pRDRAM)[(a+ 5)^3])/255.0f;
                rdp_reg.lookat_y.b_copy    = ((float)((_u8 *)pRDRAM)[(a+ 6)^3])/255.0f;
                rdp_reg.lookat_y.a_copy    = 1.0f;
                rdp_reg.lookat_y.x         = ((float)((_u8 *)pRDRAM)[(a+ 8)^3])/255.0f;
                rdp_reg.lookat_y.y         = ((float)((_u8 *)pRDRAM)[(a+ 9)^3])/255.0f;
                rdp_reg.lookat_y.z         = ((float)((_u8 *)pRDRAM)[(a+10)^3])/255.0f;
                rdp_reg.lookat_y.w         = 1.0f;

                Render_lookat_y();
#ifdef LOG_ON
                LOG_TO_FILE("\tLookat y \n");
#endif
                break;

            case 0x84:
                a = segoffset2addr(rdp_reg.cmd1);

                rdp_reg.lookat_x.r         = ((float)((_u8 *)pRDRAM)[(a+ 0)^3])/255.0f;
                rdp_reg.lookat_x.g         = ((float)((_u8 *)pRDRAM)[(a+ 1)^3])/255.0f;
                rdp_reg.lookat_x.b         = ((float)((_u8 *)pRDRAM)[(a+ 2)^3])/255.0f;
                rdp_reg.lookat_x.a         = 1.0f;
                rdp_reg.lookat_x.r_copy    = ((float)((_u8 *)pRDRAM)[(a+ 4)^3])/255.0f;
                rdp_reg.lookat_x.g_copy    = ((float)((_u8 *)pRDRAM)[(a+ 5)^3])/255.0f;
                rdp_reg.lookat_x.b_copy    = ((float)((_u8 *)pRDRAM)[(a+ 6)^3])/255.0f;
                rdp_reg.lookat_x.a_copy    = 1.0f;
                rdp_reg.lookat_x.x         = ((float)((_s8 *)pRDRAM)[(a+ 8)^3])/128.0f;
                rdp_reg.lookat_x.y         = ((float)((_s8 *)pRDRAM)[(a+ 9)^3])/128.0f;
                rdp_reg.lookat_x.z         = ((float)((_s8 *)pRDRAM)[(a+10)^3])/128.0f;
                rdp_reg.lookat_x.w         = 1.0f;

                Render_lookat_x();
#ifdef LOG_ON
                LOG_TO_FILE("\tLookat x \n");
#endif
                break;

            case 0x86:
            case 0x88:
            case 0x8a:
            case 0x8c:
            case 0x8e:
            case 0x90:
            case 0x92:
            case 0x94:
                i = (((rdp_reg.cmd0 >> 16) & 0xff) - 0x86) >> 1;
                a = segoffset2addr(rdp_reg.cmd1);

                rdp_reg.light[i].r         = ((float)((_u8 *)pRDRAM)[(a+ 0)^3])/255.0f;
                rdp_reg.light[i].g         = ((float)((_u8 *)pRDRAM)[(a+ 1)^3])/255.0f;
                rdp_reg.light[i].b         = ((float)((_u8 *)pRDRAM)[(a+ 2)^3])/255.0f;
                rdp_reg.light[i].a         = 1.0f;
                rdp_reg.light[i].r_copy    = ((float)((_u8 *)pRDRAM)[(a+ 4)^3])/255.0f;
                rdp_reg.light[i].g_copy    = ((float)((_u8 *)pRDRAM)[(a+ 5)^3])/255.0f;
                rdp_reg.light[i].b_copy    = ((float)((_u8 *)pRDRAM)[(a+ 6)^3])/255.0f;
                rdp_reg.light[i].a_copy    = 1.0f;
                rdp_reg.light[i].x         = ((float)((_s8 *)pRDRAM)[(a+ 8)^3])/127.0f;
                rdp_reg.light[i].y         = ((float)((_s8 *)pRDRAM)[(a+ 9)^3])/127.0f;
                rdp_reg.light[i].z         = ((float)((_s8 *)pRDRAM)[(a+10)^3])/127.0f;
                rdp_reg.light[i].w         = 1.0f;

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
                break;

            case 0x98:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN %03x",i);
#endif
                break;

            case 0x9a:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN %03x",i);
#endif
                break;

            case 0x9c:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN %03x",i);
#endif
                break;

            case 0x9e:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN %03x",i);
#endif
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN %03x",i);
#endif
                break;

        } // switch((rdp_reg.cmd0 >> 8) & 0xffff) 

} // static void rsp_uc06_movemem() 

static void rsp_uc06_cleargeometrymode()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_CLEARGEOMETRYMODE", ADDR, CMD0, CMD1);
    LOG_TO_FILE(
                "\t+$%08lx:\n"
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
                rdp_reg.cmd1,
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
        rdp_reg.geometrymode &= ~rdp_reg.cmd1;

//        if(rdp_reg.cmd1 & 0x00000002)
//                rdp_reg.geometrymode_textures = 0;

//      if ((rdp_reg.geometrymode & 0x00020000) != 0)
//              rdp_reg.useLights = 0;

//      if (rdp_reg.geometrymode & 0x00003000) Render_geometry_cullfrontback(0);
/*        switch(rdp_reg.geometrymode & 0x00003000)
        {
/*      case 0x1000:
            Render_geometry_cullfront(1);
            break;
        case 0x2000:
            Render_geometry_cullback(1);
            break;
        case 0x3000:
            Render_geometry_cullfrontback(1);
            break;
* /     default:
            Render_geometry_cullfrontback(0);
            break;
        } //** switch(rdp_reg.geometrymode & 0x00003000) 
*/      
} /* static void rsp_uc00_cleargeometrymode() */




//////////////////////////////////////////////////////////////////////////////
// Unimplemented functions
//////////////////////////////////////////////////////////////////////////////


void rsp_uc06_rdphalf_cont()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_RDPHALF_CONT NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_rdphalf_2()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_RDPHALF_2 NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_line3d()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_LINE3d NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_sprite2d()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_SPRITE2D NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_culldl()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_CULLDL NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_tri1()
{
    Mtrx_Base = segoffset2addr(CMD0 & 0x0ffffff);
    Vrtx_Base = segoffset2addr(CMD1 & 0x0ffffff);
    UseJFG = TRUE;
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_TRI1 NI\n", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc06_rdphalf_1()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC6_RDPHALF_1 DATA\n", ADDR, CMD0, CMD1);
#endif
}


