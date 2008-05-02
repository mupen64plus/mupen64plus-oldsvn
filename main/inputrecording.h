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

extern int g_UseSaveData;

int BeginPlayback( char *sz_filename );
void _GetKeys( int Control, BUTTONS *Keys );
void EndPlaybackAndRecording();
void CleanUpSaveFiles();

typedef struct {
	char signature[4];
    unsigned int version_number;
    int movie_uid;
    unsigned int total_vi;
    unsigned int rerecord_count;
    char fps;
    char controllers;
    short core_type; // was: reserved1
    unsigned int input_samples;
    unsigned int start_type;
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
