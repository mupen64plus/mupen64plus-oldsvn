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

/**********************************************************************************
Common gfx plugin spec, version #1.3 maintained by zilmar (zilmar@emulation64.com)

All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
***********************************************************************************

Notes:
------

Setting the approprate bits in the MI_INTR_REG and calling CheckInterrupts which 
are both passed to the DLL in InitiateGFX will generate an Interrupt from with in 
the plugin.

The Setting of the RSP flags and generating an SP interrupt  should not be done in
the plugin

**********************************************************************************/

// rdram mask at 0x400000 bytes
#define BMASK   0x3FFFFF
#define WMASK   0x1FFFFF
#define DMASK   0x0FFFFF

// THIS FILE IS A PRECOMPILED HEADER TO DECREASE BUILD TIME.  INCLUDE ALL STANDARD
//  .H FILES HERE

#ifndef _GFX_H_INCLUDED__
#define _GFX_H_INCLUDED__

#include <windows.h>
#include <stdio.h>
#include <fstream.h>
#include <stddef.h>     // offsetof
#include <math.h>
#include <io.h>
#include <direct.h>
#include <glide.h>
#include "rdp.h"

#if defined(__cplusplus)
extern "C" {
#endif

//#define LOGGING
//#define LOGGING_1
//#define RDP_ERROR_LOG

//#define RDP_NOCLEAR

//#define NOPROCESSWINDOW       // don't process in windowed mode

//#define LOG_COMMANDS      // log cmd0 and cmd1?

//#define PERFORMANCE

//#define LFB_STRETCHING
//#define LFB_WRITE

//#define WINPROC_OVERRIDE

//#define FOGTEST

#define UNIMP_LOG           // Keep enabled, option in dialog
#define BRIGHT_RED          // Keep enabled, option in dialog

#define COLORED_DEBUGGER    // ;) pretty colors

extern __int64 perf_freq;
extern __int64 fps_last;
extern __int64 fps_next;
extern float            fps;
extern DWORD            fps_count;

extern DWORD resolutions[0x18][2];

#ifdef PERFORMANCE
extern __int64 perf_cur;
extern __int64 perf_next;
#endif

#ifdef LOGGING
extern ofstream loga;
//#define OPENLOG() loga.open("log.txt")
//#define CLOSELOG() loga.close()
#define LOG(x) loga.open("log.txt",ios::app); loga << x; loga.close();

#ifdef LOGGING_1
#define LOG1(x) LOG(x)
#else
#define LOG1(x)
#endif

#else
#define OPENLOG()
#define CLOSELOG()
#define LOG(x)
#define LOG1(x)
#endif

/*#ifdef RDP_LOGGING
extern ofstream rdp_log;
#define OPEN_RDP_LOG() rdp_log.open("rdp.txt")
#define CLOSE_RDP_LOG() rdp_log.close()
#define RDP(x) rdp_log << x
#else
#define OPEN_RDP_LOG()
#define CLOSE_RDP_LOG()
#define RDP(x)
#endif*/

#define RDP_LOGGING     // defined doesn't mean logging anymore, use checkbox
#ifdef RDP_LOGGING
extern ofstream rdp_log;
#define OPEN_RDP_LOG() if (settings.logging) rdp_log.open ("rdp.txt")
#define CLOSE_RDP_LOG() if (settings.logging) rdp_log.close ()
#define RDP(x) if (settings.logging) rdp_log << x
#else
#define OPEN_RDP_LOG()
#define CLOSE_RDP_LOG()
#define RDP(x)
#endif

#ifdef RDP_ERROR_LOG
extern ofstream rdp_err;
#define OPEN_RDP_E_LOG() rdp_err.open("rdp_e.txt")
#define CLOSE_RDP_E_LOG() rdp_err.close()
#define RDP_E(x) { RDP(x); rdp_err << x; }
#else
#define OPEN_RDP_E_LOG()
#define CLOSE_RDP_E_LOG()
#define RDP_E(x) RDP(x)
#endif

/*#ifdef RDP_LOGGING
void FRDP (char *fmt, ...);
void FRDP_E (char *fmt, ...);
#else
#ifdef RDP_ERROR_LOG
void FRDP_E (char *fmt, ...);
#else
#define FRDP()
#define FRDP_E()
#endif
#endif*/

__inline void FRDP (char *fmt, ...)
{
#ifdef RDP_LOGGING
    if (!settings.logging) return;

    va_list ap;
    va_start(ap, fmt);
    vsprintf(out_buf, fmt, ap);
    RDP (out_buf);
    va_end(ap);
#endif
}
__inline void FRDP_E (char *fmt, ...)
{
    if (!settings.logging) return;
#ifdef RDP_ERROR_LOG
    va_list ap2;
    va_start(ap2, fmt);
    vsprintf(out_buf, fmt, ap2);
    RDP_E (out_buf);
    va_end(ap2);
#else
#ifdef RDP_LOGGING
    va_list ap;
    va_start(ap, fmt);
    vsprintf(out_buf, fmt, ap);
    RDP (out_buf);
    va_end(ap);
#endif
#endif
}

//#define FRDP()
//#define FRDP_E()

extern BOOL fullscreen;
extern BOOL to_fullscreen;
extern BOOL debugging;
extern HINSTANCE hInstance;

extern BOOL exception;

BOOL InitGfx ();
void ReleaseGfx ();

/* Plugin types */
#define PLUGIN_TYPE_GFX             2

#define EXPORT                      __declspec(dllexport)
#define CALL                        _cdecl

/***** Structures *****/
typedef struct {
    WORD Version;        /* Set to 0x0103 */
    WORD Type;           /* Set to PLUGIN_TYPE_GFX */
    char Name[100];      /* Name of the DLL */

    /* If DLL supports memory these memory options then set them to TRUE or FALSE
       if it does not support it */
    BOOL NormalMemory;    /* a normal BYTE array */ 
    BOOL MemoryBswaped;  /* a normal BYTE array where the memory has been pre
                              bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;

typedef struct {
    HWND hWnd;          /* Render window */
    HWND hStatusBar;    /* if render window does not have a status bar then this is NULL */

    BOOL MemoryBswaped;    // If this is set to TRUE, then the memory has been pre
                           //   bswap on a dword (32 bits) boundry 
                           //   eg. the first 8 bytes are stored like this:
                           //        4 3 2 1   8 7 6 5

    BYTE * HEADER;  // This is the rom header (first 40h bytes of the rom
                    // This will be in the same memory format as the rest of the memory.
    BYTE * RDRAM;
    BYTE * DMEM;
    BYTE * IMEM;

    DWORD * MI_INTR_REG;

    DWORD * DPC_START_REG;
    DWORD * DPC_END_REG;
    DWORD * DPC_CURRENT_REG;
    DWORD * DPC_STATUS_REG;
    DWORD * DPC_CLOCK_REG;
    DWORD * DPC_BUFBUSY_REG;
    DWORD * DPC_PIPEBUSY_REG;
    DWORD * DPC_TMEM_REG;

    DWORD * VI_STATUS_REG;
    DWORD * VI_ORIGIN_REG;
    DWORD * VI_WIDTH_REG;
    DWORD * VI_INTR_REG;
    DWORD * VI_V_CURRENT_LINE_REG;
    DWORD * VI_TIMING_REG;
    DWORD * VI_V_SYNC_REG;
    DWORD * VI_H_SYNC_REG;
    DWORD * VI_LEAP_REG;
    DWORD * VI_H_START_REG;
    DWORD * VI_V_START_REG;
    DWORD * VI_V_BURST_REG;
    DWORD * VI_X_SCALE_REG;
    DWORD * VI_Y_SCALE_REG;

    void (*CheckInterrupts)( void );
} GFX_INFO;

extern GFX_INFO gfx;

void ReadSettings ();
void WriteSettings ();

/******************************************************************
  Function: CaptureScreen
  Purpose:  This function dumps the current frame to a file
  input:    pointer to the directory to save the file to
  output:   none
*******************************************************************/ 
EXPORT void CALL CaptureScreen ( char * Directory );

/******************************************************************
  Function: ChangeWindow
  Purpose:  to change the window between fullscreen and window 
            mode. If the window was in fullscreen this should 
            change the screen to window mode and vice vesa.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ChangeWindow (void);

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL CloseDLL (void);

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllAbout ( HWND hParent );

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllConfig ( HWND hParent );

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
EXPORT void CALL DllTest ( HWND hParent );

/******************************************************************
  Function: DrawScreen
  Purpose:  This function is called when the emulator receives a
            WM_PAINT message. This allows the gfx to fit in when
            it is being used in the desktop.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL DrawScreen (void);

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo );

/******************************************************************
  Function: InitiateGFX
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 graphics
            uses. This is not called from the emulation thread.
  Input:    Gfx_Info is passed to this function which is defined
            above.
  Output:   TRUE on success
            FALSE on failure to initialise
             
  ** note on interrupts **:
  To generate an interrupt set the appropriate bit in MI_INTR_REG
  and then call the function CheckInterrupts to tell the emulator
  that there is a waiting interrupt.
*******************************************************************/ 
EXPORT BOOL CALL InitiateGFX (GFX_INFO Gfx_Info);

/******************************************************************
  Function: MoveScreen
  Purpose:  This function is called in response to the emulator
            receiving a WM_MOVE passing the xpos and ypos passed
            from that message.
  input:    xpos - the x-coordinate of the upper-left corner of the
            client area of the window.
            ypos - y-coordinate of the upper-left corner of the
            client area of the window. 
  output:   none
*******************************************************************/ 
EXPORT void CALL MoveScreen (int xpos, int ypos);

/******************************************************************
  Function: ProcessDList
  Purpose:  This function is called when there is a Dlist to be
            processed. (High level GFX list)
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ProcessDList(void);

/******************************************************************
  Function: ProcessRDPList
  Purpose:  This function is called when there is a Dlist to be
            processed. (Low level GFX list)
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ProcessRDPList(void);

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL RomClosed (void);

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the 
            emulation thread)
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL RomOpen (void);

/******************************************************************
  Function: ShowCFB
  Purpose:  Useally once Dlists are started being displayed, cfb is
            ignored. This function tells the dll to start displaying
            them again.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ShowCFB (void);

/******************************************************************
  Function: UpdateScreen
  Purpose:  This function is called in response to a vsync of the
            screen were the VI bit in MI_INTR_REG has already been
            set
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL UpdateScreen (void);

/******************************************************************
  Function: ViStatusChanged
  Purpose:  This function is called to notify the dll that the
            ViStatus registers value has been changed.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ViStatusChanged (void);

/******************************************************************
  Function: ViWidthChanged
  Purpose:  This function is called to notify the dll that the
            ViWidth registers value has been changed.
  input:    none
  output:   none
*******************************************************************/ 
EXPORT void CALL ViWidthChanged (void);

#if defined(__cplusplus)
}
#endif
#endif
