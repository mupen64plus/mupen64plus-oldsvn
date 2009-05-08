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

#include <windows.h>
#include <process.h>  // _beginthread, _endthread 
#include <stdio.h>
#include <GL/gl.h>
//#include <glext.h>
#include <GL/glu.h>

#include "gfx.h"
#include "wingl.h"
#include "crc_ucode.h"
#include "rdp_registers.h"

//** externals
void RealExecuteDList(unsigned __int32 dwAddr);
void RefreshVisual(void);
void CreateConfigDlg();
void ResizeVisualClipRectangle();
void SetVisualClipRectangle(_u32 ulx, _u32 uly, _u32 lrx, _u32 lry);
int GetUCode();
void ClearUCode();
void DelTexture();
void init_cache();

//** defines
#define GFX_VERSION                 0x0102

char plgname[] = "TR64 OpenGL";
int vermjr = 0;
int vermnr = 9;
#ifdef DUMP_TILE
char versfx[] = ".6-Tile Dumper";
#else
char versfx[] = ".0c";
#endif

//** globals
BOOLEAN  bFullScreen = FALSE; // FullScreen Mode
//static BOOLEAN bFullScreen = 0;

int             *pInterruptMask;
unsigned char   *pRDRAM;
unsigned char   *pIMEM;
unsigned char   *pDMEM;
unsigned char   *pVIREG;
unsigned char   *pROM;
_u32            *pVIORG;
_u32            *pVISTS;
_u32            *pVIXSL;
_u32            *pVIYSL;
_u32            *pVIWTH;
_u32            *pVIHST;
_u32            *pVIHGT;

//BOOL          Draw2d = FALSE;
BOOL            Draw2d = TRUE;
BOOL            NoBlending = FALSE;         
BOOL            m_bFullScreen = FALSE;
RECT            m_WindowRect;
WINDOWPLACEMENT m_WindowPlacement;
HMENU           m_hOldMenu;
DWORD           m_iOldWindowStyle;
BOOL interlaced = FALSE;

int             ucode_version;
int skipCount;

HINSTANCE g_hInstance;
HWND g_hWnd;

DEVMODEA m_DMsaved,devMode;

extern int DList_C;
extern WINDATA_T WinData;

extern int ucode;
extern BOOL HandsetUcode;
extern float fScalex;
extern float fScaley;

float imgHeight = 240.0f;
float imgWidth  = 320.0f;

#define V_DONOTHING 0
#define V_EXEDLIST 1
#define V_CLOSEROM 2
#define V_STARTROM 3

HANDLE  hVideoThread;
BOOL VideoDone = FALSE;
int VideoDoNow = V_DONOTHING;
BOOL VideoBusy = FALSE;
BOOL vi_Hires = FALSE;

void rdp_reset();

void SetWindowScale()
{
    float vi_height = (float)(*pVIHGT&0xffff - (*pVIHGT>>0x10));
    float vi_width = (float)((*pVIHST&0xffff) - (*pVIHST>>0x10));// *pVIWTH;
    float vi_xscale = (float)(*pVIXSL  & 0xFFF) / 1024;
    float vi_yscale = (float)(*pVIYSL  & 0xFFF) / 1024;

    float vi_scl_height = 1.0f;
    float vi_scl_width = 1.0f;

    RECT clRect;
    LPRECT lpRect = &clRect;
    RECT clRectSB;
    LPRECT lpRectSB = &clRectSB;

    vi_Hires = (*pVISTS & 0x40)? TRUE : FALSE;

    vi_height *= vi_xscale;
    vi_width *= vi_yscale;

    //if ((!vi_width) && (*pVIXSL  & 0xFFF))
    //  vi_width = (_u32)((float)(*pVIWTH  & 0xFFF) / ((float)(*pVIXSL  & 0xFFF) / 1024));
    if (!vi_width)
        vi_width = (float)(*pVIWTH  & 0xFFF);

    if (!vi_height)
        vi_height = imgHeight;

    if (!vi_width) 
        vi_width = imgWidth;

    vi_scl_height = (float)vi_height / imgHeight;
    vi_scl_width = (float)vi_width / imgWidth;

    GetClientRect(WinData.hWnd,lpRect);
    if ((WinData.hStatusBar) && !bFullScreen)
        GetClientRect(WinData.hStatusBar,lpRectSB);
    else
    {
        clRectSB.bottom = 0;
        clRectSB.top = 0;
        clRectSB.right = 0;
        clRectSB.left = 0;
    }

    WinData.Width = clRect.right-clRect.left;
    WinData.HeightSB = (clRectSB.bottom-clRectSB.top);
    WinData.Height = (clRect.bottom-clRect.top);

    fScalex = (float)WinData.Width / (float)imgWidth;
//  fScalex *= ((float)(*pVIXSL  & 0xFFF) / 1024.0f);

    fScaley = (float)(WinData.Height - WinData.HeightSB) / (float)imgHeight;
//  fScaley *= ((float)(*pVIYSL  & 0xFFF) / 1024.0f);

//  fScaley *= vi_scl_height;
//  fScalex *= vi_scl_width;
}

void VideoThread()
{
    WGL_Init();
    SetThreadPriority(hVideoThread, THREAD_PRIORITY_HIGHEST);

    while(!VideoDone)
    {
        switch (VideoDoNow)
        {
        case V_DONOTHING:
            break;
        case V_EXEDLIST:
            {
                unsigned __int32 addr = ((unsigned __int32 *)pDMEM)[0xff0>>2];

                SetWindowScale();
                SetOrtho();

                SetVisualClipRectangle((int)0, (int)0, (int)imgWidth, (int)imgHeight);


                ucode_version = GetUCode();
                
                VideoBusy = TRUE;
                RealExecuteDList(addr);
            }
            break;
        case V_CLOSEROM:
            DelTexture();
            //WGL_UnInit();
            break;
        case V_STARTROM:
            ClearUCode();
            //WGL_Init();
            break;
        }

        VideoBusy = FALSE;
        SuspendThread(hVideoThread);
    }

    _endthreadex(1);
}

/*
===============================================================================

  All our exported functions...

===============================================================================
*/
void SaveUCode(int _ucode);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    ucode = -1;
    HandsetUcode = FALSE;

    g_hInstance = hinstDLL;


    return TRUE;
}

//void   (__cdecl* DllClose)(void) = NULL;

void   __cdecl DllClose (void)
{
    VideoDone = TRUE;

#ifdef USE_THREADS
    TerminateThread(hVideoThread, 0);
#endif USE_THREADS
}

// __________________________________________________________________________________________________
// SetWindowMode
//
int 
SetWindowedMode(int _width, int _height, int _bpp)
{



    //SetWindowLong(WinData.hWnd, GWL_STYLE, m_iWindowStyle);
    SetWindowLong(WinData.hWnd, GWL_STYLE, 0);
    ShowWindow(WinData.hWnd, SW_SHOWNORMAL);
    
    if (!ChangeDisplaySettings(NULL,CDS_TEST)) {    // If The Shortcut Doesn't Work
        ChangeDisplaySettings(NULL,CDS_RESET);      // Do It Anyway (To Get The Values Out Of The Registry)
        ChangeDisplaySettings(&m_DMsaved,CDS_RESET);    // Change It To The Saved Settings
    } else {
        ChangeDisplaySettings(NULL,CDS_RESET);      // If It Works, Go Right Ahead
    }

    //
    // Reinit Winmdow
    if (WinData.hStatusBar) { ShowWindow(WinData.hStatusBar,SW_SHOW); }
    if (m_hOldMenu) { 
        SetMenu(WinData.hWnd,m_hOldMenu); 
        m_hOldMenu = NULL;
    }
    SetWindowLong(WinData.hWnd, GWL_STYLE, m_iOldWindowStyle);
    SetWindowPos(WinData.hWnd, HWND_NOTOPMOST, m_WindowPlacement.rcNormalPosition.left, 
        m_WindowPlacement.rcNormalPosition.top, 640, 480, SWP_NOSIZE|SWP_SHOWWINDOW);
    ShowCursor(TRUE);
    
    m_bFullScreen = FALSE;

    SetWindowPos(WinData.hWnd, HWND_TOP, m_WindowRect.left, m_WindowRect.top, 
                            m_WindowRect.right - m_WindowRect.left,
                            m_WindowRect.bottom - m_WindowRect.top,
                            SWP_SHOWWINDOW);

    GetWindowRect(WinData.hWnd, &m_WindowRect);

//  _trace_("COpenGL: Window-Mode\n");
    return(FALSE);
}


// __________________________________________________________________________________________________
// SetFullscreenMode
//
int SetFullscreenMode(int _width, int _height, int _bpp);

int SetFullscreenMode(int _width, int _height, int _bpp)
{

    ZeroMemory(&devMode, sizeof(DEVMODE));
    
    devMode.dmSize       = sizeof(DEVMODE);
    devMode.dmPelsWidth  = _width;
    devMode.dmPelsHeight = _height;
    devMode.dmBitsPerPel = _bpp;
    devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
    
    if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    {
        m_bFullScreen = FALSE;
        return FALSE;
    }

    //
    // dont show the mouse cursor in fullscreen
    ShowCursor(0);

    //
    // ReInit the Window
    m_WindowPlacement.length = sizeof(m_WindowPlacement);
    GetWindowPlacement(WinData.hWnd, &m_WindowPlacement);
    if (WinData.hStatusBar) 
    { 
        ShowWindow(WinData.hStatusBar,SW_HIDE); 
    }
    m_hOldMenu= GetMenu(WinData.hWnd);
    if (m_hOldMenu) 
    { 
        SetMenu(WinData.hWnd,NULL); 
    }

    m_iOldWindowStyle = GetWindowLong(WinData.hWnd,GWL_STYLE);
    SetWindowLong(WinData.hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
    SetWindowLong(WinData.hWnd, GWL_STYLE, WS_VISIBLE|WS_POPUP);
    
    SetWindowPos(WinData.hWnd, HWND_TOPMOST, 0, 0, _width,_height, SWP_SHOWWINDOW);


    //
    // setup
    m_bFullScreen = TRUE;
//  m_iFullscreenWidth = _width;
//  m_iFullscreenHeight = _height;


//  _trace3_("COpenGL: Fullscreen set to width: %i height: %i bpp: %i\n", _width, _height, _bpp);
    return TRUE;
}

void ChangeWinSize()
{
}

//EXPORT void CALL CloseDLL (void);
void   __cdecl CloseDLL (void)
{
    SaveUCode(ucode_version);
//  WGL_UnInit();
}

//void   (__cdecl* ChangeWindow)(BOOL) = NULL;

void   __cdecl ChangeWindow (void)
{

    if (bFullScreen == 0) 
    {
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &m_DMsaved);
                
        GetWindowRect(WinData.hWnd, &m_WindowRect);    // remember windowed mode's rect for later
//        ChangeWinSize(WinData.hWnd, m_DMsaved.dmPelsWidth, m_DMsaved.dmPelsHeight, NULL);
        bFullScreen = SetFullscreenMode(m_DMsaved.dmPelsWidth, m_DMsaved.dmPelsHeight, 16);
    }
    else
    {
//        ChangeWinSize(WinData.hWnd, 640, 480, NULL);
        bFullScreen = SetWindowedMode(640, 480, 16);
    }
}

//void   (__cdecl* DllAbout)(HWND) = NULL;

void   __cdecl DllAbout ( HWND hParent )
{
    char tMsg[256];
    char tTilte[256];

    sprintf(tTilte,"%s v%i.%i%s - About Box",plgname,vermjr,vermnr,versfx);
    sprintf(tMsg,"%s v%i.%i%s\nby Icepir8\ntexture cache modified by Hacktarux",plgname,vermjr,vermnr,versfx);

    MessageBox(NULL, tMsg, tTilte, MB_OK|MB_ICONINFORMATION);
}

//void   (__cdecl* DllConfig)(HWND) = NULL;

void   __cdecl DllConfig ( HWND hParent )
{
    CreateConfigDlg();
}

//void   (__cdecl* DllTest)(HWND) = NULL;

void   __cdecl DllTest ( HWND hParent )
{
    MessageBox(NULL, "No Test Box yet", "message", MB_OK);
}

//void   (__cdecl* DrawScreen)(void) = NULL;

void   __cdecl DrawScreen (void)
{

}

//void   (__cdecl* GetDllInfo )( PLUGIN_INFO *) = NULL;

void   __cdecl GetDllInfo ( PLUGIN_INFO * PluginInfo )
{   
    PluginInfo->NormalMemory = FALSE;
    PluginInfo->MemoryBswaped = TRUE;
    PluginInfo->Version = GFX_VERSION;
    PluginInfo->Type = PLUGIN_TYPE_GFX;
    sprintf(PluginInfo->Name,"%s v%i.%i%s",plgname,vermjr,vermnr,versfx);
}

//#define USE_THREADS

void (*CheckInterrupts)( void );

//BOOL   (__cdecl* InitiateGFX)(GFX_INFO) = NULL;

BOOL   __cdecl InitiateGFX (GFX_INFO Gfx_Info)
{
    RECT clRect;
    LPRECT lpRect = &clRect;
    RECT clRectSB;
    LPRECT lpRectSB = &clRectSB;

    pInterruptMask = (int*)Gfx_Info.MI_INTR_REG;
    pRDRAM = Gfx_Info.RDRAM;
    pDMEM = Gfx_Info.DMEM;
    pIMEM = Gfx_Info.IMEM;
    pROM = Gfx_Info.HEADER;
    pVIREG = (unsigned char*)Gfx_Info.DPC_START_REG;
    pVIORG = (_u32*)Gfx_Info.VI_ORIGIN_REG;
    pVISTS = (_u32*)Gfx_Info.VI_STATUS_REG;
    pVIXSL = (_u32*)Gfx_Info.VI_X_SCALE_REG;
    pVIYSL = (_u32*)Gfx_Info.VI_Y_SCALE_REG;
    pVIWTH = (_u32*)Gfx_Info.VI_WIDTH_REG;
    pVIHGT = (_u32*)Gfx_Info.VI_V_START_REG;
    pVIHST = (_u32*)Gfx_Info.VI_H_START_REG;

    interlaced = ((*pVISTS & 0x040) != 0);
    CheckInterrupts = Gfx_Info.CheckInterrupts;


//  WinData.Width = 640;
//  WinData.Height = 480;
    WinData.hWnd = Gfx_Info.hWnd;
    WinData.hStatusBar = Gfx_Info.hStatusBar;

    GetClientRect(WinData.hWnd,lpRect);
    if ((WinData.hStatusBar) && !bFullScreen)
        GetClientRect(WinData.hStatusBar,lpRectSB);
    else
    {
        clRectSB.bottom = 0;
        clRectSB.top = 0;
        clRectSB.right = 0;
        clRectSB.left = 0;
    }

    WinData.Width = clRect.right-clRect.left;
    WinData.HeightSB = -(clRectSB.bottom-clRectSB.top);
    WinData.Height = (clRect.bottom-clRect.top) - (clRectSB.bottom-clRectSB.top);

    fScalex = WinData.Width / imgWidth;
    fScaley = WinData.Height / imgHeight;

//  if (interlaced) fScaley /= 2.0f;

//  if(!WGL_Init()) return FALSE;
#ifdef USE_THREADS
    hVideoThread = (HANDLE)_beginthread((void*)VideoThread, 4096, NULL);
#else //USE_THREADS
//  WGL_Init();
#endif //USE_THREADS

    return TRUE;
}

//void   (__cdecl* MoveScreen)(int, int) = NULL;

void   __cdecl MoveScreen (int xpos, int ypos)
{
}

//void   (__cdecl* ProcessDList)(void) = NULL;
void   __cdecl ProcessDList(void)
{
    unsigned __int32 DlistAddr = ((unsigned __int32 *)pDMEM)[0xff0>>2];

#ifdef USE_THREADS    
    VideoBusy = TRUE;
    VideoDoNow = V_EXEDLIST;
    
    ResumeThread(hVideoThread);

    while(VideoBusy)
    {
        Sleep(1);
    }
#else // USE_THREADS
    {
        unsigned __int32 addr = ((unsigned __int32 *)pDMEM)[0xff0>>2];
/*
        RECT clRect;
        LPRECT lpRect = &clRect;
        RECT clRectSB;
        LPRECT lpRectSB = &clRectSB;

        GetClientRect(WinData.hWnd,lpRect);

        if ((WinData.hStatusBar) && !bFullScreen)
            GetClientRect(WinData.hStatusBar,lpRectSB);
        else
        {
            clRectSB.bottom = 0;
            clRectSB.top = 0;
            clRectSB.right = 0;
            clRectSB.left = 0;
        }

        WinData.Width = clRect.right-clRect.left;
        WinData.HeightSB = (clRectSB.bottom-clRectSB.top);
        WinData.Height = (clRect.bottom-clRect.top);

        {
            imgHeight = (imgWidth / 4.0f) * 3.0f;
        }

        interlaced = ((*pVIORG & 0x040) != 0);

        fScalex = WinData.Width / imgWidth;
        fScaley = (WinData.Height - WinData.HeightSB) / imgHeight;
*/
        SetWindowScale();
        SetOrtho();

        SetVisualClipRectangle((int)0, (int)0, (int)imgWidth, (int)imgHeight);

        ucode_version = GetUCode();
                
        VideoBusy = TRUE;
        RealExecuteDList(addr);
        CheckInterrupts();
    }
#endif // USE_THREADS
}

void   __cdecl ProcessRDPList(void)
{
}

//void   (__cdecl* RomClosed)(void) = NULL;
extern HWND OglWindow;

void   __cdecl RomClosed (void)
{
    SaveUCode(ucode_version);
#ifdef USE_THREADS
    ResumeThread(hVideoThread);
    VideoDoNow = V_CLOSEROM;
    while(VideoBusy)
    {
        Sleep(1);
    }
    ResumeThread(hVideoThread);
#else
    __try
    {
        rdp_reg.halt = 1;
        DelTexture();
        rdp_reset();
        WGL_UnInit();
    }
    __except(NULL, EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif // USE_THREADS
//  if (OglWindow)
//      EndDialog(OglWindow, TRUE);
}

//void   (__cdecl* UpdateScreen)(void) = NULL;

void   __cdecl UpdateScreen (void)
{
}

//void   (__cdecl* ViStatusChanged)(void) = NULL;

void   __cdecl ViStatusChanged (void)
{
}

//void   (__cdecl* ViWidthChanged)(void) = NULL;

void   __cdecl ViWidthChanged (void)
{
}

//void   (__cdecl* RomOpen)(void) = NULL;

void   __cdecl RomOpen (void)
{
    init_cache();
    ClearUCode();

#ifdef USE_THREADS
    VideoDoNow = V_STARTROM;
    while(VideoBusy)
    {
        Sleep(1);
    }
    ResumeThread(hVideoThread);
//  if(!WGL_Init()) return;
#else // USE_THREADS
    WGL_Init();
#endif // USE_THREADS
}

void   __cdecl DLL_ResetGFX (void)
{
}
