/**
 * Mupen64Plus - audio.c
 * Copyright (C) 2002 Zilmar
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA
 *
**/

#include "../main/winlnxdefs.h"
#include <stdio.h>

#include "audio.h"

AUDIO_INFO AudioInfo;


EXPORT void CALL
AiDacrateChanged( int SystemType )
{
}

EXPORT void CALL
AiLenChanged( void )
{
}

EXPORT DWORD CALL
AiReadLength( void )
{
    return 0;
}

EXPORT void CALL
AiUpdate( BOOL Wait )
{
}

EXPORT void CALL
CloseDLL( void )
{
}

EXPORT void CALL
DllAbout( HWND hParent )
{
    printf ("No Audio Plugin\n" );
}

EXPORT void CALL
DllConfig ( HWND hParent )
{
}

EXPORT void CALL
DllTest ( HWND hParent )
{
}

EXPORT void CALL
GetDllInfo( PLUGIN_INFO * PluginInfo )
{
    PluginInfo->Version = 0x0101;
    PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
    sprintf(PluginInfo->Name,"No Audio");
    PluginInfo->NormalMemory  = TRUE;
    PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL
InitiateAudio( AUDIO_INFO Audio_Info )
{
    AudioInfo = Audio_Info;
    return TRUE;
}

EXPORT void CALL RomOpen()
{
}

EXPORT void CALL
RomClosed( void )
{
}

EXPORT void CALL
ProcessAlist( void )
{
}
