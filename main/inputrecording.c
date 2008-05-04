/**
 * Mupen64Plus - inputrecording.c
 * Copyright (C) 2008 okaygo
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/


// Rename to input_handler.c

#include "plugin.h"
#include "rom.h"
#include "savestates.h"
#include "main.h"
#include "inputrecording.h"
#include "../memory/memory.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int l_CurrentSample = 0;
int l_CurrentVI = 0;
char l_InputDisplay[64];
int l_LastInput = 0;

int l_TotalSamples = 0;

int g_UseSaveData = 1; // TAS will always use its own version of savedata. This is a global variable

FILE *PlaybackFile;
m64_header Header;

int BeginPlayback(char *sz_filename)
{
    PlaybackFile = fopen(sz_filename,"rb");
    if (!PlaybackFile)
    {
    	EndPlaybackAndRecording();
    	printf("Could not open file %s for playback.",sz_filename);
    	return 0;
    }
    g_EmulatorPlayback = 1;
	printf("Reading header file of the m64 file.\n");
	fread(&Header,sizeof(Header),1,PlaybackFile);
	SetupEmulationState();
	// todo: verify all header information

    return 1;
}

int SetupEmulationState()
{
    if (Header.signature[0] != 0x4D && Header.signature[1] != 0x36
    && Header.signature[2] != 0x34 && Header.signature[3] != 0x1A)
    {
    	printf("Invalid signature in header file.\n");
    	return 0;
    }
    
    if (Header.version_number != 3) 
    {
    	printf("Invalid version number: %i\n",Header.version_number);
    }
    
    printf("Movie UID: %i\n",Header.movie_uid);
    printf("Total VI's: %i\n",Header.total_vi);
    printf("Rerecord Count: %i\n",Header.rerecord_count);
    printf("FPS: %i\n",Header.fps);
    printf("Controllers: %i\n",Header.controllers);
    // todo: enable this many controllers
    printf("Input Samples: %i\n",Header.input_samples);
    l_TotalSamples = Header.input_samples;
    
    return 1;
}

// taken from mupen64rerecording v8
void InputToString ()
{

	// input display
	l_InputDisplay[0] = '\0';
	{
		BOOL a, b, z, l, r, s, cl, cu, cr, cd, dl, du, dr, dd;
		signed char x, y;
		dr = (l_LastInput & (0x0001)) != 0;
		dl = (l_LastInput & (0x0002)) != 0;
		dd = (l_LastInput & (0x0004)) != 0;
		du = (l_LastInput & (0x0008)) != 0;
		s  = (l_LastInput & (0x0010)) != 0; // start button
		z  = (l_LastInput & (0x0020)) != 0;
		b  = (l_LastInput & (0x0040)) != 0;
		a  = (l_LastInput & (0x0080)) != 0;
		cr = (l_LastInput & (0x0100)) != 0;
		cl = (l_LastInput & (0x0200)) != 0;
		cd = (l_LastInput & (0x0400)) != 0;
		cu = (l_LastInput & (0x0800)) != 0;
		r  = (l_LastInput & (0x1000)) != 0;
		l  = (l_LastInput & (0x2000)) != 0;
		x = ((l_LastInput & (0x00FF0000)) >> 16);
		y = ((l_LastInput & (0xFF000000)) >> 24);

		if(!x && !y)
			strcpy(l_InputDisplay, "");
		else
		{
			int xamt = (x<0?-x:x) * 99/127; if(!xamt && x) xamt = 1;
			int yamt = (y<0?-y:y) * 99/127; if(!yamt && y) yamt = 1;
			if(x && y)
				sprintf(l_InputDisplay, "%c%d %c%d ", x<0?'<':'>', xamt, y<0?'v':'^', yamt);
			else if(x)
				sprintf(l_InputDisplay, "%c%d ", x<0?'<':'>', xamt);
			else //if(y)
				sprintf(l_InputDisplay, "%c%d ", y<0?'v':'^', yamt);
		}

		if(s) strcat(l_InputDisplay, "S");
		if(z) strcat(l_InputDisplay, "Z");
		if(a) strcat(l_InputDisplay, "A");
		if(b) strcat(l_InputDisplay, "B");
		if(l) strcat(l_InputDisplay, "L");
		if(r) strcat(l_InputDisplay, "R");
		if(cu||cd||cl||cr)
		{
			strcat(l_InputDisplay, " C");
			if(cu) strcat(l_InputDisplay, "^");
			if(cd) strcat(l_InputDisplay, "v");
			if(cl) strcat(l_InputDisplay, "<");
			if(cr) strcat(l_InputDisplay, ">");
		}
		if(du||dd||dl||dr)
		{
			strcat(l_InputDisplay, " D");
			if(du) strcat(l_InputDisplay, "^");
			if(dd) strcat(l_InputDisplay, "v");
			if(dl) strcat(l_InputDisplay, "<");
			if(dr) strcat(l_InputDisplay, ">");
		}
	}
	
    printf("Input: %s\n",l_InputDisplay);
}

void _StartROM()
{
	l_CurrentSample = 0;
	l_CurrentVI = 0;
	g_UseSaveData = 1;
}

void EndPlaybackAndRecording()
{
	l_TotalSamples = 0;
	l_CurrentSample = 0;
	l_CurrentVI = 0;
	g_UseSaveData = 1;
	g_EmulatorRecording = 0;
	g_EmulatorPlayback = 0;
}

void _NewVI()
{
	l_CurrentVI++;
	if (g_EmulatorRecording)
	{
	    printf("Current frame: %i\n",l_CurrentVI);
	}
	
	// What other functions do we need to hook?
}

void _GetKeys( int Control, BUTTONS *Keys )
{
	// Since we handle input here now, we always want to start with getKeys.
    getKeys(Control, Keys);
    
    if (g_EmulatorPlayback)
    {
        if (Control == 0) // hack: assume only 1 controller
        {
            fread(Keys,sizeof(BUTTONS),1,PlaybackFile);
        }
        else
        {
            memset(Keys,0,sizeof(BUTTONS));
        }
        
        // it doesn't matter if there isn't 4 controllers plugged in, 
        // our sample stops at this point.
        if (Control == 3)
        {
        	l_CurrentSample++;
        	if (l_CurrentSample > l_TotalSamples)
        	{
        		printf("Playback complete.\n");
        		EndPlaybackAndRecording();
        	}
        }
        // read keys from file.
    }
    
    if (g_EmulatorRecording)
    {
        // record keys
    }
    
    // print out the data of the controller input
    if (Control == 0)
    {
        memcpy(&l_LastInput,&Keys->Value,sizeof(int));
        InputToString();
    }
}

void CleanUpSaveFiles ()
{

    char *filename;

	filename = malloc(strlen(get_savespath())+strlen("CurrentPlayback")+4+1);

    sprintf(filename,"%sCurrentPlayback.sra",get_savespath());
    remove(filename);
    sprintf(filename,"%sCurrentPlayback.eep",get_savespath());
    remove(filename);
    sprintf(filename,"%sCurrentPlayback.fla",get_savespath());
    remove(filename);
    sprintf(filename,"%sCurrentPlayback.mpk",get_savespath());
    remove(filename);
  
    free(filename);
  
}
