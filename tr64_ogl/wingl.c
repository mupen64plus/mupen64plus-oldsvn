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

/**
 * TrueReality - wingl.c
 * Copyright (C) 1998, 1999 Niki W. Waibel
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
**/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>
#include "wingl.h"
#include "resource.h"

void SetWindowScale();

extern char plgname[];
extern int vermjr;
extern int vermnr;
extern char versfx[];
extern float imgHeight;
extern float imgWidth;

// this is the interface windata, it can be initialized with a HWND 
// from the main prog, or it can create its own window
WINDATA_T WinData;
char szAppName[256];

HWND OglWindow = NULL;

void Render_viewport();

LRESULT CALLBACK OglProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    switch (message) 
    {
       case WM_INITDIALOG:
           break;
       case WM_COMMAND:
            switch (wParam) 
            {
            case IDOK:
                break;
            case IDCANCEL:
                EndDialog(hDlg, TRUE);
                OglWindow = NULL;
                return (TRUE);
                break;
            }
            break;
    }
    return FALSE;
}

extern HINSTANCE g_hInstance;
extern HWND g_hWnd;

//#define USE32BIT

// GL WindowHandling Functions
BOOL WGL_Init(void)
{
    int          pfm;
//  LOGPALETTE*  pPal;
//  int          paletteSize;

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                       // version number
        PFD_DRAW_TO_WINDOW |     // support window
        PFD_SUPPORT_OPENGL |     // support OpenGL
//      PFD_SUPPORT_GDI    |     // support GDI drawing
        PFD_DOUBLEBUFFER,        // double buffered
        PFD_TYPE_RGBA,           // RGBA type
#ifdef USE32BIT
        32,                      // 32-bit color depth
        8, 0, 8, 0, 8, 0,        // color bits ignored
        8,                       // no alpha buffer
        0,                       // shift bit ignored
#else USE32BIT // otherwise use 16 bit color
        16,                      // 16-bit color depth for 3dfx boards?
        4, 0, 4, 0, 4, 0,        // color bits ignored
        4,                       // no alpha buffer
        0,                       // shift bit ignored
#endif
        0,                       // no accumulation buffer
        0, 0, 0, 0,              // accum bits ignored
        16,                      // 32-bit z-buffer      
        0,                       // no stencil buffer
        4,                       // no auxiliary buffer
        PFD_MAIN_PLANE,          // main layer
        0,                       // reserved
        0, 0, 0};                // layer masks ignored
/*
    if (!OglWindow)
    { // 
        g_hWnd = WinData.hWnd;
        OglWindow = CreateDialog(g_hInstance, IDD_OGL1, g_hWnd, (DLGPROC)OglProc);
        WinData.hWnd = OglWindow;
    }
*/
    sprintf(szAppName,"%s v%i.%i%s",plgname,vermjr,vermnr,versfx);

    if ((WinData.hDC = GetDC(WinData.hWnd)) == NULL)
    {
        MessageBox(NULL, "GetDC on main window failed", szAppName, MB_OK|MB_ICONSTOP);
        return FALSE;
    }

    if ((pfm = ChoosePixelFormat(WinData.hDC, &pfd)) == 0)
    {
        MessageBox(NULL, "ChoosePixelFormat failed\n", szAppName, MB_OK|MB_ICONSTOP);
        return FALSE;
    }
    if (SetPixelFormat(WinData.hDC, pfm, &pfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed\n", szAppName, MB_OK|MB_ICONSTOP);
        return FALSE;
    }


    DescribePixelFormat(WinData.hDC, pfm, sizeof(pfd), &pfd);


/*  if (pfd.dwFlags & PFD_NEED_PALETTE) 
    {
        paletteSize = 1 << pfd.cColorBits;

        pPal = (LOGPALETTE*)
            malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
        pPal->palVersion = 0x300;
        pPal->palNumEntries = paletteSize;

        // Create Palette RGB
        {
            int redMask = (1 << pfd.cRedBits) - 1;
            int greenMask = (1 << pfd.cGreenBits) - 1;
            int blueMask = (1 << pfd.cBlueBits) - 1;
            int i;

            for (i=0; i<paletteSize; ++i)
            {
                pPal->palPalEntry[i].peRed =
                    (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
                pPal->palPalEntry[i].peGreen =
                    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
                pPal->palPalEntry[i].peBlue =
                    (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
                pPal->palPalEntry[i].peFlags = 0;
            }
        }

        WinData.hPalette = (HPALETTE)CreatePalette(pPal);
        free(pPal);

        if (WinData.hPalette) 
        {
            SelectPalette(WinData.hDC, WinData.hPalette, FALSE);
            RealizePalette(WinData.hDC);
        }
    }*/

    
    if ((WinData.hGLRC = wglCreateContext(WinData.hDC)) == 0)
    {
        MessageBox(NULL, "wglCreateContext failed!\n", szAppName, MB_OK|MB_ICONSTOP);
        WGL_UnInit();
        return FALSE;
    }

    if (!wglMakeCurrent(WinData.hDC, WinData.hGLRC))
    {
        MessageBox(NULL, "wglMakeCurrent failed!\n", szAppName, MB_OK|MB_ICONSTOP);
        WGL_UnInit();
        return FALSE;
    }


    /*wsprintf(str, "Running on %s OpenGL by %s", glGetString(GL_RENDERER), glGetString(GL_VENDOR));
    strcat(buffer, str);

    if (pfd.dwFlags & PFD_GENERIC_FORMAT)
    {
        if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
            strcat(buffer, "\n\nMCD (Mini Client Driver) detected.");
        else
        {
            strcat(buffer, "\n\nWARNING: OpenGL Driver is not accelerated!");
            //return FALSE;
        }
    }
    else
        strcat(buffer, "\n\nICD (Installable Client Driver) detected.");

    // output the driver info we have been strcat'ing into buffer
    MessageBox(WinData.hWnd, buffer, szAppName, MB_OK|MB_ICONINFORMATION);*/


    WGL_InitState();

    return TRUE;
}

/*void WGL_SetupPixelFormat()
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  // size 
        1,                              // version 
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,               // support double-buffering
        PFD_TYPE_RGBA,                  // color type
        24,                             // prefered color depth
        0, 0, 0, 0, 0, 0,               // color bits (ignored)
        0,                              // no alpha buffer
        0,                              // alpha bits (ignored) 
        0,                              // no accumulation buffer 
        0, 0, 0, 0,                     // accum bits (ignored) 
        32,                             // depth buffer 
        0,                              // no stencil buffer 
        0,                              // no auxiliary buffers 
        PFD_MAIN_PLANE,                 // main layer 
        0,                              // reserved 
        0, 0, 0                         // no layer, visible, damage masks 
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(WGL_hdc, &pfd);
    if (pixelFormat == 0) {
        MessageBox(WindowFromDC(WGL_hdc), "ChoosePixelFormat failed.", "ErrorWinGL",
            MB_ICONERROR | MB_OK);
        exit(-1);
    }

    if (SetPixelFormat(WGL_hdc, pixelFormat, &pfd) != TRUE) {
        MessageBox(WindowFromDC(WGL_hdc), "SetPixelFormat failed.", "Error WinGL",
            MB_ICONERROR | MB_OK);
        exit(-1);
    }
}*/
void DelTexture();

void WGL_UnInit(void)
{
    if (WinData.hGLRC)
    {
        if (!wglMakeCurrent(WinData.hDC, WinData.hGLRC))
        {
            MessageBox(NULL, "wglMakeCurrent failed!\n", "ERROR", MB_OK|MB_ICONSTOP);
        }
        DelTexture();
        if (!wglDeleteContext(WinData.hGLRC))
        {
            MessageBox(NULL, "wglDeleteContext failed!\n", "ERROR", MB_OK|MB_ICONSTOP);
        }
        WinData.hGLRC = NULL;
    }
    if (WinData.hPalette)
    {
        if(!DeleteObject(WinData.hPalette))
        {
            MessageBox(NULL, "DeleteObject for hPalette failed!\n", "ERROR", MB_OK|MB_ICONSTOP);
        }
        WinData.hPalette = NULL;
    }
    if (WinData.hDC)
    {
        if (!ReleaseDC(WinData.hWnd, WinData.hDC))
        {
            MessageBox(NULL, "ReleaseDC failed!\n", "ERROR", MB_OK|MB_ICONSTOP);
        }
        WinData.hDC = NULL;
    }
}

BOOL WGL_IsGLRCValid ()
{
    if (WinData.hGLRC)
        return TRUE;
    else
        return FALSE;
}

BOOL WGL_IsPaletteValid ()
{
    if (WinData.hPalette)
        return TRUE;
    else
        return FALSE;
}

void WGL_RedoPalette ()
{
    UnrealizeObject(WinData.hPalette);
    SelectPalette(WinData.hDC, WinData.hPalette, FALSE);
    RealizePalette(WinData.hDC);
}

BOOL WGL_SwapBuffers(void)
{   
    return SwapBuffers(WinData.hDC);
}

#define MAX_CACHED_TEXTURES 4096 //512
GLuint tList[MAX_CACHED_TEXTURES];

void CreateDisplayLists();
void WGL_InitState(void)
{
    glViewport(0, 0, WinData.Width, WinData.Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    SetWindowScale();
    SetOrtho();

    glGenTextures(MAX_CACHED_TEXTURES, tList);

    // position viewer 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClearDepth(1.0f);

    glShadeModel(GL_SMOOTH);

    glAlphaFunc(GL_GEQUAL,AlphaLevel);
//  glAlphaFunc(GL_GEQUAL,0.005f);
    glDisable(GL_ALPHA_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);

    CreateDisplayLists();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

/*****************************************************************************/
/* Copiled Displaylists for speedup                                          */
/*****************************************************************************/
#define kListEnd2D          1
#define kListBegin2D160   160
#define kListBegin2D320   320
#define kListBegin2D640   640
#define kListBegin2D800   800
#define kListBegin2D960   960
#define kListBegin2D1024 1024
#define kListBegin2D1200 1200

const float proj_matrix[2][4][4] =
{
    /* 320 * 240 */
        {
                { 0.0062500f, 0.0000000f, 0.0000000f, 0.0000000f},
                { 0.0000000f,-0.0083333f, 0.0000000f, 0.0000000f},
                { 0.0000000f, 0.0000000f,-1.0000000f, 0.0000000f},
                {-1.0000000f, 1.0000000f, 0.0000000f, 1.0000000f}
        },
    /* 640 * 480 */
        {
                { 0.0031250f, 0.0000000f, 0.0000000f, 0.0000000f},
                { 0.0000000f,-0.0041667f, 0.0000000f, 0.0000000f},
                { 0.0000000f, 0.0000000f,-1.0000000f, 0.0000000f},
                {-1.0000000f, 1.0000000f, 0.0000000f, 1.0000000f}
        }
};

void CreateDisplayLists()
{

    /* Gil: Some precompiled display lists that will make
            the emulation slighty faster. 
            I think we shold use more of these, if possible. */
    /* niki: gil - this is a fantastic idea!!! */

    /* This routine should be called when you init your opengl graphics */
       
    /* BEGIN_2D_320 */
        glNewList(kListBegin2D320, GL_COMPILE);
            glPushAttrib(GL_ENABLE_BIT);
        
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
               //** I use precalculated matrixes for speed 
            glLoadMatrixf((GLfloat *)(&(proj_matrix[0][0][0])));
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
        glEndList();

   //** BEGIN_2D_640 
        glNewList(kListBegin2D640, GL_COMPILE);
            glPushAttrib(GL_ENABLE_BIT);

            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
               //** I use precalculated matrixes for speed 
            glLoadMatrixf((GLfloat *)(&(proj_matrix[1][0][0])));
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
        glEndList();

   //** BEGIN_Lights 
        glNewList(kListBeginLights, GL_COMPILE);
            glPushAttrib(GL_ENABLE_BIT);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
               //** I use precalculated matrixes for speed 
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
        glEndList();

    /* END_2D */
        glNewList(kListEnd2D, GL_COMPILE);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopAttrib();
            glMatrixMode(GL_PROJECTION);
        glEndList();
}

void SetOrtho()
{
//  glOrtho(0, WinData.Width, WinData.Height - WinData.HeightSB, 0, -1, 1);
    glOrtho(0, imgWidth, imgHeight, 0, -1, 1);
    //Render_viewport();
}