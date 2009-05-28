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
// Perfect Darkness                                                         //
// Goldeneye                                                                //
//////////////////////////////////////////////////////////////////////////////

void rsp_uc05_matrix();
void rsp_uc05_movemem();
void rsp_uc05_vertex();
void rsp_uc05_displaylist();
void rsp_uc05_sprite2d();

void rsp_uc05_tri4();
void rsp_uc05_rdphalf_cont();
void rsp_uc05_rdphalf_2();
void rsp_uc05_rdphalf_1();
void rsp_uc05_line3d();
void rsp_uc05_cleargeometrymode();
void rsp_uc05_setgeometrymode();
void rsp_uc05_enddl();
void rsp_uc05_setothermode_l();
void rsp_uc05_setothermode_h();
void rsp_uc05_texture();
void rsp_uc05_moveword();
void rsp_uc05_popmatrix();
void rsp_uc05_culldl();
void rsp_uc05_tri1();



//////////////////////////////////////////////////////////////////////////////
// structure is tricky for changing the endians                             //
//////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    _s16 y;
    _s16 x;
    _u16 idx;

    _s16 z;

    _s16 t;
    _s16 s;
} t_vtx_uc5;


//////////////////////////////////////////////////////////////////////////////
// LoadVertex                                                               //
//////////////////////////////////////////////////////////////////////////////
void rsp_uc05_vertex()
{
    _u32    a = segoffset2addr(rdp_reg.cmd1);
    int     v0, i, n;
//  int  len;
    int offset = sizeof(t_vtx_uc5);
//  _s16 *Coord;
    _u32 cIdx;

    v0 = (CMD0 & 0x0F0000) >> 16;
    n = ((CMD0 & 0xF00000) >> 20) + 1;

    if (refresh_matrix) update_cmbmatrix();

    if ((CMD0 & 0xFFF00) != 0)
    {
        int t = 0;
    }

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_LOADVTX  vertex %i..%i",
                 ADDR, CMD0, CMD1,  v0, (v0+n-1));
    LOG_TO_FILE("\t Lighting: %s\n",
        (0x00020000 & rdp_reg.geometrymode)?"on":"off");
#endif

    for(i = 0; i < n; i++)
    {
        t_vtx_uc5 *vtx = (t_vtx_uc5 *)&pRDRAM[a+(i*offset)];

        rdp_reg.vtx[v0].x     = (float)vtx->x;
        rdp_reg.vtx[v0].y     = (float)vtx->y;
        rdp_reg.vtx[v0].z     = (float)vtx->z;

        rdp_reg.vtx[v0].s     = (float)vtx->s;
        rdp_reg.vtx[v0].t     = (float)vtx->t;

        cIdx                  = rdp_reg.ColorInfoAdd + (vtx->idx &0xff);

        rdp_reg.vtx[v0].a     = (_u8)pRDRAM[(cIdx)];
        rdp_reg.vtx[v0].b     = (_u8)pRDRAM[(cIdx+1)];
        rdp_reg.vtx[v0].g     = (_u8)pRDRAM[(cIdx+2)];
        rdp_reg.vtx[v0].r     = (_u8)pRDRAM[(cIdx+3)];

        {
            t_vtx *vertex = &rdp_reg.vtx[v0];
            float tmpvtx[4];
            tmpvtx[0] = vertex->x;
            tmpvtx[1] = vertex->y;
            tmpvtx[2] = vertex->z;
            tmpvtx[3] = 1.0f;

            cmbtrans_vector(&tmpvtx[0]);

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
//          if (rdp_reg.ColorMode)
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

            if (0x00040000 & rdp_reg.geometrymode)
            {
                float tVtr[3] = {vertex->x, vertex->y, vertex->z};

                NormalizeVector(tVtr);

                // G_TEXTURE_GEN enable the automatic generation of the texture
                // coordinates s and t.  A spherical mapping is used, based on the normal.

                vertex->PDn1 = tVtr[0];
                vertex->PDn2 = tVtr[1];
                vertex->PDn3 = tVtr[2];
            }

        }
#ifdef LOG_ON
        LOG_TO_FILE("\tvtx[%02i]: [xyz %04X %04X %04X] -> %12.5f %12.5f %12.5f\n"
                    "\ts=%9.4f, t=%9.4f\n\tcolor   [%02X,%02X,%02X]\n"
                    "\tnormals [%12.5f,%12.f,%12.5f]\n\tcoloridx = 0x%04X\n",
              v0, vtx->x, vtx->y, vtx->z, 
              rdp_reg.vtx[v0].x, rdp_reg.vtx[v0].y, rdp_reg.vtx[v0].z,
              rdp_reg.vtx[v0].s, rdp_reg.vtx[v0].t,
              rdp_reg.vtx[v0].r, rdp_reg.vtx[v0].g, rdp_reg.vtx[v0].b,
              rdp_reg.vtx[v0].n1, rdp_reg.vtx[v0].n2, rdp_reg.vtx[v0].n3,(vtx->idx &0xffff));
#endif
        v0++;
    }
#ifdef LOG_ON
//  LOG_TO_FILE("");
#endif
}

extern float VtSc[256];
extern float VtTc[256];

void rsp_uc05_tri4()
{
    int     vn[3];
    float   v1[3],v2[3],v3[3],vr[3];
    t_vtx* vertex1;
    t_vtx* vertex2;
    t_vtx* vertex3;

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_TRI4", ADDR, CMD0, CMD1);
#endif
    
    vn[2] = ((CMD1 & 0xf0) >> 4);
    vn[1] = ((CMD1 & 0x0f) >> 0);
    vn[0] = ((CMD0 & 0x0f) >> 0);

    if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
    {
        if (0x00040000 & rdp_reg.geometrymode)
        {
            vertex1 = &rdp_reg.vtx[vn[0]];
            vertex2 = &rdp_reg.vtx[vn[1]];
            vertex3 = &rdp_reg.vtx[vn[2]];

            v1[0] = vertex1->x - vertex2->x;
            v1[1] = vertex1->y - vertex2->y;
            v1[2] = vertex1->z - vertex2->z;
            NormalizeVector(v1);

            v2[0] = vertex3->x - vertex2->x;
            v2[1] = vertex3->y - vertex2->y;
            v2[2] = vertex3->z - vertex2->z;
            NormalizeVector(v2);

            CrossProduct(v1,v2,v3);
            NormalizeVector(v3);

            v1[0] = vertex1->x;
            v1[1] = vertex1->y;
            v1[2] = vertex1->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex2->x;
            v1[1] = vertex2->y;
            v1[2] = vertex2->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+1]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+1]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex3->x;
            v1[1] = vertex3->y;
            v1[2] = vertex3->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+2]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+2]   = (0.5f - (vr[1]*.5f));
        }

        DrawVisualTriangle(vn);
//      Render_triangle(vn);
#ifdef LOG_ON
        LOG_TO_FILE("\t(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
    }

    vn[2] = ((CMD1 & 0xf000) >> 12);
    vn[1] = ((CMD1 & 0x0f00) >>  8);
    vn[0] = ((CMD0 & 0xf0)   >>  4);
    if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
    {
        if (0x00040000 & rdp_reg.geometrymode)
        {
            vertex1 = &rdp_reg.vtx[vn[0]];
            vertex2 = &rdp_reg.vtx[vn[1]];
            vertex3 = &rdp_reg.vtx[vn[2]];

            v1[0] = vertex1->x - vertex2->x;
            v1[1] = vertex1->y - vertex2->y;
            v1[2] = vertex1->z - vertex2->z;
            NormalizeVector(v1);

            v2[0] = vertex3->x - vertex2->x;
            v2[1] = vertex3->y - vertex2->y;
            v2[2] = vertex3->z - vertex2->z;
            NormalizeVector(v2);

            CrossProduct(v1,v2,v3);
            NormalizeVector(v3);

            v1[0] = vertex1->x;
            v1[1] = vertex1->y;
            v1[2] = vertex1->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex2->x;
            v1[1] = vertex2->y;
            v1[2] = vertex2->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+1]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+1]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex3->x;
            v1[1] = vertex3->y;
            v1[2] = vertex3->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+2]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+2]   = (0.5f - (vr[1]*.5f));
        }

        DrawVisualTriangle(vn);
//      Render_triangle(vn);
#ifdef LOG_ON
        LOG_TO_FILE("\t(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
    }

    vn[2] = ((CMD1 & 0xf00000) >> 20);
    vn[1] = ((CMD1 & 0x0f0000) >> 16);
    vn[0] = ((CMD0 & 0x0f00)   >>  8);
    if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
    {
        if (0x00040000 & rdp_reg.geometrymode)
        {
            vertex1 = &rdp_reg.vtx[vn[0]];
            vertex2 = &rdp_reg.vtx[vn[1]];
            vertex3 = &rdp_reg.vtx[vn[2]];

            v1[0] = vertex1->x - vertex2->x;
            v1[1] = vertex1->y - vertex2->y;
            v1[2] = vertex1->z - vertex2->z;
            NormalizeVector(v1);

            v2[0] = vertex3->x - vertex2->x;
            v2[1] = vertex3->y - vertex2->y;
            v2[2] = vertex3->z - vertex2->z;
            NormalizeVector(v2);

            CrossProduct(v1,v2,v3);
            NormalizeVector(v3);

            v1[0] = vertex1->x;
            v1[1] = vertex1->y;
            v1[2] = vertex1->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex2->x;
            v1[1] = vertex2->y;
            v1[2] = vertex2->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+1]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+1]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex3->x;
            v1[1] = vertex3->y;
            v1[2] = vertex3->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+2]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+2]   = (0.5f - (vr[1]*.5f));
        }

        DrawVisualTriangle(vn);
#ifdef LOG_ON
        LOG_TO_FILE("\t(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
    }

    vn[2] = ((CMD1 & 0xf0000000) >> 28);
    vn[1] = ((CMD1 & 0x0f000000) >> 24);
    vn[0] = ((CMD0 & 0xf000)     >> 12);
    if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
    {
        if (0x00040000 & rdp_reg.geometrymode)
        {
            vertex1 = &rdp_reg.vtx[vn[0]];
            vertex2 = &rdp_reg.vtx[vn[1]];
            vertex3 = &rdp_reg.vtx[vn[2]];

            v1[0] = vertex1->x - vertex2->x;
            v1[1] = vertex1->y - vertex2->y;
            v1[2] = vertex1->z - vertex2->z;
            NormalizeVector(v1);

            v2[0] = vertex3->x - vertex2->x;
            v2[1] = vertex3->y - vertex2->y;
            v2[2] = vertex3->z - vertex2->z;
            NormalizeVector(v2);

            CrossProduct(v1,v2,v3);
            NormalizeVector(v3);

            v1[0] = vertex1->x;
            v1[1] = vertex1->y;
            v1[2] = vertex1->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex2->x;
            v1[1] = vertex2->y;
            v1[2] = vertex2->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+1]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+1]   = (0.5f - (vr[1]*.5f));

            v1[0] = vertex3->x;
            v1[1] = vertex3->y;
            v1[2] = vertex3->z;
            NormalizeVector(v1);

            reflect_vector(v1,v3,vr);
            NormalizeVector(vr);
            VtSc[Vtcnt+2]   = (0.5f + (vr[0]*.5f));
            VtTc[Vtcnt+2]   = (0.5f - (vr[1]*.5f));
        }

        DrawVisualTriangle(vn);
//      Render_triangle(vn);
#ifdef LOG_ON
        LOG_TO_FILE("\t(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
    }
#ifdef LOG_ON
    LOG_TO_FILE("");
#endif
} // void rsp_uc05_tri4()


//seems to be okay
void rsp_uc05_texture()
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
            s = 1.0f;
        else
            s /=65535.f;
        
        tmp_tile->SScale=s;
        tmp_tile1->SScale=s;

        if (t<=1)
            t = 1.0f;
        else
            t /= 65535.f;

        tmp_tile->TScale=t;
        tmp_tile1->TScale=t;

        tmp_tile->TScale/=32.f;
        tmp_tile->SScale/=32.f;

        rdp_reg.m_CurTile = tmp_tile1;
        MathTextureScales();

        rdp_reg.m_CurTile = tmp_tile;
        MathTextureScales();

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_TEXTURE NC", ADDR, CMD0, CMD1);
    LOG_TO_FILE("\tTile SScale = %2.5f TScale = %2.5f\n", tile, s, t);
#endif
}

void rsp_uc05_popmatrix()
{
//  _u32 param = (CMD1 / 64);


//  while(param)
//  {
        //Render_pop_modelview();
        pop_matrix();
//      param--;
//  }

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_POPMATRIX\n", ADDR, CMD0, CMD1);
#endif
}


void rsp_uc05_matrix()
{

        _u32   a = segoffset2addr(rdp_reg.cmd1);
        _u8   command =(_u8)((CMD0 & 0xF0000) >> 16);
        float   m[4][4];
//        int     i, j;

        hleGetMatrix((float*)&m[0,0], &pRDRAM[a]);
        refresh_lights = TRUE;

#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_LOADMTX  at %08X ", ADDR, CMD0, CMD1, a);
#endif

         switch(command)
        {

            case 0: // modelview  mul  nopush 
#ifdef LOG_ON
                LOG_TO_FILE("\tmodelview  mul  nopush\n");
#endif
                mult_matrix((GLfloat *)m);
                break;

            case 1: // projection mul  nopush 
            case 5: // projection mul  push   
#ifdef LOG_ON
                LOG_TO_FILE("\tprojection mul  nopush\n");
#endif
                //Render_mul_projection(m);
                mult_prj_matrix((GLfloat *)m);
                break;

            case 2: // modelview  load nopush 
#ifdef LOG_ON
                LOG_TO_FILE("\tmodelview  load nopush\n");
#endif
                load_matrix((GLfloat *)m);
                break;

            case 3: // projection load nopush 
            case 7: // projection load push   
#ifdef LOG_ON
                LOG_TO_FILE("\tprojection load nopush\n");
#endif
                //glLoadMatrixf((GLfloat *)m);
                load_prj_matrix((GLfloat *)m);
                break;

            case 4: // modelview  mul  nopush 
#ifdef LOG_ON
                LOG_TO_FILE("\tmodelview  mul  nopush\n");
#endif
                push_mult_matrix((GLfloat *)m);
                break;

            case 6: // modelview  load push   
#ifdef LOG_ON
                LOG_TO_FILE("\tmodelview  load push\n");
#endif
                push_load_matrix((GLfloat *)m);
                break;


            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tunknown Command %x\n", command);
#endif
                break;
        } /* switch(command) */

        //
#ifdef LOG_ON
        LOG_TO_FILE(
                "\tLoad matrix\n"
                "\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\t{ %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n",
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
                );
#endif
}

void rsp_uc05_displaylist()
{
        _u32 addr = segoffset2addr(CMD1);
        _u8  push = (_u8)(CMD0 >> 16) & 0xff; 

#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_DISPLAYLIST ", ADDR, CMD0, CMD1);
#endif
        switch(push)
        {
            case 0:   // push: we do a call of the dl 
                rdp_reg.pc_i++;
#ifdef LOG_ON
    LOG_TO_FILE("\tcall -> %08x level(%d)\n", addr, rdp_reg.pc_i);
#endif
                if(rdp_reg.pc_i > (RDP_STACK_SIZE-1))
                {
                    return;
                }
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            case 1:   // branch 
#ifdef LOG_ON
    LOG_TO_FILE("\tbranch -> %08x\n", addr);
#endif
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            default:
#ifdef LOG_ON
    LOG_TO_FILE("\tUnknow DList command -> %08x level(%d)\n", addr, rdp_reg.pc_i);
#endif
//                DebugBox("Unknow DList command");
                break;

        } // switch(push) 

}

void rsp_uc05_enddl()
{
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
#ifdef LOG_ON
    {
        _u32 addr = rdp_reg.pc[rdp_reg.pc_i];
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_ENDDISPLAYLIST ", (rdp_reg.pc[rdp_reg.pc_i+1] - 8), CMD0, CMD1);
        if (rdp_reg.halt)
            LOG_TO_FILE("\tHalt\n");
        else
            LOG_TO_FILE("\treturn -> %08x level(%d)\n", addr, rdp_reg.pc_i);
    }
#endif
}






//////////////////////////////////////////////////////////////////////////////
// UCode 1 functions
//////////////////////////////////////////////////////////////////////////////



void rsp_uc05_setgeometrymode()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_SETGEOMETRYMODE", ADDR, CMD0, CMD1);
        LOG_TO_FILE(
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
                (rdp_reg.cmd1 & 0x00000001) ? "\tzbuffer\n" : "",
                (rdp_reg.cmd1 & 0x00000002) ? "\ttexture\n" : "",
                (rdp_reg.cmd1 & 0x00000004) ? "\tshade\n" : "",
                (rdp_reg.cmd1 & 0x00000200) ? "\tshade smooth\n" : "",
                (rdp_reg.cmd1 & 0x00001000) ? "\tcull front\n" : "",
                (rdp_reg.cmd1 & 0x00002000) ? "\tcull back\n" : "",
                (rdp_reg.cmd1 & 0x00010000) ? "\tfog\n" : "",
                (rdp_reg.cmd1 & 0x00020000) ? "\tlightning\n" : "",
                (rdp_reg.cmd1 & 0x00040000) ? "\ttexture gen\n" : "",
                (rdp_reg.cmd1 & 0x00080000) ? "\ttexture gen lin\n" : "",
                (rdp_reg.cmd1 & 0x00100000) ? "\tlod\n" : ""
                );
#endif
        rdp_reg.geometrymode |= rdp_reg.cmd1;
//        rdp_reg.geometrymode |= 0x00020000;

//        if(rdp_reg.geometrymode & 0x00000002)
//                rdp_reg.geometrymode_textures = 1;

//      if (rdp_reg.geometrymode & 0x00003000) Render_geometry_cullfrontback(0);
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
        } //** switch(rdp_reg.geometrymode & 0x00003000) 


//      if((rdp_reg.geometrymode & 0x00020000) != 0)
//              rdp_reg.useLights = 1;

}

static void rsp_uc05_moveword()
{
        int i;
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_MOVEWORD ", ADDR, CMD0, CMD1);
#endif
        switch(rdp_reg.cmd0 & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                LOG_TO_FILE("\tMATRIX");
#endif                
                break;

            case 0x02:
                rdp_reg.lights = (rdp_reg.cmd1 - 0x80000000) / 32 - 1;
                rdp_reg.ambient_light =  rdp_reg.lights;
                //rdp_reg.geometrymode |= 0x00020000;
                //calculate_light_vectors();
#ifdef LOG_ON
                LOG_TO_FILE("\tNUMLIGHT: ");
                LOG_TO_FILE("\tlights: %d\n", rdp_reg.lights);
#endif
                break;

            case 0x04:
#ifdef LOG_ON
                PRINT_RDP_INFO("\tCLIP: ");
#endif
                switch((rdp_reg.cmd0 >> 8) & 0xffff)
                {
                    case 0x0004:
#ifdef LOG_ON
                        PRINT_RDP_MNEMONIC("\t-x = ");
#endif
                        rdp_reg.clip.nx = rdp_reg.cmd1;
                        break;

                    case 0x000c:
#ifdef LOG_ON
                        PRINT_RDP_MNEMONIC("\t-y = ");
#endif
                        rdp_reg.clip.ny = rdp_reg.cmd1;
                        break;

                    case 0x0014:
#ifdef LOG_ON
                        PRINT_RDP_MNEMONIC("\t+x = ");
#endif
                        rdp_reg.clip.px = rdp_reg.cmd1;
                        break;

                    case 0x001c:
#ifdef LOG_ON
                        PRINT_RDP_MNEMONIC("\t+y = ");
#endif
                        rdp_reg.clip.py = rdp_reg.cmd1;
                        break;

                    default:
#ifdef LOG_ON
                        PRINT_RDP_NOT_IMPLEMENTED("\tCLIP - wrong offset\n");
#endif
                        ;

                } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */
                
#ifdef LOG_ON
                LOG_TO_FILE("\t%d", (_s32)(_s16)rdp_reg.cmd1);
                LOG_TO_FILE("\tignored\n");
#endif
                break;

            case 0x06:
#ifdef LOG_ON
                LOG_TO_FILE("\tSEGMENT: ");
                LOG_TO_FILE("\t$%08lx -> seg#%d \n", CMD1, (CMD0 >> 10) & 0xf);
#endif
                rdp_reg.segment[(rdp_reg.cmd0 >> 10) & 0xf] = rdp_reg.cmd1;
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
                    LOG_TO_FILE("\tFog min = %f, max = %f\n",min,max);
#endif
                }
                break;

            case 0x0a:
                i = (rdp_reg.cmd0 & 0x0000e000) >> 13;

                if(rdp_reg.cmd0 & 0x00000400)
                {
#ifdef LOG_ON
                        LOG_TO_FILE("\tLIGHTCOL ");
#endif
                        rdp_reg.light[i].r         = ((float)((rdp_reg.cmd1 >> 24) & 0xff))/255.0f;
                        rdp_reg.light[i].g         = ((float)((rdp_reg.cmd1 >> 16) & 0xff))/255.0f;
                        rdp_reg.light[i].b         = ((float)((rdp_reg.cmd1 >>  8) & 0xff))/255.0f;
                        rdp_reg.light[i].a         = 1.0f;

                }
                else
                {
#ifdef LOG_ON
                        LOG_TO_FILE("\tLIGHTCOL (copy) ");
#endif
                        rdp_reg.light[i].r_copy    = ((float)((rdp_reg.cmd1 >> 24) & 0xff))/255.0f;
                        rdp_reg.light[i].g_copy    = ((float)((rdp_reg.cmd1 >> 16) & 0xff))/255.0f;
                        rdp_reg.light[i].b_copy    = ((float)((rdp_reg.cmd1 >>  8) & 0xff))/255.0f;
                        rdp_reg.light[i].a_copy    = 1.0f;
                }

#ifdef LOG_ON
                LOG_TO_FILE("%d: rgb?=$%08lx", i, rdp_reg.cmd1);
                LOG_TO_FILE(
                        "        rgba=%04.2f,%04.2f,%04.2f,%04.2f\n",
                        ((float)((rdp_reg.cmd1 >> 24) & 0xff))/255.0f,
                        ((float)((rdp_reg.cmd1 >> 16) & 0xff))/255.0f,
                        ((float)((rdp_reg.cmd1 >>  8) & 0xff))/255.0f,
                        1.0f
                        );
#endif
                //calculate_light_vectors();
                break;

            case 0x0c:
#ifdef LOG_ON
                LOG_TO_FILE("\tPOINTS\n");
#endif                
                break;

            case 0x0e:
                rdp_reg.perspnorm = (float)(rdp_reg.cmd1 & 0xFFFF) / 65535.0f;
#ifdef LOG_ON
                LOG_TO_FILE("\tPERSPNORM: ");
                LOG_TO_FILE("\tfactor: %d ($%lx)\n", rdp_reg.cmd1, rdp_reg.cmd1);
#endif
            /**
            *
            * We don't need this. This is to make multipltiplication for s15.16 values 
            * more precise. We use float values. They are precise enough!
            *
            **/
/*
                perspective_normal = ((float)rdp_reg.cmd1) / 65536.0;
*/
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD index = %d?\n",(rdp_reg.cmd0 & 0xff));
#endif                
                    ;

        } /* switch(rdp_reg.cmd0 & 0xff) */

} /* static void rsp_uc05_moveword() */



void rsp_uc05_setothermode_l()
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
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_SETOTHERMODE_L ", ADDR, CMD0, CMD1);
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

            case 0x16:
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
                //LOG_TO_FILE("SETOTHERMODE_L ?\n");
                ;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */

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

void rsp_uc05_setothermode_h()
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
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_SETOTHERMODE_H ", ADDR, CMD0, CMD1);
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
                LOG_TO_FILE("\tUNDEFINED\n");
#endif
                break;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */
}


static void rsp_uc05_movemem()
{

        _u32 a;
        int   i;
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_MOVEMEM",ADDR, CMD0, CMD1);
#endif

        switch((rdp_reg.cmd0 >> 16) & 0xff)
        {
            case 0x80:
                /* we do '>> 1' here because we just need _s16's */
                a = segoffset2addr(rdp_reg.cmd1) >> 1;

                rdp_reg.vp[0] = ((float)((_s16 *)pRDRAM)[(a+0)^1]) / 4.0f;
                rdp_reg.vp[1] = ((float)((_s16 *)pRDRAM)[(a+1)^1]) / 4.0f;
                rdp_reg.vp[2] = ((float)((_s16 *)pRDRAM)[(a+2)^1]) / 4.0f;
                rdp_reg.vp[3] = ((float)((_s16 *)pRDRAM)[(a+3)^1]) / 4.0f;
                rdp_reg.vp[4] = ((float)((_s16 *)pRDRAM)[(a+4)^1]) / 4.0f;
                rdp_reg.vp[5] = ((float)((_s16 *)pRDRAM)[(a+5)^1]) / 4.0f;
                rdp_reg.vp[6] = ((float)((_s16 *)pRDRAM)[(a+6)^1]) / 4.0f;
                rdp_reg.vp[7] = ((float)((_s16 *)pRDRAM)[(a+7)^1]) / 4.0f;
#ifdef LOG_ON
                LOG_TO_FILE("\tVIEWPORT: ");
                LOG_TO_FILE("\taddr=$%08lx (seg#%ld, offset=$%lx)",
                        a<<1,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff );
                LOG_TO_FILE(
                        "\tscale: % 7.2f % 7.2f % 7.2f % 7.2f\n" 
                        "\ttrans: % 7.2f % 7.2f % 7.2f % 7.2f\n",
                        rdp_reg.vp[0],
                        rdp_reg.vp[1],
                        rdp_reg.vp[2],
                        rdp_reg.vp[3],
                        rdp_reg.vp[4],
                        rdp_reg.vp[5],
                        rdp_reg.vp[6],
                        rdp_reg.vp[7] );
#endif
                Render_viewport();
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

#ifdef LOG_ON
                LOG_TO_FILE("\tLOOKATY");
                LOG_TO_FILE("\taddr=$%08lx (seg#%x, offset=$%lx)",
                        a,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff);
                LOG_TO_FILE(
                        "\trgba=%04.2f,%04.2f,%04.2f,%04.2f xyzw=%04.2f,%04.2f,%04.2f,%04.2f\n",
                        rdp_reg.lookat_y.r,
                        rdp_reg.lookat_y.g,
                        rdp_reg.lookat_y.b,
                        rdp_reg.lookat_y.a,
                        rdp_reg.lookat_y.x,
                        rdp_reg.lookat_y.y,
                        rdp_reg.lookat_y.z,
                        rdp_reg.lookat_y.w
                        );
#endif
                Render_lookat_y();
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

#ifdef LOG_ON
                LOG_TO_FILE("\tLOOKATX");
                LOG_TO_FILE("\taddr=$%08lx (seg#%d, offset=$%lx)",
                        a,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff );
                LOG_TO_FILE(
                        "        rgba=%04.2f,%04.2f,%04.2f,%04.2f xyzw=%04.2f,%04.2f,%04.2f,%04.2f\n",
                        rdp_reg.lookat_x.r,
                        rdp_reg.lookat_x.g,
                        rdp_reg.lookat_x.b,
                        rdp_reg.lookat_x.a,
                        rdp_reg.lookat_x.x,
                        rdp_reg.lookat_x.y,
                        rdp_reg.lookat_x.z,
                        rdp_reg.lookat_x.w
                        );
#endif
                Render_lookat_x();
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
                LOG_TO_FILE("\tLIGHT %d\n\taddr=$%08lx (seg#%ld, offset=$%lx)",
                        i,
                        a,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff);
                LOG_TO_FILE(
                        "        rgba=%04.2f,%04.2f,%04.2f,%04.2f xyzw=%04.2f,%04.2f,%04.2f,%04.2f\n",
                        rdp_reg.light[i].r,
                        rdp_reg.light[i].g,
                        rdp_reg.light[i].b,
                        rdp_reg.light[i].a,
                        rdp_reg.light[i].x,
                        rdp_reg.light[i].y,
                        rdp_reg.light[i].z,
                        rdp_reg.light[i].w
                        );
#endif
                //Render_light(i);
                break;

            case 0x98:
#ifdef LOG_ON
                LOG_TO_FILE("\tMATRIX 0\tN/I");
#endif                
                break;

            case 0x9a:
#ifdef LOG_ON
                LOG_TO_FILE("\tMATRIX 1\tN/I");
#endif                
                break;

            case 0x9c:
#ifdef LOG_ON
                LOG_TO_FILE("\tMATRIX 2\tN/I");
#endif                
                break;

            case 0x9e:
#ifdef LOG_ON
                LOG_TO_FILE("\tMATRIX 3\tN/I");
#endif                
                break;

            default:
#ifdef LOG_ON
                LOG_TO_FILE("\tUNKNOWN\tIndex = %x",((rdp_reg.cmd0 >> 16) & 0xff));
#endif                
                ;

        } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */


} /* static void rsp_uc05_movemem() */

static void rsp_uc05_cleargeometrymode()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_CLEARGEOMETRYMODE ", ADDR, CMD0, CMD1);
    LOG_TO_FILE(
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
                (rdp_reg.cmd1 & 0x00000001) ? "\tzbuffer\n" : "",
                (rdp_reg.cmd1 & 0x00000002) ? "\ttexture\n" : "",
                (rdp_reg.cmd1 & 0x00000004) ? "\tshade\n" : "",
                (rdp_reg.cmd1 & 0x00000200) ? "\tshade smooth\n" : "",
                (rdp_reg.cmd1 & 0x00001000) ? "\tcull front\n" : "",
                (rdp_reg.cmd1 & 0x00002000) ? "\tcull back\n" : "",
                (rdp_reg.cmd1 & 0x00010000) ? "\tfog\n" : "",
                (rdp_reg.cmd1 & 0x00020000) ? "\tlightning\n" : "",
                (rdp_reg.cmd1 & 0x00040000) ? "\ttexture gen\n" : "",
                (rdp_reg.cmd1 & 0x00080000) ? "\ttexture gen lin\n" : "",
                (rdp_reg.cmd1 & 0x00100000) ? "\tlod\n" : ""
                );
#endif
        rdp_reg.geometrymode &= ~rdp_reg.cmd1;
//        rdp_reg.geometrymode |= 0x00020000;

/*        if(rdp_reg.cmd1 & 0x00000002)
                rdp_reg.geometrymode_textures = 0;
*/
//      if ((rdp_reg.geometrymode & 0x00020000) != 0)
//              rdp_reg.useLights = 0;

//      if (rdp_reg.geometrymode & 0x00003000) Render_geometry_cullfrontback(0);
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
        } //** switch(rdp_reg.geometrymode & 0x00003000) 
        
} /* static void rsp_uc00_cleargeometrymode() */




//////////////////////////////////////////////////////////////////////////////
// Unimplemented functions
//////////////////////////////////////////////////////////////////////////////


void rsp_uc05_rdphalf_cont()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_RDPHALF_CONT NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_rdphalf_2()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_RDPHALF_2 NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_line3d()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_LINE3d NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_sprite2d()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_SPRITE2D NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_culldl()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_CULLDL NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_tri1()
{
#ifdef LOG_ON
    LOG_TO_FILE("%08X: %08X %08X CMD UC5_TRI1 NI", ADDR, CMD0, CMD1);
#endif
}

void rsp_uc05_rdphalf_1()
{
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_RDPHALF_1 NI", ADDR, CMD0, CMD1);
#endif
}
