/***************************************************************************
                          config.c  -  description
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


#include <windows.h>
#include <stdio.h>
#include "config.h"
#include "main_win.h"
#include "rombrowser.h"
#include "commandline.h"

#define CfgFileName "mupen64.cfg"

extern int no_audio_delay;
extern int no_compiled_jump;

////////////////////// Service functions and structures ////////////////////////

CONFIG Config;

// is this the best way to handle this?
int *autoinc_save_slot = &Config.AutoIncSaveSlot;

char *CfgFilePath()
{
   static char *cfgpath = NULL;
   if (cfgpath == NULL)
     {
       	cfgpath = malloc(strlen(AppPath)+1+strlen(CfgFileName));
	    strcpy(cfgpath, AppPath);
    	strcat(cfgpath, CfgFileName);
     }
   return cfgpath;
}


void WriteCfgString   (char *Section,char *Key,char *Value) 
{
    WritePrivateProfileString( Section, Key, Value, CfgFilePath());
}


void WriteCfgInt      (char *Section,char *Key,int Value) 
{
    static char TempStr[20];
    sprintf( TempStr, "%d", Value);
    WriteCfgString( Section, Key, TempStr );
}


void ReadCfgString    (char *Section,char *Key,char *DefaultValue,char *retValue) 
{
    GetPrivateProfileString( Section, Key, DefaultValue, retValue, MAX_PATH, CfgFilePath());
}


int ReadCfgInt        (char *Section,char *Key,int DefaultValue) 
{
    return GetPrivateProfileInt( Section, Key, DefaultValue, CfgFilePath());
}

//////////////////////////// Load and Save Config //////////////////////////////

void LoadRecentRoms()
{
    int i;
    char tempStr[50];
    
    Config.RecentRomsFreeze = ReadCfgInt( "Recent Roms", "Freeze", 0 ) ;
    for ( i=0; i < MAX_RECENT_ROMS; i++)
    {
        
        sprintf( tempStr, "RecentRom%d", i) ; 
        ReadCfgString( "Recent Roms", tempStr, "", Config.RecentRoms[i]) ;
        
    }
  
}

void LoadConfig()
{
   LoadRecentRoms();
   
   // Language
    ReadCfgString( "Language", "Default", "English" , Config.DefaultLanguage);
       
   //Window position and size 
    Config.WindowWidth = ReadCfgInt("Window","Width",640); 
    Config.WindowHeight = ReadCfgInt("Window","Height",480); 
    Config.WindowPosX = ReadCfgInt("Window","X",(GetSystemMetrics( SM_CXSCREEN ) - Config.WindowWidth) / 2);
    Config.WindowPosY = ReadCfgInt("Window","Y",(GetSystemMetrics( SM_CYSCREEN ) - Config.WindowHeight) / 2);
    
    //General Vars
    Config.showFPS = ReadCfgInt("General","Show FPS",1);
    Config.showVIS = ReadCfgInt("General","Show VIS",1);
    Config.alertBAD = ReadCfgInt("General","Alert Bad roms",1);
    Config.alertHACK = ReadCfgInt("General","Alert Hacked roms",1);
    Config.savesERRORS = ReadCfgInt("General","Alert Saves errors",1);
    Config.limitFps = ReadCfgInt("General","Alert Saves errors",1);
    Config.compressedIni = ReadCfgInt("General","Compressed Ini",1);
    Config.UseFPSmodifier = ReadCfgInt("General","Use Fps Modifier",1);
    Config.FPSmodifier = ReadCfgInt("General","Fps Modifier",100);
    
    
    Config.guiDynacore = ReadCfgInt("CPU","Core",1);
    
    
    
    //Advaced vars
    Config.StartFullScreen = ReadCfgInt("Advanced","Start Full Screen",0);
    Config.PauseWhenNotActive = ReadCfgInt("Advanced","Pause when not active",1);
    Config.OverwritePluginSettings = ReadCfgInt("Advanced","Overwrite Plugins Settings ",0);
    Config.GuiToolbar = ReadCfgInt( "Advanced", "Use Toolbar", 1);
    Config.GuiStatusbar = ReadCfgInt( "Advanced", "Use Statusbar", 1);
    Config.AutoIncSaveSlot = ReadCfgInt( "Advanced", "Auto Increment Save Slot", 0);
    
    //Compatibility Settings
    no_audio_delay = ReadCfgInt("Compatibility","No Audio Delay", 0);
    no_compiled_jump = ReadCfgInt("Compatibility","No Compiled Jump", 0);
    
    //RomBrowser Columns
    Config.Column_GoodName = ReadCfgInt("Rom Browser Columns","Good Name", 1);
    Config.Column_InternalName = ReadCfgInt("Rom Browser Columns","Internal Name", 0);
    Config.Column_Country = ReadCfgInt("Rom Browser Columns","Country", 1);
    Config.Column_Size = ReadCfgInt("Rom Browser Columns","Size", 1);
    Config.Column_Comments = ReadCfgInt("Rom Browser Columns","Comments", 1);
    Config.Column_FileName = ReadCfgInt("Rom Browser Columns","File Name", 0);
    Config.Column_MD5 = ReadCfgInt("Rom Browser Columns","MD5", 0);
    
      // Directories
    Config.DefaultPluginsDir = ReadCfgInt("Directories","Default Plugins Directory",1);
    Config.DefaultSavesDir = ReadCfgInt("Directories","Default Saves Directory",1);
    Config.DefaultScreenshotsDir = ReadCfgInt("Directories","Default Screenshots Directory",1);
    
    sprintf(Config.PluginsDir,"%sPlugin\\",AppPath);
    ReadCfgString("Directories","Plugins Directory",Config.PluginsDir, Config.PluginsDir);
    
    sprintf(Config.SavesDir,"%sSave\\",AppPath);
    ReadCfgString("Directories","Saves Directory",Config.SavesDir, Config.SavesDir);
    
    sprintf(Config.ScreenshotsDir,"%sScreenShots\\",AppPath);
    ReadCfgString("Directories","Screenshots Directory",Config.ScreenshotsDir, Config.ScreenshotsDir);    
    
    
    
    // Rom Browser
    Config.RomBrowserSortColumn = ReadCfgInt( "Rom Browser", "Sort Column" , 0);
    Config.RomBrowserRecursion = ReadCfgInt("Rom Browser","Recursion",0);
    ReadCfgString("Rom Browser","Sort Method","ASC",Config.RomBrowserSortMethod);
  
}

/////////////////////////////////////////////////////////////////////////////////

void saveWindowSettings()
{
    RECT rcMain;
    GetWindowRect(mainHWND, &rcMain);
	Config.WindowPosX=rcMain.left;
	Config.WindowPosY=rcMain.top;
	Config.WindowWidth = rcMain.right - rcMain.left;
	Config.WindowHeight = rcMain.bottom - rcMain.top;
	WriteCfgInt("Window","Width",Config.WindowWidth); 
    WriteCfgInt("Window","Height",Config.WindowHeight); 
    WriteCfgInt("Window","X",Config.WindowPosX);
    WriteCfgInt("Window","Y",Config.WindowPosY);
}

void saveBrowserSettings()
{
    int Column,ColWidth,index;
    index=0;
    for (Column = 0; Column < ROM_COLUMN_FIELDS; Column ++) {
        if ( isFieldInBrowser( Column) ) {
        ColWidth = ListView_GetColumnWidth(hRomList,index);
        WriteCfgInt("Rom Browser",getFieldName(Column),ColWidth);
        index++;
        }
    }
    
    WriteCfgInt( "Rom Browser", "Sort Column", Config.RomBrowserSortColumn);
    WriteCfgInt( "Rom Browser", "Recursion", Config.RomBrowserRecursion);
    WriteCfgString("Rom Browser","Sort Method", Config.RomBrowserSortMethod);
      
}

void SaveRecentRoms()
{
    int i;
    char tempStr[50];
    
    WriteCfgInt( "Recent Roms", "Freeze", Config.RecentRomsFreeze) ;
    for ( i=0; i < MAX_RECENT_ROMS; i++)
    {
        sprintf( tempStr, "RecentRom%d", i) ; 
        WriteCfgString( "Recent Roms", tempStr, Config.RecentRoms[i]) ;
    }
}

void SaveConfig()
{
    saveWindowSettings();
    
    if (!cmdlineNoGui) {
        saveBrowserSettings();
        SaveRecentRoms();
    }    
    
    //Language
    WriteCfgString( "Language", "Default", Config.DefaultLanguage);
    
    //General Vars
    WriteCfgInt( "General", "Show FPS", Config.showFPS);
    WriteCfgInt( "General", "Show VIS", Config.showVIS);
    WriteCfgInt( "General", "Alert Bad roms", Config.alertBAD);
    WriteCfgInt( "General", "Alert Hacked roms", Config.alertHACK);
    WriteCfgInt( "General", "Alert Saves errors", Config.savesERRORS);
    WriteCfgInt( "General", "Alert Saves errors", Config.limitFps);
    WriteCfgInt( "General", "Compressed Ini", Config.compressedIni);
    WriteCfgInt( "General", "Fps Modifier", Config.FPSmodifier);
    WriteCfgInt( "General", "Use Fps Modifier", Config.UseFPSmodifier);
    
    //Advanced Vars
    WriteCfgInt( "Advanced", "Start Full Screen", Config.StartFullScreen);
    WriteCfgInt( "Advanced", "Pause when not active", Config.PauseWhenNotActive);
    WriteCfgInt( "Advanced", "Overwrite Plugins Settings", Config.OverwritePluginSettings);
    WriteCfgInt( "Advanced", "Use Toolbar", Config.GuiToolbar);
    WriteCfgInt( "Advanced", "Use Statusbar", Config.GuiStatusbar);
    WriteCfgInt( "Advanced", "Auto Increment Save Slot", Config.AutoIncSaveSlot);
    
    WriteCfgInt( "CPU", "Core", Config.guiDynacore);
    
    //Compatibility Settings
    WriteCfgInt( "Compatibility", "No Audio Delay", no_audio_delay);
    WriteCfgInt( "Compatibility", "No Compiled Jump", no_compiled_jump);
    
    //Rom Browser columns
    WriteCfgInt("Rom Browser Columns","Good Name", Config.Column_GoodName);
    WriteCfgInt("Rom Browser Columns","Internal Name", Config.Column_InternalName);
    WriteCfgInt("Rom Browser Columns","Country", Config.Column_Country);
    WriteCfgInt("Rom Browser Columns","Size", Config.Column_Size);
    WriteCfgInt("Rom Browser Columns","Comments", Config.Column_Comments);
    WriteCfgInt("Rom Browser Columns","File Name", Config.Column_FileName);
    WriteCfgInt("Rom Browser Columns","MD5", Config.Column_MD5);
    
    // Directories
    WriteCfgInt( "Directories", "Default Plugins Directory", Config.DefaultPluginsDir);
    WriteCfgString("Directories","Plugins Directory",Config.PluginsDir);
    WriteCfgInt( "Directories","Default Saves Directory", Config.DefaultSavesDir);
    WriteCfgString("Directories","Saves Directory",Config.SavesDir);
    WriteCfgInt( "Directories", "Default Screenshots Directory", Config.DefaultScreenshotsDir);
    WriteCfgString("Directories","Screenshots Directory",Config.ScreenshotsDir);    
}


