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

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators (tested mostly with Project64)
// Project started on December 29th, 2001
//
// Rules & Instructions
// v1.01
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
// To add new combine modes:
// * You MUST include an example of something that uses it
// * You MUST write down the names of the GCCMUX_ things
// (see combine.cpp for details)
//
// You may create, modify, steal, use in another plugin, or do whatever you want with this code, but you MUST credit people for their work, and not claim it as your own.
// Neither I, nor anyone who is working with me, take ANY responsibility for your actions or if this program causes harm to anything, including you and your computer, in any way, physically or mentally.
//
// Official Glide64 development channel: #Glide64 on DALnet
//
// Original author: Dave2001 (aka. CM256 or CodeMaster256), Dave2999@hotmail.com
// Other authors listed by the code that they submitted.
//
//****************************************************************

#include "Gfx #1.3.h"
#include "util.h"
#include "combine.h"
#include "3dmath.h"
#include "Debugger.h"

#define Vj rdp.vtxbuf2[j]
#define Vi rdp.vtxbuf2[i]

VERTEX *vtx_list1[32];  // vertex indexing
VERTEX *vtx_list2[32];

//
// util_init - initialize data for the functions in this file
//

void util_init ()
{
    for (int i=0; i<32; i++)
    {
        vtx_list1[i] = &rdp.vtx1[i];
        vtx_list2[i] = &rdp.vtx2[i];
    }
}

//software backface culling. Gonetz
// mega modifications by Dave2001
BOOL cull_tri(VERTEX **v)   // type changed to VERTEX** [Dave2001]
{
    int i;

    // Check if completely off the screen (quick frustrum clipping for 90 FOV)
    if ((v[0]->x < -v[0]->z && v[1]->x < -v[1]->z && v[2]->x < -v[2]->z) ||
        (v[0]->x > v[0]->z && v[1]->x > v[1]->z && v[2]->x > v[2]->z) ||
        (v[0]->y < -v[0]->z && v[1]->y < -v[1]->z && v[2]->y < -v[2]->z) ||
        (v[0]->y > v[0]->z && v[1]->y > v[1]->z && v[2]->y > v[2]->z) ||
        (v[0]->z < 0.1f && v[1]->z < 0.1f && v[2]->z < 0.1f))
    {
        RDP (" clipped\n");
        return TRUE;
    }
    
    // Triangle can't be culled, if it need clipping
    BOOL draw = 0;
    for (i=0; i<3; i++)
    {
        if (!v[i]->screen_translated)
        {
            v[i]->sx = rdp.view_trans[0] + v[i]->x_z * rdp.view_scale[0];
            v[i]->sy = rdp.view_trans[1] + v[i]->y_z * rdp.view_scale[1];
            v[i]->screen_translated = 1;
        }

        if (v[i]->z < 0.01f) //need clip_z. can't be culled now
            draw = 1;
    }

    if (draw) return FALSE; // z-clipping, can't be culled by software
        
#define SW_CULLING
#ifdef SW_CULLING
    //now we need to check, if triangle's vertices are in clockwise order
    // Use precalculated x/z and y/z coordinates.
    float x1 = v[0]->sx - v[1]->sx;
    float y1 = v[0]->sy - v[1]->sy;
    float x2 = v[2]->sx - v[1]->sx;
    float y2 = v[2]->sy - v[1]->sy;
    
    DWORD mode = (rdp.flags & CULLMASK) >> CULLSHIFT;
    switch (mode)
    {
    case 1: // cull front
        if ((x1*y2 - y1*x2) < 0.0f) //counter-clockwise, positive
        {
            RDP (" culled!\n");
            return TRUE;
        }
        return FALSE;
    case 2: // cull back
        if ((x1*y2 - y1*x2) >= 0.0f) //clockwise, negative
        {
            RDP (" culled!\n");
            return TRUE;
        }
        return FALSE;
    }
#endif

    return FALSE;
}

void DrawTri (VERTEX **vtx)
{
    for (int i=0; i<3; i++)
    {
        VERTEX *v = vtx[i];

        if (v->uv_calculated != rdp.tex_ctr)
        {
            v->uv_calculated = rdp.tex_ctr;

            if (!(rdp.geom_mode & 0x00020000))
            {
                if (!(rdp.geom_mode & 0x00000200))
                {
                    if (rdp.geom_mode & 0x00000004) // flat shading
                    {
                        int flag = (rdp.cmd1 >> 24) & 3;
                        v->a = v[flag].a;
                        v->b = v[flag].b;
                        v->g = v[flag].g;
                        v->r = v[flag].r;
                    }
                    else    // prim color
                    {
                        v->a = (BYTE)((rdp.prim_color >> 24) & 0xFF);
                        v->b = (BYTE)(rdp.prim_color & 0xFF);
                        v->g = (BYTE)((rdp.prim_color >> 8) & 0xFF);
                        v->r = (BYTE)((rdp.prim_color >> 16) & 0xFF);
                    }
                }
            }

            if (rdp.cmb_flags & CMB_MULT)
            {
                if (rdp.col[0] > 1.0f) rdp.col[0] = 1.0f;
                if (rdp.col[1] > 1.0f) rdp.col[1] = 1.0f;
                if (rdp.col[2] > 1.0f) rdp.col[2] = 1.0f;
                if (rdp.col[0] < 0.0f) rdp.col[0] = 0.0f;
                if (rdp.col[1] < 0.0f) rdp.col[1] = 0.0f;
                if (rdp.col[2] < 0.0f) rdp.col[2] = 0.0f;
                v->r = (BYTE)(v->r * rdp.col[0]);
                v->g = (BYTE)(v->g * rdp.col[1]);
                v->b = (BYTE)(v->b * rdp.col[2]);
            }
            if (rdp.cmb_flags & CMB_A_MULT)
            {
                if (rdp.col[3] > 1.0f) rdp.col[3] = 1.0f;
                if (rdp.col[3] < 0.0f) rdp.col[3] = 0.0f;
                v->a = (BYTE)(v->a * rdp.col[3]);
            }

            if (rdp.cmb_flags & CMB_SET)
            {
                if (rdp.col[0] > 1.0f) rdp.col[0] = 1.0f;
                if (rdp.col[1] > 1.0f) rdp.col[1] = 1.0f;
                if (rdp.col[2] > 1.0f) rdp.col[2] = 1.0f;
                if (rdp.col[0] < 0.0f) rdp.col[0] = 0.0f;
                if (rdp.col[1] < 0.0f) rdp.col[1] = 0.0f;
                if (rdp.col[2] < 0.0f) rdp.col[2] = 0.0f;
                v->r = (BYTE)(255.0f * rdp.col[0]);
                v->g = (BYTE)(255.0f * rdp.col[1]);
                v->b = (BYTE)(255.0f * rdp.col[2]);
            }
            if (rdp.cmb_flags & CMB_A_SET)
            {
                if (rdp.col[3] > 1.0f) rdp.col[3] = 1.0f;
                if (rdp.col[3] < 0.0f) rdp.col[3] = 0.0f;
                v->a = (BYTE)(255.0f * rdp.col[3]);
            }

            // Fix texture coordinates
            if ((rdp.geom_mode & 0x00060000) != 0x00060000)
            {
                if (rdp.tex >= 1 && rdp.cur_cache[0])
                {
                    v->u0 = v->ou * rdp.tiles[rdp.cur_tile].s_scale;
                    v->v0 = v->ov * rdp.tiles[rdp.cur_tile].t_scale;

                    if (rdp.tiles[rdp.cur_tile].shift_s)
                    {
                        if (rdp.tiles[rdp.cur_tile].shift_s > 10)
                            v->u0 *= (float)(1 << (16 - rdp.tiles[rdp.cur_tile].shift_s));
                        else
                            v->u0 /= (float)(1 << rdp.tiles[rdp.cur_tile].shift_s);
                    }
                    if (rdp.tiles[rdp.cur_tile].shift_t)
                    {
                        if (rdp.tiles[rdp.cur_tile].shift_t > 10)
                            v->v0 *= (float)(1 << (16 - rdp.tiles[rdp.cur_tile].shift_t));
                        else
                            v->v0 /= (float)(1 << rdp.tiles[rdp.cur_tile].shift_t);
                    }

                    v->u0 -= rdp.tiles[rdp.cur_tile].f_ul_s;
                    v->v0 -= rdp.tiles[rdp.cur_tile].f_ul_t;

                    v->u0 -= rdp.cur_cache[0]->offset_s;
                    v->v0 -= rdp.cur_cache[0]->offset_t;

                    v->u0 += 0.5f;
                    v->v0 += 0.5f;

                    v->u0 *= rdp.cur_cache[0]->scale;
                    v->v0 *= rdp.cur_cache[0]->scale;

                    v->u0_z = v->u0 / v->z;
                    v->v0_z = v->v0 / v->z;
                }
                if (rdp.tex >= 2 && rdp.cur_cache[1])
                {
                    v->u1 = v->ou * rdp.tiles[rdp.cur_tile].s_scale;
                    v->v1 = v->ov * rdp.tiles[rdp.cur_tile].t_scale;

                    if (rdp.tiles[rdp.cur_tile+1].shift_s)
                    {
                        if (rdp.tiles[rdp.cur_tile+1].shift_s > 10)
                            v->u1 *= (float)(1 << (16 - rdp.tiles[rdp.cur_tile+1].shift_s));
                        else
                            v->u1 /= (float)(1 << rdp.tiles[rdp.cur_tile+1].shift_s);
                    }
                    if (rdp.tiles[rdp.cur_tile+1].shift_t)
                    {
                        if (rdp.tiles[rdp.cur_tile+1].shift_t > 10)
                            v->v1 *= (float)(1 << (16 - rdp.tiles[rdp.cur_tile+1].shift_t));
                        else
                            v->v1 /= (float)(1 << rdp.tiles[rdp.cur_tile+1].shift_t);
                    }

                    v->u1 -= rdp.tiles[rdp.cur_tile+1].f_ul_s;
                    v->v1 -= rdp.tiles[rdp.cur_tile+1].f_ul_t;

                    v->u1 -= rdp.cur_cache[1]->offset_s;
                    v->v1 -= rdp.cur_cache[1]->offset_t;

                    v->u1 += 0.5f;
                    v->v1 += 0.5f;

                    v->u1 *= rdp.cur_cache[1]->scale;
                    v->v1 *= rdp.cur_cache[1]->scale;

                    v->u1_z = v->u1 / v->z;
                    v->v1_z = v->v1 / v->z;
                }
            }
            else
            {
                // If we are spherical mapping, we still must divide by z
                if (rdp.tex >= 1)
                {
                    v->u0_z = v->u0 / v->z;
                    v->v0_z = v->v0 / v->z;
                }
                if (rdp.tex >= 2)
                {
                    v->u1_z = v->u1 / v->z;
                    v->v1_z = v->v1 / v->z;
                }
            }
        }
    }

    VERTEX v[3];

    v[0] = *vtx[0];
    v[1] = *vtx[1];
    v[2] = *vtx[2];

    clip_z (v);

    for (i=0; i<rdp.n_global; i++)
    {
        // Calculate screen coords (precalculate if not z-clipped)
        if (rdp.vtxbuf[i].not_zclipped)
        {
            rdp.vtxbuf[i].x = rdp.vtxbuf[i].sx;
            rdp.vtxbuf[i].y = rdp.vtxbuf[i].sy;
            rdp.vtxbuf[i].q = rdp.vtxbuf[i].ooz;
            rdp.vtxbuf[i].u0 = rdp.vtxbuf[i].u0_z;
            rdp.vtxbuf[i].v0 = rdp.vtxbuf[i].v0_z;
            rdp.vtxbuf[i].u1 = rdp.vtxbuf[i].u1_z;
            rdp.vtxbuf[i].v1 = rdp.vtxbuf[i].v1_z;
        }
        else
        {
            rdp.vtxbuf[i].x = rdp.view_trans[0] + rdp.vtxbuf[i].x / rdp.vtxbuf[i].z * rdp.view_scale[0];
            rdp.vtxbuf[i].y = rdp.view_trans[1] + rdp.vtxbuf[i].y / rdp.vtxbuf[i].z * rdp.view_scale[1];
            rdp.vtxbuf[i].q = 1.0f / rdp.vtxbuf[i].z;
            if (rdp.tex >= 1)
            {
                rdp.vtxbuf[i].u0 *= rdp.vtxbuf[i].q;
                rdp.vtxbuf[i].v0 *= rdp.vtxbuf[i].q;
            }
            if (rdp.tex >= 2)
            {
                rdp.vtxbuf[i].u1 *= rdp.vtxbuf[i].q;
                rdp.vtxbuf[i].v1 *= rdp.vtxbuf[i].q;
            }
        }

        // Don't remove clipping, or it will freeze
        if (rdp.vtxbuf[i].x >= rdp.scissor.lr_x) rdp.clip |= CLIP_XMAX;
        if (rdp.vtxbuf[i].x < rdp.scissor.ul_x) rdp.clip |= CLIP_XMIN;
        if (rdp.vtxbuf[i].y >= rdp.scissor.lr_y) rdp.clip |= CLIP_YMAX;
        if (rdp.vtxbuf[i].y < rdp.scissor.ul_y) rdp.clip |= CLIP_YMIN;
    }

    clip_tri ();
}

//
// clip_z - clips along the z-axis, also copies the vertex buffer for clip_tri
//   * ALWAYS * processes it, even if it does not need z-clipping.  It needs
//   to copy the buffer anyway.
//

void clip_z (VERTEX v[3])
{
    int i,j,index;
    float percent;

    // Set vertex buffers
    rdp.vtxbuf = rdp.vtx1;  // copy from v to rdp.vtx1
    rdp.vtxbuf2 = v;
    index = 0;
    rdp.vtx_buffer = 0;

    // Check the vertices for clipping
    for (i=0; i<3; i++)
    {
        j = i+1;
        if (j == 3) j = 0;

        if (Vi.z >= 0.01f)
        {
            if (Vj.z >= 0.01f)      // Both are in, save the last one
            {
                rdp.vtxbuf[index] = Vj;
                rdp.vtxbuf[index++].not_zclipped = 1;
            }
            else            // First is in, second is out, save intersection
            {
                percent = (-Vi.z) / (Vj.z - Vi.z);
                rdp.vtxbuf[index].not_zclipped = 0;
                rdp.vtxbuf[index].x = Vi.x + (Vj.x - Vi.x) * percent;
                rdp.vtxbuf[index].y = Vi.y + (Vj.y - Vi.y) * percent;
                rdp.vtxbuf[index].z = 0.01f;
                rdp.vtxbuf[index].u0 = Vi.u0 + (Vj.u0 - Vi.u0) * percent;
                rdp.vtxbuf[index].v0 = Vi.v0 + (Vj.v0 - Vi.v0) * percent;
                rdp.vtxbuf[index].u1 = Vi.u1 + (Vj.u1 - Vi.u1) * percent;
                rdp.vtxbuf[index].v1 = Vi.v1 + (Vj.v1 - Vi.v1) * percent;
                rdp.vtxbuf[index].b = (BYTE)(Vi.b + (Vj.b - Vi.b) * percent);
                rdp.vtxbuf[index].g = (BYTE)(Vi.g + (Vj.g - Vi.g) * percent);
                rdp.vtxbuf[index].r = (BYTE)(Vi.r + (Vj.r - Vi.r) * percent);
                rdp.vtxbuf[index++].a = (BYTE)(Vi.a + (Vj.a - Vi.a) * percent);
            }
        }
        else
        {
            //if (Vj.z < 0.01f) // Both are out, save nothing
            if (Vj.z >= 0.01f)  // First is out, second is in, save intersection & in point
            {
                percent = (-Vj.z) / (Vi.z - Vj.z);
                rdp.vtxbuf[index].not_zclipped = 0;
                rdp.vtxbuf[index].x = Vj.x + (Vi.x - Vj.x) * percent;
                rdp.vtxbuf[index].y = Vj.y + (Vi.y - Vj.y) * percent;
                rdp.vtxbuf[index].z = 0.01f;
                rdp.vtxbuf[index].u0 = Vj.u0 + (Vi.u0 - Vj.u0) * percent;
                rdp.vtxbuf[index].v0 = Vj.v0 + (Vi.v0 - Vj.v0) * percent;
                rdp.vtxbuf[index].u1 = Vj.u1 + (Vi.u1 - Vj.u1) * percent;
                rdp.vtxbuf[index].v1 = Vj.v1 + (Vi.v1 - Vj.v1) * percent;
                rdp.vtxbuf[index].b = (BYTE)(Vj.b + (Vi.b - Vj.b) * percent);
                rdp.vtxbuf[index].g = (BYTE)(Vj.g + (Vi.g - Vj.g) * percent);
                rdp.vtxbuf[index].r = (BYTE)(Vj.r + (Vi.r - Vj.r) * percent);
                rdp.vtxbuf[index++].a = (BYTE)(Vj.a + (Vi.a - Vj.a) * percent);

                // Save the in point
                rdp.vtxbuf[index] = Vj;
                rdp.vtxbuf[index++].not_zclipped = 1;
            }
        }
    }
    
    rdp.n_global = index;

    rdp.vtxbuf2 = rdp.vtx2; // this needs to be buffer 2 so that it can swap later
}

void clip_tri ()
{
    int i,j,index,n=rdp.n_global;
    float percent;

    // rdp.vtxbuf and rdp.vtxbuf2 were set by clip_z

    // Check which clipping is needed
    if (rdp.clip & CLIP_XMAX)   // right of the screen
    {
        // Swap vertex buffers
        VERTEX *tmp = rdp.vtxbuf2;
        rdp.vtxbuf2 = rdp.vtxbuf;
        rdp.vtxbuf = tmp;
        rdp.vtx_buffer ^= 1;
        index = 0;

        // Check the vertices for clipping
        for (i=0; i<n; i++)
        {
            j = i+1;
            if (j == n) j = 0;

            if (Vi.x < rdp.scissor.lr_x)
            {
                if (Vj.x < rdp.scissor.lr_x)        // Both are in, save the last one
                {
                    rdp.vtxbuf[index++] = Vj;
                }
                else            // First is in, second is out, save intersection
                {
                    percent = (rdp.scissor.lr_x - Vi.x) / (Vj.x - Vi.x);
                    rdp.vtxbuf[index].x = (float)rdp.scissor.lr_x;
                    rdp.vtxbuf[index].y = Vi.y + (Vj.y - Vi.y) * percent;
                    rdp.vtxbuf[index].q = Vi.q + (Vj.q - Vi.q) * percent;
                    rdp.vtxbuf[index].u0 = Vi.u0 + (Vj.u0 - Vi.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vi.v0 + (Vj.v0 - Vi.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vi.u1 + (Vj.u1 - Vi.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vi.v1 + (Vj.v1 - Vi.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vi.b + (Vj.b - Vi.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vi.g + (Vj.g - Vi.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vi.r + (Vj.r - Vi.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vi.a + (Vj.a - Vi.a) * percent);
                }
            }
            else
            {
                //if (Vj.x >= rdp.scissor.lr_x) // Both are out, save nothing
                if (Vj.x < rdp.scissor.lr_x)    // First is out, second is in, save intersection & in point
                {
                    percent = (rdp.scissor.lr_x - Vj.x) / (Vi.x - Vj.x);
                    rdp.vtxbuf[index].x = (float)rdp.scissor.lr_x;
                    rdp.vtxbuf[index].y = Vj.y + (Vi.y - Vj.y) * percent;
                    rdp.vtxbuf[index].q = Vj.q + (Vi.q - Vj.q) * percent;
                    rdp.vtxbuf[index].u0 = Vj.u0 + (Vi.u0 - Vj.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vj.v0 + (Vi.v0 - Vj.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vj.u1 + (Vi.u1 - Vj.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vj.v1 + (Vi.v1 - Vj.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vj.b + (Vi.b - Vj.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vj.g + (Vi.g - Vj.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vj.r + (Vi.r - Vj.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vj.a + (Vi.a - Vj.a) * percent);

                    // Save the in point
                    rdp.vtxbuf[index++] = Vj;
                }
            }
        }
        n = index;
    }
    if (rdp.clip & CLIP_XMIN)   // left of the screen
    {
        // Swap vertex buffers
        VERTEX *tmp = rdp.vtxbuf2;
        rdp.vtxbuf2 = rdp.vtxbuf;
        rdp.vtxbuf = tmp;
        rdp.vtx_buffer ^= 1;
        index = 0;

        // Check the vertices for clipping
        for (i=0; i<n; i++)
        {
            j = i+1;
            if (j == n) j = 0;

            if (Vi.x >= rdp.scissor.ul_x)
            {
                if (Vj.x >= rdp.scissor.ul_x)       // Both are in, save the last one
                {
                    rdp.vtxbuf[index++] = Vj;
                }
                else            // First is in, second is out, save intersection
                {
                    percent = (rdp.scissor.ul_x - Vi.x) / (Vj.x - Vi.x);
                    rdp.vtxbuf[index].x = (float)rdp.scissor.ul_x;
                    rdp.vtxbuf[index].y = Vi.y + (Vj.y - Vi.y) * percent;
                    rdp.vtxbuf[index].q = Vi.q + (Vj.q - Vi.q) * percent;
                    rdp.vtxbuf[index].u0 = Vi.u0 + (Vj.u0 - Vi.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vi.v0 + (Vj.v0 - Vi.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vi.u1 + (Vj.u1 - Vi.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vi.v1 + (Vj.v1 - Vi.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vi.b + (Vj.b - Vi.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vi.g + (Vj.g - Vi.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vi.r + (Vj.r - Vi.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vi.a + (Vj.a - Vi.a) * percent);
                }
            }
            else
            {
                //if (Vj.x < rdp.scissor.ul_x)  // Both are out, save nothing
                if (Vj.x >= rdp.scissor.ul_x)   // First is out, second is in, save intersection & in point
                {
                    percent = (rdp.scissor.ul_x - Vj.x) / (Vi.x - Vj.x);
                    rdp.vtxbuf[index].x = (float)rdp.scissor.ul_x;
                    rdp.vtxbuf[index].y = Vj.y + (Vi.y - Vj.y) * percent;
                    rdp.vtxbuf[index].q = Vj.q + (Vi.q - Vj.q) * percent;
                    rdp.vtxbuf[index].u0 = Vj.u0 + (Vi.u0 - Vj.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vj.v0 + (Vi.v0 - Vj.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vj.u1 + (Vi.u1 - Vj.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vj.v1 + (Vi.v1 - Vj.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vj.b + (Vi.b - Vj.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vj.g + (Vi.g - Vj.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vj.r + (Vi.r - Vj.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vj.a + (Vi.a - Vj.a) * percent);

                    // Save the in point
                    rdp.vtxbuf[index++] = Vj;
                }
            }
        }
        n = index;
    }
    if (rdp.clip & CLIP_YMAX)   // top of the screen
    {
        // Swap vertex buffers
        VERTEX *tmp = rdp.vtxbuf2;
        rdp.vtxbuf2 = rdp.vtxbuf;
        rdp.vtxbuf = tmp;
        rdp.vtx_buffer ^= 1;
        index = 0;

        // Check the vertices for clipping
        for (i=0; i<n; i++)
        {
            j = i+1;
            if (j == n) j = 0;

            if (Vi.y < rdp.scissor.lr_y)
            {
                if (Vj.y < rdp.scissor.lr_y)        // Both are in, save the last one
                {
                    rdp.vtxbuf[index++] = Vj;
                }
                else            // First is in, second is out, save intersection
                {
                    percent = (rdp.scissor.lr_y - Vi.y) / (Vj.y - Vi.y);
                    rdp.vtxbuf[index].x = Vi.x + (Vj.x - Vi.x) * percent;
                    rdp.vtxbuf[index].y = (float)rdp.scissor.lr_y;
                    rdp.vtxbuf[index].q = Vi.q + (Vj.q - Vi.q) * percent;
                    rdp.vtxbuf[index].u0 = Vi.u0 + (Vj.u0 - Vi.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vi.v0 + (Vj.v0 - Vi.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vi.u1 + (Vj.u1 - Vi.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vi.v1 + (Vj.v1 - Vi.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vi.b + (Vj.b - Vi.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vi.g + (Vj.g - Vi.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vi.r + (Vj.r - Vi.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vi.a + (Vj.a - Vi.a) * percent);
                }
            }
            else
            {
                //if (Vj.y >= rdp.scissor.lr_y) // Both are out, save nothing
                if (Vj.y < rdp.scissor.lr_y)    // First is out, second is in, save intersection & in point
                {
                    percent = (rdp.scissor.lr_y - Vj.y) / (Vi.y - Vj.y);
                    rdp.vtxbuf[index].x = Vj.x + (Vi.x - Vj.x) * percent;
                    rdp.vtxbuf[index].y = (float)rdp.scissor.lr_y;
                    rdp.vtxbuf[index].q = Vj.q + (Vi.q - Vj.q) * percent;
                    rdp.vtxbuf[index].u0 = Vj.u0 + (Vi.u0 - Vj.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vj.v0 + (Vi.v0 - Vj.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vj.u1 + (Vi.u1 - Vj.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vj.v1 + (Vi.v1 - Vj.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vj.b + (Vi.b - Vj.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vj.g + (Vi.g - Vj.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vj.r + (Vi.r - Vj.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vj.a + (Vi.a - Vj.a) * percent);

                    // Save the in point
                    rdp.vtxbuf[index++] = Vj;
                }
            }
        }
        n = index;
    }
    if (rdp.clip & CLIP_YMIN)   // bottom of the screen
    {
        // Swap vertex buffers
        VERTEX *tmp = rdp.vtxbuf2;
        rdp.vtxbuf2 = rdp.vtxbuf;
        rdp.vtxbuf = tmp;
        rdp.vtx_buffer ^= 1;
        index = 0;

        // Check the vertices for clipping
        for (i=0; i<n; i++)
        {
            j = i+1;
            if (j == n) j = 0;

            if (Vi.y >= rdp.scissor.ul_y)
            {
                if (Vj.y >= rdp.scissor.ul_y)       // Both are in, save the last one
                {
                    rdp.vtxbuf[index++] = Vj;
                }
                else            // First is in, second is out, save intersection
                {
                    percent = (rdp.scissor.ul_y - Vi.y) / (Vj.y - Vi.y);
                    rdp.vtxbuf[index].x = Vi.x + (Vj.x - Vi.x) * percent;
                    rdp.vtxbuf[index].y = (float)rdp.scissor.ul_y;
                    rdp.vtxbuf[index].q = Vi.q + (Vj.q - Vi.q) * percent;
                    rdp.vtxbuf[index].u0 = Vi.u0 + (Vj.u0 - Vi.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vi.v0 + (Vj.v0 - Vi.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vi.u1 + (Vj.u1 - Vi.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vi.v1 + (Vj.v1 - Vi.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vi.b + (Vj.b - Vi.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vi.g + (Vj.g - Vi.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vi.r + (Vj.r - Vi.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vi.a + (Vj.a - Vi.a) * percent);
                }
            }
            else
            {
                //if (Vj.y < rdp.scissor.ul_y)  // Both are out, save nothing
                if (Vj.y >= rdp.scissor.ul_y)   // First is out, second is in, save intersection & in point
                {
                    percent = (rdp.scissor.ul_y - Vj.y) / (Vi.y - Vj.y);
                    rdp.vtxbuf[index].x = Vj.x + (Vi.x - Vj.x) * percent;
                    rdp.vtxbuf[index].y = (float)rdp.scissor.ul_y;
                    rdp.vtxbuf[index].q = Vj.q + (Vi.q - Vj.q) * percent;
                    rdp.vtxbuf[index].u0 = Vj.u0 + (Vi.u0 - Vj.u0) * percent;
                    rdp.vtxbuf[index].v0 = Vj.v0 + (Vi.v0 - Vj.v0) * percent;
                    rdp.vtxbuf[index].u1 = Vj.u1 + (Vi.u1 - Vj.u1) * percent;
                    rdp.vtxbuf[index].v1 = Vj.v1 + (Vi.v1 - Vj.v1) * percent;
                    rdp.vtxbuf[index].b = (BYTE)(Vj.b + (Vi.b - Vj.b) * percent);
                    rdp.vtxbuf[index].g = (BYTE)(Vj.g + (Vi.g - Vj.g) * percent);
                    rdp.vtxbuf[index].r = (BYTE)(Vj.r + (Vi.r - Vj.r) * percent);
                    rdp.vtxbuf[index++].a = (BYTE)(Vj.a + (Vi.a - Vj.a) * percent);

                    // Save the in point
                    rdp.vtxbuf[index++] = Vj;
                }
            }
        }
        n = index;
    }

    if (n < 3) return;

    if (fullscreen)
    {
        if (settings.wireframe)
        {
            for (i=0; i<n; i++)
            {
                j = i+1;
                if (j == n) j = 0;
                grDrawLine (&rdp.vtxbuf[i], &rdp.vtxbuf[j]);
            }
        }
        else
        {
            grDrawVertexArray (GR_TRIANGLE_FAN, n, rdp.vtx_buffer?(&vtx_list2):(&vtx_list1));
        }
    }

    if (debug.capture) add_tri (rdp.vtxbuf, n);
}

void add_tri (VERTEX *v, int n)
{
    //FRDP ("ENTER (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n", v[0].x, v[0].y, v[0].z,
    //  v[1].x, v[1].y, v[1].z, v[2].x, v[2].y, v[2].z);

    // Debug capture
    if (debug.capture)
    {
        TRI_INFO *info = new TRI_INFO;
        info->nv = n;
        info->v = new VERTEX [n];
        memcpy (info->v, v, sizeof(VERTEX)*n);
        info->cycle_mode = rdp.cycle_mode;
        info->cycle1 = rdp.cycle1;
        info->cycle2 = rdp.cycle2;
        info->uncombined = rdp.uncombined;
        info->geom_mode = rdp.geom_mode;
        info->render_mode = rdp.render_mode;
        info->mode_h = rdp.mode_h;
        info->tri_n = rdp.tri_n;
        info->temp = rdp.temp;

        for (int i=0; i<2; i++)
        {
            int j = rdp.cur_tile+i;
            info->t[i].cur_cache = rdp.cur_cache_n[j];
            info->t[i].format = rdp.tiles[j].format;
            info->t[i].size = rdp.tiles[j].size;
            info->t[i].width = rdp.tiles[j].width;
            info->t[i].height = rdp.tiles[j].height;
            info->t[i].line = rdp.tiles[j].line;
            info->t[i].palette = rdp.tiles[j].palette;
            info->t[i].clamp_s = rdp.tiles[j].clamp_s;
            info->t[i].clamp_t = rdp.tiles[j].clamp_t;
            info->t[i].mirror_s = rdp.tiles[j].mirror_s;
            info->t[i].mirror_t = rdp.tiles[j].mirror_t;
            info->t[i].shift_s = rdp.tiles[j].shift_s;
            info->t[i].shift_t = rdp.tiles[j].shift_t;
            info->t[i].mask_s = rdp.tiles[j].mask_s;
            info->t[i].mask_t = rdp.tiles[j].mask_t;
            info->t[i].ul_s = rdp.tiles[j].ul_s;
            info->t[i].ul_t = rdp.tiles[j].ul_t;
            info->t[i].lr_s = rdp.tiles[j].lr_s;
            info->t[i].lr_t = rdp.tiles[j].lr_t;
            info->t[i].t_ul_s = rdp.tiles[7].t_ul_s;
            info->t[i].t_ul_t = rdp.tiles[7].t_ul_t;
            info->t[i].t_lr_s = rdp.tiles[7].t_lr_s;
            info->t[i].t_lr_t = rdp.tiles[7].t_lr_t;
            info->t[i].scale_s = rdp.tiles[j].s_scale;
            info->t[i].scale_t = rdp.tiles[j].t_scale;
        }

        info->fog_color = rdp.fog_color;
        info->fill_color = rdp.fill_color;
        info->prim_color = rdp.prim_color;
        info->blend_color = rdp.blend_color;
        info->env_color = rdp.env_color;
        info->real_fog_color = rdp.real_fog_color;
        info->real_fill_color = rdp.real_fill_color;
        info->real_prim_color = rdp.real_prim_color;
        info->real_blend_color = rdp.real_blend_color;
        info->real_env_color = rdp.real_env_color;
        info->prim_lodmin = rdp.prim_lodmin;
        info->prim_lodfrac = rdp.prim_lodfrac;

        info->pNext = debug.tri_list;
        debug.tri_list = info;

        if (debug.tri_last == NULL)
            debug.tri_last = debug.tri_list;
    }
}

//
// update - update states if they need it
//

void update ()
{
    RDP ("-+ update called\n");
    // Check for rendermode changes
    // Z buffer
    if (rdp.render_mode_changed & 0x00000C30)
    {
        FRDP (" |- render_mode_changed zbuf - decal: %s, update: %s, compare: %s\n",
            str_yn[(rdp.render_mode&0x00000C00)?1:0],
            str_yn[(rdp.render_mode&0x00000020)?1:0],
            str_yn[(rdp.render_mode&0x00000010)?1:0]);

        rdp.render_mode_changed &= ~0x00000C30;
        rdp.update |= UPDATE_ZBUF_ENABLED;

        // Decal?
        if ((rdp.render_mode & 0x00000C00) == 0x00000C00)
            rdp.flags |= ZBUF_DECAL;
        else
            rdp.flags &= ~ZBUF_DECAL;

        // Update?
        if (rdp.render_mode & 0x00000020)
            rdp.flags |= ZBUF_UPDATE;
        else
            rdp.flags &= ~ZBUF_UPDATE;

        // Compare?
        if (rdp.render_mode & 0x00000010)
            rdp.flags |= ZBUF_COMPARE;
        else
            rdp.flags &= ~ZBUF_COMPARE;
    }

    // Alpha compare
    if (rdp.render_mode_changed & 0x00001000)
    {
        FRDP (" |- render_mode_changed alpha compare - on: %s\n",
            str_yn[(rdp.render_mode&0x00001000)?1:0]);
        rdp.render_mode_changed &= ~0x00001000;
        rdp.update |= UPDATE_ALPHA_COMPARE;

        if (rdp.render_mode & 0x00001000)
            rdp.flags |= ALPHA_COMPARE;
        else
            rdp.flags &= ~ALPHA_COMPARE;
    }

    if (rdp.render_mode_changed & 0x00002000)   // alpha cvg sel
    {
        FRDP (" |- render_mode_changed alpha cvg sel - on: %s\n",
            str_yn[(rdp.render_mode&0x00002000)?1:0]);
        rdp.render_mode_changed &= ~0x00002000;
        rdp.update |= UPDATE_COMBINE;
    }

    // Force blend
    if (rdp.render_mode_changed & 0xFFFF0000)
    {
        FRDP (" |- render_mode_changed force_blend - %08lx\n", rdp.render_mode&0xFFFF0000);
        rdp.render_mode_changed &= 0x0000FFFF;

        rdp.fbl_a0 = (BYTE)((rdp.render_mode>>30)&0x3);
        rdp.fbl_b0 = (BYTE)((rdp.render_mode>>26)&0x3);
        rdp.fbl_c0 = (BYTE)((rdp.render_mode>>22)&0x3);
        rdp.fbl_d0 = (BYTE)((rdp.render_mode>>18)&0x3);
        rdp.fbl_a1 = (BYTE)((rdp.render_mode>>28)&0x3);
        rdp.fbl_b1 = (BYTE)((rdp.render_mode>>24)&0x3);
        rdp.fbl_c1 = (BYTE)((rdp.render_mode>>20)&0x3);
        rdp.fbl_d1 = (BYTE)((rdp.render_mode>>16)&0x3);

        rdp.update |= UPDATE_COMBINE;
    }

    if (rdp.update & UPDATE_CLEAR_CACHE)
    {
        RDP (" |- clear_cache\n");
        rdp.update ^= UPDATE_CLEAR_CACHE;

        rdp.tmem_ptr[0] = offset_textures;
        rdp.tmem_ptr[1] = offset_textures;
        rdp.n_cached[0] = 0;
        rdp.n_cached[1] = 0;
    }

    //if (fullscreen)
    //{
        // Combine MUST go before texture
        if ((rdp.update & UPDATE_COMBINE) && rdp.allow_combine)
        {
            RDP (" |-+ update_combine\n");
            rdp.update ^= UPDATE_COMBINE;
            Combine ();
        }

        if (rdp.update & UPDATE_TEXTURE)
        {
            rdp.update ^= UPDATE_TEXTURE;

            rdp.tex_ctr ++;
            if (rdp.tex_ctr == 0xFFFFFFFF)
                rdp.tex_ctr = 0;

            for (int tmu=0; tmu<rdp.tex; tmu++)
            {
                FRDP (" |-+ update_texture #%d\n", tmu);
                TexCache (tmu, tmu);

                if (fullscreen)
                {
                    int tile = rdp.cur_tile+tmu;

                    grTexFilterMode (tmu,
                        (settings.filtering==1)?GR_TEXTUREFILTER_BILINEAR:GR_TEXTUREFILTER_POINT_SAMPLED,
                        (settings.filtering==1)?GR_TEXTUREFILTER_BILINEAR:GR_TEXTUREFILTER_POINT_SAMPLED);

                    DWORD mode_s, mode_t;

                    if ((rdp.tiles[tile].clamp_s || rdp.tiles[tile].mask_s == 0) &&
                        rdp.tiles[tile].lr_s-rdp.tiles[tile].ul_s < 256)
                        mode_s = GR_TEXTURECLAMP_CLAMP;
                    else
                    {
                        if (rdp.tiles[tile].mirror_s)
                            mode_s = GR_TEXTURECLAMP_MIRROR_EXT;
                        else
                            mode_s = GR_TEXTURECLAMP_WRAP;
                    }

                    if ((rdp.tiles[tile].clamp_t || rdp.tiles[tile].mask_t == 0) &&
                        rdp.tiles[tile].lr_t-rdp.tiles[tile].ul_t < 256)
                        mode_t = GR_TEXTURECLAMP_CLAMP;
                    else
                    {
                        if (rdp.tiles[tile].mirror_t)
                            mode_t = GR_TEXTURECLAMP_MIRROR_EXT;
                        else
                            mode_t = GR_TEXTURECLAMP_WRAP;
                    }

                    grTexClampMode (tmu,
                        mode_s,
                        mode_t);
                }
            }
        }

    if (fullscreen)
    {
        // Z buffer
        if (rdp.update & UPDATE_ZBUF_ENABLED)
        {
            // already logged above
            rdp.update ^= UPDATE_ZBUF_ENABLED;

            if (rdp.flags & ZBUF_DECAL)
            {
                grDepthBiasLevel (0x00);
                grDepthMask(FXFALSE);
                if ((rdp.flags & ZBUF_ENABLED) && (rdp.flags & ZBUF_COMPARE))
                    grDepthBufferFunction (GR_CMP_LESS);
                else
                    grDepthBufferFunction (GR_CMP_ALWAYS);
            }
            else
            {
                grDepthBiasLevel (0x20);
                if (rdp.flags & ZBUF_ENABLED)
                {
                    if (rdp.flags & ZBUF_COMPARE)
                        grDepthBufferFunction (GR_CMP_LESS);
                    else
                        grDepthBufferFunction (GR_CMP_ALWAYS);

                    if (rdp.flags & ZBUF_UPDATE)
                        grDepthMask (FXTRUE);
                    else
                        grDepthMask (FXFALSE);
                }
                else
                {
                    grDepthBufferFunction (GR_CMP_ALWAYS);
                    grDepthMask (FXFALSE);
                }
            }
        }

        // Alpha compare
        if (rdp.update & UPDATE_ALPHA_COMPARE)
        {
            // already logged above
            rdp.update ^= UPDATE_ALPHA_COMPARE;

            if (rdp.flags & ALPHA_COMPARE)
            {
                grAlphaTestFunction (GR_CMP_GEQUAL);
                grAlphaTestReferenceValue (/*(BYTE)(rdp.real_fill_color & 0xFF)*/0xFF);
            }
            else
            {
                if (rdp.acmp == 1)
                {
                    grAlphaTestFunction (GR_CMP_GEQUAL);
                    grAlphaTestReferenceValue ((BYTE)(rdp.real_blend_color&0xFF));
                }
                else
                {
                    grAlphaTestFunction (GR_CMP_ALWAYS);
                }
            }
        }

        // Cull mode (leave this in for z-clipped triangles)
        if (rdp.update & UPDATE_CULL_MODE)
        {
            rdp.update ^= UPDATE_CULL_MODE;
            DWORD mode = (rdp.flags & CULLMASK) >> CULLSHIFT;
            FRDP (" |- cull_mode - mode: %s\n", str_cull[mode]);
            switch (mode)
            {
            case 0: // cull none
            case 3: // cull both
                grCullMode(GR_CULL_DISABLE);
                break;
            case 1: // cull front
                grCullMode(GR_CULL_POSITIVE);
                break;
            case 2: // cull back
                grCullMode (GR_CULL_NEGATIVE);
                break;
            }
        }

        if (rdp.update & UPDATE_VIEWPORT)
        {
            rdp.update ^= UPDATE_VIEWPORT;

            DWORD min_x = (DWORD)max(rdp.view_trans[0] - rdp.view_scale[0], 0);
            DWORD min_y = (DWORD)max(rdp.view_trans[1] - rdp.view_scale[1], 0);
            DWORD max_x = (DWORD)min(rdp.view_trans[0] + rdp.view_scale[0], settings.res_x);
            DWORD max_y = (DWORD)min(rdp.view_trans[1] + rdp.view_scale[1], settings.res_y);
            FRDP (" |- viewport - (%d, %d, %d, %d)\n", min_x, min_y, max_x, max_y);
            grClipWindow (min_x, min_y, max_x, max_y);
        }
    }

    RDP (" + update end\n");
}

//
// TexCache - cache textures
//

BYTE texture[256*256*4];        // temporary texture

__inline DWORD segoffset (DWORD so)
{
    return (rdp.segment[(so>>24)&0x0f] + (so&0x00ffffff));
}

void TexCache (int tmu, int tileoff)
{
    int i=0;
    DWORD x, y, lod, aspect;
    DWORD eppl;     // extra pixels per line
    DWORD col, intensity, col2, a;
    BYTE *btptr;
    WORD *wtptr;
    DWORD *dtptr;
    CACHE_LUT *cache;
    int rx, ry;

    int td = rdp.cur_tile + tileoff;

    //if (settings.wireframe)
    //  return;

    DWORD addr = segoffset(rdp.timg[rdp.tiles[td].t_mem].addr) & 0x3FFFFF;

    DWORD bpl;  // bytes per line

    // Get width and height
    int mask_width = (rdp.tiles[td].mask_s)?(1 << rdp.tiles[td].mask_s):(rdp.tiles[td].lr_s-rdp.tiles[td].ul_s+1);
    int mask_height = (rdp.tiles[td].mask_t)?(1 << rdp.tiles[td].mask_t):(rdp.tiles[td].lr_t-rdp.tiles[td].ul_t+1);

    int tile_width = rdp.tiles[td].lr_s - rdp.tiles[td].ul_s + 1;
    int tile_height = rdp.tiles[td].lr_t - rdp.tiles[td].ul_t + 1;
    
    if (rdp.tiles[td].lr_s - rdp.tiles[td].ul_s >= 256)
        rdp.tiles[td].width = mask_width;
    else
        rdp.tiles[td].width = max(tile_width, mask_width);
    if (rdp.tiles[td].lr_t - rdp.tiles[td].ul_t >= 256)
        rdp.tiles[td].height = mask_height;
    else
        rdp.tiles[td].height = max(tile_height, mask_height);
    
    // Get bytes per line
    if (rdp.timg[rdp.tiles[td].t_mem].set_by)
    {
        if (rdp.tiles[td].size)
            bpl = rdp.timg[rdp.tiles[td].t_mem].width << (rdp.tiles[td].size - 1);
        else
            bpl = rdp.timg[rdp.tiles[td].t_mem].width;
    }
    else
    {
        if (rdp.timg[rdp.tiles[td].t_mem].dxt == 0)
            bpl = rdp.tiles[td].line << 3;
        else
            bpl = rdp.timg[rdp.tiles[td].t_mem].dxt << 3;
        
        if (rdp.tiles[td].size == 3)
            bpl = rdp.tiles[td].line << 4;
    }

    if (rdp.tiles[td].width <= 0 ||
        rdp.tiles[td].height <= 0 ||
        rdp.tiles[td].width > 640 ||
        rdp.tiles[td].height > 480 ||
        bpl >= 4000 ||
        rdp.tiles[td].size < 0 ||
        rdp.tiles[td].size > 3)
    {
        RDP (" | |- ! TexCache disapproved\n");
        return;
    }

    RDP (" | |-+ TexCache approved:\n");
    FRDP (" | | |- addr: %08lx\n", addr);
    FRDP (" | | |- bpl: %d\n", bpl);
    FRDP (" | | |- width: %d\n", rdp.tiles[td].width);
    FRDP (" | | +- height: %d\n", rdp.tiles[td].height);
    RDP (" | |- Calculating CRC... ");

    DWORD crc = 0;

    // Do CRC check
    DWORD off = addr;
    if (rdp.tiles[td].size)
        off += rdp.tiles[7].t_ul_t * bpl + (rdp.tiles[7].t_ul_s << (rdp.tiles[td].size-1));
    else
        off += rdp.tiles[7].t_ul_t * bpl + (rdp.tiles[7].t_ul_s >> 1);
    int count;
    DWORD wid;
    if (rdp.tiles[td].size)
        wid = (tile_width << (rdp.tiles[td].size-1)) >> 2;
    else
        wid = tile_width >> 3;
    for (ry=0; ry<tile_height; ry++)
    {
        dtptr = ((DWORD*)gfx.RDRAM) + ((off+ry*bpl) >> 2);
        count = wid;
        while (count > 0)
        {
            crc += *(dtptr++);
            crc += *(dtptr++);
            crc += *(dtptr++);
            crc += *(dtptr++);
            count -= 4;
        }
    }
    crc ^= rdp.timg[rdp.tiles[td].t_mem & 0x1FF].addr;
    /*crc += (rdp.tiles[td].lr_s<<4) + (rdp.tiles[td].lr_t<<12);
    if (rdp.timg[rdp.tiles[td].t_mem].set_by == 1)
        crc += rdp.tiles[td].ul_s + (rdp.tiles[td].ul_t<<8);*/
    crc += (rdp.tiles[7].t_lr_s<<4) + (rdp.tiles[7].t_lr_t<<12)
        + rdp.tiles[7].t_ul_s + (rdp.tiles[7].t_ul_t<<8);

    RDP ("Done.\n");

    DWORD pal_crc = 0;
    if (rdp.tiles[td].format == 2)
    {
        if (rdp.tiles[td].size == 0)
            pal_crc = rdp.pal_8_crc[rdp.tiles[td].palette];
        else
            pal_crc = rdp.pal_8_crc[16];
    }

    // Check wrapping/mirroring
    DWORD flags = (rdp.tiles[td].mirror_s << 1) | rdp.tiles[td].mirror_t |
        (rdp.render_mode & 0x00002000) |
        (rdp.tiles[td].mask_s << 16) | (rdp.tiles[td].mask_t << 20);

    RDP (" | |- Checking cache... ");

    // Check if this texture was already cached
    for (i=0; i<rdp.n_cached[tmu]; i++)
    {
        cache = &rdp.cache[tmu][i];
        if (crc == cache->crc &&
            rdp.tiles[td].width == cache->width &&
            rdp.tiles[td].height == cache->height &&
            rdp.tiles[td].format == cache->format &&
            rdp.tiles[td].size == cache->size &&
            rdp.tiles[td].palette == cache->palette &&
            bpl == cache->bpl &&
            pal_crc == cache->pal_crc &&
            flags == cache->flags)
        {
            RDP ("Texture found in cache, exiting.\n");
            if (fullscreen)
            {
                rdp.cur_cache_n[tmu] = i;
                rdp.cur_cache[tmu] = cache;
                rdp.cur_cache[tmu]->last_used = frame_count;
                grTexSource (tmu,
                    (grTexMinAddress(tmu) + cache->tmem_addr),
                    GR_MIPMAPLEVELMASK_BOTH,
                    &cache->t_info);
            }
            return;
        }
    }
    RDP ("Texture not found, creating new.\n");

    // Otherwise, we need to cache it:

    // Clear the cache if it's full
    if (rdp.n_cached[tmu] >= MAX_CACHE)
    {
        rdp.tmem_ptr[tmu] = offset_textures;
        rdp.n_cached[tmu] = 0;
    }

    // Get this cache object
    cache = &rdp.cache[tmu][rdp.n_cached[tmu]];
    rdp.cur_cache[tmu] = cache;
    rdp.cur_cache_n[tmu] = rdp.n_cached[tmu];

    // Set the data
    cache->dxt = rdp.timg[rdp.tiles[td].t_mem].dxt;
    cache->line = rdp.tiles[td].line;
    cache->wid = rdp.timg[rdp.tiles[td].t_mem].width;

    cache->addr = addr;
    cache->crc = crc;
    cache->palette = rdp.tiles[td].palette;
    cache->pal_crc = pal_crc;
    cache->width = rdp.tiles[td].width;
    cache->height = rdp.tiles[td].height;
    cache->format = rdp.tiles[td].format;
    cache->size = rdp.tiles[td].size;
    cache->flags = flags;
    cache->tmem_addr = rdp.tmem_ptr[tmu];
    cache->bpl = bpl;
    cache->set_by = rdp.timg[rdp.tiles[td].t_mem].set_by;
    cache->swapped = rdp.timg[rdp.tiles[td].t_mem].swapped;
    cache->texrecting = rdp.texrecting;
    cache->last_used = frame_count;
    cache->offset_s = 0;//rdp.tiles[7].t_ul_s;
    cache->offset_t = 0;//rdp.tiles[7].t_ul_t;
    off = ((rdp.tiles[td].size != 0) ?
        (rdp.timg[rdp.tiles[td].t_mem & 0x1FF].addr >> (rdp.tiles[td].size-1)) :
        rdp.timg[rdp.tiles[td].t_mem & 0x1FF].addr);
    if (rdp.tiles[td].size)
    {
        off += rdp.tiles[7].t_ul_t * (bpl >> (rdp.tiles[td].size-1)) +
            rdp.tiles[7].t_ul_s;
    }
    else
    {
        // 4-bit uses byte pointer, so don't shift bpl
        off += rdp.tiles[7].t_ul_t * bpl +
            rdp.tiles[7].t_ul_s;
    }
    rdp.n_cached[tmu] ++;

    /*if (rdp.texrecting && cache->width > 256)
    {
        cache->warpheight = cache->height;
        cache->width /= 2;
        cache->height *= 2;
        cache->warpx = 2;
    } else
    {
        cache->warpx = 1;
        cache->warpheight = 0;
    }*/

    // temporary
    cache->t_info.format = GR_TEXFMT_ARGB_1555;

    // Calculate wrapping mask
    DWORD wrap_mask_s, wrap_mask_t, mirror_bit_s, mirror_bit_t, mask_mask_t, mask_mask_s;
    if (rdp.tiles[td].mask_s)
    {
        wrap_mask_s = 0xFFFF >> (16 - rdp.tiles[td].mask_s);
        mirror_bit_s = 1 << rdp.tiles[td].mask_s;
        mask_mask_s = 0xFFFF << rdp.tiles[td].mask_s;
    }
    else
    {
        wrap_mask_s = 0xFFFF;
        mirror_bit_s = 0;
        mask_mask_s = 0xFFFF;
    }

    if (rdp.tiles[td].mask_t)
    {
        wrap_mask_t = 0xFFFF >> (16 - rdp.tiles[td].mask_t);
        mirror_bit_t = 1 << rdp.tiles[td].mask_t;
        mask_mask_t = 0xFFFF << rdp.tiles[td].mask_t;
    }
    else
    {
        wrap_mask_t = 0xFFFF;
        mirror_bit_t = 0;
        mask_mask_s = 0xFFFF;
    }

    //LOG ("w: " << cache->width << ", h: " << cache->height << "\n");

    // Calculate lod and aspect
    DWORD size_x = cache->width;
    DWORD size_y = cache->height;

    // make size_x and size_y both powers of two
    if (size_x > 256) size_x = 256;
    if (size_y > 256) size_y = 256;

    int shift;
    for (shift=0; (1<<shift) < (int)size_x; shift++);
    size_x = 1 << shift;
    for (shift=0; (1<<shift) < (int)size_y; shift++);
    size_y = 1 << shift;

    // Calculate the maximum size
    int size_max = max (size_x, size_y);
    DWORD real_x=size_max, real_y=size_max;
    switch (size_max)
    {
    case 1:
        lod = GR_LOD_LOG2_1;
        cache->scale = 256.0f;
        break;
    case 2:
        lod = GR_LOD_LOG2_2;
        cache->scale = 128.0f;
        break;
    case 4:
        lod = GR_LOD_LOG2_4;
        cache->scale = 64.0f;
        break;
    case 8:
        lod = GR_LOD_LOG2_8;
        cache->scale = 32.0f;
        break;
    case 16:
        lod = GR_LOD_LOG2_16;
        cache->scale = 16.0f;
        break;
    case 32:
        lod = GR_LOD_LOG2_32;
        cache->scale = 8.0f;
        break;
    case 64:
        lod = GR_LOD_LOG2_64;
        cache->scale = 4.0f;
        break;
    case 128:
        lod = GR_LOD_LOG2_128;
        cache->scale = 2.0f;
        break;
    //case 256:
    default:
        lod = GR_LOD_LOG2_256;
        cache->scale = 1.0f;
        break;

        // No default here, can't be a non-power of two, see above
    }

    // Calculate the aspect ratio
    if (size_x >= size_y)
    {
        int ratio = size_x / size_y;
        switch (ratio)
        {
        case 1:
            aspect = GR_ASPECT_LOG2_1x1;
            cache->scale_x = 1.0f;
            cache->scale_y = 1.0f;
            break;
        case 2:
            aspect = GR_ASPECT_LOG2_2x1;
            cache->scale_x = 1.0f;
            cache->scale_y = 0.5f;
            real_y >>= 1;
            break;
        case 4:
            aspect = GR_ASPECT_LOG2_4x1;
            cache->scale_x = 1.0f;
            cache->scale_y = 0.25f;
            real_y >>= 2;
            break;
        //case 8:
        default:
            aspect = GR_ASPECT_LOG2_8x1;
            cache->scale_x = 1.0f;
            cache->scale_y = 0.125f;
            real_y >>= 3;
            break;
        }
    }
    else
    {
        int ratio = size_y / size_x;
        switch (ratio)
        {
        case 2:
            aspect = GR_ASPECT_LOG2_1x2;
            cache->scale_x = 0.5f;
            cache->scale_y = 1.0f;
            real_x >>= 1;
            break;
        case 4:
            aspect = GR_ASPECT_LOG2_1x4;
            cache->scale_x = 0.25f;
            cache->scale_y = 1.0f;
            real_x >>= 2;
            break;
        //case 8:
        default:
            aspect = GR_ASPECT_LOG2_1x8;
            cache->scale_x = 0.125f;
            cache->scale_y = 1.0f;
            real_x >>= 3;
            break;
        }
    }

    DWORD min_x = min (real_x, cache->width);
    DWORD min_y = min (real_y, cache->height);

    // Set the extra pixels per line (if the texture is not log2 like mario64 face background)
    eppl = real_x - min_x;
    if (real_x != cache->width || real_y != cache->height)
    {
        //if (real_x > min_x)
        //  eppl = real_x - min_x;
        cache->scale_x *= (float)cache->width / (float)real_x;
        cache->scale_y *= (float)cache->height / (float)real_y;
    }

    RDP (" | |- Texture loading... ");

    // Convert this texture
    switch (cache->size)
    {
    case 0: // 4bit
        switch (cache->format)
        {
        case 0:     // RGBA
            RDP_E ("** 4-bit RGBA\n");
            break;
        case 1:     // YUV
            RDP_E ("** 4-bit YUV\n");
            break;
        case 2:     // CI
            cache->t_info.format = GR_TEXFMT_ARGB_1555;
            wtptr = (WORD*)texture;
            if (min_x & 1) eppl ++; // all 4-bit textures in case of rounding up
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x>>1; x++)
                {
                    rx = x << 1;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+(rx>>1)+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = ((BYTE*)gfx.RDRAM)[a&BMASK];
                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        col2 = rdp.pal_8[(((rdp.tiles[td].palette<<4) + (col & 0x0F))^1)&0xFF];
                    else
                        col2 = rdp.pal_8[(((rdp.tiles[td].palette<<4) + (col >> 4))^1)&0xFF];
                    *(wtptr++) = (WORD)(((col2&0xFFFE)>>1) | ((col2&0x0001) << 15));
                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        col2 = rdp.pal_8[(((rdp.tiles[td].palette<<4) + (col >> 4))^1)&0xFF];
                    else
                        col2 = rdp.pal_8[(((rdp.tiles[td].palette<<4) + (col & 0x0F))^1)&0xFF];
                    col = (WORD)(((col2&0xFFFE)>>1) | ((col2&0x0001) << 15));
                    *(wtptr++) = (WORD)col;
                }

                // fill the rest of the line
                for (; x<real_x>>1; x++)
                {
                    *(wtptr++) = (WORD)col;
                    *(wtptr++) = (WORD)col;
                }
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x<<1);
                wtptr += real_x;
            }
            break;
        case 3:     // IA
            cache->t_info.format = GR_TEXFMT_ALPHA_INTENSITY_44;
            btptr = (BYTE*)texture;
            if (min_x & 1) eppl ++; // all 4-bit textures in case of rounding up
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x>>1; x++)
                {
                    rx = x << 1;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+(rx>>1)+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = ((BYTE*)gfx.RDRAM)[a&BMASK];
                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        *(btptr++) = (BYTE)(((col & 0x08)?0xF0:0x00) |
                            ((col & 0x07) << 1) | ((col & 0x04) >> 2));
                    else
                        *(btptr++) = (BYTE)(((col & 0x10)?0xF0:0x00) |
                            ((col & 0xE0) >> 4) | ((col & 0x80) >> 7));
                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        col = (BYTE)(((col & 0x80)?0xF0:0x00) |
                            ((col & 0x70) >> 3) | ((col & 0x40) >> 5));
                    else
                        col = (BYTE)(((col & 0x01)?0xF0:0x00) |
                            ((col & 0x0E)) | ((col & 0x08) >> 3));
                    *(btptr++) = (BYTE)col;
                }

                // fill the rest of the line
                for (; x<real_x>>1; x++)
                {
                    *(btptr++) = (BYTE)col;
                    *(btptr++) = (BYTE)col;
                }
                //btptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (btptr, btptr-real_x, real_x);
                btptr += real_x;
            }
            break;
        case 4:     // I
            cache->t_info.format = GR_TEXFMT_ALPHA_INTENSITY_44;
            btptr = (BYTE*)texture;
            if (min_x & 1) eppl ++; // all 4-bit textures in case of rounding up
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x>>1; x++)
                {
                    rx = x << 1;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+(rx>>1)+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = ((BYTE*)gfx.RDRAM)[a&BMASK];

                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        *(btptr++) = (BYTE)((col << 4) | (col & 0x0F));
                    else
                        *(btptr++) = (BYTE)((col & 0xF0) | (col >> 4));
                    if (rdp.tiles[td].mirror_s && ((x<<1) & mirror_bit_s))
                        col = (BYTE)((col & 0xF0) | (col >> 4));
                    else
                        col = (BYTE)((col << 4) | (col & 0x0F));
                    *(btptr++) = (BYTE)col;
                }
                
                // fill the rest of the line
                for (; x<real_x>>1; x++)
                {
                    *(btptr++) = (BYTE)col;
                    *(btptr++) = (BYTE)col;
                }
                //btptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (btptr, btptr-real_x, real_x);
                btptr += real_x;
            }
            break;
        default:
            RDP_E ("** 4-bit unknown\n");
        }
        break;
    case 1: // 8bit
        switch (cache->format)
        {
        case 0:     // RGBA
            cache->t_info.format = GR_TEXFMT_ARGB_1555;
            wtptr = (WORD*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = rdp.pal_8[((BYTE*)gfx.RDRAM)[a&BMASK]^1];
                    col = (((col&0xFFFE) >> 1) | ((col&0x0001) << 15));
                    *(wtptr++) = (WORD)col;
                }

                // fill the rest of the line
                for (; x<real_x; x++)
                    *(wtptr++) = (WORD)col;
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x*2);
                wtptr += real_x;
            }
            break;
        case 1:     // YUV
            RDP_E ("** 8-bit YUV\n");
            break;
        case 2:     // CI
            cache->t_info.format = GR_TEXFMT_ARGB_1555;
            wtptr = (WORD*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = rdp.pal_8[((BYTE*)gfx.RDRAM)[a&BMASK]^1];
                    col = (WORD)(((col&0xFFFE) >> 1) | ((col&0x0001) << 15));
                    *(wtptr++) = (WORD)col;
                }
                
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(wtptr++) = (WORD)col;
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x*2);
                wtptr += real_x;
            }
            break;
        case 3:     // IA
            cache->t_info.format = GR_TEXFMT_ALPHA_INTENSITY_44;
            btptr = (BYTE*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = ((BYTE*)gfx.RDRAM)[a&BMASK];
                    col = (BYTE)(col >> 4) | (BYTE)(col << 4);
                    *(btptr++) = (BYTE)col;
                }
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(btptr++) = (BYTE)col;
                //btptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (btptr, btptr-real_x, real_x);
                btptr += real_x;
            }
            break;
        case 4:     // I
            cache->t_info.format = GR_TEXFMT_ALPHA_8;
            btptr = (BYTE*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*bpl)^3);
                    if (cache->swapped && (y & 1)) a ^= 4;
                    col = ((BYTE*)gfx.RDRAM)[a&BMASK];
                    *(btptr++) = (BYTE)col;
                }
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(btptr++) = (BYTE)col;
                //btptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (btptr, btptr-real_x, real_x);
                btptr += real_x;
            }
            break;
        default:
            RDP_E ("** 8-bit unknown\n");
        }
        break;
    case 2: // 16bit
        switch (cache->format)
        {
        case 0:     // RGBA
            cache->t_info.format = GR_TEXFMT_ARGB_1555;
            wtptr = (WORD*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*(bpl>>1))^1);
                    if (cache->swapped && (y & 1)) a ^= 2;
                    col = ((WORD*)gfx.RDRAM)[a&WMASK];
                    col = (WORD)(((col & 0xFFFE) >> 1) | ((col & 1) << 15));
                    *(wtptr++) = (WORD)col;
                }
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(wtptr++) = (WORD)col;
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x*2);
                wtptr += real_x;
            }
            break;
        case 1:     // YUV
            RDP_E ("** 16-bit YUV\n");
            break;
        case 2:     // CI
            RDP_E ("** 16-bit CI\n");
            break;
        case 3:     // IA
            cache->t_info.format = GR_TEXFMT_ARGB_4444;
            wtptr = (WORD*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = ((off+rx+ry*(bpl>>1))^1);
                    if (cache->swapped && (y & 1)) a ^= 2;
                    col = ((WORD*)gfx.RDRAM)[a&WMASK];
                    intensity = col >> 12;
                    *(wtptr++) = (WORD)(intensity | (intensity << 4) | (intensity << 8) | ((col & 0xF0) << 8));
                }
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(wtptr++) = (WORD)col;
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x*2);
                wtptr += real_x;
            }
            break;
        case 4:     // I
            RDP_E ("** 16-bit I\n");
            break;
        default:
            RDP_E ("** 16-bit unknown\n");
            break;
        }
        break;
    
    case 3: // 32bit
        switch (cache->format)
        {
        case 0:
            cache->t_info.format = GR_TEXFMT_ARGB_4444;
            wtptr = (WORD*)texture;
            for (y=0; y<min_y; y++)
            {
                for (x=0; x<min_x; x++)
                {
                    rx = x;
                    ry = y;
                    if (rdp.tiles[td].mirror_s) rx = (rx&mirror_bit_s) +
                        ((rx&mirror_bit_s)?-1:1) * (rx&wrap_mask_s) -
                        ((rx&mirror_bit_s)?1:0);
                    else rx &= wrap_mask_s;
                    if (rdp.tiles[td].mirror_t) ry = (ry&mirror_bit_t) +
                        ((ry&mirror_bit_t)?-1:1) * (ry&wrap_mask_t) -
                        ((ry&mirror_bit_t)?1:0);
                    else
                        ry &= wrap_mask_t;

                    a = (off+rx+ry*(bpl>>2));
                    //if (cache->swapped && (y & 1)) a ^= 1;
                    col = ((DWORD*)gfx.RDRAM)[a&DMASK];
                    *(wtptr++) = (WORD)(((col & 0xF0000000) >> 20) |
                        ((col & 0x00F00000) >> 16) |
                        ((col & 0x0000F000) >> 12) |
                        ((col & 0x000000F0) << 8));
                }
                // fill the rest of the line
                for (; x<real_x; x++)
                    *(wtptr++) = (WORD)col;
                //wtptr += eppl;
            }
            for (; y<real_y; y++)
            {
                memcpy (wtptr, wtptr-real_x, real_x*2);
                wtptr += real_x;
            }
            break;
        case 1:
            RDP_E ("** 32-bit YUV\n");
            break;
        case 2:
            RDP_E ("** 32-bit CI\n");
            break;
        case 3:
            RDP_E ("** 32-bit IA\n");
            break;
        case 4:
            RDP_E ("** 32-bit I\n");
            break;
        }
        break;

    default:
        RDP_E ("** Unknown texture size");
    }
    RDP ("Done.\n");

    cache->realwidth = real_x;
    cache->realheight = real_y;
    cache->lod = lod;
    cache->aspect = aspect;

    //FRDP_E ("## TS: lod %d, aspect %d\n", lod, aspect);

    if (fullscreen)
    {
        // Load the texture into texture memory
        GrTexInfo *t_info = &cache->t_info;
        t_info->data = texture;
        //t_info->format = GR_TEXFMT_ARGB_1555;
        t_info->smallLodLog2 = lod;
        t_info->largeLodLog2 = lod;
        t_info->aspectRatioLog2 = aspect;

        DWORD texture_size = grTexTextureMemRequired (GR_MIPMAPLEVELMASK_BOTH, t_info);

        // Check for 2mb boundary
        if ((rdp.tmem_ptr[tmu] < TEXMEM_2MB_EDGE) && (rdp.tmem_ptr[tmu]+texture_size > TEXMEM_2MB_EDGE))
        {
            rdp.tmem_ptr[tmu] = TEXMEM_2MB_EDGE;
            cache->tmem_addr = rdp.tmem_ptr[tmu];
        }

        // Check for end of memory (too many textures to fit, clear cache)
        if (rdp.tmem_ptr[tmu]+texture_size >= grTexMaxAddress(tmu))
        {
            rdp.tmem_ptr[tmu] = offset_textures;
            rdp.n_cached[tmu] = 0;
            TexCache (tmu, tileoff);
            return;     // DON'T continue, update will have already done this
        }

        grTexDownloadMipMap (tmu,
            grTexMinAddress(tmu) + rdp.tmem_ptr[tmu],
            GR_MIPMAPLEVELMASK_BOTH,
            t_info);

        grTexSource (tmu,
            grTexMinAddress(tmu) + rdp.tmem_ptr[tmu],
            GR_MIPMAPLEVELMASK_BOTH,
            t_info);

        rdp.tmem_ptr[tmu] += texture_size;
    }

    RDP (" | + Texcache end\n");
}