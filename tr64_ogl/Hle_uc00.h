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

//#define DEBUG

//** variables and fuction for ucode0
static void uc00_DrawVisualTriangle(int vn[3]);

//** end: variables and fuction for ucode0

/* rdp graphics instructions */

static void rsp_uc00_matrix();
static void rsp_uc00_movemem();
static void rsp_uc00_vertex();
static void rsp_uc00_displaylist();
static void rsp_uc00_sprite2d();
static void rsp_uc00_tri1();
static void rsp_uc00_tri2();
static void rsp_uc00_culldl();
static void rsp_uc00_popmatrix();
static void rsp_uc00_moveword();
static void rsp_uc00_texture();
static void rsp_uc00_setothermode_h();
static void rsp_uc00_setothermode_l();
static void rsp_uc00_enddl();
static void rsp_uc00_setgeometrymode();
static void rsp_uc00_cleargeometrymode();
static void rsp_uc00_line3d();
static void rsp_uc00_rdphalf_1();
static void rsp_uc00_rdphalf_2();
static void rsp_uc00_rdphalf_cont();

void math_lightingN(t_vtx *vtx, float color[4]);

#include "texture.h"
#include "3dmath.h"
#include <math.h>

extern _u32 TexMode;
extern _u32 TexColor;

/******************************************************************************\
*                                                                              *
*   HLE (High Level Emulation) of RSP gfx. Currently just uc0.                 *
*                                                                              *
\******************************************************************************/


/**
*
* Most matrix stuff was first done by Jeff.
* Great work, Jeff! (Niki)
*
**/

static void rsp_uc00_matrix()
{
        _u32   a = segoffset2addr(rdp_reg.cmd1);
        _u8   command = (_u8)((rdp_reg.cmd0 >> 16) & 0xff);
        float   m[4][4];

#ifdef LOG_ON
        PRINT_RDP_INFO("MATRIX ");
#endif

    hleGetMatrix((float*)&m[0,0], &pRDRAM[a]);
    refresh_lights = TRUE;

/*        a = a >> 1;   // we have half words 

        for(i = 0; i < 16; i+=4)
        {
                for(j = 0; j < 4; j++)
                {
                        m[i>>2][j] = (float)
                                (
                                        (((_s32)((_u16 *)pRDRAM)[(a + i + j)^1]) << 16)
                                        |
                                        ((_u16 *)pRDRAM)[(a + i + j + 16)^1]
                                ) / 65536.0f;
                }
        }*/

#ifdef LOG_ON
        PRINT_RDP_MNEMONIC(
                "addr=$%08lx (seg#%ld, offset=$%lx)\n"
                "        param=$%02x:\n"
                "        %s\n"
                "        %s\n"
                "        %s\n"
                "\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "        { %#+12.5f %#+12.5f %#+12.5f %#+12.5f }\n"
                "\n",
                a,
                (rdp_reg.cmd1 >> 24 & 0x0f),
                rdp_reg.cmd1 & 0x00ffffff,
                command,
                (command & 0x01) ? "projection" : "modelview",
                (command & 0x02) ? "load" : "mul",
                (command & 0x04) ? "push" : "nopush",
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
                );

#endif
        switch(command)
        {
            case 0: /* modelview  mul  nopush */
                mult_matrix((GLfloat *)m);
                break;

            case 1: /* projection mul  nopush */
            case 5: /* projection mul  push   */ /* the same, because there is just 1 proj matrix */
                //glMultMatrixf((GLfloat *)m);
                mult_prj_matrix((GLfloat *)m);
                break;

            case 2: /* modelview  load nopush */
                load_matrix((GLfloat *)m);
                break;

            case 3: /* projection load nopush */
            case 7: /* projection load push   */ /* the same, because there is just 1 proj matrix */ /* thanks Gil */
                load_prj_matrix((GLfloat *)m);
                //glLoadMatrixf((GLfloat *)m);
                break;

            case 4: /* modelview  mul  push   */
                /* (Gil) Push the stack */
                /* (Gil) We should probably use pointers instead, as they are much faster to move around. */
                /* (Niki) This comes from Gil - thanks */
                push_mult_matrix((GLfloat *)m);
                break;

            case 6: /* modelview  load push   */
                push_load_matrix((GLfloat *)m);
                break;

            default:
                break;
                

        } /* switch(command) */

        //

} /* static void rsp_uc00_matrix() */


static void rsp_uc00_movemem()
{
        _u32 a;
        int   i;



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
                PRINT_RDP_INFO("MOVEMEM VIEWPORT: ");
                PRINT_RDP_MNEMONIC("addr=$%08lx (seg#%ld, offset=$%lx)\n",
                        a<<1,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff );
                PRINT_RDP_MNEMONIC(
                        "        scale: % 7.2f % 7.2f % 7.2f % 7.2f\n" 
                        "        trans: % 7.2f % 7.2f % 7.2f % 7.2f\n",
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
                rdp_reg.lookat_y.x         = ((float)((_u8 *)pRDRAM)[(a+ 8)^3]);//255.0f;
                rdp_reg.lookat_y.y         = ((float)((_u8 *)pRDRAM)[(a+ 9)^3]);//255.0f;
                rdp_reg.lookat_y.z         = ((float)((_u8 *)pRDRAM)[(a+10)^3]);//255.0f;
                rdp_reg.lookat_y.w         = 1.0f;

#ifdef LOG_ON
                PRINT_RDP_INFO("MOVEMEM LOOKATY");
                PRINT_RDP_MNEMONIC("addr=$%08lx (seg#%d, offset=$%lx)\n",
                        a,
                        rdp_reg.cmd1 >> 24,
                        rdp_reg.cmd1 & 0x00ffffff );
                PRINT_RDP_MNEMONIC(
                        "        rgba=%04.2f,%04.2f,%04.2f,%04.2f xyzw=%04.2f,%04.2f,%04.2f,%04.2f\n",
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
                rdp_reg.lookat_x.x         = ((float)((_u8 *)pRDRAM)[(a+ 8)^3])/255.0f;
                rdp_reg.lookat_x.y         = ((float)((_u8 *)pRDRAM)[(a+ 9)^3])/255.0f;
                rdp_reg.lookat_x.z         = ((float)((_u8 *)pRDRAM)[(a+10)^3])/255.0f;
                rdp_reg.lookat_x.w         = 1.0f;

#ifdef LOG_ON
                PRINT_RDP_INFO("MOVEMEM LOOKATX");
                PRINT_RDP_MNEMONIC("addr=$%08lx (seg#%d, offset=$%lx)\n",
                        a,
                        rdp_reg.cmd1 >> 24,
                        rdp_reg.cmd1 & 0x00ffffff );
                PRINT_RDP_MNEMONIC(
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

                //Render_light(i);
                refresh_lights = TRUE;

#ifdef LOG_ON
                PRINT_RDP_INFO("MOVEMEM LIGHT ");
                PRINT_RDP_MNEMONIC("%d addr=$%08lx (seg#%ld, offset=$%lx)\n",
                        i,
                        a,
                        rdp_reg.cmd1 >> 24,
                        rdp_reg.cmd1 & 0x00ffffff );
                PRINT_RDP_MNEMONIC(
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
                printf("MOVEMEM LIGHT ");
                printf("%d addr=$%08lx (seg#%ld, offset=$%lx)\n",
                        i,
                        a,
                        rdp_reg.cmd1 >> 24,
                        rdp_reg.cmd1 & 0x00ffffff );
                printf(
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
                break;

            case 0x98:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEMEM MATRIX 0");
#endif
                break;

            case 0x9a:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEMEM MATRIX 1");
#endif
                break;

            case 0x9c:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEMEM MATRIX 2");
#endif
                break;

            case 0x9e:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEMEM MATRIX 3");
#endif          
                break;

            default:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEMEM ?");
#endif
                break;
        } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */

} /* static void rsp_uc00_movemem() */



static void rsp_uc00_vertex()
{
        _u32   a = segoffset2addr(rdp_reg.cmd1);
        _u16  len;
        int     v0, i, n;

#ifdef LOG_ON
        PRINT_RDP_INFO("VERTEX");
#endif
        v0 = (rdp_reg.cmd0 >> 16) & 0xf;
        n = ((rdp_reg.cmd0 >> 20) & 0xf) + 1;
        len = (_u16)rdp_reg.cmd0 & 0xffff;

        if (refresh_matrix) update_cmbmatrix();

        //rdp_reg.m_CurTile=&rdp_reg.td[0];
        //rdp_reg.m_NexTile = &rdp_reg.td[1];
        rdp_reg.loadtile = 0;

        for(i = 0; i < (n<<4); i+=16)
        {
                rdp_reg.vtx[v0].x     = (float)((_s16 *)pRDRAM)[(((a+i) >> 1) + 0)^1];
                rdp_reg.vtx[v0].y     = (float)((_s16 *)pRDRAM)[(((a+i) >> 1) + 1)^1];
                rdp_reg.vtx[v0].z     = (float)((_s16 *)pRDRAM)[(((a+i) >> 1) + 2)^1];
                rdp_reg.vtx[v0].flags = ((_u16 *)pRDRAM)[(((a+i) >> 1) + 3)^1];
                rdp_reg.vtx[v0].s     = ((float)((_s16 *)pRDRAM)[(((a+i) >> 1) + 4)^1]);
                rdp_reg.vtx[v0].t     = ((float)((_s16 *)pRDRAM)[(((a+i) >> 1) + 5)^1]);
                rdp_reg.vtx[v0].r     = ((_u8  *)pRDRAM)[(  a+i        +12)^3];
                rdp_reg.vtx[v0].g     = ((_u8  *)pRDRAM)[(  a+i        +13)^3];
                rdp_reg.vtx[v0].b     = ((_u8  *)pRDRAM)[(  a+i        +14)^3];
                rdp_reg.vtx[v0].a     = ((_u8  *)pRDRAM)[(  a+i        +15)^3];

#ifdef LOG_ON
                PRINT_RDP_MNEMONIC("        rdp_reg.vtx#%02d: x=% 6f, y=% 6f, z=% 6f, flags=$%04x, s=%+7.2f, t=%+7.2f, rgba=%02x%02x%02x%02x\n",
                        v0,
                        rdp_reg.vtx[v0].x,
                        rdp_reg.vtx[v0].y,
                        rdp_reg.vtx[v0].z,
                        rdp_reg.vtx[v0].flags,
                        rdp_reg.vtx[v0].s,
                        rdp_reg.vtx[v0].t,
                        rdp_reg.vtx[v0].r,
                        rdp_reg.vtx[v0].g,
                        rdp_reg.vtx[v0].b,
                        rdp_reg.vtx[v0].a );

#endif
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

            if (0x00040000 & rdp_reg.geometrymode)
            {
                // G_TEXTURE_GEN enable the automatic generation of the texture
                // coordinates s and t.  A spherical mapping is used, based on the normal.
                tmpvtx[0] = (float)(_s8)vertex->r;
                tmpvtx[1] = (float)(_s8)vertex->g;
                tmpvtx[2] = (float)(_s8)vertex->b;

                cmbtrans_normal(&tmpvtx[0]);

                vertex->n1 = tmpvtx[0];
                vertex->n2 = tmpvtx[1];
                vertex->n3 = tmpvtx[2];

//              if (0x00080000 & rdp_reg.geometrymode)
                {
                    float fLenght = VectorLength(tmpvtx);
                    vertex->s = (0.5f + (0.5f * vertex->n1 / fLenght));
                    vertex->t = (0.5f - (0.5f * vertex->n2 / fLenght));
//                  vertex->s = (float)acos((double)vertex->n1) * 325.94931f;
//                  vertex->t = (float)acos((double)vertex->n2) * 325.94931f;
                }
//              else
//              {
//                  vertex->s = (vertex->n1 + 1.0f) * 512.0f;
//                  vertex->t = (vertex->n2 + 1.0f) * 512.0f;
//              }
            }

        }

                v0++;
        }
        //


} /* static void rsp_uc00_vertex() */





static void rsp_uc00_displaylist()
{

        _u32 addr = segoffset2addr(rdp_reg.cmd1);
        _u32 push = (rdp_reg.cmd0 >> 16) & 0xff; 

#ifdef LOG_ON
        PRINT_RDP_INFO("DISPLAYLIST: ");
#endif
        switch(push)
        {
            case 0:   /* push: we do a call of the dl */
                rdp_reg.pc_i++;
                if(rdp_reg.pc_i > (RDP_STACK_SIZE-1))
                {
#ifdef LOG_ON
                        PRINT_RDP_WARNING("DL STACK OVERFLOW (too much (>9) DISPLAYLIST calls\nwill not call dl!!! - ignored");
#endif
                        return;
                }

#ifdef LOG_ON
                PRINT_RDP_MNEMONIC("call dl#%d; addr=$%08lx (seg#%d, offset=$%lx)\n",
                        rdp_reg.pc_i,
                        addr,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff );
#endif
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            case 1:   /* no push: we jump to the dl */
//puts("DISPLAYLIST: no push");
#ifdef LOG_ON
                PRINT_RDP_MNEMONIC("jump dl#%d; addr=$%08lx (seg#%d, offset=$%lx)\n",
                        rdp_reg.pc_i,
                        addr,
                        (rdp_reg.cmd1 >> 24) & 0x0f,
                        rdp_reg.cmd1 & 0x00ffffff );
#endif
                rdp_reg.pc[rdp_reg.pc_i] = addr;
                break;

            default:
#ifdef LOG_ON
                PRINT_RDP_WARNING("DISPLAYLIST - parameter not supported");
#endif                
                break;

        } /* switch(push) */

} /* static void rsp_uc00_displaylist() */






static void rsp_uc00_sprite2d()
{
        static char *format[]   = { "RGBA", "YUV", "CI", "IA", "I", "?", "?", "?" };
        static char *size[]     = { "4bit", "8bit", "16bit", "32bit" };

        _u32   a               = segoffset2addr(rdp_reg.cmd1);
        _u32   dst_p;  /* dest pointer */
        _u32   dst_w;  /* dest with (normally 320 or 640) */
        _u32   dst_b;  /* dest depth (16bit: 2; 32bit: 3) */

        _u32   src             = ((_u32 *) pRDRAM)[((a>>2)+ 0)^0];
        _u32   src_ci          = ((_u32 *) pRDRAM)[((a>>2)+ 1)^0];
        _u16  src_w           = ((_u16 *)pRDRAM)[((a>>1)+ 4)^1];
        _u16  sprite_w        = ((_u16 *)pRDRAM)[((a>>1)+ 5)^1];
        _u16  sprite_h        = ((_u16 *)pRDRAM)[((a>>1)+ 6)^1];
        _u8   src_type        = ((_u8 *) pRDRAM)[( a    +14)^3];   /* rgba, ci, ai, a, i */
        _u8   src_bits        = ((_u8 *) pRDRAM)[( a    +15)^3];   /* 4, 8, 16, 32 */
        float   scale_x         = ((float)((_u16 *)pRDRAM)[((a>>1)+ 8)^1]) / 1024.0f;   /* s5.10 */
        float   scale_y         = ((float)((_u16 *)pRDRAM)[((a>>1)+ 9)^1]) / 1024.0f;   /* s5.10 */
        _u8   flip_x          = ((_u8 *) pRDRAM)[( a    +20)^3];
        _u8   flip_y          = ((_u8 *) pRDRAM)[( a    +21)^3];
        _u16  src_ox          = ((_u16 *)pRDRAM)[((a>>1)+11)^1];
        _u16  src_oy          = ((_u16 *)pRDRAM)[((a>>1)+12)^1];
        _u16  dst_ox          = ((_u16 *)pRDRAM)[((a>>1)+13)^1];
        _u16  dst_oy          = ((_u16 *)pRDRAM)[((a>>1)+14)^1];

        dst_b = ((_u32 *)pVIREG)[0] & 0x03;
        if(dst_b != 2 && dst_b != 3)
        {
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("SPRITE2D: dest screen has unsupported depth\n");
#endif                
                return;
        }
        dst_w = ((_u32 *)pVIREG)[2];
        dst_p = ((_u32 *)pVIREG)[1];
        dst_p &= 0x007fffff;

#ifdef LOG_ON
        PRINT_RDP_INFO("SPRITE2D: ");
        PRINT_RDP_MNEMONIC("addr=$%08lx (seg#%d, offset=$%lx):\n"
                           "        src     : $%08lx\n"
                           "        src_ci  : $%08lx\n"
                           "        src_w   : $%04lx (%d)\n"
                           "        sprite_w: $%04lx (%d)\n"
                           "        sprite_h: $%04lx (%d)\n"
                           "        src_type: $%02lx   (%s)\n"
                           "        src_bits: $%02lx   (%s)\n"
                           "        scale_x : %f\n"
                           "        scale_y : %f\n"
                           "        flip_x  : $%02lx\n"
                           "        flip_y  : $%02lx\n"
                           "        src_ox  : $%04lx (%d)\n"
                           "        src_oy  : $%04lx (%d)\n"
                           "        dst_ox  : $%04lx (%d)\n"
                           "        dst_oy  : $%04lx (%d)\n",
                           a, (rdp_reg.cmd1 >> 24) & 0x0f, rdp_reg.cmd1 & 0x00ffffff,
                           src, src_ci, src_w, src_w,
                           sprite_w, sprite_w, sprite_h, sprite_h, src_type, format[src_type], src_bits, size[src_bits],
                           scale_x, scale_y,
                           flip_x, flip_y,
                           src_ox, src_ox, src_oy, src_oy,
                           dst_ox, dst_ox, dst_oy, dst_oy);
        PRINT_RDP_WARNING("scale and flip are ignored");
#endif


        /* BUG? the following might be just used with the TEXRECT and FILLRECT commands */
        /* - i don't know */
        if(!(rdp_reg.mode_h & 0x00200000))
        {
            /* if we are not in copy or fill mode (we are in 1 or 2 cycle mode) */
            /* we must not draw the bottom and right edges */
                //xl--;
                //yl--;
        }


                                                                                                                                                    
        SetVisualColor(0xff0000);
        Render_DrawVisualRectangle(dst_ox, dst_oy, dst_ox+sprite_w, dst_oy+sprite_h);
/*
        draw2d(src, src_ci, src_type, src_bits, src_w, src_ox, src_oy,
                sprite_w, sprite_h, flip_x, flip_y,
                dst_p, 0, dst_b, dst_w, dst_ox, dst_oy );
*/
} /* static void rsp_uc00_sprite2d() */





static void rsp_uc00_tri1()
{
        int vn[3], f;

        vn[0] = ((rdp_reg.cmd1 >> 16) & 0xff) / 10;
        vn[1] = ((rdp_reg.cmd1 >>  8) & 0xff) / 10;
        vn[2] = ( rdp_reg.cmd1        & 0xff) / 10;

        f     =  (rdp_reg.cmd1 >> 24) & 0xff;

#ifdef LOG_ON
        PRINT_RDP_INFO("TRI1 ");
        PRINT_RDP_MNEMONIC("v0=%2u v1=%2u v2=%2u flag=$%02x\n", vn[0], vn[1], vn[2], f);
#endif
//        uc00_DrawVisualTriangle(vn);
        DrawVisualTriangle(vn);

} /* static void rsp_uc00_tri1() */

BOOL Goldeneye = FALSE;

static void rsp_uc00_tri2()
{
    int   vn[6];

    if ((CMD1 >> 24) | Goldeneye)
    {
#ifdef LOG_ON
        LOG_TO_FILE("%08X: %08X %08X CMD UC5_TRI2", ADDR, CMD0, CMD1);
#endif
        Goldeneye = TRUE;

        vn[2] = ((CMD1 & 0xf0) >> 4);
        vn[1] = ((CMD1 & 0x0f) >> 0);
        vn[0] = ((CMD0 & 0x0f) >> 0);
    
        if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
        {
            DrawVisualTriangle(vn);
#ifdef LOG_ON
            LOG_TO_FILE("(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
        }

        vn[2] = ((CMD1 & 0xf000) >> 12);
        vn[1] = ((CMD1 & 0x0f00) >>  8);
        vn[0] = ((CMD0 & 0xf0)   >>  4);
        if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
        {
            DrawVisualTriangle(vn);
#ifdef LOG_ON
            LOG_TO_FILE("(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
        }

        vn[2] = ((CMD1 & 0xf00000) >> 20);
        vn[1] = ((CMD1 & 0x0f0000) >> 16);
        vn[0] = ((CMD0 & 0x0f00)   >>  8);
        if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
        {
            DrawVisualTriangle(vn);
#ifdef LOG_ON
            LOG_TO_FILE("(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
        }
    
        vn[2] = ((CMD1 & 0xf0000000) >> 28);
        vn[1] = ((CMD1 & 0x0f000000) >> 24);
        vn[0] = ((CMD0 & 0xf000)     >> 12);
        if ((vn[0] !=0) || (vn[1] !=0) || (vn[2] !=0))
        {
            DrawVisualTriangle(vn);
#ifdef LOG_ON
            LOG_TO_FILE("(%i,%i,%i)", vn[0], vn[1], vn[2]);
#endif
        }
#ifdef LOG_ON
        LOG_TO_FILE("\n");
#endif
    }
    else
    {
        vn[0] = ((rdp_reg.cmd0 >> 16) & 0xff) / 10;
        vn[1] = ((rdp_reg.cmd0 >>  8) & 0xff) / 10;
        vn[2] = ((rdp_reg.cmd0      ) & 0xff) / 10;

        vn[3] = ((rdp_reg.cmd1 >> 16) & 0xff) / 10;
        vn[4] = ((rdp_reg.cmd1 >>  8) & 0xff) / 10;
        vn[5] = ((rdp_reg.cmd1      ) & 0xff) / 10;

        DrawVisualTriangle(vn);
        DrawVisualTriangle(vn+3);

#ifdef LOG_ON
        PRINT_RDP_INFO("TRI2\n");
        PRINT_RDP_MNEMONIC("v0=%u v1=%u v2=%u\n", vn[0], vn[1], vn[2]);
        PRINT_RDP_MNEMONIC("v3=%u v4=%u v5=%u\n", vn[3], vn[4], vn[5]);
#endif
    }
} /* static void rsp_uc01_tri2() */






static void rsp_uc00_culldl()
{
#ifdef LOG_ON
        PRINT_RDP_NOT_IMPLEMENTED("CULLDL");
#endif
} /* static void rsp_uc00_culldl() */





static void rsp_uc00_popmatrix()
{

    /* Compleately from Gil - thanks */
        _u32           param;



#ifdef LOG_ON
        PRINT_RDP_INFO("POPMATRIX ");
#endif
        param = rdp_reg.cmd1;

        switch(param)
        {
            case 0: /* modelview pop */
                    /* We should probably use pointers instead, as they are much faster to move around. */
                pop_matrix();
                break;

            case 1: /* projection pop */
                    /* only 1 projection matrix . Not possible! */
                break;
        }

} /* static void rsp_uc00_popmatrix() */





static void rsp_uc00_moveword()
{
        int i;

        switch(rdp_reg.cmd0 & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD MATRIX Ignored.");
#endif                
                break;

            case 0x02:
                rdp_reg.lights = (rdp_reg.cmd1 - 0x80000000) / 32 - 1;
                rdp_reg.ambient_light =  rdp_reg.lights;
                //calculate_light_vectors();
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD NUMLIGHT: ");
                LOG_TO_FILE("\tlights: %d\n", rdp_reg.lights);
#endif
                break;

            case 0x04:
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD CLIP: ");
#endif
                switch((rdp_reg.cmd0 >> 8) & 0xffff)
                {
                    case 0x0004:
#ifdef LOG_ON
                        LOG_TO_FILE("-x = %8x",rdp_reg.cmd1);
#endif
                        rdp_reg.clip.nx = rdp_reg.cmd1;
                        break;

                    case 0x000c:
#ifdef LOG_ON
                        LOG_TO_FILE("-y = %8x",rdp_reg.cmd1);
#endif
                        rdp_reg.clip.ny = rdp_reg.cmd1;
                        break;

                    case 0x0014:
#ifdef LOG_ON
                        LOG_TO_FILE("+x = %8x",rdp_reg.cmd1);
#endif
                        rdp_reg.clip.px = rdp_reg.cmd1;
                        break;

                    case 0x001c:
#ifdef LOG_ON
                        LOG_TO_FILE("+y = %8x",rdp_reg.cmd1);
#endif
                        rdp_reg.clip.py = rdp_reg.cmd1;
                        break;

                    default:
#ifdef LOG_ON
                        LOG_TO_FILE("MOVEWORD CLIP - wrong offset");
#endif
                        break;
                } /* switch((rdp_reg.cmd0 >> 8) & 0xffff) */
                break;

            case 0x06:
#ifdef LOG_ON
                LOG_TO_FILE("\tMOVEWORD SEGMENT: ");
                LOG_TO_FILE("$%08lx -> seg#%d\n", rdp_reg.cmd1, (rdp_reg.cmd0 >> 10) & 0xf);
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
                    LOG_TO_FILE("\tFog min = %f, max = %f",min,max);
#endif
                }
                break;

            case 0x0a:
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

                //calculate_light_vectors();
                break;

            case 0x0c:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEWORD POINTS");
#endif                
                break;

            case 0x0e:
                rdp_reg.perspnorm = (float)(rdp_reg.cmd1 & 0xFFFF) / 65535.0f;
#ifdef LOG_ON
                PRINT_RDP_INFO("MOVEWORD PERSPNORM: ");
                PRINT_RDP_MNEMONIC("factor: %d ($%lx)\n", rdp_reg.cmd1, rdp_reg.cmd1);
                #endif
            /**
            *
            * We don't need this. This is to make multipltiplication for s15.16 values 
            * more precise. We use float values. They are precise enough!
            *
            **/
                break;

            default:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("MOVEWORD ?");
#endif                
                break;
        } /* switch(rdp_reg.cmd0 & 0xff) */
} /* static void rsp_uc00_moveword() */





static void rsp_uc00_texture()
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
        PRINT_RDP_INFO("TEXTUREMODE");
/*      PRINT_RDP_MNEMONIC("    tile: %ld; scale_s: %9.6f; scale_t: %9.6f; mipmap_level: %ld; on: %ld\n",
                tile,
                rdp_reg.texture[tile].scale_s,
                rdp_reg.texture[tile].scale_t,
                rdp_reg.texture[tile].mipmap_level,
                rdp_reg.texture[tile].on );
*/        
#endif
} /* static void rsp_uc00_texture() */





static void rsp_uc00_setothermode_h()
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

        switch((rdp_reg.cmd0 >> 8) & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                PRINT_RDP_WARNING("SETOTHERMODE_H BLENDMASK - ignored");
#endif                
                break;

            case 0x04:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H ALPHADITHER: ");
                PRINT_RDP_MNEMONIC("%s\n", ad[(rdp_reg.cmd1>>0x04) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00000030;
                rdp_reg.cmd1   &=  0x00000030;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x06:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H RGBDITHER: ");
                PRINT_RDP_MNEMONIC("%s\n", rd[(rdp_reg.cmd1>>0x06) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x000000c0;
                rdp_reg.cmd1   &=  0x000000c0;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x08:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H COMBINEKEY: ");
                PRINT_RDP_MNEMONIC("%s\n", ck[(rdp_reg.cmd1>>0x08) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00000100;
                rdp_reg.cmd1   &=  0x00000100;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x09:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTURECONVERT: ");
                PRINT_RDP_MNEMONIC("%s\n", tc[(rdp_reg.cmd1>>0x09) & 0x7]);
#endif
                rdp_reg.mode_h &= ~0x00000e00;
                rdp_reg.cmd1   &=  0x00000e00;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x0c:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTUREFILTER: ");
                PRINT_RDP_MNEMONIC("%s\n", tf[(rdp_reg.cmd1>>0x0c) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00003000;
                rdp_reg.cmd1   &=  0x00003000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x0e:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTURELUT: ");
                PRINT_RDP_MNEMONIC("%s\n", tt[(rdp_reg.cmd1>>0x0e) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x0000c000;
                rdp_reg.cmd1   &=  0x0000c000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x10:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTURELOD: ");
                PRINT_RDP_MNEMONIC("%s\n", tl[(rdp_reg.cmd1>>0x10) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00010000;
                rdp_reg.cmd1   &=  0x00010000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x11:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTUREDETAIL: ");
                PRINT_RDP_MNEMONIC("%s\n", td[(rdp_reg.cmd1>>0x11) & 0x3]);
#endif
                rdp_reg.mode_h &= ~0x00060000;
                rdp_reg.cmd1   &=  0x00060000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x13:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H TEXTUREPERSP: ");
                PRINT_RDP_MNEMONIC("%s\n", tp[(rdp_reg.cmd1>>0x13) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00080000;
                rdp_reg.cmd1   &=  0x00080000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x14:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H CYCLETYPE: ");
                PRINT_RDP_MNEMONIC("%s\n", ct[(rdp_reg.cmd1>>0x14) & 0x3]);
#endif
                cycle_mode = (_u8)((rdp_reg.cmd1>>0x14) & 0x3);
                rdp_reg.mode_h &= ~0x00300000;
                rdp_reg.cmd1   &=  0x00300000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x16:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H COLORDITHER: ");
                PRINT_RDP_MNEMONIC("%s\n", cd[(rdp_reg.cmd1>>0x16) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00400000;
                rdp_reg.cmd1   &=  0x00400000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            case 0x17:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_H PIPELINEMODE: ");
                PRINT_RDP_MNEMONIC("%s\n", pm[(rdp_reg.cmd1>>0x17) & 0x1]);
#endif
                rdp_reg.mode_h &= ~0x00800000;
                rdp_reg.cmd1   &=  0x00800000;
                rdp_reg.mode_h |=  rdp_reg.cmd1;
                break;

            default:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("SETOTHERMODE_H ?");
#endif                
                break;
        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */

        if ((rdp_reg.mode_l & 0x0c00) != 0xc00)
            glDepthRange(-1,1.);
        else
            glDepthRange(-1,0.9995);

        if (rdp_reg.mode_l & 0x00000020)
            Render_geometry_zwrite(1);
        else
            Render_geometry_zwrite(0);
} /* static void rsp_uc00_setothermode_h() */





static void rsp_uc00_setothermode_l()
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

        switch((rdp_reg.cmd0 >> 8) & 0xff)
        {
            case 0x00:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_L ALPHACOMPARE: ");
                PRINT_RDP_MNEMONIC("%s\n", ac[(rdp_reg.cmd1>>0x00) & 0x3]);
#endif
                rdp_reg.mode_l &= ~0x00000003;
                rdp_reg.cmd1   &=  0x00000003;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
                break;

            case 0x02:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_L ZSRCSEL: ");
                PRINT_RDP_MNEMONIC("%s\n", zs[(rdp_reg.cmd1>>0x02) & 0x1]);
#endif
                rdp_reg.mode_l &= ~0x00000004;
                rdp_reg.cmd1   &=  0x00000004;
                rdp_reg.mode_l |=  rdp_reg.cmd1;
                break;

            case 0x03:
#ifdef LOG_ON
                PRINT_RDP_INFO("SETOTHERMODE_L RENDERMODE: ");
                PRINT_RDP_MNEMONIC("$%08lx:\n", rdp_reg.cmd1 & 0xfffffff8,
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
                        rdp_reg.cmd1 & 0xfffffff8,
                        (rdp_reg.cmd1 & 0x00000008) ? "        anti alias\n" : "",
                        (rdp_reg.cmd1 & 0x00000010) ? "        z_cmp\n" : "",
                        (rdp_reg.cmd1 & 0x00000020) ? "        z_upd\n" : "",
                        (rdp_reg.cmd1 & 0x00000040) ? "        im_rd\n" : "",
                        (rdp_reg.cmd1 & 0x00000080) ? "        clr_on_cvg\n" : "",
                        (rdp_reg.cmd1 & 0x00000100) ? "        cvg_dst_warp\n" : "",
                        (rdp_reg.cmd1 & 0x00000200) ? "        cvg_dst_full\n" : "",
                        (rdp_reg.cmd1 & 0x00000400) ? "        z_inter\n" : "",
                        (rdp_reg.cmd1 & 0x00000800) ? "        z_xlu\n" : "",
                        (rdp_reg.cmd1 & 0x00001000) ? "        cvg_x_alpha\n" : "",
                        (rdp_reg.cmd1 & 0x00002000) ? "        alpha_cvg_sel\n" : "",
                        (rdp_reg.cmd1 & 0x00004000) ? "        force_bl\n" : "",
                        (rdp_reg.cmd1 & 0x00008000) ? "        tex_edge?\n" : "",
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
                PRINT_RDP_NOT_IMPLEMENTED("SETOTHERMODE_L BLENDER");
#endif
                break;

            default:
#ifdef LOG_ON
                PRINT_RDP_NOT_IMPLEMENTED("SETOTHERMODE_L ?");
#endif                
                break;
        } /* switch((rdp_reg.cmd0 >> 8) & 0xff) */


        if ((rdp_reg.mode_l & 0x0c00) != 0xc00)
            glDepthRange(-1,1.);
        else
            glDepthRange(-1,0.9995);

    if(rdp_reg.mode_l & 0x00000010)
        Render_geometry_zbuffer(1);
    else
        Render_geometry_zbuffer(0);

        if (rdp_reg.mode_l & 0x00000020)
            Render_geometry_zwrite(1);
        else
            Render_geometry_zwrite(0);

//  switch (rdp_reg.mode_l & ~0x00000003)
//  {
//  case 0:
//  case 2:
//      Src_Alpha = GL_ONE;
//      Dst_Alpha = GL_ZERO;
//      break;
//  case 1:
//  case 3:
//      Src_Alpha = GL_SRC_ALPHA;
//      Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
//      break;
//  }

    if ((rdp_reg.mode_l & 0x0f0f0000) != 0x5000000)
    {
        Src_Alpha = GL_SRC_ALPHA;
        Dst_Alpha = GL_ONE_MINUS_SRC_ALPHA;
    }
    else
    {
//      Src_Alpha = GL_ONE;
//      Dst_Alpha = GL_ZERO;
    }

} /* static void rsp_uc00_setothermode_l() */





static void rsp_uc00_enddl()
{

#ifdef LOG_ON
        PRINT_RDP_INFO("ENDDL: ");
        PRINT_RDP_MNEMONIC("return from dl#%d\n", rdp_reg.pc_i);
#endif
        if(rdp_reg.pc_i < 0)
        {
#ifdef LOG_ON
                PRINT_RDP_WARNING("DL STACK UNDERRUN (too much ENDDL calls)\nwill not return from dl!!! - ignored");
#endif
                return;
        }

        if(rdp_reg.pc_i == 0)
        {
#ifdef LOG_ON
                PRINT_RDP_MNEMONIC("RDP stops here - RSP BREAK will be simulated\n");
#endif
                //Render_FlushVisualRenderBuffer();
                rdp_reg.halt = 1;
        }

        rdp_reg.pc_i--;

} /* static void rsp_uc00_enddl() */



static void rsp_uc00_setgeometrymode()
{

#ifdef LOG_ON
        PRINT_RDP_INFO("SETGEOMETRYMODE ");
        PRINT_RDP_MNEMONIC(
                "+$%08lx:\n"
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
                (rdp_reg.cmd1 & 0x00000001) ? "        zbuffer\n" : "",
                (rdp_reg.cmd1 & 0x00000002) ? "        texture\n" : "",
                (rdp_reg.cmd1 & 0x00000004) ? "        shade\n" : "",
                (rdp_reg.cmd1 & 0x00000200) ? "        shade smooth\n" : "",
                (rdp_reg.cmd1 & 0x00001000) ? "        cull front\n" : "",
                (rdp_reg.cmd1 & 0x00002000) ? "        cull back\n" : "",
                (rdp_reg.cmd1 & 0x00010000) ? "        fog\n" : "",
                (rdp_reg.cmd1 & 0x00020000) ? "        lightning\n" : "",
                (rdp_reg.cmd1 & 0x00040000) ? "        texture gen\n" : "",
                (rdp_reg.cmd1 & 0x00080000) ? "        texture gen lin\n" : "",
                (rdp_reg.cmd1 & 0x00100000) ? "        lod\n" : ""
                );
#endif
        rdp_reg.geometrymode |= rdp_reg.cmd1;

        if(rdp_reg.geometrymode & 0x00000002)
                rdp_reg.geometrymode_textures = 1;

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

} /* static void rsp_uc00_setgeometrymode() */





static void rsp_uc00_cleargeometrymode()
{
#ifdef LOG_ON
        PRINT_RDP_INFO("CLEARGEOMETRYMODE ");
        PRINT_RDP_MNEMONIC(
                "-$%08lx\n"
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
                (rdp_reg.cmd1 & 0x00000001) ? "        zbuffer\n" : "",
                (rdp_reg.cmd1 & 0x00000002) ? "        texture\n" : "",
                (rdp_reg.cmd1 & 0x00000004) ? "        shade\n" : "",
                (rdp_reg.cmd1 & 0x00000200) ? "        shade smooth\n" : "",
                (rdp_reg.cmd1 & 0x00001000) ? "        cull front\n" : "",
                (rdp_reg.cmd1 & 0x00002000) ? "        cull back\n" : "",
                (rdp_reg.cmd1 & 0x00010000) ? "        fog\n" : "",
                (rdp_reg.cmd1 & 0x00020000) ? "        lightning\n" : "",
                (rdp_reg.cmd1 & 0x00040000) ? "        texture gen\n" : "",
                (rdp_reg.cmd1 & 0x00080000) ? "        texture gen lin\n" : "",
                (rdp_reg.cmd1 & 0x00100000) ? "        lod\n" : ""
                );
#endif
        rdp_reg.geometrymode &= ~rdp_reg.cmd1;

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





static void rsp_uc00_line3d()
{
        int   vn[2], f;

        vn[0] = ((rdp_reg.cmd1 >> 16) & 0xff) / 10;
        vn[1] = ((rdp_reg.cmd1 >>  8) & 0xff) / 10;
        f     =  (rdp_reg.cmd1 >> 24) & 0xff;

#ifdef LOG_ON
        PRINT_RDP_INFO("LINE3D\n");
        PRINT_RDP_MNEMONIC("v0=%u v1=%u flag=$%02x\n", vn[0], vn[1], f);
#endif
        Render_line3d(vn);

        //
} /* static void rsp_uc00_line3d() */





static void rsp_uc00_rdphalf_1()
{
        //puts("RDPHALF_1");

        //

} /* static void rsp_uc00_rdphalf_1() */





static void rsp_uc00_rdphalf_2()
{
        //puts("RDPHALF_2");

        //

} /* static void rsp_uc00_rdphalf_2() */





static void rsp_uc00_rdphalf_cont()
{
        //puts("RDPHALF_CONT");

        //

} /* static void rsp_uc00_rdphalf_cont() */

