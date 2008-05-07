/***************************************************************************
    debugger/memory.h  -  handle debugger memory access in Mupen64Plus
                             -------------------
    begin                :  Wed 7 May 2008
    copyright            : (C) 2008 by DarkJezter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DEBUGGER_MEMORY_H__
#define __DEBUGGER_MEMORY_H__

#include "debugger.h"
#include "../memory/memory.h"

#define MEM_INVALID 0xFFFFFFFF

#define MEM_FLAG_READABLE    0x01
#define MEM_FLAG_WRITABLE    0x02

enum {
  MEM_NOMEM,
  MEM_NOTHING,
  MEM_RDRAM,
  MEM_RDRAMREG,
  MEM_RSPMEM,
  MEM_RSPREG,
  MEM_RSP,
  MEM_DP,
  MEM_DPS,
  MEM_VI,
  MEM_AI,
  MEM_PI,
  MEM_RI,
  MEM_SI,
  MEM_FLASHRAMSTAT,
  MEM_ROM,
  MEM_PIF,
  MEM_MI
};

extern void update_memory(void);

extern uint32 read_memory_32(uint32);
extern void write_memory_32(uint32, uint32);
extern uint32 get_memory_flags(uint32);
extern int get_memory_type(uint32);

#endif //__DEBUGGER_MEMORY_H__
