/*
*   Glide64 - Glide video plugin for Nintendo 64 emulators.
*   Copyright (c) 2002  Dave2001
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public
*   License along with this program; if not, write to the Free
*   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
*   Boston, MA  02110-1301, USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators (tested mostly with Project64)
// Project started on December 29th, 2001
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
// Official Glide64 development channel: #Glide64 on EFnet
//
// Original author: Dave2001 (Dave2999@hotmail.com)
// Other authors: Gonetz, Gugaman
//
//****************************************************************

#include "Gfx1.3.h"

#ifdef GCC
#include <stdio.h>
#endif

HWND        g_hwndDlg = NULL;

#ifdef USE_TOOLTIPS

#include "resource.h"
#include "ToolTips.h"

BOOL EnumChildProc(HWND hwndCtrl, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
VOID OnWMNotify(LPARAM lParam);

HINSTANCE   g_hinst = NULL;
HWND        g_hwndTT = NULL;
HHOOK       g_hhk = NULL;

typedef struct TOOLTIPMSG_t {
    int id, id_alt;
    char *title;
    char *text;
} TOOLTIPMSG;

char gctitle[] = "Glide card #";
char gcmsg[] = "This option is only for users with multiple glide cards installed in one "
    "machine. This selects the glide card number to use. If you only have one glide card "
    "installed in your system, set this option to #1. Otherwise, select the number that "
    "corresponds to the glide card you want to use.\n"
    "[recommended: #1 if only one glide card present]";

#define NUM_MSG 39
TOOLTIPMSG ttmsg[NUM_MSG] = {
    { IDC_AUTODETECT, 0,
        "Autodetect Microcode",
        "If this option is checked, the microcode of the game "
        "will be detected automatically from the INI, and "
        "therefore it will not need to be set in this "
        "configuration dialog.\n"
        "[recommended: on]" },
    { IDC_UCODE, IDC_UCODE_T,
        "Force Microcode",
        "This option ONLY has an effect if Autodetect Microcode "
        "is unchecked, the crc from the game could not be "
        "found in the INI, OR after the game has already started "
        "running. In any of those three cases, this will "
        "select the microcode to use\n"
        "[recommended: any, turn on Autodetect Microcode]" },
    { IDC_RESOLUTION, IDC_RESOLUTION_T,
        "Resolution",
        "This option selects the resolution to set when switching to fullscreen mode "
        "(note again that the plugin must be in fullscreen mode to see anything).\n"
        "[recommended: 640x480, 800x600, 1024x768]" },
    { IDC_WIREFRAME, 0,
        "Wireframe",
        "This option, when checked, makes it so that the plugin will draw only the "
        "outlines of objects.  The colors specified in the combo box to the right "
        "determines the color that the wireframes show up as.\n"
        "[recommended: off]" },
    { IDC_WFMODE, 0,
        "Wireframe Colors",
        "This selects the colors to use for the wireframes (if wireframe mode is enabled). "
        "There are 3 modes:\n"
        "* Original colors - draw exactly as it would normally, textures and all, only "
        "in wireframes.\n"
        "* Vertex colors - use the colors specified in the vertices to draw the wireframes "
        "with.\n"
        "* Red only - use a constant red color to draw the wireframes.\n"
        "[recommended: Vertex colors]" },
    { IDC_LOD, 0,
        "Per-pixel level-of-detail calculation",
        "N64 uses special mechanism for mip-mapping, which nearly impossible to reproduce "
        "correctly on PC hardware. This option enables approximate emulation of this feature. "
        "For example, it is required for the Peach/Bowser portrait's transition in Super Mario 64. "
        "There are 3 modes:\n"
        "* off - LOD is not calculated\n"
        "* fast - fast imprecise LOD calculation.\n"
        "* precise - most precise LOD calculation possible, but more slow.\n"
        "[recommended: your preference]" },
    { IDC_FILTER, IDC_FILTER_T,
        "Filtering mode",
        "There are three filtering modes possible:\n\n"
        "* Automatic filtering - filter exactly how the N64 specifies.\n"
        "* Point-sampled filtering - causes texels to appear square and sharp.\n"
        "* Bilinear filtering - interpolates the texture to make it appear more smooth.\n"
        "[recommended: Force Bilinear]" },
    { IDC_FOG, 0,
        "Fog enabled",
        "This option, when checked, will allow fog to be used.\n"
        "[recommended: on]" },
    { IDC_BUFFERCLEAR, 0,
        "Buffer clear on every frame",
        "If this option is checked, the buffer will be cleared (screen initialized to black) "
        "at the beginning of each frame. If this option is disabled, the speed might increase "
        "slightly, but some trails may appear.\n"
        "[recommended: on]" },
    { IDC_VSYNC, 0,
        "Vertical Sync",
        "This option will enable the vertical sync, which will prevent tearing.  "
        "Note: this option will ONLY have effect if vsync is set to \"Software Controlled\" "
        "in the 3dfx Hub. (double-click the 3dfx icon in system tray).\n"
        "[recommended: off]" },
    { IDC_CRC, 0,
        "Fast texture checksum calculation",
        "This option will enable fast algorithm of checksum calculation.  "
        "Fast algorithm sometime returns the same checksum for different textures, which "
        "causes various texture errors. Thus, use it only if you have performance problems.\n"
        "[recommended: off]" },
    { IDC_SWAP, IDC_SWAP_T,
        "Buffer swapping method",
        "There are 3 buffer swapping methods:\n\n"
        "* old - swap buffers when vertical interrupt has occured.\n"
        "* new - swap buffers when set of conditions is satisfied. \n"
        "        Prevents flicker on some games.\n"
        "* hybrid - mix of first two methods.\n" 
        "        Can prevent even more flickering then previous method,\n"
        "        but also can cause artefacts.\n"
        "If you have flickering problems in a game (or graphics that don't show), "
        "try to change swapping method.\n"
        "[recommended: new (hybrid for Paper Mario)]" },
    { IDC_LOGGING, 0,
        "Log to RDP.txt",
        "RECOMMENDED FOR DEBUGGING ONLY - this option, when checked, will log EVERY SINGLE "
        "COMMAND the plugin processes to a file called RDP.txt in the current directory. "
        "This is incredibly slow, so I recommend keeping it disabled.\n"
        "[recommended: off]" },
    { IDC_LOGCLEAR, 0,
        "Log clear every frame",
        "RECOMMENDED FOR DEBUGGING ONLY - this option has no effect unless 'Log to RDP.txt' "
        "is checked. This will make it so that the log, RDP.txt, will be cleared at the "
        "beginning of every frame.\n"
        "[recommended: off]" },
    { IDC_RUNINWINDOW, 0,
        "Run and log in window",
        "RECOMMENDED FOR DEBUGGING ONLY - this option will make it so that the plugin will "
        "still process dlists in windowed mode. This allows for logging to occur while not "
        "in fullscreen, possibly allowing you to debug a crash.\n"
        "[recommended: off]" },
    { IDC_CACHEFILTER, 0,
        "Bilinear filter texture cache",
        "RECOMMENDED FOR DEBUGGING ONLY - when checked, this option will make the graphical "
        "debugger texture cache use bilinear filtering as opposed to point-sampled filtering, "
        "which it will use otherwise. See 'Filtering mode' for descriptions of bilinear and "
        "point-sampled filtering.\n"
        "[recommended: off]" },
    { IDC_UNKRED, 0,
        "Unknown combiners as red",
        "Objects that use an unimplemented combine mode will show up as red instead of "
        "assuming texture with full alpha. Disable this option to remove the red stuff "
        "and at least have a guess at the correct combine mode.\n"
        "[recommended: off]" },
    { IDC_LOGUNK, 0,
        "Log unknown combiners",
        "RECOMMENDED FOR DEBUGGING ONLY - when checked, this option will cause every "
        "unimplemented combiner drawn to be logged to a file called Unimp.txt in the "
        "current directory. This becomes slow when there are unimplemented combiners "
        "on the screen, so I recommend keeping it disabled.\n"
        "[recommended: off]" },
    { IDC_UNKCLEAR, 0,
        "Clear unknown combiner log every frame",
        "RECOMMENDED FOR DEBUGGING ONLY - this option works much like 'Log clear every "
        "frame' except it clears the combiner log (Unimp.txt) instead of RDP.txt at the "
        "beginning of each frame. Again, this option has no effect if 'combiner logging' "
        "is disabled.\n"
        "[recommended: off]" },
    { IDC_GID1, IDC_GID2,
        gctitle, gcmsg },
    { IDC_GID3, IDC_GID4,
        gctitle, gcmsg },
    
    { IDC_FPS, 0,
        "FPS counter",
        "When this option is checked, a FPS (frames per second) counter will be shown "
        "in the lower left corner of the screen.\n"
        "[recommended: your preference]" },
    { IDC_FPS1, 0,
        "VI/s counter",
        "When this option is checked, a VI/s (vertical interrupts per second) counter "
        "will be shown in the lower left corner of the screen.  This is like the FPS "
        "counter but will be consistent at 60 VI/s for full speed on NTSC (U) games and "
        "50 VI/s for full speed on PAL (E) ones.\n"
        "[recommended: your preference]" },
    { IDC_FPS2, 0,
        "% speed NTSC",
        "This displays a percentage of the actual N64 speed in the lower "
        "left corner of the screen.\n"
        "[recommended: your preference]" },
    { IDC_FPS3, 0,
        "FPS transparent",
        "If this is checked, the FPS counter will have a transparent background.  Otherwise, "
        "it will have a solid black background.\n"
        "[reccomended: your preference]" },
    { IDC_WRAP_BIG_TEX, 0,
        "Wrap textures which are too big for tmem",
        "This option can fix load of textures which can't fit their place in N64 texture memory. "
        "Example: trees in Zelda MM intro.\n"
        "[reccomended: mostly on]" },
    { IDC_CUSTOM, 0,
        "Use custom settings from the ini file",
        "Most of settings, available on this page, can be set individually for each game. "
        "If this option is on, custom settings will override common ones. "
        "This is very useful since you won't have to change or even pick out the best settings for a game "
        "because it is already done for you. However, if you want to run games with your own settings, set it off. "
        "The following settings are set in custom part: filtering mode, depth buffer mode, fog, "
        "buffer clear, buffer swapping mode, smart frame buffer read, "
        "depth buffer clear and frame buffer read alpha.\n"
        "[reccomended: on]" },
    { IDC_HOTKEYS, 0,
        "Enable hotkeys",
        "Currently Glide64 uses 4 hotkeys which you can use during gameplay:\n"
        "BACKSPACE - switch between filtering modes\n"
        "ALT+V - frame buffer read always on/off\n"
        "ALT+B - motion blur on/off\n"
        "ALT+C - corona fix on/off\n"
        "If you need to use these keys for other purposes, disable this option.\n"
        "[reccomended: on]" },
    { IDC_WRAPPER, 0,
        "Glide wrapper configuration",
         "If you are using Hacktarux's glide wrapper, press this button to configure it" },
    { IDC_CORONA, 0,
        "Fix for flame's corona in Zelda OOT and Zelda MM",
        "By default corona will be drawn without depth compare, i.e. can be seen through the objects. "
        "This option fixes it. However, it's better to use depth buffer render instead, "
        "to correctly emulate this effect.\n"
        "[reccomended: on for slow PC]" },
        /*
    { IDC_RE2_VIDEO, 0,
        "Show Resident Evil II videos in native resolution",
        "Video in RE2 is not of good quality, and it's clearly visible when video is scaled up to PC resolution. "
        "This option allows you to watch RE2 videos as is, i.e. without scaling and quality loss.\n"
        "[reccomended: your preference]" },
        */
    { IDC_CLOCK, 0,
        "Clock enabled",
        "This option will put a clock in the lower right corner of the screen, showing the current "
        "time.\n"
        "[reccomended: your preference]" },
    { IDC_FB_READ_ALWAYS, 0,
        "Read every frame",
        "In some games plugin can't detect frame buffer usage. "
        "In such case you need to enable this option to see frame buffer effects. "
        "Every drawn frame will be read from video card -> it works very slow.\n"
        "[reccomended: mostly off (needed only for a few games)]" },
    { IDC_FB_ALPHA, 0,
        "Read with alpha information",
        "N64 keeps alpha information in its frame buffer, Glide64 does not. "
        "In rare cases this information is vital. Example: Pokemon Puzzle League. "
        "When this option is on, all black pixels readed from frame buffer will have zero alpha.\n"
        "[reccomended: off]" },
    { IDC_FB_SMART, 0,
        "Smart frame buffer reading",
        "If on, plugin will try to detect frame buffer usage and read frame from video card only when it's needed.\n"
        "[reccomended: on for games which uses frame buffer effects]" },
    { IDC_ENABLEMB, 0,
        "Enable motion blur effect",
        "Some games use frame buffer for motion blur effect. "
        "It looks very cool, but may work slow since every frame must be read during this effect.\n"
        "[reccomended: your preference]" },
    { IDC_HIRESFB, 0,
        "Enable hardware frame buffer emulation",
        "If this option is on, plugin will create auxilary frame buffers in video memory instead of copying "
        "frame buffer content into main memory. This allows plugin to run frame buffer effects without slowdown "
        "and without scaling image down to N64's native resolution. This feature is fully supported by "
        "Voodoo 4/5 cards and partially by Voodoo3 and Banshee.\n"
        "[reccomended: on, if supported by your hardware]" },
    { IDC_FB_INFO, 0,
        "Get information about frame buffers",
        "This is compatibility option. It must be set on for Mupen64 and off for 1964\n" },
    { IDC_DB_CLEAR, 0,
        "Enable depth buffer clear",
        "Some special effects work only if N64 depth buffer filled with correct values. "
        "Example: Lens flare from the sun in Zelda. This option enables these effects."
        "You may set it off to save a bit of CPU time.\n"
        "[reccomended: on]" },
    { IDC_DB_RENDER, 0,
        "Enable depth buffer rendering",
        "This option works in conjunction with depth buffer clear to fully emulate N64 depth buffer. "
        "It is required for correct emulation of depth buffer based effects. "
        "However, it requires fast CPU to work full speed.\n"
        "[reccomended: on for fast PC]" },
};

// ** TOOLTIP CODE FROM MSDN LIBRARY SAMPLE WITH SEVERAL MODIFICATIONS **

// DoCreateDialogTooltip - creates a tooltip control for a dialog box, 
//     enumerates the child control windows, and installs a hook 
//     procedure to monitor the message stream for mouse messages posted 
//     to the control windows. 
// Returns TRUE if successful, or FALSE otherwise. 
// 
// Global variables 
// g_hinst - handle to the application instance. 
// g_hwndTT - handle to the tooltip control. 
// g_hwndDlg - handle to the dialog box. 
// g_hhk - handle to the hook procedure. 

BOOL DoCreateDialogTooltip(void) 
{
    // Ensure that the common control DLL is loaded, and create
    // a tooltip control.
    InitCommonControls();
    g_hwndTT = CreateWindowEx(0, TOOLTIPS_CLASS, (LPSTR) NULL,
        TTS_ALWAYSTIP|/*TTS_BALLOON*/0x40, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, g_hwndDlg, (HMENU) NULL, g_hinst, NULL);
    
    if (g_hwndTT == NULL)
        return FALSE;
    
    // Enumerate the child windows to register them with the tooltip
    // control.
    if (!EnumChildWindows(g_hwndDlg, (WNDENUMPROC) EnumChildProc, 0))
        return FALSE;
    
    // Install a hook procedure to monitor the message stream for mouse
    // messages intended for the controls in the dialog box.
    g_hhk = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,
        (HINSTANCE) NULL, GetCurrentThreadId());
    
    if (g_hhk == (HHOOK) NULL)
        return FALSE;
    
    return TRUE;
} 

// EmumChildProc - registers control windows with a tooltip control by
//     using the TTM_ADDTOOL message to pass the address of a 
//     TOOLINFO structure. 
// Returns TRUE if successful, or FALSE otherwise. 
// hwndCtrl - handle of a control window. 
// lParam - application-defined value (not used). 
BOOL EnumChildProc(HWND hwndCtrl, LPARAM lParam) 
{ 
    TOOLINFO ti; 
    
    ti.cbSize = sizeof(TOOLINFO); 
    ti.uFlags = TTF_IDISHWND; 
    ti.hwnd = g_hwndDlg; 
    ti.uId = (UINT) hwndCtrl; 
    ti.hinst = 0; 
    ti.lpszText = LPSTR_TEXTCALLBACK; 
    SendMessage(g_hwndTT, TTM_ADDTOOL, 0, 
       (LPARAM) (LPTOOLINFO) &ti); 
    return TRUE; 
} 

// GetMsgProc - monitors the message stream for mouse messages intended 
//     for a control window in the dialog box. 
// Returns a message-dependent value. 
// nCode - hook code. 
// wParam - message flag (not used). 
// lParam - address of an MSG structure. 
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
    MSG *lpmsg; 
    
    lpmsg = (MSG *) lParam; 
    if (nCode < 0 || !(IsChild(g_hwndDlg, lpmsg->hwnd))) 
        return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
    
    switch (lpmsg->message) { 
    case WM_MOUSEMOVE: 
    case WM_LBUTTONDOWN: 
    case WM_LBUTTONUP: 
    case WM_RBUTTONDOWN: 
    case WM_RBUTTONUP: 
        if (g_hwndTT != NULL) { 
            MSG msg; 

            int idCtrl = GetDlgCtrlID((HWND)lpmsg->hwnd);
            
            msg.lParam = lpmsg->lParam; 
            msg.wParam = lpmsg->wParam; 
            msg.message = lpmsg->message; 
            msg.hwnd = lpmsg->hwnd; 
            SendMessage(g_hwndTT, TTM_RELAYEVENT, 0, 
                (LPARAM) (LPMSG) &msg); 
#ifdef TTM_SETMAXTIPWIDTH
            SendMessage(g_hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);
#endif
            SendMessage(g_hwndTT, TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM)MAKELONG(500,0));
            SendMessage(g_hwndTT, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAKELONG(32000,0));

            for (int i=0; i<NUM_MSG; i++)
            {
                if (idCtrl == ttmsg[i].id ||
                    idCtrl == ttmsg[i].id_alt)
                {
                    SendMessage(g_hwndTT, /*TTM_SETTITLE*/(WM_USER + 32), 1, (LPARAM)ttmsg[i].title);
                    break;
                }
            }
        } 
        break; 
    default: 
        break; 
    } 
    return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
} 


// OnWMNotify - provides the tooltip control with the appropriate text 
//     to display for a control window. This function is called by 
//     the dialog box procedure in response to a WM_NOTIFY message. 
// lParam - second message parameter of the WM_NOTIFY message. 
VOID OnWMNotify(LPARAM lParam) 
{ 
    LPTOOLTIPTEXT lpttt; 
    int idCtrl; 
    
    if ((((LPNMHDR) lParam)->code) == TTN_NEEDTEXT) { 
        idCtrl = GetDlgCtrlID((HWND) ((LPNMHDR) lParam)->idFrom); 
        lpttt = (LPTOOLTIPTEXT) lParam; 

        for (int i=0; i<NUM_MSG; i++)
        {
            if (idCtrl == ttmsg[i].id ||
                idCtrl == ttmsg[i].id_alt)
            {
                lpttt->lpszText = ttmsg[i].text;
                return;
            }
        }
    } 
    return;
}


#endif /* USE_TOOLTIPS */
