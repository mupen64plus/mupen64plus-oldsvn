/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Copyright (C) 2009 icepir8                                            *
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

#ifndef _CRC_UCODE_
#define _CRC_UCODE_

#define _u64   unsigned __int64   /* 64 bit */
#define _u32   unsigned __int32   /* 32 bit */
#define _u16   unsigned __int16   /* 16 bit */
#define _u8    unsigned __int8    /*  8 bit */


typedef struct MicrocodeCRCEntry {
    _u32        crc;
    _u32        microcode;
} MicrocodeCRCEntry;


typedef struct {
    _u32    type;
    _u32    flags;

    _u32    ucode_boot;         //Mem Pointer
    _u32    ucode_boot_size;

    _u32    ucode;              //Mem Pointer
    _u32    ucode_size;

    _u32    ucode_data;         //Mem Pointer
    _u32    ucode_data_size;

    _u32    dram_stack;         //Mem Pointer
    _u32    dram_stack_size;

    _u32    output_buff;        //Mem Pointer
    _u32    output_buff_size;   //Mem Pointer

    _u32    data_ptr;           //Mem Pointer
    _u32    data_size;

    _u32    yield_data_ptr;     //Mem Pointer
    _u32    yield_data_size;

} OSTask_t;

typedef struct 
{       
        _u16 validation;       /* 0x00 */
        _u8  compression;      /* 0x02 */
        _u8  unknown1;         /* 0x03 */
        _u32  clockrate;        /* 0x04 */
        _u32  programcounter;   /* 0x08 */
        _u32  release;          /* 0x0c */
        
        _u32  crc1;             /* 0x10 */
        _u32  crc2;             /* 0x14 */
        _u64 unknown2;         /* 0x18 */
        
        _u8  name[20];         /* 0x20 - 0x33 */
        
        _u8  unknown3;         /* 0x34 */
        _u8  unknown4;         /* 0x35 */
        _u8  unknown5;         /* 0x36 */
        _u8  unknown6;         /* 0x37 */
        _u8  unknown7;         /* 0x38 */
        _u8  unknown8;         /* 0x39 */
        _u8  unknown9;         /* 0x3a */
        _u8  manufacturerid;   /* 0x3b */
        _u16 cartridgeid;      /* 0x3c */
        _u8  countrycode;      /* 0x3e */
        _u8  unknown10;        /* 0x3f */
} t_romheader;

extern int AutodetectUCode();
extern void AutodetectLoadCRCTable();
extern void GetEmuDir( char * Directory );

#endif