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


#include "tr_windows.h"
#include <stdlib.h>
#include <malloc.h>

#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

//#include "display.h"
#include "rdp_registers.h"

_u32 flags; 
HWND HWNDMss;

#ifdef WIN32
void msg_box(char *textl)
{
    MessageBox(HWNDMss, textl, "Display", MB_OK);
}
#else
void msg_box(char *text)
{
    puts(text);
}
#endif

int RePaint;                // are used by DEBUG.cpp and MAIN.cpp
extern int iRenderWindowX;
extern int iRenderWindowY;
extern BOOL Draw2d;
extern  char    *pVIREG;
extern  char    *pRDRAM;
extern  char    *pIDMEM;


void DrawScreenGL(unsigned int width, unsigned int height,unsigned int Scrwidth, unsigned int Scrheight, unsigned int mode);
//********************************************************************************
//** Init Stuff                                                                 **
//********************************************************************************

int OpenVisual()
{
    return 0;
}
int CloseVisual()
{
    return 0;
}
void GetVisualName(char *name, int *n)
{
}
void SetVisualTitle(char *name)
{
}

//********************************************************************************
//** For 2d-demos only, very, very, 'uncool' hack                               **
//********************************************************************************

void Render_FlushVisualRenderBuffer(void);

void RefreshVisual()
{
    _u32    width, height, mode;
    if (Draw2d)
    {
        width = ((_u32 *)pVIREG)[2];
        mode = ((_u32 *)pVIREG)[0] & 0x03;
        height = (width * 3) >> 2;
        if ((width == 0) || (height == 0)) return;
        DrawScreenGL(256, 256,width,height,mode);
        RePaint = 1;
        Render_FlushVisualRenderBuffer();
    }
} 

/* This will draw the current n64 frame buffer to the screen */
/* At the moment only 15 bit graphics at a resolution of 320*240 is supported. */
/* It probably only works right with big endian at the momment. */
/* This routine was coded by Gil Pedersen */

static unsigned char tbuffer[256*256*4];
void DrawScreenGL(unsigned int width, unsigned int height,unsigned int Scrwidth, unsigned int Scrheight, unsigned int mode)
{
    unsigned long i, j, ulx, uly;

    _u16 *src, *base_src_ptr;
    _u32 *lsrc, *lbase_src_ptr;
    unsigned char *dest;
    GLint hadDepthTest, hadBlending;

    glGetIntegerv(GL_DEPTH_TEST, &hadDepthTest); //** Should be glGetBooleanv, but 
    glDisable(GL_DEPTH_TEST);
    glGetIntegerv(GL_BLEND, &hadBlending); //** Should be glGetBooleanv, but 
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
//  glOrtho(0, Scrwidth, Scrheight, 0, 0, 1);
    glOrtho(0, Scrwidth, Scrheight, 0, -0, 1023);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glEnable(GL_TEXTURE_2D);

    base_src_ptr = (_u16*)(_u32*)(pRDRAM + ((_u32*)pVIREG)[1]);
    lbase_src_ptr = (_u32*)(pRDRAM + ((_u32*)pVIREG)[1]);

    for (ulx = 0; (ulx * 256)<Scrwidth; ulx++)
        for (uly = 0; (uly * 256)<Scrheight; uly++)
        {
        dest = (unsigned char*)tbuffer;
        if (mode = 2)
        for(i=0; i<height; i++)
        {
            _u32 tsrc = (_u32)((base_src_ptr + (i + (uly * 256))*Scrwidth) + (ulx * 256));
            for(j=0; j<width; j++)
            {               
                src = ((_u16 *)tsrc) + (j ^ 0x01);
                *(dest) = (((*src >> 11 ) & 0x001f)<<3); dest++;
                *(dest) = (((*src >>  6 ) & 0x001f)<<3); dest++;
                *(dest) = (((*src >>  1 ) & 0x001f)<<3); dest++;
                *(dest) = 0xff; dest++;  //** Alpha

            }
        }
        else
        for(i=0; i<height; i++)
        {
            _u32 tsrc;
            tsrc = (_u32)((lbase_src_ptr + (i + (uly * 256))*Scrwidth) + (ulx * 256));
            for(j=0; j<width; j++)
            {
                lsrc = (_u32*)(tsrc + (j ^ 0x01));
                //lsrc = (lbase_src_ptr + (i + (uly * 256))*Scrwidth) + (ulx * 256)+(j ^ 0x01);
                *(dest) = (_u8)(((*lsrc >> 24 ) & 0x001f)<<3); dest++;
                *(dest) = (_u8)(((*lsrc >> 16 ) & 0x001f)<<3); dest++;
                *(dest) = (_u8)(((*lsrc >> 8 ) & 0x001f)<<3); dest++;
                *(dest) = 0xff; dest++;  //** Alpha

            }
        };
        glBindTexture(GL_TEXTURE_2D, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, tbuffer);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f((float)(ulx * 256) +  0.0f  ,(float)(uly * 256.0f) + 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f((float)(ulx * 256) +  0.0f  ,(float)(uly * 256.0f) + 256.0f);          
            glTexCoord2f(1.0f, 1.0f); glVertex2f((float)(ulx * 256) +  256.0f,(float)(uly * 256.0f) + 256.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f((float)(ulx * 256) +  256.0f,(float)(uly * 256.0f) + 0.0f);
        glEnd();
        } // end for ulx uly

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    if (hadBlending) glEnable(GL_BLEND);
    if (hadDepthTest) glEnable(GL_DEPTH_TEST);
}


