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

#include "../r4300/ops.h"

#if !defined(NO_ASM) && (defined(__i386__) || defined(__x86_64__))

#include <dis-asm.h>
#include <stdarg.h>

int  lines_recompiled;
uint32 addr_recompiled;
int  num_decoded;

char opcode_recompiled[564][MAX_DISASSEMBLY];
char args_recompiled[564][MAX_DISASSEMBLY];
int  opaddr_recompiled[564];

disassemble_info dis_info;

void process_opcode_out(void *strm, const char *fmt, ...){
  va_list ap = {0};
  va_start(ap, fmt);
  char *arg;
  char buff[256];

  if(num_decoded==0)
    {
      if(strcmp(fmt,"%s")==0)
	{
	  arg = va_arg(ap, char*);
	  strcpy(opcode_recompiled[lines_recompiled],arg);
	}
      else
	strcpy(opcode_recompiled[lines_recompiled],"OPCODE-X");
      num_decoded++;
      *(args_recompiled[lines_recompiled])=0;
    }
  else
    {
      vsprintf(buff, fmt, ap);
      sprintf(args_recompiled[lines_recompiled],"%s%s", args_recompiled[lines_recompiled],buff);
    }
  va_end(ap);
}

// Callback function that will be called by libopcodes to read the 
// bytes to disassemble ('read_memory_func' member of 'disassemble_info').
int read_memory_func(bfd_vma memaddr, bfd_byte *myaddr, 
                            unsigned int length, disassemble_info *info) {
  char* from = (char*)(memaddr);
  char* to =   (char*)myaddr;
  
  while (length-- != 0) {
    *to++ = *from++;
  }
  return (0);
}

void init_host_disassembler(void){

  dis_info.fprintf_func = process_opcode_out;
  dis_info.bytes_per_line=40;
  dis_info.endian = 1;
  dis_info.mach = bfd_mach_i386_i8086;
  dis_info.disassembler_options = (char*) "i386,suffix";
  dis_info.read_memory_func = read_memory_func;
}

void decode_recompiled(uint32 addr)
{
    unsigned char *assemb, *end_addr;
    unsigned char *as_inc;

    lines_recompiled=0;

    if(blocks[addr>>12] == NULL)
        return;

    if(blocks[addr>>12]->block[(addr&0xFFF)/4].ops == NOTCOMPILED)
    //      recompile_block((int *) SP_DMEM, blocks[addr>>12], addr);
      {
	strcpy(opcode_recompiled[0],"NOTCOMPILED");
	strcpy(args_recompiled[0],"NOTCOMPILED");
	opaddr_recompiled[0]=0;
	addr_recompiled=0;
	lines_recompiled++;
	return;
      }

    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) == 0xFFF)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;

    for(as_inc=assemb; as_inc<end_addr; as_inc++)
      printf("%02x", *as_inc);
    while(assemb < end_addr)
      {
        opaddr_recompiled[lines_recompiled] = (uint32)assemb;
	num_decoded=0;
	assemb += print_insn_i386((uint64)assemb, &dis_info);
        //assemb += host_decode_op(assemb, opcode_recompiled[lines_recompiled], 
        //                         args_recompiled[lines_recompiled]);
        lines_recompiled++;
      }
    addr_recompiled=addr;
    printf("\n");
}

char* get_recompiled_opcode(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opcode_recompiled[index];
    else
        return NULL;
}

char* get_recompiled_args(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return args_recompiled[index];
    else
        return NULL;
}

int get_recompiled_addr(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opaddr_recompiled[index];
    else
        return 0;
}

int get_num_recompiled(uint32 addr)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    return lines_recompiled;
}

#else

char* get_recompiled(uint32 addr, int index)
{
    return NULL;
}

int get_num_recompiled(uint32 addr)
{
    return 0;
}

void init_host_disassembler(void)
{

}

#endif


void update_memory(void){
  int i;
  for(i=0; i<0x10000; i++)
    get_memory_flags(i*0x10000);
}

uint32 read_memory_32(uint32 addr){
  switch(get_memory_type(addr))
    {
    case MEM_NOMEM:
      if(tlb_LUT_r[addr>>12])
	return read_memory_32((tlb_LUT_r[addr>>12]&0xFFFFF000)|(addr&0xFFF));
      return MEM_INVALID;
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
    case MEM_NOMEM:
      if(tlb_LUT_r[addr>>12])
	flags|=MEM_FLAG_READABLE;
      break;
    case MEM_RSPMEM:
      if((addr & 0xFFFF) < 0x2000)
	flags|=MEM_FLAG_READABLE;
      break;
    case MEM_RDRAM:
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

