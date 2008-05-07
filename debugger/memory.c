/***************************************************************************
    debugger/memory.c  -  handle debugger memory access in Mupen64Plus
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

#include "memory.h"

void update_memory(void){
  int i;
  for(i=0; i<0x10000; i++)
    get_memory_flags(i*0x10000);
}

uint32 read_memory_32(uint32 addr){
  switch(get_memory_type(addr))
    {
    case MEM_RDRAM:
      return *((uint32 *)(rdramb + (addr & 0xFFFFFF)));
    case MEM_RSPMEM:
      if ((addr & 0xFFFF) < 0x1000)
	return *((uint32 *)(SP_DMEMb + (addr&0xFFF)));
      else if ((addr&0xFFFF) < 0x2000)
	return *((uint32 *)(SP_IMEMb + (addr&0xFFF)));
      else
	return MEM_INVALID;
    case MEM_ROM:
      return *((uint32 *)(rom + (addr & 0x03FFFFFF)));
    default:
      return MEM_INVALID;
    }
}

void write_memory_32(uint32 addr, uint32 value){
  switch(get_memory_type(addr))
    {
    case MEM_RDRAM:
      *((uint32 *)(rdramb + (addr & 0xFFFFFF))) = value;
      break;
    }
}

uint32 get_memory_flags(uint32 addr){
  int type=get_memory_type(addr);
  uint32 flags = 0;

  switch(type)
    {
    case MEM_RDRAM:
    case MEM_RSPMEM:
    case MEM_ROM:
      flags|=MEM_FLAG_READABLE;
    }

  switch(type)
    {
    case MEM_RDRAM:
      flags|=MEM_FLAG_WRITABLE;
    }

  return flags;
}

int get_memory_type(uint32 addr){
  if(readmem[addr >> 16] == read_nomem)
    return MEM_NOMEM;
  else if(readmem[addr >> 16] == read_nothing)
    return MEM_NOTHING;
  else if(readmem[addr >> 16] == read_rdram)
    return MEM_RDRAM;
  else if(readmem[addr >> 16] == read_rdramreg)
    return MEM_RDRAMREG;
  else if(readmem[addr >> 16] == read_rsp_mem)
    return MEM_RSPMEM;
  else if(readmem[addr >> 16] == read_rsp_reg)
    return MEM_RSPREG;
  else if(readmem[addr >> 16] == read_rsp)
    return MEM_RSP;
  else if(readmem[addr >> 16] == read_dp)
    return MEM_DP;
  else if(readmem[addr >> 16] == read_dps)
    return MEM_DPS;
  else if(readmem[addr >> 16] == read_vi)
    return MEM_VI;
  else if(readmem[addr >> 16] == read_ai)
    return MEM_AI;
  else if(readmem[addr >> 16] == read_pi)
    return MEM_PI;
  else if(readmem[addr >> 16] == read_ri)
    return MEM_RI;
  else if(readmem[addr >> 16] == read_si)
    return MEM_SI;
  else if(readmem[addr >> 16] == read_flashram_status)
    return MEM_FLASHRAMSTAT;
  else if(readmem[addr >> 16] == read_rom)
    return MEM_ROM;
  else if(readmem[addr >> 16] == read_pif)
    return MEM_PIF;
  else if(readmem[addr >> 16] == read_mi)
    return MEM_MI;
  else
    printf("Unknown memory type in get_memory_type: %x\n", readmem[addr>>16]);
}
