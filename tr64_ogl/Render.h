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

#ifndef __RENDER_H
#define __RENDER_H

extern void Render_FlushVisualRenderBuffer(void);
extern void Render_ClearVisual();
extern void Render_ResetViewPort();

extern void Render_load_modelview(float m[4][4]);
extern void Render_load_projection(float m[4][4]);
extern void Render_mul_modelview(float m[4][4]);
extern void Render_mul_projection(float m[4][4]);
extern void Render_push_load_modelview(float m[4][4]);
extern void Render_push_mul_modelview(float m[4][4]);
extern void Render_pop_modelview();
extern void Render_viewport();
extern void Render_lookat_x();
extern void Render_lookat_y();
extern void Render_light(int n);
extern void Render_line3d(int vn[]);
extern void Render_geometry_zbuffer(int on);
extern void Render_geometry_cullfront(int on);
extern void Render_geometry_cullback(int on);
extern void Render_geometry_cullfrontback(int on);
extern void Render_TexRectangle(float xh, float yh, 
                                float xl, float yl,
                                int tile_num,
                                float s, float t, 
                                float dsdx, float dtdy);
extern void Render_TexRectangleFastFB(float xh, float yh, 
                         float xl, float yl,
                         int tile_num,
                         float s, float t, 
                         float dsdx, float dtdy);

extern void Render_FillRectangle(int xh, int yh, 
                                int xl, int yl,
                                _u32 fillclr);
extern void Render_tri1(int vn[], _u8 flag);
extern void Render_tri2(int vn[], _u8 flag);

extern void Render_DrawVisualRectangle(int ulx, int uly, int lrx, int lry);
extern void Render_geometry_zwrite(int on);
extern void Render_FrameBuffer(void);
extern void Render_FrameBuffer5(void);

extern void Render_Fog(int on);
#endif

