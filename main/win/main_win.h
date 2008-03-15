/***************************************************************************
                          main_win.h  -  description
                             -------------------
    copyright            : (C) 2003 by ShadowPrince
    email                : shadow@emulation64.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAIN_WIN_H
#define MAIN_WIN_H

BOOL CALLBACK CfgDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void ShowMessage(LPSTR lpszMessage) ;
void EnableToolbar();
void CreateStatusBarWindow( HWND hwnd );
void SetStatusMode( int mode );
char *getPluginName( char *pluginpath, int plugintype);
char* getExtension(char *str);

/********* Global Variables **********/
char TempMessage[800];
int emu_launched;
int emu_paused;
int recording;
HWND hTool, mainHWND, hStatus, hRomList, hStatusProgress;
HINSTANCE app_hInstance;
extern BOOL manualFPSLimit;
char statusmsg[800];

char gfx_name[255];
char input_name[255];
char sound_name[255];
char rsp_name[255];

extern HWND hwnd_plug;
extern HANDLE EmuThreadHandle;

extern char AppPath[MAX_PATH];

void EnableEmulationMenuItems(BOOL flag);
BOOL StartRom(char *fullRomPath);
void resetEmu() ;
void resumeEmu();
void pauseEmu() ;
//void closeRom();
void search_plugins();
void rewind_plugin();
int get_plugin_type();
char *next_plugin();
void exec_config(char *name);
void exec_test(char *name);
void exec_about(char *name);
void EnableStatusbar();

typedef struct _CONFIG {
    unsigned char ConfigVersion ;
    
    //Language
    char DefaultLanguage[100];
    
    // Alert Messages variables
    BOOL showFPS;
    BOOL showVIS;
    BOOL alertBAD;
    BOOL alertHACK;
    BOOL savesERRORS;
    
    // General vars
    BOOL limitFps;
    BOOL compressedIni;
    int guiDynacore;
    BOOL UseFPSmodifier;
    int FPSmodifier;
    
    // Advanced vars
    BOOL StartFullScreen;
    BOOL PauseWhenNotActive;
    BOOL OverwritePluginSettings;
    BOOL GuiToolbar;
    BOOL GuiStatusbar;
    BOOL AutoIncSaveSlot;
    
    //Compatibility Options
    //BOOL NoAudioDelay;
    //BOOL NoCompiledJump;
    
    //Rom Browser Columns
    BOOL Column_GoodName;
    BOOL Column_InternalName;
    BOOL Column_Country;
    BOOL Column_Size;
    BOOL Column_Comments;
    BOOL Column_FileName;
    BOOL Column_MD5;
                                             
    // Directories
    BOOL DefaultPluginsDir;
    BOOL DefaultSavesDir;
    BOOL DefaultScreenshotsDir;
    char PluginsDir[MAX_PATH];
    char SavesDir[MAX_PATH];
    char ScreenshotsDir[MAX_PATH];    
    
    // Recent Roms
    char RecentRoms[10][MAX_PATH];
    BOOL RecentRomsFreeze;
    
    //Window
    int WindowWidth;
    int WindowHeight;
    int WindowPosX;
    int WindowPosY;
        
    //Rom Browser
    int RomBrowserSortColumn;
    char RomBrowserSortMethod[10];
    BOOL RomBrowserRecursion;
} CONFIG;

extern CONFIG Config;

#endif
