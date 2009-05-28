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

#ifndef _INITGL_H_
#define _INITGL_H_


typedef struct 
{
    HWND       hWnd;
    HINSTANCE  hInstance;
    char       *szCmdLine;
    int        Width;
    int        Height;
    int        iCmdShow;
    int        HeightSB;

    HDC hDC;
    HGLRC hGLRC;
    HPALETTE hPalette;
    HWND hStatusBar;    /* if render window does not have a status bar then this is NULL */

} WINDATA_T;


extern WINDATA_T WinData;
extern char szAppName[];

#define AlphaLevel 0.021f

/*****************************************************************************/
/* Copiled Displaylists for speedup                                          */
/*****************************************************************************/
#define kListEnd2D          1
#define kListBeginLights    9
#define kListBegin2D160   160
#define kListBegin2D320   320
#define kListBegin2D640   640
#define kListBegin2D800   800
#define kListBegin2D960   960
#define kListBegin2D1024 1024
#define kListBegin2D1200 1200

BOOL CreateDisplayWindow(void);
extern BOOL WGL_Init (void);
extern void WGL_UnInit (void);

extern void WGL_RedoPalette (void);

extern BOOL WGL_IsPaletteValid (void);
extern BOOL WGL_IsGLRCValid (void);

extern void WGL_InitState(void);
extern BOOL WGL_SwapBuffers(void);

extern void WGL_SetViewport(int x, int y);
extern void SetOrtho();

#endif