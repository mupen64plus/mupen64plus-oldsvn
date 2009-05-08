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

#ifndef _RDP_GL_
#define _RDP_GL_

extern BOOL USE_MM_HACK;

extern int Src_Alpha, Dst_Alpha;

extern void DrawVisualRectangle(_u32 xh, _u32 yh, _u32 xl, _u32 yl);
extern void DrawTexRectangle(float xh, float yh, 
                      float xl, float yl,
                      int tile,
                      float f_s, float f_t, 
                      float dsdx, float dtdy);

extern void SetVisualClipRectangle(_u32 ulx, _u32 uly, _u32 lrx, _u32 lry);
extern void SetVisualRenderBuffer(void);
extern void SetVisualColor(_u32 Color);

extern void MDrawVisualTriangle (int vn[3], _u32 t1, _u32 s1, _u32 ss1);
extern void MDrawVisualTriangle2(int vn[3], int t1, int s1, int ss1);


extern void MDrawVisualTriangle3(int vn[3], _u8 color);

extern void CreateDisplayLists();

extern void DrawTexRectangleT(float u1, float u2, float v1, float v2, int tile,
                       int xh, int xl, int yh, int yl);
#endif