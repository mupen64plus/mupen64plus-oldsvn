/**
 * Mupen64 - rom.h
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
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

#ifndef ROM_H
#define ROM_H

#include "7zip/7zMain.h"

int open_rom(const char* filename, unsigned int archivefile);
int close_rom(void);
unsigned char* load_single_rom(const char* filename, int* romsize, unsigned short* compressiontype, int* loadlength);
unsigned char* load_archive_rom(const char* filename, int* romsize, unsigned short* compressiontype, int* loadlength, unsigned int* archivefile, UInt32* blockIndex, Byte** outBuffer, size_t* outBufferSize, CFileInStream* archiveStream, CArchiveDatabaseEx* db);
void swap_rom(unsigned char* localrom, unsigned short *imagetype, int loadlength);

extern unsigned char* rom;
extern int taille_rom;

typedef struct _rom_header
{
   unsigned char init_PI_BSB_DOM1_LAT_REG; //0x00
   unsigned char init_PI_BSB_DOM1_PGS_REG; //0x01
   unsigned char init_PI_BSB_DOM1_PWD_REG; //0x02
   unsigned char init_PI_BSB_DOM1_PGS_REG2; //0x03
   unsigned int ClockRate; //0x04
   unsigned int PC; //0x08
   unsigned int Release; //0x0C
   unsigned int CRC1; //0x10
   unsigned int CRC2; //0x14
   unsigned int Unknown[2]; //0x18
   unsigned char nom[20]; //0x20
   unsigned int unknown;  //0x34
   unsigned int Manufacturer_ID; //0x38
   unsigned short Cartridge_ID;  //0x3C //Game serial number
   unsigned short Country_code;  //0x3E //Possible byte alignment padding here???
   unsigned int Boot_Code[1008]; //0x40
} rom_header;

extern rom_header* ROM_HEADER;

typedef struct _rom_settings
{
   char goodname[256];
   int eeprom_16kb;
   char MD5[33];
} rom_settings;
extern rom_settings ROM_SETTINGS;

//Supported rom compressiontypes.
enum 
{
    UNCOMPRESSED,
    ZIP_COMPRESSION,
    GZIP_COMPRESSION,
    BZIP2_COMPRESSION,
    LZMA_COMPRESSION,
    SZIP_COMPRESSION
};

//Supported rom image types.
enum 
{
    Z64IMAGE,
    V64IMAGE,
    N64IMAGE
};

enum
{
    CIC_NUS_6101,
    CIC_NUS_6102,
    CIC_NUS_6103,
    CIC_NUS_6105,
    CIC_NUS_6106
};

enum
{
    EEPROM_4KB,
    EEPROM_16KB,
    SRAM,
    FLASH_RAM,
    CONTROLLER_PACK,
    NONE
};

#endif

