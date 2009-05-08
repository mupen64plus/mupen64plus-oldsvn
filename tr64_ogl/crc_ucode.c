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

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "resource.h"
#include "crc_ucode.h"

extern unsigned char    *pROM;

char video_ini[1024];
int ucode;
int tucode;
BOOL tFrameBuffer;
BOOL tFogEnabled;
BOOL tDumpTexture = FALSE;
BOOL tVRMLOutput = FALSE;

BOOL HandsetUcode;
HWND hConfig;
extern HWND g_hWnd;
extern HINSTANCE g_hInstance;
extern char szAppName[256];

BOOL FrameBuffer = FALSE;
BOOL FogEnabled = FALSE;
BOOL DumpTexture = FALSE;
BOOL VRMLOutput = FALSE;
BOOL realVRMLOutput = FALSE;
BOOL USE_MM_HACK = FALSE;

LRESULT CALLBACK ConfigProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
void CreateConfigDlg();

typedef struct
{
    _u32 CRC;
    _u16 ucode;
} t_CRC_record;

t_CRC_record CRC_Table[] = // uCodeCRC-6333FBF9=1
{
    {0xFEB18051,1}, // MarioKart 64
    {0xF6881807,0}, // Golden Eye
    {0xDAA0DF73,4}, // Zelda Majora's Mask
    {0xCF35DF5D,4}, // Pokemon Stadium 2
    {0xBD24F5CE,4}, // Super Smash Brothers
    {0xBA53B4A5,5}, // Perfect Dark
    {0xADC05B94,4}, // Armymen Sarges Heroes 2
    {0xA773522B,4}, // Mario Golf 64
    {0x8BA4A545,6}, // Jet Force Gemini
    {0x86CB2DF5,0}, // Killer Instinct Gold
    {0x785CA593,4}, // Harvest Moon 64
    {0x6BBA9076,4}, // Banjo Tooie
    {0x680A7F5E,4}, // Zelda OoT
    {0x6333FBF9,1}, // Yoshi's Story
    {0x548db096,7}, // Conker's Bd Fur Day
    {0x490D67E5,3}, // Star Fox 64
    {0x4281C96A,1}, // Banjo Kazooie
    {0x3D4EBB4C,6}, // Diddy Kong Racing
    {0x3C252716,0}, // Super Mario 64
    {0x2CBC00E3,1}  // 1080 Snowboarding
};


void GetPluginDir( char * Directory ) 
{
    char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
    char fname[_MAX_FNAME],ext[_MAX_EXT];
//  GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
    GetModuleFileName(g_hInstance,path_buffer,sizeof(path_buffer));
    _splitpath( path_buffer, drive, dir, fname, ext );
    strcpy(Directory,drive);
    strcat(Directory,dir);
//  strcat(Directory,"Plugin\\");
}

void GetEmuDir( char * Directory ) 
{
    char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
    char fname[_MAX_FNAME],ext[_MAX_EXT];
    GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
    _splitpath( path_buffer, drive, dir, fname, ext );
    strcpy(Directory,drive);
    strcat(Directory,dir);
}

char CRC_Entry[1024];
char ucCRC_Entry[1024];

void ClearUCode()
{
    ucode = -1;
//  ucode_version = -1;
}

extern unsigned char    *pRDRAM;
extern unsigned char    *pIMEM;
extern unsigned char    *pDMEM;
extern unsigned char    *pVIREG;
extern unsigned char    *pROM;
extern _u32             *pVIORG;
extern _u32             *pVISTS;

_u32 Calc_uCode_CRC()
{
    _u32 ucstart = *((unsigned long*)&pDMEM[0xFD0]);
    int i;
    _u32 uCodeCRC = 0;
    _u16 *puCode = (_u16*)&pRDRAM[(ucstart) & 0x7FFFFF];

    for (i=0; i<3072; i+=2)
    {
        uCodeCRC = (uCodeCRC >> (11 + (i & 0x0e))) | (uCodeCRC << (21 - (i & 0x0e))) ;
        uCodeCRC ^= (*puCode << (7 + (i & 0x0e)));
        uCodeCRC ^= (uCodeCRC >> (27 - (i & 0x0e)));
        uCodeCRC = (uCodeCRC >> (11 + (i & 0x0e))) | (uCodeCRC << (21 - (i & 0x0e))) ;
        uCodeCRC += (*puCode << (1 + (i & 0x0e)));
        puCode++;
    }
    
    return uCodeCRC;
}

int GetUCode()
{
    t_romheader header;
    char *p = (char*)&header;
    int i;
    _u32 ucCRC;
    int CRCucode = -1;
/*
extern char plgname[];
extern int vermjr;
extern int vermnr;
extern char versfx[];

    sprintf(szAppName,"%s v%i.%i%s",plgname,vermjr,vermnr,versfx);
*/
    if (ucode != -1) 
        return ucode;

    for (i = 0; i<sizeof(t_romheader); i++)
    {
        p[i] = pROM[i^3];
    }

    ucCRC = Calc_uCode_CRC();

    USE_MM_HACK = FALSE;

    if (ucCRC == 0xdaa0df73)
    {
        USE_MM_HACK = TRUE;
    }
// Generate String of video.ini
    GetPluginDir(video_ini);
    strcat(video_ini, "\\tr64_ogl.ini");
    WritePrivateProfileStringW( NULL, NULL, NULL, L"appname.ini" ); 

    sprintf(CRC_Entry,"%08X%08X-%02X", header.crc1, header.crc2, header.countrycode);
    sprintf(ucCRC_Entry,"uCodeCRC-%08X", ucCRC);

// Load Number of UCodeTable-entries
    ucode = GetPrivateProfileInt(CRC_Entry, "UCode", -1, video_ini);
    CRCucode = GetPrivateProfileInt("uCodeCRC", ucCRC_Entry, -1, video_ini);
    FrameBuffer  = (GetPrivateProfileInt(CRC_Entry, "FrameBuffer", 0, video_ini) != 0);
    FogEnabled  = (GetPrivateProfileInt(CRC_Entry, "FogEmulation", 0, video_ini) != 0);
//  DumpTexture = FALSE;
//  VRMLOutput = FALSE;

    if (CRCucode == -1)
    {
        int idx; // t_CRC_record CRC_Table
        for (idx=0;idx < (sizeof(CRC_Table)/sizeof(t_CRC_record));idx++)
        {
            if (CRC_Table[idx].CRC == ucCRC)
            {
                char temp[32];
                CRCucode = CRC_Table[idx].ucode;

                sprintf(temp, "%i", CRCucode);
                WritePrivateProfileString("uCodeCRC", ucCRC_Entry, temp, video_ini);    
                break;
            }
        }
    }

    if (ucode == -1)
    {
        ucode = CRCucode;
    }
    else
        if (CRCucode == -1)
        {
            char temp[32];
            sprintf(temp, "%i", ucode);
            WritePrivateProfileString("uCodeCRC", ucCRC_Entry, temp, video_ini);    
        }

    if (ucode == -1)
    {
        WritePrivateProfileString(CRC_Entry, "Name", (char*)header.name, video_ini);
        WritePrivateProfileString(CRC_Entry, "UCode", "-1", video_ini);
        WritePrivateProfileString(CRC_Entry, "FrameBuffer", "0", video_ini);
        WritePrivateProfileString(CRC_Entry, "FogEmulation", "0", video_ini);
    }

    while (ucode == -1)
    {
        char temp[1024];
//      tucode = 0;

// the message handling sux if i create the Dlg here :(((
// i think it is a thread problem :((

        MessageBox(NULL, "Config your UCode\nChange to the GUI and click Config", szAppName, MB_OK);
        sprintf(temp, "Cant find UCode for ROM \"%s\".\nLook for UCode=-1 in the tr64_ogl.ini\n [%s]", (char*)header.name, CRC_Entry);
    }
    return ucode;
}


//////////////////////////////////////////////////////////////////////////////
//                                                          //
//////////////////////////////////////////////////////////////////////////////
int GetSelectedUCode()
{
    if (SendDlgItemMessage(hConfig, IDC_TEXDUMP ,BM_GETCHECK, 0,0) == BST_CHECKED) 
        DumpTexture = TRUE;
    else
        DumpTexture = FALSE;

    if (SendDlgItemMessage(hConfig, IDC_VRML ,BM_GETCHECK, 0,0) == BST_CHECKED) 
        VRMLOutput = TRUE;
    else
        VRMLOutput = FALSE;

    if (SendDlgItemMessage(hConfig, IDC_FRAMEBUFFER ,BM_GETCHECK, 0,0) == BST_CHECKED) 
        FrameBuffer = TRUE;
    else
        FrameBuffer = FALSE;

    if (SendDlgItemMessage(hConfig, IDC_FOG ,BM_GETCHECK, 0,0) == BST_CHECKED) 
        FogEnabled = TRUE;
    else
        FogEnabled = FALSE;

    if (SendDlgItemMessage(hConfig, IDC_UC0 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 0;
    if (SendDlgItemMessage(hConfig, IDC_UC1 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 1;
    if (SendDlgItemMessage(hConfig, IDC_UC2 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 2;
    if (SendDlgItemMessage(hConfig, IDC_UC3 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 3;
    if (SendDlgItemMessage(hConfig, IDC_UC4 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 4;
    if (SendDlgItemMessage(hConfig, IDC_UC5 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 5;
    if (SendDlgItemMessage(hConfig, IDC_UC6 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 6;
    if (SendDlgItemMessage(hConfig, IDC_UC7 ,BM_GETCHECK, 0,0) == BST_CHECKED)
        return 7;

    return -1;
}


//////////////////////////////////////////////////////////////////////////////
// //
//////////////////////////////////////////////////////////////////////////////
void SetUCodeButton(int button)
{
/*  switch (button)
    {
    case 0:
        SendDlgItemMessage(hConfig, IDC_UC0 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 1:
        SendDlgItemMessage(hConfig, IDC_UC1 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 2:
        SendDlgItemMessage(hConfig, IDC_UC2 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 3:
        SendDlgItemMessage(hConfig, IDC_UC3 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 4:
        SendDlgItemMessage(hConfig, IDC_UC4 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 5:
        SendDlgItemMessage(hConfig, IDC_UC5 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    case 6:
        SendDlgItemMessage(hConfig, IDC_UC6 ,BM_SETCHECK, BST_CHECKED,0);   
        break;
    }
*/
    SendDlgItemMessage(hConfig, IDC_UCODE,CB_SETCURSEL, (WPARAM)button, 0);

//BOOL DumpTexture = FALSE;
//BOOL VRMLOutput = FALSE;
/*
    if (DumpTexture)
        SendMessage (GetDlgItem (hConfig, IDC_TEXDUMP), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_TEXDUMP), BM_SETCHECK,BST_UNCHECKED, 0);

    if (VRMLOutput)
        SendMessage (GetDlgItem (hConfig, IDC_VRML), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_VRML), BM_SETCHECK,BST_UNCHECKED, 0);

    if (FrameBuffer)
        SendMessage (GetDlgItem (hConfig, IDC_FRAMEBUFFER), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_FRAMEBUFFER), BM_SETCHECK,BST_UNCHECKED, 0);

    if (FogEnabled)
        SendMessage (GetDlgItem (hConfig, IDC_FOG), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_FOG), BM_SETCHECK,BST_UNCHECKED, 0);
*/
}


//////////////////////////////////////////////////////////////////////////////
// Set Comment Proc                                                         //
//////////////////////////////////////////////////////////////////////////////
void SaveUCode(int _ucode)
{
    t_romheader header;
    char *p = (char*)&header;
    int i;
//  char CRC_Entry[1024], temp[32];
    char temp[32];

    if (!HandsetUcode) return;
    if (MessageBox(NULL, "Do ya want to Save your selected uCode ??", szAppName, MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL) == IDNO) return;

    for (i = 0; i<sizeof(t_romheader); i++)
    {
        p[i] = pROM[i^3];
    }

// Generate String of video.ini
    GetPluginDir(video_ini);
    strcat(video_ini, "\\tr64_ogl.ini");

    HandsetUcode = FALSE;


    sprintf(temp, "%i", _ucode);
//  sprintf(CRC_Entry,"%08X%08X-%02X", header.crc1, header.crc2, header.countrycode);
//  WritePrivateProfileString(CRC_Entry, "Name", (char*)header.name, video_ini);
    WritePrivateProfileString(CRC_Entry, "Name", (char*)header.name, video_ini);
    WritePrivateProfileString(CRC_Entry, "UCode", temp, video_ini); 
    if (FrameBuffer)
        WritePrivateProfileString(CRC_Entry, "FrameBuffer", "-1", video_ini);   
    else
        WritePrivateProfileString(CRC_Entry, "FrameBuffer", "0", video_ini);    
    if (FogEnabled)
        WritePrivateProfileString(CRC_Entry, "FogEmulation", "-1", video_ini);  
    else
        WritePrivateProfileString(CRC_Entry, "FogEmulation", "0", video_ini);   
}



//////////////////////////////////////////////////////////////////////////////
// Init Comment Proc                                                        //
//////////////////////////////////////////////////////////////////////////////
void CreateConfigDlg()
{
    if (!hConfig)
        hConfig = CreateDialog(g_hInstance, "IDD_CONFIG", g_hWnd, (DLGPROC)ConfigProc);
    //SetUCodeButton(1);
/*
   if (FrameBuffer)
       SendDlgItemMessage(hConfig, IDC_FRAMEBUFFER, BM_SETCHECK,BST_CHECKED, 1);
   else
       SendDlgItemMessage(hConfig, IDC_FRAMEBUFFER, BM_SETCHECK,BST_UNCHECKED, 0);

   if (FogEnabled)
       SendDlgItemMessage(hConfig, IDC_FOG, BM_SETCHECK,BST_CHECKED, 1);
   else
       SendDlgItemMessage(hConfig, IDC_FOG, BM_SETCHECK,BST_UNCHECKED, 0);
*/
}



//////////////////////////////////////////////////////////////////////////////
// Set Comment Proc                                                         //
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ConfigProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    char* szUCodes[] = {
        "0 - Ex. Mario64, Demos",
        "1 - Ex. Mario Kart",
        "2 - Ex. Waveracer USA",
        "3 - Ex. Mortal Kombat 4",
        "4 - Ex. Zelda",
        "5 - Ex. Perfect Dark",
        "6 - Ex. Diddy Kong Racing",
        "7 - Ex. Conker's Bad Fur Day"  };

    switch (message) 
    {
       case WM_INITDIALOG:
           tucode = ucode;
           if (tucode < 0) tucode = 0;
           tFrameBuffer = FrameBuffer;
           tFogEnabled = FogEnabled;
           tDumpTexture = DumpTexture;
           tVRMLOutput = VRMLOutput;

           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[0]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[1]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[2]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[3]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[4]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[5]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[6]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_ADDSTRING,0,(LPARAM)szUCodes[7]);
           SendDlgItemMessage(hDlg, IDC_UCODE,CB_SETCURSEL, (WPARAM)tucode, 0);

            if (tDumpTexture)
                SendDlgItemMessage(hDlg, IDC_TEXDUMP, BM_SETCHECK,BST_CHECKED, 1);
            else
                SendDlgItemMessage(hDlg, IDC_TEXDUMP, BM_SETCHECK,BST_UNCHECKED, 0);    

            if (tVRMLOutput)
                SendDlgItemMessage(hDlg, IDC_VRML, BM_SETCHECK,BST_CHECKED, 1);
            else
                SendDlgItemMessage(hDlg, IDC_VRML, BM_SETCHECK,BST_UNCHECKED, 0);

           if (tFrameBuffer)
               SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_CHECKED, 1);
           else
               SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_UNCHECKED, 0);

           if (tFogEnabled)
               SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_CHECKED, 1);
           else
               SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_UNCHECKED, 0);
/*
           switch (ucode)
           {
           case -1:
           case 0:
               SendDlgItemMessage(hDlg, IDC_UC0 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 1:
               SendDlgItemMessage(hDlg, IDC_UC1 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 2:
               SendDlgItemMessage(hDlg, IDC_UC2 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 3:
               SendDlgItemMessage(hDlg, IDC_UC3 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 4:
               SendDlgItemMessage(hDlg, IDC_UC4 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 5:
               SendDlgItemMessage(hDlg, IDC_UC5 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           case 6:
               SendDlgItemMessage(hDlg, IDC_UC6 ,BM_SETCHECK, BST_CHECKED,0);
               break;
           }
*/
           return TRUE;
           break;
/*
       case WM_ACTIVATE:
           if (tFrameBuffer)
               SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_CHECKED, 0);
           else
               SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_UNCHECKED, 0);

           if (tFogEnabled)
               SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_CHECKED, 0);
           else
               SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_UNCHECKED, 0);
           return TRUE;
           break;
BOOL DumpTexture = FALSE;
BOOL VRMLOutput = FALSE;
    if (DumpTexture)
        SendMessage (GetDlgItem (hConfig, IDC_TEXDUMP), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_TEXDUMP), BM_SETCHECK,BST_UNCHECKED, 0);

    if (VRMLOutput)
        SendMessage (GetDlgItem (hConfig, IDC_VRML), BM_SETCHECK,BST_CHECKED, 1);
    else
        SendMessage (GetDlgItem (hConfig, IDC_VRML), BM_SETCHECK,BST_UNCHECKED, 0);
*/
       case WM_COMMAND:
            switch (wParam) 
            {
            case IDC_VRML:
                if (!tVRMLOutput)
                {
                    tVRMLOutput = -1;
                    SendDlgItemMessage(hDlg, IDC_VRML, BM_SETCHECK,BST_CHECKED, 1);
                }
                else
                {
                    tVRMLOutput = 0;
                    SendDlgItemMessage(hDlg, IDC_VRML, BM_SETCHECK,BST_UNCHECKED, 0);
                }
                break;
            case IDC_TEXDUMP:
                if (!tDumpTexture)
                {
                    tDumpTexture = -1;
                    SendDlgItemMessage(hDlg, IDC_TEXDUMP, BM_SETCHECK,BST_CHECKED, 1);
                }
                else
                {
                    tDumpTexture = 0;
                    SendDlgItemMessage(hDlg, IDC_TEXDUMP, BM_SETCHECK,BST_UNCHECKED, 0);
                }
                break;
            case IDC_FRAMEBUFFER:
                if (!tFrameBuffer)
                {
                    tFrameBuffer = -1;
                    SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_CHECKED, 1);
                }
                else
                {
                    tFrameBuffer = 0;
                    SendDlgItemMessage(hDlg, IDC_FRAMEBUFFER, BM_SETCHECK,BST_UNCHECKED, 0);
                }
//              return (TRUE);
                break;
            case IDC_FOG:
                if (!tFogEnabled)
                {
                    tFogEnabled = -1;
                    SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_CHECKED, 1);
                }
                else
                {
                    tFogEnabled = 0;
                    SendDlgItemMessage(hDlg, IDC_FOG, BM_SETCHECK,BST_UNCHECKED, 0);
                }
//              return (TRUE);
                break;
            case IDC_UCODE:
                tucode = SendDlgItemMessage(hDlg, IDC_UCODE, CB_GETCURSEL, 0, 0);
                break;
            case IDC_UC0:
            case IDC_UC1:
            case IDC_UC2:
            case IDC_UC3:
            case IDC_UC4:
            case IDC_UC5:
            case IDC_UC6:
            case IDC_UC7:
                tucode = wParam - IDC_UC0;
                break;
            case IDOK:
                {
                    GetSelectedUCode();
                    ucode = tucode;
                    //GetSelectedUCode();
                    HandsetUcode = TRUE;
                    EndDialog(hDlg, TRUE);
                    hConfig = NULL;
                }
//              return (TRUE);
                break;
            case IDSAVE:
                {
                    GetSelectedUCode();
                    ucode = tucode;
                    HandsetUcode = TRUE;
                    SaveUCode(ucode);
                    //ucode_version = ucode;
                    HandsetUcode = FALSE;
                    EndDialog(hDlg, TRUE);
                    hConfig = NULL;
                }
//              return (TRUE);
                break;

            case IDCANCEL:
                EndDialog(hDlg, TRUE);
                hConfig = NULL;
//              return (TRUE);
                break;
            }
            break;
    }
    return FALSE;
}
