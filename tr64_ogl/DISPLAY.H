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

#define __DISPLAY_H

#include "type_sizes.h"


#define NO_DISPLAY            0
#define LOCAL_DISPLAY         1
#define LOCAL_DISPLAY_PROCESS 2
#define REMOTE_DISPLAY        3





int  OpenVisual();      /* should open a window/screen     */
int  CloseVisual();     /* should close that window/screen */

void RefreshVisual();   /* should redraw the window/screen
                           (this is called whenever
                           a redraw is needed)             */


void GetVisualName(char *, int *n);    /* guess! :)        */
void SetVisualTitle(char *);           /* guess! :)        */
void SetVisualSize(int w, int h);      /* guess! :)        */





/**
*
* renderer related
*
**/

void tr_ren_load_modelview(float m[4][4]);
void tr_ren_load_projection(float m[4][4]);
void tr_ren_mul_modelview(float m[4][4]);
void tr_ren_mul_projection(float m[4][4]);
void tr_ren_push_load_modelview(float m[4][4]);
void tr_ren_push_mul_modelview(float m[4][4]);
void tr_ren_pop_modelview();

void tr_ren_viewport();
void tr_ren_lookat_x();
void tr_ren_lookat_y();

void tr_ren_light(int n);
void tr_ren_numlights();
void tr_ren_lightcol(int n);

void tr_ren_line3d(int vn[]);
void tr_ren_tri1(int vn[]);
void tr_ren_tri2(int vn[]);

void tr_ren_geometry_zbuffer(int on);
void tr_ren_geometry_cullfront(int on);
void tr_ren_geometry_cullback(int on);
void tr_ren_geometry_cullfrontback(int on);







/**
*
* for RDP
*
**/



/* The next 3 routines are for the render buffer.
   If you can draw directly to RDRAM then do it (do nothing in this routines)!
   In standard X you cannot (or maybe i just don't know how). */

void InitVisualRenderBuffer();
/*
 * Called when gfx HLE is started.
 */

void SetVisualRenderBuffer(int format, int size, int width, char *address);
/*
 * format:  0 rgba, 1 yuv, 2 ci, 3 ia, 4 i   ... i think that just rgba is used.
 * size:    0 4bit, 1 8bit, 2 16bit, 3 32bit ... i think that just 16 and 32 bit are used.
 * width:   You might ask where is the height ... i don't know. i use height=width*3/4
 * address: This also could be a parameter for FlushVisualRenderBuffer().
 *          It says where to draw the buffer after rendering (when FlushVisualRenderBuffer() occurs).
 */


void Visual3D_FPS();



/* Sets the clipping rectangle.
   This routine is called from SETSCISSOR. */
void SetVisualClipRectangle(int ulx, int uly, int lrx, int lry);



/* Sets the foreground color for the point, line, tri and rect routines. */

void SetVisualColor(int color);

void ClearVisual();

void DrawVisualFillRect(int ulx, int uly, int lrx, int lry);

void DrawVisualLine(int ulx, int uly, int lrx, int lry);

void DrawVisualTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

void DrawVisualRectangle(int ulx, int uly, int lrx, int lry);

/* obsolete - now done in DrawVisualTexRectangle()/-Triangle()
void LoadVisualBlock(int uls, int ult, int lrs, int dxt, int tile);
*/
/*
 * Loads a texture into tmem - later we should buffer the textures.
 *
 * uls, ult: always 0? this is used for offset
 * lrs: lrs - uls + 1 = pixel of tile
 * dxt: used with offset.
 *      look at the source in x11/x11.c how i have used it.
 * tile: 0..7
 */

/* obsolete - now done in DrawVisualTexRectangle()/-Triangle()
void LoadVisualTile(int uls, int ult, int lrs, int lrt, int tile);
*/
/*
 * Loads a texture into tmem - later we should buffer the textures.
 */

void DrawVisualTexRectangle(
        int ulx, int uly,
        int lrx, int lry,
        int tile,
        float s, float t, float dsdx, float dtdy
        );

void DrawVisualTexTriangle(int vn[3]);

