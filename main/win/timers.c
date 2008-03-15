/***************************************************************************
                          timers.c  -  description
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
#define _WIN32_IE 0x0500
#include <commctrl.h>
#include "../guifuncs.h"
#include "main_win.h"
#include "../rom.h"
#include "translation.h"

static float VILimit = 60.0;
static double VILimitMilliseconds = 1000.0/60.0;

int GetVILimit()
{
    if (ROM_HEADER==NULL) return 60;
    switch (ROM_HEADER->Country_code&0xFF)
    {
      case 0x44:
	   case 0x46:
	   case 0x49:
	   case 0x50:
	   case 0x53:
	   case 0x55:
	   case 0x58:
	   case 0x59:
	     return 50;
	     break;
	   case 0x37:
	   case 0x41:
	   case 0x45:
	   case 0x4a:
	     return 60;
	     break;  
	   default:
         return 60;  
    }
}

void InitTimer() {
   int temp;
   VILimit = GetVILimit();
   if (Config.UseFPSmodifier) {
       temp = Config.FPSmodifier ; 
      }  
   else {
       temp = 100;
   }    
   VILimitMilliseconds = (double) 1000.0/(VILimit * temp / 100);
   Translate( "Speed Limit", TempMessage );
   sprintf(TempMessage,"%s: %d%%", TempMessage, temp);
   SendMessage( hStatus, SB_SETTEXT, 0, (LPARAM)TempMessage );    
}


void new_frame() {
   
   DWORD CurrentFPSTime;
   float FPS;
   char mes[100];
   static DWORD LastFPSTime;
   static DWORD CounterTime;
   static int Fps_Counter=0;
   
   if (!Config.showFPS) return;
   Fps_Counter++;
   
   CurrentFPSTime = timeGetTime();
   
   if (CurrentFPSTime - CounterTime >= 1000 ) {
      FPS = (float) (Fps_Counter * 1000.0 / (CurrentFPSTime - CounterTime));
      sprintf(mes,"FPS: %.1f",FPS);
      SendMessage( hStatus, SB_SETTEXT, 1, (LPARAM)mes );
      CounterTime = timeGetTime();
      Fps_Counter = 0;
     }
   
   LastFPSTime = CurrentFPSTime ;
}

void new_vi() {
   DWORD Dif;
   DWORD CurrentFPSTime;
   char mes[100];
   static DWORD LastFPSTime = 0;
   static DWORD CounterTime = 0;
   static DWORD CalculatedTime ;
   static int VI_Counter = 0;
   long time;
   
   if ( (!Config.showVIS) && (!Config.limitFps) ) return;
   VI_Counter++;
         
   CurrentFPSTime = timeGetTime();
   
   Dif = CurrentFPSTime - LastFPSTime;
   if (Config.limitFps && manualFPSLimit) {
     if (Dif <  VILimitMilliseconds )
      {
          CalculatedTime = CounterTime + (double)VILimitMilliseconds * (double)VI_Counter;
          time = (int)(CalculatedTime - CurrentFPSTime);
          if (time>0&&time<1000) {
                    Sleep(time);
          }
          CurrentFPSTime = CurrentFPSTime + time;
      }
     }
     
     if ( Config.showVIS ) {
          if (CurrentFPSTime - CounterTime >= 1000.0 ) {
             sprintf(mes,"VI/s: %.1f",(float) (VI_Counter * 1000.0 / (CurrentFPSTime - CounterTime)));
             if (Config.showFPS) SendMessage( hStatus, SB_SETTEXT, 2, (LPARAM)mes );
             else SendMessage( hStatus, SB_SETTEXT, 1, (LPARAM)mes );
             CounterTime = timeGetTime() ;
             VI_Counter = 0 ;
          }
   }
   LastFPSTime = CurrentFPSTime ;
}
