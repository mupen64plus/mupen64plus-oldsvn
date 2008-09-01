/**
 * Mupen64Plus - input.cpp
 * Copyright (C) 2008 Scott Gorman (okaygo)
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


#include <limits.h>
#include <string.h>
#include "input.h"
#include "Input_1.1.h"

char pluginName[] = "No Input";
char configdir[PATH_MAX] = {0};

#ifndef __LINUX__

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {

    }
    return TRUE;
}
#else
void _init( void )
{
}
#endif // !__LINUX__

EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
    PluginInfo->Version = 0x0101;
    PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
    strcpy( PluginInfo->Name, pluginName );
    PluginInfo->Reserved1 = FALSE;
    PluginInfo->Reserved2 = FALSE;
}

EXPORT void CALL InitiateControllers (CONTROL_INFO ControlInfo)
{
    ControlInfo.Controls[0].Present = TRUE;
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys )
{
    Keys->Value = 0x0000;
}
