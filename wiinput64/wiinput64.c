/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Wiinput64 plugin : wiinput64.c                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Lionel Fuentes (Funto)                             *
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "wiinput64.h"
#include "Input_1.1.h"
#include "gui.h"
#include <wiimote_api.h>
#include <pthread.h>

#define MAX_NUNCHUK_JOY_Y 223.0
#define MIN_NUNCHUK_JOY_Y 32.0
#define MAX_NUNCHUK_JOY_X 235.0
#define MIN_NUNCHUK_JOY_X 26.0

char* config_dir = NULL;	// Configuration files directory
bluetooth_addr wiimote_addresses[4];	// Bluetooth addresses of the wiimotes
static wiimote_t wiimotes[4] = {WIIMOTE_INIT, WIIMOTE_INIT, WIIMOTE_INIT, WIIMOTE_INIT};	// The wiimotes

// Variables for the wiimote_update() thread
static pthread_t thread_update;
static bool should_stop_update = false;

// Utility to convert values from the nunchuk to values suitable for the N64 joypad
static void NunchukToN64(int nunchuk_joy_x, int nunchuk_joy_y, int* n64_joy_x, int* n64_joy_y)
{
	float fx = nunchuk_joy_x;
	float fy = nunchuk_joy_y;

	*n64_joy_x = (int)(160.0*(fx - MIN_NUNCHUK_JOY_X) / (MAX_NUNCHUK_JOY_X - MIN_NUNCHUK_JOY_X)) - 80;
	*n64_joy_y = (int)(160.0*(fy - MIN_NUNCHUK_JOY_Y) / (MAX_NUNCHUK_JOY_Y - MIN_NUNCHUK_JOY_Y)) - 80;
}

// Function called in an independent thread which updates the wiimote's status.
// I had to do this because the frequency of updates of Mupen64Plus was too low for the wiimote.
static void* Wiinput64UpdateThread(void* data)
{
	while(1)
	{
		if(should_stop_update)
			break;

		wiimote_update(&wiimotes[0]);
	}
	return NULL;
}

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

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL CloseDLL (void)
{
	if(config_dir != NULL)
	{
		free(config_dir);
		config_dir = NULL;
	}
}

/******************************************************************
  Function: ControllerCommand
  Purpose:  To process the raw data that has just been sent to a
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none

  note:     This function is only needed if the DLL is allowing raw
            data, or the plugin is set to raw

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
EXPORT void CALL ControllerCommand ( int Control, BYTE * Command)
{
	//printf("DEBUG : ControllerCommand\n");
}

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllAbout ( HWND hParent )
{
	AboutDialog();
}

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllConfig ( HWND hParent )
{
	ConfigDialog();
}

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllTest ( HWND hParent )
{
	//printf("DEBUG : DllTest\n");
}

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/
EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x0101;
	PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
	strcpy( PluginInfo->Name, PLUGIN_NAME );	// Name to be displayed in the Configure dialog
	PluginInfo->Reserved1 = FALSE;
	PluginInfo->Reserved2 = FALSE;

	//printf("DEBUG : GetDllInfo\n");
}

/******************************************************************
  Function: GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with
            the controller state.
  output:   none
*******************************************************************/
EXPORT void CALL GetKeys(int Control, BUTTONS * Keys )
{
	int x=0, y=0;

	// NB : we don't lock/unlock any mutex for performance reasons...
	if(Control == 0)	// For the moment, we only handle 1 joypad.
	{
		Keys->R_DPAD = 0;
		Keys->L_DPAD = 0;
		Keys->D_DPAD = 0;
		Keys->U_DPAD = 0;
		Keys->START_BUTTON = wiimotes[0].keys.home;
		Keys->Z_TRIG = wiimotes[0].ext.nunchuk.keys.z;
		Keys->B_BUTTON = wiimotes[0].keys.b;
		Keys->A_BUTTON = wiimotes[0].keys.a;

		Keys->R_CBUTTON = wiimotes[0].keys.right;
		Keys->L_CBUTTON = wiimotes[0].keys.left;
		Keys->D_CBUTTON = wiimotes[0].keys.down;
		Keys->U_CBUTTON = wiimotes[0].keys.up;
		Keys->R_TRIG = wiimotes[0].keys.plus;
		Keys->L_TRIG = wiimotes[0].keys.minus;
		Keys->Reserved1 = 0;
		Keys->Reserved2 = 0;

		NunchukToN64(wiimotes[0].ext.nunchuk.joyx, wiimotes[0].ext.nunchuk.joyy, &x, &y);
		Keys->Y_AXIS = x;
		Keys->X_AXIS = y;
	}
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
EXPORT void CALL InitiateControllers (CONTROL_INFO ControlInfo)
{
	//printf("DEBUG : InitiateControllers\n");

	ControlInfo.Controls[0].Present = TRUE;
}

/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
EXPORT void CALL ReadController ( int Control, BYTE * Command )
{
	//printf("DEBUG : ReadController\n");
}

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL RomClosed (void)
{
	// We signal the update thread that it should stop
	should_stop_update = true;

	// We wait for the update thread to stop
	pthread_join(thread_update, NULL);

	// We disconnect the wiimote
	wiimote_disconnect(&wiimotes[0]);
}

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the
            emulation thread)
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL RomOpen (void)
{
	printf("Press 1+2 on the wiimote for connection (bluetooth address : %s)...\n", wiimote_addresses[0]);
	do
		wiimote_connect(&wiimotes[0], wiimote_addresses[0]);
	while(!wiimote_is_open(&wiimotes[0]));

	wiimotes[0].mode.acc = 0;	// No need for accelerometers

	// Launch the update thread
	should_stop_update = false;
	pthread_create(&thread_update, NULL, &Wiinput64UpdateThread, NULL);
}

/******************************************************************
  Function: WM_KeyDown
  Purpose:  To pass the WM_KeyDown message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL WM_KeyDown( WPARAM wParam, LPARAM lParam )
{
	//printf("DEBUG : WM_KeyDown\n");
}

/******************************************************************
  Function: WM_KeyUp
  Purpose:  To pass the WM_KEYUP message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL WM_KeyUp( WPARAM wParam, LPARAM lParam )
{
	//printf("DEBUG : WM_KeyUp\n");
}

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: SetConfigDir
  Purpose:  To pass the location where config files should be read/
            written to.
  input:    path to config directory
  output:   none
*******************************************************************/
EXPORT void CALL SetConfigDir( char *configDir )
{
	FILE* f = NULL;
	char* buffer = NULL;
	size_t len = strlen(configDir);
	int i=0;

	// Memorize the configuration files directory
	config_dir = (char*)malloc(len + 1);
	strcpy(config_dir, configDir);

	// Remove the ending '/' if there is one :
	if(len >= 2 && config_dir[len-2] == '/')
		config_dir[len-2] = '\0';

	// Load the config file, if there is one
	buffer = (char*)malloc(len + 50);
	sprintf(buffer, "%s/%s", config_dir, CONFIG_FILE_NAME);
	if((f=fopen(buffer, "r")) != NULL)
	{
		// Look for wiimote number i's address
		for(i=0 ; i<4 ; i++)
		{
			char str_format[40] = "";
			char str_line[40] = "";
			sprintf(str_format, "addr%d=%%s\n", i);

			fseek(f, 0, SEEK_SET);

			while(!feof(f))
			{
				if(fgets(str_line, sizeof(str_line), f) != NULL)
				{
					if(sscanf(str_line, str_format, wiimote_addresses[i]) == 1)
						break;
				}
			}
		}
		fclose(f);
	}

	free(buffer);
}
