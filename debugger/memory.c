/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memory.c                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 DarkJeztr                                          *
 *   Copyright (C) 2002 Blight                                             *
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

#include "memory.h"

#include "../r4300/r4300.h"
#include "../r4300/ops.h"

#if !defined(NO_ASM) && (defined(__i386__) || defined(__x86_64__))

#include <dis-asm.h>
#include <stdarg.h>

int  lines_recompiled;
uint32 addr_recompiled;
int  num_decoded;

char opcode_recompiled[564][MAX_DISASSEMBLY];
char args_recompiled[564][MAX_DISASSEMBLY*4];
void *opaddr_recompiled[564];

disassemble_info dis_info;

#define CHECK_MEM(address) \
   if (!invalid_code[(address) >> 12] && blocks[(address) >> 12]->block[((address) & 0xFFF) / 4].ops != NOTCOMPILED) \
     invalid_code[(address) >> 12] = 1;

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
      sprintf(args_recompiled[lines_recompiled],"%s%s", 
          args_recompiled[lines_recompiled],buff);
    }
  va_end(ap);
}

// Callback function that will be called by libopcodes to read the 
// bytes to disassemble ('read_memory_func' member of 'disassemble_info').
int read_memory_func(bfd_vma memaddr, bfd_byte *myaddr, 
                            unsigned int length, disassemble_info *info) {
  char* from = (char*)(long)(memaddr);
  char* to =   (char*)myaddr;
  
  while (length-- != 0) {
    *to++ = *from++;
  }
  return (0);
}

void init_host_disassembler(void){


  INIT_DISASSEMBLE_INFO(dis_info, stderr, process_opcode_out);
  dis_info.fprintf_func = (fprintf_ftype) process_opcode_out;
  dis_info.stream = stderr;
  dis_info.bytes_per_line=1;
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
    opaddr_recompiled[0] = (void *) 0;
    addr_recompiled=0;
    lines_recompiled++;
    return;
      }

    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) >= 0xFFC)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;

    //for(as_inc=assemb; as_inc<end_addr; as_inc++)
    //  printf("%02x", *as_inc);

    while(assemb < end_addr)
      {
        opaddr_recompiled[lines_recompiled] = assemb;
        num_decoded=0;

        assemb += print_insn_i386((bfd_vma)(long) assemb, &dis_info);

        lines_recompiled++;
      }

    addr_recompiled = addr;
    //printf("\n");
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

void * get_recompiled_addr(uint32 addr, int index)
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

int get_has_recompiled(uint32 addr)
{
    unsigned char *assemb, *end_addr;

    if(!dynacore || blocks[addr>>12] == NULL)
        return FALSE;

    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) >= 0xFFC)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;
    if(assemb==end_addr)
      return FALSE;

    return TRUE;
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

uint64 read_memory_64(uint32 addr)
{
    return ((uint64)read_memory_32(addr) << 32) | (uint64)read_memory_32(addr + 4);
}

uint64 read_memory_64_unaligned(uint32 addr)
{
    uint64 w[2];
    
    w[0] = read_memory_32_unaligned(addr);
    w[1] = read_memory_32_unaligned(addr + 4);
    return (w[0] << 32) | w[1];
}

void write_memory_64(uint32 addr, uint64 value)
{
    write_memory_32(addr, value >> 32);
    write_memory_32(addr + 4, value & 0xFFFFFFFF);
}

void write_memory_64_unaligned(uint32 addr, uint64 value)
{
    write_memory_32_unaligned(addr, value >> 32);
    write_memory_32_unaligned(addr + 4, value & 0xFFFFFFFF);
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

uint32 read_memory_32_unaligned(uint32 addr)
{
    uint8 i, b[4];
    
    for(i=0; i<4; i++) b[i] = read_memory_32(addr + i);
    return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

void write_memory_32(uint32 addr, uint32 value){
  switch(get_memory_type(addr))
    {
    case MEM_RDRAM:
      *((uint32 *)(rdramb + (addr & 0xFFFFFF))) = value;
      CHECK_MEM(addr)
      break;
    }
}

void write_memory_32_unaligned(uint32 addr, uint32 value)
{
    write_memory_8(addr + 3, value >> 24);
    write_memory_8(addr + 2, (value >> 16) & 0xFF);
    write_memory_8(addr + 1, (value >> 8) & 0xFF);
    write_memory_8(addr + 0, value & 0xFF);
}



//read_memory_16_unaligned and write_memory_16_unaligned don't exist because
//read_memory_16 and write_memory_16 work unaligned already.
uint16 read_memory_16(uint32 addr)
{
    return ((uint16)read_memory_8(addr) << 8) | (uint16)read_memory_8(addr+1); //cough cough hack hack
}

void write_memory_16(uint32 addr, uint16 value)
{
    write_memory_8(addr, value >> 8); //this isn't much better
    write_memory_8(addr + 1, value & 0xFF); //then again, it works unaligned
}

uint8 read_memory_8(uint32 addr)
{
    uint32 word;
    
    word = read_memory_32(addr & ~3);
    return (word >> ((3 - (addr & 3)) * 8)) & 0xFF;
}

void write_memory_8(uint32 addr, uint8 value)
{
    uint32 word, mask;
    
    word = read_memory_32(addr & ~3);
    mask = 0xFF << ((3 - (addr & 3)) * 8);
    word = (word & ~mask) | (value << ((3 - (addr & 3)) * 8));
    write_memory_32(addr & ~3, word);
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

