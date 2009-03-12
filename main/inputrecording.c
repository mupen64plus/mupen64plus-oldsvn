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
#include "config.h"
#include "../r4300/interupt.h"
#include "translate.h"
#include "../opengl/osd.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define MUP_MAGIC (0x1a34364d) // M64\0x1a
#define MUP_VERSION (3)
#define MUP_HEADER_SIZE (sizeof(m64_header))
#define CONTROLLER_PRESENT          (0x001)
#define CONTROLLER_MEMPACK          (0x010)
#define CONTROLLER_RUMBLEPACK       (0x100)

int l_CurrentSample = 0;
int l_CurrentVI = 0;
char l_InputDisplay[64];
int l_LastInput = 0;

int l_TotalSamples = 0;

int g_UseSaveData = 1; // TAS will always use its own version of savedata. This is a global variable
int l_ForceManual = 0; // use manual settings - ie: do not let emulator configure for you.

char progress_file[255];
char playback_file[255];

FILE *PlaybackFile;
FILE *RecordingFile;
m64_header Header;

char* get_m64_filename()
{
    size_t length = strlen(get_savespath()) + strlen((char*)ROM_HEADER->nom) + 4 + 1;
    char *file = (char *) malloc(length);
    snprintf(file, length, "%s%s.m64", get_savespath(), ROM_HEADER->nom);
    file[length] = '\0';
    return file;
}

int BeginPlayback(const char *sz_filename)
{
    int result = 0;
    if (g_EmulatorRunning) {
        strcpy(playback_file, sz_filename);
        strcpy(progress_file, sz_filename);
        strcat(progress_file, ".progress");
	    if (g_ReadOnlyPlayback == 1) {
            PlaybackFile = fopen(sz_filename,"rb");
            if (!PlaybackFile) {
            	EndPlaybackAndRecording();
                main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Could not open file %s for playback."), sz_filename);
            	return 0;
            }
            g_EmulatorPlayback = 1;
            fread(&Header,sizeof(Header),1,PlaybackFile);
            if (SetupEmulationState()) {
                if (Header.start_type == MOVIE_START_FROM_RESET) {
                    add_interupt_event(HW2_INT, 0);  /* Hardware 2 Interrupt immediately */
                    add_interupt_event(NMI_INT, 50000000);  /* Non maskable Interrupt after 1/2 second */
                    main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Playback started from reset: %s"), sz_filename);
                } else {
                    savestates_select_slot((unsigned int) Header.movie_uid);
                    savestates_job |= LOADSTATE;
                    main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Playback from savestate started"));
                }
            }
            result = 1;
        } else {
            RecordingFile = fopen(sz_filename,"ab+");
            if (!RecordingFile) {
                EndPlaybackAndRecording();
                main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Could not open file %s for recording."), sz_filename);
                result = 0;
            } else {            
                g_EmulatorRecording = 1;
        	    savestates_select_filename(progress_file);
                savestates_job |= LOADSTATE;
	            fseek(RecordingFile, 0L, SEEK_SET);
	            fread(&Header,sizeof(Header),1,RecordingFile);
	            fseek(RecordingFile, 0L, SEEK_END);
                // TODO
                // 1. Load .progress filestate.
                // 2. Open the .m64 file and start appending ...
            }
        }
    }
    return result;
}

int BeginRecording(char *sz_filename, int fromSnapshot, const char *authorUTF8, const char *descriptionUTF8 )
{
    int result = 0;
    if (g_EmulatorRunning) {
        strcpy(progress_file, sz_filename);
        strcat(progress_file, ".progress");

        RecordingFile = fopen(sz_filename,"wb");
        if (!RecordingFile) {
            EndPlaybackAndRecording();
            main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Could not create file %s for recording."), sz_filename);
            return 0;
        }
        g_EmulatorRecording = 1;
        WriteEmulationState(fromSnapshot, authorUTF8, descriptionUTF8);
        fwrite(&Header,sizeof(Header),1,RecordingFile);
        if (SetupEmulationState()) {
            if (Header.start_type == MOVIE_START_FROM_RESET) {
                add_interupt_event(HW2_INT, 0);  /* Hardware 2 Interrupt immediately */
                add_interupt_event(NMI_INT, 50000000);  /* Non maskable Interrupt after 1/2 second */
                main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Recording to file started: %s"), sz_filename);
            } else {
                savestates_job |= SAVESTATE;
                main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Recording from savestate started"));
            }
        }
        result = 1;
    }
    return result;
}

int WriteEmulationState(int fromSnapshot, const char *authorUTF8, const char *descriptionUTF8)
{
    int i;
	memset(&Header, 0, MUP_HEADER_SIZE);
    Header.signature = MUP_MAGIC;
    Header.version_number = MUP_VERSION;
    Header.movie_uid = (int) savestates_get_slot();
    Header.rerecord_count = 0;
    for (i = 0; i < 4; i++) {
        if (Controls[i].Present) {
            Header.controllers ++;
            Header.controller_flags |= CONTROLLER_PRESENT << i;
            if (Controls[i].Plugin == PLUGIN_MEMPAK) {
                Header.controller_flags |= CONTROLLER_MEMPACK << i;
            }
            if (Controls[i].Plugin == PLUGIN_RUMBLE_PAK) {
                Header.controller_flags |= CONTROLLER_RUMBLEPACK << i;
            }
        }
    }
    Header.core_type = (short) dynacore; // was: reserved1
    if (fromSnapshot == 0) {
        Header.start_type = MOVIE_START_FROM_RESET;
    } else {
        Header.start_type = MOVIE_START_FROM_SNAPSHOT;
    }
    sprintf(Header.utf_authorname, "%s", authorUTF8);
    sprintf(Header.utf_moviedesc, "%s", descriptionUTF8);
    sprintf(Header.rom_name, "%s", ROM_HEADER->nom);
    Header.rom_crc = ROM_HEADER->CRC1;
    Header.rom_cc = (short) ROM_HEADER->Country_code;

    snprintf(Header.video_plugin, 64, "%s", getGfxName());
    snprintf(Header.input_plugin, 64, "%s", getInputName());
    snprintf(Header.sound_plugin, 64, "%s", getAudioName());
    snprintf(Header.rsp_plugin, 64, "%s", getRspName());
}

int SetupEmulationState()
{
    if (Header.signature != MUP_MAGIC) {
        main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Invalid signature in header file."));
    	return 0;
    }
    
    if (Header.version_number != 3) {
        main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Invalid version number: %i"), Header.version_number);
    }
    
    printf("Movie UID: %i\n",Header.movie_uid);
    printf("Total VI's: %i\n",Header.total_vi);
    printf("Rerecord Count: %i\n",Header.rerecord_count);
    printf("FPS: %i\n",Header.fps);
    printf("Controllers: %i\n",Header.controllers);
    // todo: enable this many controllers, check force_manual
    printf("Input Samples: %i\n",Header.input_samples);
    l_TotalSamples = Header.input_samples;
    printf("Start Type: %s\n", (Header.start_type == 1 ? "Savestate" : "Start"));
    if (Header.start_type == 1) {
        // TODO: look for .st of the same file name
    }
    //TODO: check  020 4-byte unsigned int: controller flags
    
    printf("ROM Name: %s\n",Header.rom_name);
    printf("ROM CRC: %i\n",Header.rom_crc);
    printf("ROM CC: %i\n",Header.rom_cc);
    
    //TODO: plugin checking
    
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

		if(!x && !y) {
			strcpy(l_InputDisplay, "");
		} else {
			int xamt = (x<0?-x:x) * 99/127; if(!xamt && x) xamt = 1;
			int yamt = (y<0?-y:y) * 99/127; if(!yamt && y) yamt = 1;
			if(x && y) {
				sprintf(l_InputDisplay, "%c%d %c%d ", x<0?'<':'>', xamt, y<0?'v':'^', yamt);
			} else if(x) {
				sprintf(l_InputDisplay, "%c%d ", x<0?'<':'>', xamt);
			} else { //if(y)
				sprintf(l_InputDisplay, "%c%d ", y<0?'v':'^', yamt);
			}
		}

		if(s) strcat(l_InputDisplay, "S");
		if(z) strcat(l_InputDisplay, "Z");
		if(a) strcat(l_InputDisplay, "A");
		if(b) strcat(l_InputDisplay, "B");
		if(l) strcat(l_InputDisplay, "L");
		if(r) strcat(l_InputDisplay, "R");
		if(cu||cd||cl||cr) {
			strcat(l_InputDisplay, " C");
			if(cu) strcat(l_InputDisplay, "^");
			if(cd) strcat(l_InputDisplay, "v");
			if(cl) strcat(l_InputDisplay, "<");
			if(cr) strcat(l_InputDisplay, ">");
		}
		if(du||dd||dl||dr) {
			strcat(l_InputDisplay, " D");
			if(du) strcat(l_InputDisplay, "^");
			if(dd) strcat(l_InputDisplay, "v");
			if(dl) strcat(l_InputDisplay, "<");
			if(dr) strcat(l_InputDisplay, ">");
		}
	}
}

void _StartROM()
{
	l_CurrentSample = 0;
	l_CurrentVI = 0;
	g_UseSaveData = 1;
}

void EndPlaybackAndRecording()
{
	g_UseSaveData = 1;
	if (g_EmulatorRecording) {
	    savestates_select_filename(progress_file);
        savestates_job |= SAVESTATE; // Save a in progress savestate for easy resume.
        Header.total_vi = (unsigned int) l_CurrentVI;
        Header.rerecord_count ++;   // TODO: Needs some work ...
        Header.fps = fpsByCountrycode();
        Header.input_samples = (unsigned int) l_TotalSamples;
	    fseek(RecordingFile, 0L, SEEK_SET);
	    fwrite(&Header, 1, MUP_HEADER_SIZE, RecordingFile);
	    fseek(RecordingFile, 0L, SEEK_END);
	    fclose(RecordingFile);
    	g_EmulatorRecording = 0;
        main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Recording Ended."));
	    l_TotalSamples = 0;
	    l_CurrentSample = 0;
	    l_CurrentVI = 0;
	}
	if (g_EmulatorPlayback) {
	    fclose(PlaybackFile);
    	g_EmulatorPlayback = 0;
        main_message(1, 1, 1, OSD_BOTTOM_LEFT, tr("Playback Ended."));
    	if (g_ReadOnlyPlayback == 0) {
    	    BeginRecording(playback_file, Header.start_type, Header.utf_authorname, Header.utf_moviedesc);
	        l_TotalSamples = Header.input_samples;
	        l_CurrentSample = 0; // TODO
	        l_CurrentVI = Header.total_vi;
    	} else {
	        l_TotalSamples = 0;
	        l_CurrentSample = 0;
	        l_CurrentVI = 0;
    	}
	}
}

void _NewVI()
{
	l_CurrentVI++;
}

void _GetKeys( int Control, BUTTONS *Keys )
{
	// Since we handle input here now, we always want to start with getKeys.
    getKeys(Control, Keys);
    
    if (g_EmulatorPlayback) {
        // hack: assume only 1 controller
        if (Control == 0) {
            int length = fread(Keys,sizeof(BUTTONS),1,PlaybackFile);
            if (length == 0) {
        		EndPlaybackAndRecording();
            }
        } else {
            memset(Keys,0,sizeof(BUTTONS));
        }
        
        // it doesn't matter if there isn't 4 controllers plugged in, 
        // our sample stops at this point.
        // TODO: Not working properly.
        // Handled by the 'if (length == 0)' above ...
        if (Control == 3) {
        	l_CurrentSample++;
        	if (l_CurrentSample > l_TotalSamples) {
        		EndPlaybackAndRecording();
        	}
        }
        // read keys from file.
    }
    
    if (g_EmulatorRecording) {
       fwrite(Keys,sizeof(BUTTONS),1,RecordingFile);
    }
    
    // print out the data of the controller input
    if (Control == 0) {
        memcpy(&l_LastInput,&Keys->Value,sizeof(int));
        InputToString();
        if ((Keys->Value != 0) && (g_EmulatorPlayback || g_EmulatorRecording)) {
            l_TotalSamples ++;
            main_message(1, 1, 1, OSD_TOP_LEFT, "%s", l_InputDisplay);
        }
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

char* getCtrlStrInternal(int controller)
{
    char *result;
  	result = malloc(32);
    
    if (Controls[controller].Present) {
        sprintf(result,"Present");
        if(Controls[controller].Plugin == PLUGIN_MEMPAK) {
            strcat(result, " with mempak");
        }
        if(Controls[controller].Plugin == PLUGIN_RUMBLE_PAK) {
            strcat(result, " with rumble");
        }
    } else {
        sprintf(result,"Disconnected");
    }

    return result;
}

char* getCtrlStrHeader(int controller, unsigned int controller_flags)
{
    char *result;
  	result = malloc(32);
  	
    if (controller_flags & (CONTROLLER_PRESENT << controller)) { 
        sprintf(result,"Present");
        if (controller_flags & (CONTROLLER_MEMPACK << controller)) {
            strcat(result," with mempak");
        }
        if (controller_flags & (CONTROLLER_RUMBLEPACK << controller)) {
            strcat(result," with rumble pak");
        }
    } else {
        sprintf(result,"Disconnected");
    }

    return result;
}

char* getGfxName()
{
    char *result;
    result = plugin_name_by_filename(config_get_string("Gfx Plugin", ""));
    return result;
}

char* getInputName()
{
    char *result;
    result = plugin_name_by_filename(config_get_string("Input Plugin", ""));
    return result;
}

char* getAudioName()
{
    char *result;
    result = plugin_name_by_filename(config_get_string("Audio Plugin", ""));
    return result;
}

char* getRspName()
{
    char *result;
    result = plugin_name_by_filename(config_get_string("RSP Plugin", ""));
    return result;
}

int fpsByCountrycode()
{
    switch(ROM_HEADER->Country_code&0xFF)
    {
    case 0x44:
    case 0x46:
    case 0x49:
    case 0x50:
    case 0x53:
    case 0x55:
    case 0x58:
    case 0x59:
        return 25;
        break;

    case 0x37:
    case 0x41:
    case 0x45:
    case 0x4a:
        return 30;
        break;
    }

    printf( "[VCR]: Warning - unknown country code, using 30 FPS for video.\n" );
    return 30;
}

