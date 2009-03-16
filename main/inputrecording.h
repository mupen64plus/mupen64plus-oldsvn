/**
 * Mupen64Plus - inputrecording.h
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

#ifndef __WIN32__
# include "winlnxdefs.h"
#else
# include <windows.h>
#endif

#include "plugin.h"

#define MOVIE_START_FROM_SNAPSHOT   (1)
#define MOVIE_START_FROM_RESET  	(2)

extern int g_UseSaveData;
extern unsigned int dynacore;

char* get_m64_filename();
void _StartROM();
int BeginPlayback(const char *sz_filename );
int ResumeRecording(const char *sz_filename );

int WriteEmulationState(int fromSnapshot, const char *authorUTF8, const char *descriptionUTF8);
int SetupEmulationState();
int BeginRecording(char *sz_filename, int fromSnapshot, const char *authorUTF8, const char *descriptionUTF8 );

void _GetKeys( int Control, BUTTONS *Keys );
void EndPlaybackAndRecording();
void CleanUpSaveFiles();
/*
 *  000 4-byte signature: 4D 36 34 1A "M64\x1A"
    004 4-byte little-endian unsigned int: version number, should be 3
    008 4-byte little-endian integer: movie "uid" - identifies the movie-savestate relationship,
               also used as the recording time in Unix epoch format
    00C 4-byte little-endian unsigned int: number of frames (vertical interrupts)
    010 4-byte little-endian unsigned int: rerecord count
    014 1-byte unsigned int: frames (vertical interrupts) per second
    015 1-byte unsigned int: number of controllers
    016 2-byte unsigned int: reserved, should be 0
    018 4-byte little-endian unsigned int: number of input samples for any controllers
    01C 2-byte unsigned int: movie start type
        value 1: movie begins from snapshot (the snapshot will be loaded from an external file
                                             with the movie's filename and a .st extension)
        value 2: movie begins from poweron
        other values: invalid movie
    01E 2-byte unsigned int: reserved, should be 0
    020 4-byte unsigned int: controller flags
        bit 0: controller 1 present
        bit 4: controller 1 has mempak
        bit 8: controller 1 has rumblepak
        +1..3 for controllers 2..4.
    024 160 bytes: reserved, should be 0
    0C4 32-byte ASCII string: internal name of ROM used when recording, directly from ROM
    0E4 4-byte unsigned int: CRC32 of ROM used when recording, directly from ROM
    0E8 2-byte unsigned int: country code of ROM used when recording, directly from ROM
    0EA 56 bytes: reserved, should be 0
    122 64-byte ASCII string: name of video plugin used when recording, directly from plugin
    162 64-byte ASCII string: name of sound plugin used when recording, directly from plugin
    1A2 64-byte ASCII string: name of input plugin used when recording, directly from plugin
    1E2 64-byte ASCII string: name of rsp plugin used when recording, directly from plugin
    222 222-byte UTF-8 string: author's name info
    300 256-byte UTF-8 string: author's movie description info
 * 
 */
typedef struct {
	int signature;
    unsigned int version_number;
    int movie_uid;
    unsigned int total_vi;
    unsigned int rerecord_count;
    char fps;
    char controllers;
    short core_type; // was: reserved1
    unsigned int input_samples;
    short start_type;
    short reserved2;
    unsigned int controller_flags;
    char reserved3[160];
    char rom_name[32];
    unsigned int rom_crc;
    short rom_cc;
    char reserved4[56];
    char video_plugin[64];
    char sound_plugin[64];
    char input_plugin[64];
    char rsp_plugin[64];
    char utf_authorname[222];
    char utf_moviedesc[256];  
} m64_header;

char* getCtrlStrInternal(int controller);
char* getCtrlStrHeader(int controller, unsigned int controller_flags);
char* getGfxName();
char* getInputName();
char* getAudioName();
char* getRspName();
int fpsByCountrycode();

