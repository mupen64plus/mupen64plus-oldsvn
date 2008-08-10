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
*   Licence along with this program; if not, write to the Free
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

#ifdef _WIN32

#include "resource.h"
#include "ToolTips.h"

PROPSHEETHEADER m_PropSheet;
PROPSHEETPAGE m_psp[3];

/******************************************************************
Function: DllConfig
Purpose:  This function is optional function that is provided
to allow the user to configure the dll
input:    a handle to the window that calls this function
output:   none
*******************************************************************/ 
void CALL DllConfig ( HWND hParent )
{
  ReadSettings ();
  if (romopen)
  {
    char name[21] = "DEFAULT";
    ReadSpecialSettings (name);
    
    // get the name of the ROM
    for (int i=0; i<20; i++)
      name[i] = gfx.HEADER[(32+i)^3];
    name[20] = 0;
    
    // remove all trailing spaces
    while (name[strlen(name)-1] == ' ')
      name[strlen(name)-1] = 0;
    
    ReadSpecialSettings (name);
  }
  m_PropSheet.hwndParent = hParent;
  if (PropertySheet(&m_PropSheet))
  {
    // re-init evoodoo graphics to resize window
    if (evoodoo && fullscreen && !ev_fullscreen) {
      ReleaseGfx ();
      InitGfx (TRUE);
    }
  }
}


//
// ConfigProc - window proceedure for the configuration dialog
//
BOOL CALLBACK ConfigPageProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_NOTIFY:
    switch (((NMHDR FAR *) lParam)->code) {
#ifdef USE_TOOLTIPS
    case PSN_SETACTIVE: 
      if (!g_hhk)
      {
        g_hwndDlg = hwndDlg;
        g_hinst = hInstance;
        g_hwndTT = NULL;
        DoCreateDialogTooltip ();
      }
      break;
    case TTN_NEEDTEXT:
      OnWMNotify (lParam);
      break;
    case PSN_KILLACTIVE: 
    case PSN_QUERYCANCEL:
      //user clicked OK or selected another page
      if (g_hwndDlg == hwndDlg)
      {
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
        g_hwndTT = NULL;
        g_hwndDlg = NULL;
      }
      break;        
#endif
    case PSN_APPLY:
      // Save the resolution
      HWND hwndRes = GetDlgItem (hwndDlg, IDC_RESOLUTION);
      settings.res_data = SendMessage (hwndRes, CB_GETCURSEL, 0, 0);
      HWND hwndWireframe = GetDlgItem (hwndDlg, IDC_WIREFRAME);
      settings.wireframe = (SendMessage (hwndWireframe, BM_GETCHECK, 0, 0)==BST_CHECKED)?1:0;
      HWND hwndWfmode = GetDlgItem (hwndDlg, IDC_WFMODE);
      settings.wfmode = SendMessage (hwndWfmode, CB_GETCURSEL, 0, 0);
      HWND hwndU = GetDlgItem (hwndDlg, IDC_UCODE);
      settings.ucode = SendMessage (hwndU, CB_GETCURSEL, 0, 0);
      
      // Actually set the new resolution numbers
      settings.scr_res_x = settings.res_x = resolutions[settings.res_data][0];
      settings.scr_res_y = settings.res_y = resolutions[settings.res_data][1];
      
      // Rendering
      // Filter modes
      settings.filtering = (BYTE)SendMessage (GetDlgItem (hwndDlg, IDC_FILTER),
        CB_GETCURSEL, 0, 0);
      // Buffer swapping modes
      settings.swapmode = (BYTE)SendMessage (GetDlgItem (hwndDlg, IDC_SWAP),
        CB_GETCURSEL, 0, 0);
    //LOD calculation modes 
      settings.lodmode = (BYTE)SendMessage (GetDlgItem (hwndDlg, IDC_LOD),
        CB_GETCURSEL, 0, 0);


     
#define GETBN(id,var) var=((SendMessage(GetDlgItem(hwndDlg,id),BM_GETCHECK,0,0)==BST_CHECKED)?1:0)
      
      GETBN (IDC_BUFFERCLEAR, settings.buff_clear);
      
      // Autodetect
      GETBN (IDC_AUTODETECT, settings.autodetect_ucode);
      
      // Glide card #
      if (SendMessage (GetDlgItem (hwndDlg, IDC_GID1),
        BM_GETCHECK, 0, 0)==BST_CHECKED)
        settings.card_id = 0;
      if (SendMessage (GetDlgItem (hwndDlg, IDC_GID2),
        BM_GETCHECK, 0, 0)==BST_CHECKED)
        settings.card_id = 1;
      if (SendMessage (GetDlgItem (hwndDlg, IDC_GID3),
        BM_GETCHECK, 0, 0)==BST_CHECKED)
        settings.card_id = 2;
      if (SendMessage (GetDlgItem (hwndDlg, IDC_GID4),
        BM_GETCHECK, 0, 0)==BST_CHECKED)
        settings.card_id = 3;
      
      GETBN (IDC_WRAP_BIG_TEX, settings.wrap_big_tex);
      GETBN (IDC_CORONA, settings.flame_corona);
      //GETBN (IDC_RE2_VIDEO, settings.RE2_native_video);
      
      GETBN (IDC_FOG, settings.fog);
      GETBN (IDC_VSYNC, settings.vsync);
      GETBN (IDC_CRC, settings.fast_crc);
      
      GETBN (IDC_FB_READ_ALWAYS, settings.fb_read_always);
      GETBN (IDC_FB_INFO, settings.fb_get_info);
      GETBN (IDC_FB_ALPHA, settings.fb_read_alpha);
      GETBN (IDC_FB_SMART, settings.fb_smart);
      GETBN (IDC_ENABLEMB, settings.fb_motionblur);
      GETBN (IDC_HIRESFB,  settings.fb_hires);
      GETBN (IDC_DB_CLEAR, settings.fb_depth_clear);
      GETBN (IDC_DB_RENDER, settings.fb_depth_render);

      GETBN (IDC_CUSTOM, settings.custom_ini);
      GETBN (IDC_HOTKEYS, settings.hotkeys);

      WriteSettings ();
      if (g_hwndDlg == hwndDlg)
      {
#ifdef USE_TOOLTIPS
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
#endif
        g_hwndDlg = NULL;
      }
      break;
    }
    return 0;

    case WM_INITDIALOG:
      {
        // Add the video modes to the list
        HWND hwndRes = GetDlgItem (hwndDlg, IDC_RESOLUTION);
        
#define ADDRES(s,x) SendMessage(hwndRes,CB_ADDSTRING,0,(LPARAM)s);
        
        // MUST be in this order (SST1VID.H)
        ADDRES ("320x200", 0x00);
        ADDRES ("320x240", 0x01);
        ADDRES ("400x256", 0x02);
        ADDRES ("512x384", 0x03);
        ADDRES ("640x200", 0x04);
        ADDRES ("640x350", 0x05);
        ADDRES ("640x400", 0x06);
        ADDRES ("640x480", 0x07);
        ADDRES ("800x600", 0x08);
        ADDRES ("960x720", 0x09);
        ADDRES ("856x480", 0x0A);
        ADDRES ("512x256", 0x0B);
        ADDRES ("1024x768", 0x0C);
        ADDRES ("1280x1024", 0x0D);
        ADDRES ("1600x1200", 0x0E);
        ADDRES ("400x300", 0x0F);
        ADDRES ("1152x864", 0x10);
        ADDRES ("1280x960", 0x11);
        ADDRES ("1600x1024", 0x12);
        ADDRES ("1792x1344", 0x13);
        ADDRES ("1856x1392", 0x14);
        ADDRES ("1920x1440", 0x15);
        ADDRES ("2048x1536", 0x16);
        ADDRES ("2048x2048", 0x17);
        SendMessage (hwndRes, CB_SETCURSEL, settings.res_data, 0);
        
        HWND hwndWireframe = GetDlgItem (hwndDlg, IDC_WIREFRAME);
        SendMessage (hwndWireframe, BM_SETCHECK, settings.wireframe?BST_CHECKED:BST_UNCHECKED, 0);
        HWND hwndWfmode = GetDlgItem (hwndDlg, IDC_WFMODE);
        SendMessage (hwndWfmode, CB_ADDSTRING, 0, (LPARAM)"Original colors");
        SendMessage (hwndWfmode, CB_ADDSTRING, 0, (LPARAM)"Vertex colors");
        SendMessage (hwndWfmode, CB_ADDSTRING, 0, (LPARAM)"Red only");
        SendMessage (hwndWfmode, CB_SETCURSEL, settings.wfmode, 0);
        
        HWND hwndU = GetDlgItem (hwndDlg, IDC_UCODE);
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"0: RSP SW 2.0X (ex. Mario)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"1: F3DEX 1.XX (ex. Star Fox)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"2: F3DEX 2.XX (ex. Zelda OOT)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"3: RSP SW 2.0D EXT (ex. Waverace)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"4: RSP SW 2.0D EXT (ex. Shadows of the Empire)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"5: RSP SW 2.0 (ex. Diddy Kong Racing)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"6: S2DEX 1.XX (ex. Yoshi's Story)");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"7: RSP SW PD Perfect Dark");
        SendMessage (hwndU, CB_ADDSTRING, 0, (LPARAM)"8: F3DEXBG 2.08 Conker's Bad Fur Day");
        SendMessage (hwndU, CB_SETCURSEL, settings.ucode, 0);
        
        // Rendering
        // Filter modes
        HWND hwndFilter = GetDlgItem (hwndDlg, IDC_FILTER);
        SendMessage (hwndFilter, CB_ADDSTRING, 0, (LPARAM)"Automatic");
        SendMessage (hwndFilter, CB_ADDSTRING, 0, (LPARAM)"Force Bilinear");
        SendMessage (hwndFilter, CB_ADDSTRING, 0, (LPARAM)"Force Point-sampled");
        SendMessage (hwndFilter, CB_SETCURSEL, settings.filtering, 0);
        
        // Buffer swapping modes
        HWND hwndSwap = GetDlgItem (hwndDlg, IDC_SWAP);
        SendMessage (hwndSwap, CB_ADDSTRING, 0, (LPARAM)"old");
        SendMessage (hwndSwap, CB_ADDSTRING, 0, (LPARAM)"new");
        SendMessage (hwndSwap, CB_ADDSTRING, 0, (LPARAM)"hybrid");
        SendMessage (hwndSwap, CB_SETCURSEL, settings.swapmode, 0);
        
        // LOD calculation modes modes
        HWND hwndLOD = GetDlgItem (hwndDlg, IDC_LOD);
        SendMessage (hwndLOD, CB_ADDSTRING, 0, (LPARAM)"off");
        SendMessage (hwndLOD, CB_ADDSTRING, 0, (LPARAM)"fast");
        SendMessage (hwndLOD, CB_ADDSTRING, 0, (LPARAM)"precise");
        SendMessage (hwndLOD, CB_SETCURSEL, settings.lodmode, 0);
        
#define CHECKBN(id,set) SendMessage(GetDlgItem(hwndDlg,id),BM_SETCHECK,(set)?BST_CHECKED:BST_UNCHECKED, 0);
        
        CHECKBN (IDC_BUFFERCLEAR, settings.buff_clear);
        
        // Autodetect
        CHECKBN (IDC_AUTODETECT, settings.autodetect_ucode);
        
        // Glide card #
        CHECKBN (IDC_GID1, settings.card_id==0);
        CHECKBN (IDC_GID2, settings.card_id==1);
        CHECKBN (IDC_GID3, settings.card_id==2);
        CHECKBN (IDC_GID4, settings.card_id==3);
        
        
        // Special fixes
        CHECKBN (IDC_WRAP_BIG_TEX, settings.wrap_big_tex);
        CHECKBN (IDC_CORONA, settings.flame_corona);
        //CHECKBN (IDC_RE2_VIDEO, settings.RE2_native_video);
        
        CHECKBN (IDC_FOG, settings.fog);
        CHECKBN (IDC_VSYNC, settings.vsync);
        CHECKBN (IDC_CRC, settings.fast_crc);
        
        //Frame buffer
        CHECKBN (IDC_FB_READ_ALWAYS, settings.fb_read_always);
        CHECKBN (IDC_FB_INFO, settings.fb_get_info);
        CHECKBN (IDC_FB_ALPHA, settings.fb_read_alpha);
        CHECKBN (IDC_FB_SMART, settings.fb_smart);
        CHECKBN (IDC_ENABLEMB, settings.fb_motionblur);
        CHECKBN (IDC_HIRESFB,  settings.fb_hires);
        CHECKBN (IDC_DB_CLEAR, settings.fb_depth_clear);
        CHECKBN (IDC_DB_RENDER, settings.fb_depth_render);

        CHECKBN (IDC_CUSTOM, settings.custom_ini);
        CHECKBN (IDC_HOTKEYS, settings.hotkeys);

        BOOL enable = !settings.custom_ini;
        EnableWindow(hwndFilter, enable);

        HWND hwndFog = GetDlgItem (hwndDlg, IDC_FOG);
        EnableWindow(hwndFog, enable);

        HWND hwndBuffClear = GetDlgItem (hwndDlg, IDC_BUFFERCLEAR);
        EnableWindow(hwndBuffClear, enable);

        EnableWindow(hwndSwap, enable);

        HWND hwndFBSmart = GetDlgItem (hwndDlg, IDC_FB_SMART);
        EnableWindow(hwndFBSmart, enable);

        HWND hwndAlpha = GetDlgItem (hwndDlg, IDC_FB_ALPHA);
        EnableWindow(hwndAlpha, enable);

        HWND hwndFBClear = GetDlgItem (hwndDlg, IDC_DB_CLEAR);
        EnableWindow(hwndFBClear, enable);
      }
      return 1;
      
    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED) 
      {
        if (LOWORD(wParam) == IDC_CUSTOM)
        {
          HWND hwndCustom = GetDlgItem (hwndDlg, IDC_CUSTOM);
          BOOL enable = (SendMessage(hwndCustom, BM_GETCHECK, 0, 0) != BST_CHECKED);

          HWND hwndFilter = GetDlgItem (hwndDlg, IDC_FILTER);
          EnableWindow(hwndFilter, enable);

          HWND hwndFog = GetDlgItem (hwndDlg, IDC_FOG);
          EnableWindow(hwndFog, enable);

          HWND hwndBuffClear = GetDlgItem (hwndDlg, IDC_BUFFERCLEAR);
          EnableWindow(hwndBuffClear, enable);

          HWND hwndSwap = GetDlgItem (hwndDlg, IDC_SWAP);
          EnableWindow(hwndSwap, enable);

          HWND hwndFBSmart = GetDlgItem (hwndDlg, IDC_FB_SMART);
          EnableWindow(hwndFBSmart, enable);

          HWND hwndAlpha = GetDlgItem (hwndDlg, IDC_FB_ALPHA);
          EnableWindow(hwndAlpha, enable);

          HWND hwndFBClear = GetDlgItem (hwndDlg, IDC_DB_CLEAR);
          EnableWindow(hwndFBClear, enable);
        }
        else if (LOWORD(wParam) == IDC_WRAPPER)
        {
          GRCONFIGWRAPPEREXT grConfigWrapperExt = (GRCONFIGWRAPPEREXT)grGetProcAddress("grConfigWrapperExt");
          if (grConfigWrapperExt)
              grConfigWrapperExt(hInstance, hwndDlg);
        }
        else if (LOWORD(wParam) == IDC_DB_RENDER)
        {
          if (SendMessage(GetDlgItem (hwndDlg, IDC_DB_RENDER), BM_GETCHECK, 0, 0) == BST_CHECKED)
          {
            settings.fb_depth_clear = TRUE;
            HWND hwndFBClear = GetDlgItem (hwndDlg, IDC_DB_CLEAR);
            SendMessage(hwndFBClear, BM_SETCHECK, BST_CHECKED, 0);
            EnableWindow(hwndFBClear, 0);
          }
          else
          {
            HWND hwndCustom = GetDlgItem (hwndDlg, IDC_CUSTOM);
            BOOL enable = (SendMessage(hwndCustom, BM_GETCHECK, 0, 0) != BST_CHECKED);
            HWND hwndFBClear = GetDlgItem (hwndDlg, IDC_DB_CLEAR);
            EnableWindow(hwndFBClear, enable);
          }
        }
      }
      return 0;
    }
    
    return 0;
}


BOOL CALLBACK DebugPageProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_NOTIFY:
    switch (((NMHDR FAR *) lParam)->code) {
#ifdef USE_TOOLTIPS
    case PSN_SETACTIVE: 
      if (!g_hhk)
      {
        g_hwndDlg = hwndDlg;
        g_hinst = hInstance;
        g_hwndTT = NULL;
        DoCreateDialogTooltip ();
      }
      break;
    case TTN_NEEDTEXT:
      OnWMNotify (lParam);
      break;
    case PSN_KILLACTIVE: 
    case PSN_QUERYCANCEL:
      //user clicked OK or selected another page
      if (g_hwndDlg == hwndDlg)
      {
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
        g_hwndDlg = NULL;
      }
      break;        
#endif
    case PSN_APPLY:
      // Debug/misc options
      GETBN (IDC_LOGGING, settings.logging);
      GETBN (IDC_LOGCLEAR, settings.log_clear);

      GETBN (IDC_CPU_HACK, settings.cpu_write_hack);

      GETBN (IDC_LOG_E, settings.elogging);
      GETBN (IDC_CACHEFILTER, settings.filter_cache);
      
      GETBN (IDC_UNKRED, settings.unk_as_red);
      GETBN (IDC_LOGUNK, settings.log_unk);
      GETBN (IDC_UNKCLEAR, settings.unk_clear);
      
      WriteSettings ();
      if (g_hwndDlg == hwndDlg)
      {
#ifdef USE_TOOLTIPS
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
#endif
        g_hwndDlg = NULL;
      }
      break;
    }
    return 0;
    
    case WM_INITDIALOG:
      {
        // Debug/misc options
        CHECKBN (IDC_LOGGING, settings.logging);
        CHECKBN (IDC_LOGCLEAR, settings.log_clear);
        CHECKBN (IDC_LOG_E, settings.elogging);
        CHECKBN (IDC_CPU_HACK, settings.cpu_write_hack);
        CHECKBN (IDC_CACHEFILTER, settings.filter_cache);
        CHECKBN (IDC_UNKRED, settings.unk_as_red);
        CHECKBN (IDC_LOGUNK, settings.log_unk);
        CHECKBN (IDC_UNKCLEAR, settings.unk_clear);
        
        
      }
      return 1;
      
    case WM_COMMAND:
      return 0;
  }
  
  return 0;
}


BOOL CALLBACK StatisticsPageProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_NOTIFY:
    switch (((NMHDR FAR *) lParam)->code) {
#ifdef USE_TOOLTIPS
    case PSN_SETACTIVE: 
      if (!g_hhk)
      {
        g_hwndDlg = hwndDlg;
        g_hinst = hInstance;
        g_hwndTT = NULL;
        DoCreateDialogTooltip ();
      }
      break;
    case TTN_NEEDTEXT:
      OnWMNotify (lParam);
      break;
    case PSN_KILLACTIVE: 
    case PSN_QUERYCANCEL:
      //user clicked OK or selected another page
      if (g_hwndDlg == hwndDlg)
      {
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
        g_hwndDlg = NULL;
      }
      break;        
#endif
    case PSN_APPLY:
      // speed counters
      settings.show_fps =
        ((SendMessage (GetDlgItem (hwndDlg, IDC_FPS),BM_GETCHECK, 0, 0)==BST_CHECKED)?1:0) |
        ((SendMessage (GetDlgItem (hwndDlg, IDC_FPS1),BM_GETCHECK, 0, 0)==BST_CHECKED)?2:0) |
        ((SendMessage (GetDlgItem (hwndDlg, IDC_FPS2),BM_GETCHECK, 0, 0)==BST_CHECKED)?4:0) |
        ((SendMessage (GetDlgItem (hwndDlg, IDC_FPS3),BM_GETCHECK, 0, 0)==BST_CHECKED)?8:0);
      
      GETBN (IDC_CLOCK, settings.clock);
      GETBN (IDC_CLOCK24, settings.clock_24_hr);
      
      WriteSettings ();
      if (g_hwndDlg == hwndDlg)
      {
#ifdef USE_TOOLTIPS
        if (g_hhk) UnhookWindowsHookEx (g_hhk);
        g_hhk = NULL;
#endif
        g_hwndDlg = NULL;
      }
      break;
    }
    return 0;
    
    case WM_INITDIALOG:
      {
        // speed counters
        CHECKBN (IDC_FPS, settings.show_fps&1);
        CHECKBN (IDC_FPS1, settings.show_fps&2);
        CHECKBN (IDC_FPS2, settings.show_fps&4);
        CHECKBN (IDC_FPS3, settings.show_fps&8);
        
        CHECKBN (IDC_CLOCK, settings.clock);
        CHECKBN (IDC_CLOCK24, settings.clock_24_hr);
      }
      return 1;
      
    case WM_COMMAND:
      return 0;
  }
  
  return 0;
}

#else // _WIN32

#include <string.h>
#include <gtk/gtk.h>

typedef struct
{
   GtkWidget *dialog;
   GtkWidget *autodetectCheckButton;
   GtkWidget *forceCombo;
   GtkWidget *windowResCombo;
   GtkWidget *fullResCombo;
   GtkWidget *texFilterCombo;
   GtkWidget *filterCombo;
   GtkWidget *lodCombo;
   GtkWidget *fogCheckButton;
   GtkWidget *bufferClearCheckButton;
   GtkWidget *vSyncCheckButton;
   GtkWidget *fastcrcCheckButton;
   GtkWidget *noDitheredAlphaCheckButton;
   GtkWidget *noGLSLCheckButton;
   GtkWidget *swapCombo;
   GtkWidget *customIniCheckButton;
   GtkWidget *wrapCheckButton;
   GtkWidget *coronaCheckButton;
   GtkWidget *readAllCheckButton;
   GtkWidget *CPUWriteHackCheckButton;
   GtkWidget *FBGetInfoCheckButton;
   GtkWidget *DepthRenderCheckButton;
   GtkWidget *FPSCheckButton;
   GtkWidget *VICheckButton;
   GtkWidget *ratioCheckButton;
   GtkWidget *FPStransCheckButton;
   GtkWidget *clockCheckButton;
   GtkWidget *clock24CheckButton;
   GtkWidget *hiresFbCheckButton;
   GtkWidget *hiresFBOCheckButton;
   GList *windowResComboList;
   GList *texFilterComboList;
   GList *forceComboList;
   GList *filterComboList;
   GList *lodComboList;
   GList *swapComboList;
} ConfigDialog;

static void customIniCheckButtonCallback(GtkWidget *widget, void *data)
{
   ConfigDialog *configDialog = (ConfigDialog*)data;
   
   BOOL enable = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->customIniCheckButton));
   gtk_widget_set_sensitive(configDialog->filterCombo, enable);
   gtk_widget_set_sensitive(configDialog->lodCombo, enable);
   gtk_widget_set_sensitive(configDialog->fogCheckButton, enable);
   gtk_widget_set_sensitive(configDialog->bufferClearCheckButton, enable);
   gtk_widget_set_sensitive(configDialog->swapCombo, enable);
}

static void okButtonCallback(GtkWidget *widget, void *data)
{
   ConfigDialog *configDialog = (ConfigDialog*)data;
   char *s;
   unsigned int i;
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->windowResCombo)->entry));
   for (i=0; i<g_list_length(configDialog->windowResComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->windowResComboList, i)))
       settings.res_data = i;
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->fullResCombo)->entry));
   for (i=0; i<g_list_length(configDialog->windowResComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->windowResComboList, i)))
       settings.full_res = i;
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->texFilterCombo)->entry));
   for (i=0; i<g_list_length(configDialog->texFilterComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->texFilterComboList, i)))
       settings.tex_filter = i;
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->forceCombo)->entry));
   for (i=0; i<g_list_length(configDialog->forceComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->forceComboList, i)))
       settings.ucode = i;
   
   settings.scr_res_x = settings.res_x = resolutions[settings.res_data][0];
   settings.scr_res_y = settings.res_y = resolutions[settings.res_data][1];
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->filterCombo)->entry));
   for (i=0; i<g_list_length(configDialog->filterComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->filterComboList, i)))
       settings.filtering = i;

   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->lodCombo)->entry));
   for (i=0; i<g_list_length(configDialog->lodComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->lodComboList, i)))
       settings.lodmode = i;
   
   s = (char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(configDialog->swapCombo)->entry));
   for (i=0; i<g_list_length(configDialog->swapComboList); i++)
     if(!strcmp(s, (char*)g_list_nth_data(configDialog->swapComboList, i)))
       settings.swapmode = i;
   
   settings.fog = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->fogCheckButton));
   settings.buff_clear = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->bufferClearCheckButton));
   settings.autodetect_ucode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->autodetectCheckButton));
   settings.wrap_big_tex = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->wrapCheckButton));
   settings.flame_corona = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->coronaCheckButton));
   settings.vsync = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->vSyncCheckButton));
   settings.fast_crc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->fastcrcCheckButton));
   settings.noditheredalpha = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->noDitheredAlphaCheckButton));
   settings.noglsl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->noGLSLCheckButton));
   settings.fb_read_always = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->readAllCheckButton));
   settings.cpu_write_hack = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->CPUWriteHackCheckButton));
   settings.fb_get_info = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->FBGetInfoCheckButton));
   settings.fb_depth_render = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->DepthRenderCheckButton));
   settings.custom_ini = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->customIniCheckButton));
   settings.fb_hires = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->hiresFbCheckButton));
   settings.FBO = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->hiresFBOCheckButton));
   
   settings.show_fps =
     (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->FPSCheckButton))?1:0) |
     (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->VICheckButton))?2:0) |
     (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->ratioCheckButton))?4:0) |
     (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->FPStransCheckButton))?8:0);
   
   settings.clock = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->clockCheckButton));
   settings.clock_24_hr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(configDialog->clock24CheckButton));
   
   WriteSettings();
   
   // re-init evoodoo graphics to resize window
   if (evoodoo && fullscreen && !ev_fullscreen) {
      ReleaseGfx ();
      InitGfx (TRUE);
   }
   
   gtk_widget_hide(configDialog->dialog);
}

static ConfigDialog *CreateConfigDialog()
{
   // objects dialog
   // dialog
   GtkWidget *dialog;
   dialog = gtk_dialog_new();
   gtk_window_set_title(GTK_WINDOW(dialog), "Glide64 Configuration");
   
   // ok button
   GtkWidget *okButton;
   okButton = gtk_button_new_with_label("OK");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), okButton);
   
   // cancel button
   GtkWidget *cancelButton;
   cancelButton = gtk_button_new_with_label("Cancel");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), cancelButton);
   
   // Autodetect ucode CheckButton
   GtkWidget *autodetectCheckButton;
   autodetectCheckButton = gtk_check_button_new_with_label("Autodetect Microcode");
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), autodetectCheckButton);
   
   // Force Microcode Container
   GtkWidget *forceContainer;
   forceContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), forceContainer);
   
   // Force Microcode Label
   GtkWidget *forceLabel;
   forceLabel = gtk_label_new("Force Microcode:");
   gtk_container_add(GTK_CONTAINER(forceContainer), forceLabel);
   
   // Force Microcode Combo
   GtkWidget *forceCombo;
   forceCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(forceCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(forceContainer), forceCombo);
   
   // horizontal container
   GtkWidget *hContainer;
   hContainer = gtk_hbox_new(0, 0);
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hContainer);
   
   // vertical container
   GtkWidget *vContainer;
   vContainer = gtk_vbox_new(0, 0);
   gtk_container_add(GTK_CONTAINER(hContainer), vContainer);
   
   // Rendering Frame
   GtkWidget *renderingFrame;
   renderingFrame = gtk_frame_new("Rendering");
   gtk_container_add(GTK_CONTAINER(vContainer), renderingFrame);

   // Rendering Container
   GtkWidget *renderingContainer;
   renderingContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingFrame), renderingContainer);
   
   // Window Mode Resolution Container
   GtkWidget *windowResContainer;
   windowResContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), windowResContainer);
   
   // Window Mode Resolution Label
   GtkWidget *windowResLabel;
   windowResLabel = gtk_label_new("Window Resolution:");
   gtk_container_add(GTK_CONTAINER(windowResContainer), windowResLabel);
   
   // Window Mode Combo
   GtkWidget *windowResCombo;
   windowResCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(windowResCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(windowResContainer), windowResCombo);
   
   // FullScreen Mode Resolution Container
   GtkWidget *fullResContainer;
   fullResContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), fullResContainer);
   
   // FullScreen Mode Resolution Label
   GtkWidget *fullResLabel;
   fullResLabel = gtk_label_new("Fullscreen Resolution:");
   gtk_container_add(GTK_CONTAINER(fullResContainer), fullResLabel);
   
   // FullScreen Mode Combo
   GtkWidget *fullResCombo;
   fullResCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(fullResCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(fullResContainer), fullResCombo);
   
   // Texture Filter Container
   GtkWidget *texFilterContainer;
   texFilterContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), texFilterContainer);
   
   // Texture Filter Label
   GtkWidget *texFilterLabel;
   texFilterLabel = gtk_label_new("Texture Filter:");
   gtk_container_add(GTK_CONTAINER(texFilterContainer), texFilterLabel);
   
   // Texture Filter Combo
   GtkWidget *texFilterCombo;
   texFilterCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(texFilterCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(texFilterContainer), texFilterCombo);
   
   // Filter Container
   GtkWidget *filterContainer;
   filterContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), filterContainer);
   
   // Filter Label
   GtkWidget *filterLabel;
   filterLabel = gtk_label_new("Filtering mode:");
   gtk_container_add(GTK_CONTAINER(filterContainer), filterLabel);
   
   // Filter Combo
   GtkWidget *filterCombo;
   filterCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(filterCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(filterContainer), filterCombo);
   
   // LOD Container
   GtkWidget *lodContainer;
   lodContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), lodContainer);
   
   // LOD Label
   GtkWidget *lodLabel;
   lodLabel = gtk_label_new("LOD calculation:");
   gtk_container_add(GTK_CONTAINER(lodContainer), lodLabel);
   
   // LOD Combo
   GtkWidget *lodCombo;
   lodCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(lodCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(lodContainer), lodCombo);
   
   // Fog CheckButton
   GtkWidget *fogCheckButton;
   fogCheckButton = gtk_check_button_new_with_label("Fog enabled");
   gtk_container_add(GTK_CONTAINER(renderingContainer), fogCheckButton);
   
   // Buffer Clear CheckButton
   GtkWidget *bufferClearCheckButton;
   bufferClearCheckButton = gtk_check_button_new_with_label("Buffer clear on every frame");
   gtk_container_add(GTK_CONTAINER(renderingContainer), bufferClearCheckButton);
   
   // Vertical Sync CheckButton
   GtkWidget *vSyncCheckButton;
   vSyncCheckButton = gtk_check_button_new_with_label("Vertical Sync");
   gtk_container_add(GTK_CONTAINER(renderingContainer), vSyncCheckButton);
   
   // Fast CRC CheckButton
   GtkWidget *fastcrcCheckButton;
   fastcrcCheckButton = gtk_check_button_new_with_label("Fast CRC");
   gtk_container_add(GTK_CONTAINER(renderingContainer), fastcrcCheckButton);
   
   // hires framebuffer CheckButton
   GtkWidget *hiresFbCheckButton;
   hiresFbCheckButton = gtk_check_button_new_with_label("Hires Framebuffer");
   gtk_container_add(GTK_CONTAINER(renderingContainer), hiresFbCheckButton);
   
   // Swap Container
   GtkWidget *swapContainer;
   swapContainer = gtk_hbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(renderingContainer), swapContainer);
   
   // Swap Combo
   GtkWidget *swapCombo;
   swapCombo = gtk_combo_new();
   gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(swapCombo)->entry), FALSE);
   gtk_container_add(GTK_CONTAINER(swapContainer), swapCombo);
   
   // Swap Label
   GtkWidget *swapLabel;
   swapLabel = gtk_label_new("Buffer swapping method");
   gtk_container_add(GTK_CONTAINER(swapContainer), swapLabel);
   
   // Rendering Frame
   GtkWidget *wrapperFrame;
   wrapperFrame = gtk_frame_new("Glide wrapper");
   gtk_container_add(GTK_CONTAINER(vContainer), wrapperFrame);

   // Wrapper Container
   GtkWidget *wrapperContainer;
   wrapperContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(wrapperFrame), wrapperContainer);
   
   // no dithered alpha CheckButton
   GtkWidget *noDitheredAlphaCheckButton;
   noDitheredAlphaCheckButton = gtk_check_button_new_with_label("Disable Dithered Alpha");
   gtk_container_add(GTK_CONTAINER(wrapperContainer), noDitheredAlphaCheckButton);

   // no glsl CheckButton
   GtkWidget *noGLSLCheckButton;
   noGLSLCheckButton = gtk_check_button_new_with_label("Disable GLSL Combiners");
   gtk_container_add(GTK_CONTAINER(wrapperContainer), noGLSLCheckButton);
   
   // FBO CheckButton
   GtkWidget *hiresFBOCheckButton;
   hiresFBOCheckButton = gtk_check_button_new_with_label("Use Framebuffer Objects");
   gtk_container_add(GTK_CONTAINER(wrapperContainer), hiresFBOCheckButton);
   
   // Other Frame
   GtkWidget *otherFrame;
   otherFrame = gtk_frame_new("Other");
   gtk_container_add(GTK_CONTAINER(vContainer), otherFrame);
   
   // Custom ini CheckButton
   GtkWidget *customIniCheckButton;
   customIniCheckButton = gtk_check_button_new_with_label("Custom ini settings");
   gtk_container_add(GTK_CONTAINER(otherFrame), customIniCheckButton);
   
   // vertical container
   vContainer = gtk_vbox_new(0, 0);
   gtk_container_add(GTK_CONTAINER(hContainer), vContainer);
   
   // Special Fixes Frame
   GtkWidget *specialFrame;
   specialFrame = gtk_frame_new("Special Fixes");
   gtk_container_add(GTK_CONTAINER(vContainer), specialFrame);
   
   // Special Fixes Container
   GtkWidget *specialContainer;
   specialContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(specialFrame), specialContainer);
   
   // Wrap CheckButton
   GtkWidget *wrapCheckButton;
   wrapCheckButton = gtk_check_button_new_with_label("Wrap textures too big for tmem");
   gtk_container_add(GTK_CONTAINER(specialContainer), wrapCheckButton);
   
   // Corona CheckButton
   GtkWidget *coronaCheckButton;
   coronaCheckButton = gtk_check_button_new_with_label("Zelda. Corona fix");
   gtk_container_add(GTK_CONTAINER(specialContainer), coronaCheckButton);
   
   // Frame Buffer Frame
   GtkWidget *framebufferFrame;
   framebufferFrame = gtk_frame_new("Frame buffer emulation options");
   gtk_container_add(GTK_CONTAINER(vContainer), framebufferFrame);
   
   // Frame Buffer Container
   GtkWidget *framebufferContainer;
   framebufferContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(framebufferFrame), framebufferContainer);
   
   // Read All CheckButton
   GtkWidget *readAllCheckButton;
   readAllCheckButton = gtk_check_button_new_with_label("Read every frame (slow!)");
   gtk_container_add(GTK_CONTAINER(framebufferContainer), readAllCheckButton);
   
   // CPU Write Hack CheckButton
   GtkWidget *CPUWriteHackCheckButton;
   CPUWriteHackCheckButton = gtk_check_button_new_with_label("Detect CPU writes");
   gtk_container_add(GTK_CONTAINER(framebufferContainer), CPUWriteHackCheckButton);
   
   // FB Get Info CheckButton
   GtkWidget *FBGetInfoCheckButton;
   FBGetInfoCheckButton = gtk_check_button_new_with_label("Get frame buffer info");
   gtk_container_add(GTK_CONTAINER(framebufferContainer), FBGetInfoCheckButton);
   
   // Depth Render CheckButton
   GtkWidget *DepthRenderCheckButton;
   DepthRenderCheckButton = gtk_check_button_new_with_label("Depth buffer render");
   gtk_container_add(GTK_CONTAINER(framebufferContainer), DepthRenderCheckButton);
   
   // Speed Frame
   GtkWidget *speedFrame;
   speedFrame = gtk_frame_new("Speed");
   gtk_container_add(GTK_CONTAINER(vContainer), speedFrame);
   
   // Speed Container
   GtkWidget *speedContainer;
   speedContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(speedFrame), speedContainer);
   
   // FPS CheckButton
   GtkWidget *FPSCheckButton;
   FPSCheckButton = gtk_check_button_new_with_label("FPS counter");
   gtk_container_add(GTK_CONTAINER(speedContainer), FPSCheckButton);
   
   // VI CheckButton
   GtkWidget *VICheckButton;
   VICheckButton = gtk_check_button_new_with_label("VI/s counter");
   gtk_container_add(GTK_CONTAINER(speedContainer), VICheckButton);
   
   // ratio CheckButton
   GtkWidget *ratioCheckButton;
   ratioCheckButton = gtk_check_button_new_with_label("% speed");
   gtk_container_add(GTK_CONTAINER(speedContainer), ratioCheckButton);
   
   // FPS trans CheckButton
   GtkWidget *FPStransCheckButton;
   FPStransCheckButton = gtk_check_button_new_with_label("FPS transparent");
   gtk_container_add(GTK_CONTAINER(speedContainer), FPStransCheckButton);
   
   // Time Frame
   GtkWidget *timeFrame;
   timeFrame = gtk_frame_new("Time");
   gtk_container_add(GTK_CONTAINER(vContainer), timeFrame);
   
   // Time Container
   GtkWidget *timeContainer;
   timeContainer = gtk_vbox_new(TRUE, 0);
   gtk_container_add(GTK_CONTAINER(timeFrame), timeContainer);
   
   // clock CheckButton
   GtkWidget *clockCheckButton;
   clockCheckButton = gtk_check_button_new_with_label("Clock enabled");
   gtk_container_add(GTK_CONTAINER(timeContainer), clockCheckButton);
   
   // clock24 CheckButton
   GtkWidget *clock24CheckButton;
   clock24CheckButton = gtk_check_button_new_with_label("Clock is 24-hour");
   gtk_container_add(GTK_CONTAINER(timeContainer), clock24CheckButton);
   
   // Filling lists
   // windowResCombo list
   GList *windowResComboList = NULL;
   windowResComboList = g_list_append(windowResComboList, (void*)"320x200");
   windowResComboList = g_list_append(windowResComboList, (void*)"320x240");
   windowResComboList = g_list_append(windowResComboList, (void*)"400x256");
   windowResComboList = g_list_append(windowResComboList, (void*)"512x384");
   windowResComboList = g_list_append(windowResComboList, (void*)"640x200");
   windowResComboList = g_list_append(windowResComboList, (void*)"640x350");
   windowResComboList = g_list_append(windowResComboList, (void*)"640x400");
   windowResComboList = g_list_append(windowResComboList, (void*)"640x480");
   windowResComboList = g_list_append(windowResComboList, (void*)"800x600");
   windowResComboList = g_list_append(windowResComboList, (void*)"960x720");
   windowResComboList = g_list_append(windowResComboList, (void*)"856x480");
   windowResComboList = g_list_append(windowResComboList, (void*)"512x256");
   windowResComboList = g_list_append(windowResComboList, (void*)"1024x768");
   windowResComboList = g_list_append(windowResComboList, (void*)"1280x1024");
   windowResComboList = g_list_append(windowResComboList, (void*)"1600x1200");
   windowResComboList = g_list_append(windowResComboList, (void*)"400x300");
   windowResComboList = g_list_append(windowResComboList, (void*)"1152x864");
   windowResComboList = g_list_append(windowResComboList, (void*)"1280x960");
   windowResComboList = g_list_append(windowResComboList, (void*)"1600x1024");
   windowResComboList = g_list_append(windowResComboList, (void*)"1792x1344");
   windowResComboList = g_list_append(windowResComboList, (void*)"1856x1392");
   windowResComboList = g_list_append(windowResComboList, (void*)"1920x1440");
   windowResComboList = g_list_append(windowResComboList, (void*)"2048x1536");
   windowResComboList = g_list_append(windowResComboList, (void*)"2048x2048");
   gtk_combo_set_popdown_strings(GTK_COMBO(windowResCombo),
                 windowResComboList);
   
   // fullResCombo list
   gtk_combo_set_popdown_strings(GTK_COMBO(fullResCombo), windowResComboList);
   
   // texFilterCombo list
   GList *texFilterComboList = NULL;
   texFilterComboList = g_list_append(texFilterComboList, (void*)"None");
   texFilterComboList = g_list_append(texFilterComboList, (void*)"Blur edges");
   texFilterComboList = g_list_append(texFilterComboList, (void*)"Super 2xSai");
   texFilterComboList = g_list_append(texFilterComboList, (void*)"Hq2x");
   texFilterComboList = g_list_append(texFilterComboList, (void*)"Hq4x");
   gtk_combo_set_popdown_strings(GTK_COMBO(texFilterCombo), 
                 texFilterComboList);
   
   // forceCombo list
   GList *forceComboList = NULL;
   forceComboList = g_list_append(forceComboList, (void*)"0: RSP SW 2.0X (ex. Mario)");
   forceComboList = g_list_append(forceComboList, (void*)"1: F3DEX 1.XX (ex. Star Fox)");
   forceComboList = g_list_append(forceComboList, (void*)"2: F3DEX 2.XX (ex. Zelda OOT)");
   forceComboList = g_list_append(forceComboList, (void*)"3: RSP SW 2.0D EXT (ex. Waverace)");
   forceComboList = g_list_append(forceComboList, (void*)"4: RSP SW 2.0D EXT (ex. Shadows of the Empire)");
   forceComboList = g_list_append(forceComboList, (void*)"5: RSP SW 2.0 (ex. Diddy Kong Racing)");
   forceComboList = g_list_append(forceComboList, (void*)"6: S2DEX 1.XX (ex. Yoshi's Story)");
   forceComboList = g_list_append(forceComboList, (void*)"7: RSP SW PD Perfect Dark");
   forceComboList = g_list_append(forceComboList, (void*)"8: F3DEXBG 2.08 Conker's Bad Fur Day");
   gtk_combo_set_popdown_strings(GTK_COMBO(forceCombo), forceComboList);
   
   // filterCombo list
   GList *filterComboList = NULL;
   filterComboList = g_list_append(filterComboList, (void*)"Automatic");
   filterComboList = g_list_append(filterComboList, (void*)"Force Bilinear");
   filterComboList = g_list_append(filterComboList, (void*)"Force Point-sampled");
   gtk_combo_set_popdown_strings(GTK_COMBO(filterCombo), filterComboList);
   
   // lodCombo list
   GList *lodComboList = NULL;
   lodComboList = g_list_append(lodComboList, (void*)"Off");
   lodComboList = g_list_append(lodComboList, (void*)"Fast");
   lodComboList = g_list_append(lodComboList, (void*)"Precise");
   gtk_combo_set_popdown_strings(GTK_COMBO(lodCombo), lodComboList);
   
   // swapCombo list
   GList *swapComboList = NULL;
   swapComboList = g_list_append(swapComboList, (void*)"old");
   swapComboList = g_list_append(swapComboList, (void*)"new");
   swapComboList = g_list_append(swapComboList, (void*)"hybrid");
   gtk_combo_set_popdown_strings(GTK_COMBO(swapCombo), swapComboList);
   
   // ConfigDialog structure creation
   ConfigDialog *configDialog = new ConfigDialog;
   
   // signal callbacks
   gtk_signal_connect_object(GTK_OBJECT(dialog), "delete-event",
                 GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete),
                 GTK_OBJECT(dialog));
   gtk_signal_connect(GTK_OBJECT(customIniCheckButton), "clicked",
              GTK_SIGNAL_FUNC(customIniCheckButtonCallback),
              (void*)configDialog);
   gtk_signal_connect(GTK_OBJECT(okButton), "clicked",
              GTK_SIGNAL_FUNC(okButtonCallback),
              (void*)configDialog);
   gtk_signal_connect_object(GTK_OBJECT(cancelButton), "clicked",
                 GTK_SIGNAL_FUNC(gtk_widget_hide),
                 GTK_OBJECT(dialog));
              
   // Outputing ConfigDialog structure
   configDialog->dialog = dialog;
   configDialog->autodetectCheckButton = autodetectCheckButton;
   configDialog->forceCombo = forceCombo;
   configDialog->windowResCombo = windowResCombo;
   configDialog->fullResCombo = fullResCombo;
   configDialog->texFilterCombo = texFilterCombo;
   configDialog->filterCombo = filterCombo;
   configDialog->lodCombo = lodCombo;
   configDialog->fogCheckButton = fogCheckButton;
   configDialog->bufferClearCheckButton = bufferClearCheckButton;
   configDialog->vSyncCheckButton = vSyncCheckButton;
   configDialog->fastcrcCheckButton = fastcrcCheckButton;
   configDialog->noDitheredAlphaCheckButton = noDitheredAlphaCheckButton;
   configDialog->noGLSLCheckButton = noGLSLCheckButton;
   configDialog->hiresFbCheckButton = hiresFbCheckButton;
   configDialog->hiresFBOCheckButton = hiresFBOCheckButton;
   configDialog->swapCombo = swapCombo;
   configDialog->customIniCheckButton = customIniCheckButton;
   configDialog->wrapCheckButton = wrapCheckButton;
   configDialog->coronaCheckButton = coronaCheckButton;
   configDialog->readAllCheckButton = readAllCheckButton;
   configDialog->CPUWriteHackCheckButton = CPUWriteHackCheckButton;
   configDialog->FBGetInfoCheckButton = FBGetInfoCheckButton;
   configDialog->DepthRenderCheckButton = DepthRenderCheckButton;
   configDialog->FPSCheckButton = FPSCheckButton;
   configDialog->VICheckButton = VICheckButton;
   configDialog->ratioCheckButton = ratioCheckButton;
   configDialog->FPStransCheckButton = FPStransCheckButton;
   configDialog->clockCheckButton = clockCheckButton;
   configDialog->clock24CheckButton = clock24CheckButton;
   configDialog->windowResComboList = windowResComboList;
   configDialog->texFilterComboList = texFilterComboList;
   configDialog->forceComboList = forceComboList;
   configDialog->filterComboList = filterComboList;
   configDialog->lodComboList = lodComboList;
   configDialog->swapComboList = swapComboList;
   return configDialog;
}

void CALL DllConfig ( HWND hParent )
{
   static ConfigDialog *configDialog = NULL;
   if (configDialog == NULL) configDialog = CreateConfigDialog();
   
   ReadSettings ();
   
   char name[21] = "DEFAULT";
   ReadSpecialSettings (name);
   
   if (gfx.HEADER)
     {
    // get the name of the ROM
    for (int i=0; i<20; i++)
      name[i] = gfx.HEADER[(32+i)^3];
    name[20] = 0;
   
    // remove all trailing spaces
    while (name[strlen(name)-1] == ' ')
      name[strlen(name)-1] = 0;
   
    ReadSpecialSettings (name);
     }
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->windowResCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->windowResComboList,
                           settings.res_data));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->fullResCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->windowResComboList,
                          settings.full_res));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->texFilterCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->texFilterComboList,
                          settings.tex_filter));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->forceCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->forceComboList,
                          settings.ucode));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->filterCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->filterComboList,
                          settings.filtering));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->lodCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->lodComboList,
                          settings.lodmode));
   
   gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(configDialog->swapCombo)->entry),
              (gchar*)g_list_nth_data(configDialog->swapComboList,
                          settings.swapmode));
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->fogCheckButton),
                settings.fog);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->bufferClearCheckButton),
                settings.buff_clear);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->autodetectCheckButton),
                settings.autodetect_ucode);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->wrapCheckButton),
                settings.wrap_big_tex);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->coronaCheckButton),
                settings.flame_corona);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->vSyncCheckButton),
                settings.vsync);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->fastcrcCheckButton),
                settings.fast_crc);

   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->noDitheredAlphaCheckButton),
                settings.noditheredalpha);

   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->noGLSLCheckButton),
                settings.noglsl);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->hiresFbCheckButton),
                settings.fb_hires);

   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->hiresFBOCheckButton),
                settings.FBO);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->readAllCheckButton),
                settings.fb_read_always);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->CPUWriteHackCheckButton),
                settings.cpu_write_hack);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->FBGetInfoCheckButton),
                settings.fb_get_info);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->DepthRenderCheckButton),
                settings.fb_depth_render);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->customIniCheckButton),
                settings.custom_ini);
   
   BOOL enable = !settings.custom_ini;
   gtk_widget_set_sensitive(configDialog->filterCombo, enable);
   gtk_widget_set_sensitive(configDialog->lodCombo, enable);
   gtk_widget_set_sensitive(configDialog->fogCheckButton, enable);
   gtk_widget_set_sensitive(configDialog->bufferClearCheckButton, enable);
   gtk_widget_set_sensitive(configDialog->swapCombo, enable);
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->FPSCheckButton),
                settings.show_fps&1);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->VICheckButton),
                settings.show_fps&2);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->ratioCheckButton),
                settings.show_fps&4);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->FPStransCheckButton),
                settings.show_fps&8);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->clockCheckButton),
                settings.clock);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(configDialog->clock24CheckButton),
                settings.clock_24_hr);
   
   gtk_widget_show_all(configDialog->dialog);
}

#endif // _WIN32
