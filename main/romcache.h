/**
 * Mupen64 - cheat.c
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

typedef struct {
    char MD5[33];
    char goodname[256];
    char path[PATH_MAX];
    unsigned long os_timestamp;
} cache_entry;

typedef struct {
	char MD5[33];
	char goodname[256];
	char path[PATH_MAX];
	unsigned int crc1;
    unsigned int crc2;
    char headername[20];
} rom_info;

int load_initial_cache();
void *rom_cache_system(void *_arg);
