/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - input.c                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Scott Gorman (okaygo)                              *
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
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "wiinput64.h"
#include "Input_1.1.h"
#include <wiimote_api.h>
#include <pthread.h>

#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#define MAX_NUNCHUK_JOY_Y 223.0
#define MIN_NUNCHUK_JOY_Y 32.0
#define MAX_NUNCHUK_JOY_X 235.0
#define MIN_NUNCHUK_JOY_X 26.0

char pluginName[] = "Wiinput64 plugin by Funto";
char configdir[PATH_MAX] = {0};
static wiimote_t wiimote;

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

		wiimote_update(&wiimote);
	}
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
	//printf("DEBUG : CloseDLL\n");
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
	//printf("DEBUG : DllAbout\n");

	char s[] = "Wiinput64 plugin for Mupen64 emulator\nMade by Funto\n";
#ifdef USE_GTK
	GtkWidget *dialog, *label, *okay_button;

	// Create the widgets
	dialog = gtk_dialog_new();
	label = gtk_label_new (s);
	okay_button = gtk_button_new_with_label("OK");

	// Ensure that the dialog box is destroyed when the user clicks ok.
	gtk_signal_connect_object	(GTK_OBJECT (okay_button), "clicked",
								GTK_SIGNAL_FUNC (gtk_widget_destroy),
								GTK_OBJECT (dialog));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);

	// Add the label, and show everything we've added to the dialog.
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			  label);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_widget_show_all (dialog);
#else
	puts(s);
#endif
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
	//printf("DEBUG : DllConfig\n");
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
	strcpy( PluginInfo->Name, pluginName );
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
		Keys->START_BUTTON = wiimote.keys.home;
		Keys->Z_TRIG = wiimote.ext.nunchuk.keys.z;
		Keys->B_BUTTON = wiimote.keys.b;
		Keys->A_BUTTON = wiimote.keys.a;

		Keys->R_CBUTTON = wiimote.keys.right;
		Keys->L_CBUTTON = wiimote.keys.left;
		Keys->D_CBUTTON = wiimote.keys.down;
		Keys->U_CBUTTON = wiimote.keys.up;
		Keys->R_TRIG = wiimote.keys.plus;
		Keys->L_TRIG = wiimote.keys.minus;
		Keys->Reserved1 = 0;
		Keys->Reserved2 = 0;

		NunchukToN64(wiimote.ext.nunchuk.joyx, wiimote.ext.nunchuk.joyy, &x, &y);
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
	wiimote_disconnect(&wiimote);
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
	// TODO : NOT hard-code my wiimote address ^^
	static const char* wiimote_address = "00:1E:A9:3D:AF:9E";
	//static const char* wiimote_address = "00:1D:BC:2A:F1:DE";

	printf("Press 1+2 on the wiimote for connection...\n");
	do
		wiimote_connect(&wiimote, wiimote_address);
	while(!wiimote_is_open(&wiimote));

	wiimote.mode.acc = 0;	// No need for accelerometers

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
	//printf("DEBUG : SetConfigDir\n");
}
