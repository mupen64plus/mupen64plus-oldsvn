/*
 * debugger/x86_64/host_decoder.c
 * 
 * Debugger for Mupen64 - davFr
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64 is copyrighted (C) 2002 Hacktarux
 * Mupen64 homepage: http://mupen64.emulation64.com
 *         email address: hacktarux@yahoo.fr
 * 
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence.
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

#include "host_decoder.h"




static unsigned char *opaddr;
static char *op;
static char *args;
static int numbytes;
static int bitmod;

#define ARCH_PTR    (sizeof(void*) * 8)


static char regs[][4] = {"EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI"};

static void set_op(char * opstr) {
    switch(numbytes)
    {
    case 1:
      sprintf(op, "[%02x] %s", *(opaddr-1), opstr);
      break;
    case 2:
      sprintf(op, "[%02x%02x] %s", *(opaddr-2), *(opaddr-1), opstr);
      break;
    case 3:
      sprintf(op, "[%02x%02x%02x] %s", *(opaddr-3), *(opaddr-2), *(opaddr-1), opstr);
      break;
    default:
      sprintf(op, "[error] %s", opstr);
    
    }
}

static void RESERV(){
    set_op("RESERV");
    sprintf(args, "Instruction Unknown");
}

int pow2(int exp){
  int i;
  int res=1;
  for(i=0; i<exp; i++)
    res*=2;
  return res;
}

static int decode_mod(char *buff) {
    int disp=0;
    numbytes++;
    if(((*opaddr) & 0xC0) == 0xC0) //register reference
      {
	sprintf(buff, "%s", regs[*opaddr & 0x7]);
	opaddr++;
	return 0;
      }
    else
      {// memory offset
	if((*opaddr & 0x80) == 0x80)
	  disp=4;
	else if((*opaddr & 0x40) == 0x40)
	  disp=1;
        if((*opaddr & 0xC7) == 5)
	  {
	    sprintf(buff, "[0x%08x]", *((uint32*)(opaddr + 1)));
	    opaddr++;
	    return 4;
	  }
	else if((*opaddr & 0x07)==4)
	  {
	    //DECODE SIB
	    opaddr++;
	    numbytes++;
	    if((*opaddr & 0x07)==5)
	      {
		sprintf(buff, "[%s*%x+0x%08x]", regs[((*(opaddr))>>3)&0x7], pow2((*opaddr)>>6), *((uint32*)(opaddr+1)));
		opaddr++;
		return 4;
	      }
	    else
	      RESERV();
	  }
	else
	  {
	    switch(disp)
	      {
	      case 0:
		sprintf(buff, "[%s]", regs[((*opaddr)>>0) & 0x7]);
		break;
	      case 1:
		sprintf(buff, "[%s+0x%02x]", regs[((*opaddr)>>0) & 0x7], *(opaddr + 1));
		break;
	      case 4:
		sprintf(buff, "[%s+0x%08x]", regs[((*opaddr)>>0) & 0x7], *((uint32*)(opaddr+1)));
	      }
	    opaddr++;
	    return disp;
	  }
      }
}

static void JE_NEAR(){
    set_op("JE");
    numbytes+=4;
    sprintf(args, "[0x%08x]", *((uint32*)opaddr));
}

void decode_second()
{
    numbytes++;
    switch(*(opaddr++))
    {
    case 0x84: JE_NEAR();            break;
    default:   RESERV();
    }
}

///////////////////////////////////////////////////////////////////////////////

static void MOV_EAX_MEM(){
    set_op("MOV");
    if(ARCH_PTR==32) {
      numbytes+=4;
      sprintf(args, "EAX, [0x%08X]", *((uint32*)opaddr));
    }
    else {
      numbytes+=8;
      sprintf(args, "EAX, [0x%016X]", *((uint64*)opaddr));
    }
}

static void MOV_REG_IMM(){
    set_op("MOV");
    switch(bitmod)
    {
    case 32:
        numbytes+=4;
        sprintf(args, "%s, 0x%08x", regs[(*(opaddr-1))&0x7],*((uint32*)opaddr));
        break;
    case 64:
        numbytes+=8;
        sprintf(args, "%s, 0x%016x", regs[(*(opaddr-1))&0x7],*((uint64*)opaddr));
        break;
    }
}

static void MOV_MEM_IMM(){
    char modbyte[256];
    int modsize = decode_mod(modbyte);
    set_op("MOV");
    numbytes+=modsize;
    switch(bitmod)
    {
    case 32:
        numbytes+=4;   
        sprintf(args, "%s, 0x%08x", modbyte, *((uint32*)(opaddr+modsize)));
        break;
    case 64:
        numbytes+=8;
	sprintf(args, "%s, 0x%016x", modbyte, *((uint64*)(opaddr+modsize)));
        break;
    }
}

static void CMP_REG_MRM(){
    char modbyte[256];
    int modsize = decode_mod(modbyte);
    set_op("CMP");
    numbytes+=modsize;
    sprintf(args, "%s, %s", regs[((*opaddr)>>3)&0x7], modbyte);
}

static void CMP_EAX_IMM(){
    set_op("CMP");
    numbytes+=4;
    sprintf(args, "EAX, 0x%08X", *((uint32*)opaddr));
}

static void AND_EAX_IMM(){
    set_op("AND");
    numbytes+=4;
    sprintf(args, "EAX, 0x%08X", *((uint32*)opaddr));
}

static void ADD_EAX_IMM(){
    set_op("ADD");
    numbytes+=4;
    sprintf(args, "EAX, 0x%08X", *((uint32*)opaddr));
}

static void INT_IMM8(){
    numbytes++;
    set_op("INT");
    sprintf(args, "0x%02x", *opaddr);
}

static void JNZ_REL(){
    set_op("JNZ");
    numbytes++;
    if(ARCH_PTR==32)
      sprintf(args, "0x%08x",((uint32)(opaddr+1+(*opaddr))));
    else
      sprintf(args, "0x%016x",((uint64)(opaddr+1+(*opaddr))));
}

static void JZ_REL(){
    set_op("JZ");
    numbytes++;
    if(ARCH_PTR==32)
      sprintf(args, "0x%08x",((uint32)(opaddr+1+(*opaddr))));
    else
      sprintf(args, "0x%016x",((uint64)(opaddr+1+(*opaddr))));
}

static void JMP_REL(){
    set_op("JMP");
    numbytes++;
    if(ARCH_PTR==32)
      sprintf(args, "0x%08x",((uint32)(opaddr+1+(*opaddr))));
    else
      sprintf(args, "0x%016x",((uint64)(opaddr+1+(*opaddr))));
}

static void MOV_MRM_REG(){
    char modbyte[256];
    int modsize=decode_mod(modbyte);
    set_op("MOV");
    numbytes+=modsize;

    if(bitmod==32)
      {
	sprintf(args, "%s, %s", modbyte, regs[((*opaddr)>>3)&0x7]);
      }
    else
	RESERV();
}

static void MOV_MRM_MRM(){
    char modbyte[256];

    int modsize=decode_mod(modbyte);

    set_op("MOV");

    numbytes+=modsize;
    sprintf(args, "%s, %s", regs[((*(opaddr-1))>>3&0x7)], modbyte);//regs[((*opaddr)>>3)&0x7]);

}

static void LOGIC_MRM_IMM() {
    char modbyte[256];

    int modsize=decode_mod(modbyte);

    switch(*(opaddr-1) & 0x38)
      {
      case 0x00: set_op("ADD"); break;
      case 0x08: set_op("OR");  break;
      case 0x10: set_op("ADC"); break;
      case 0x18: set_op("SBB"); break;
      case 0x20: set_op("AND"); break;
      case 0x28: set_op("SUB"); break;
      case 0x30: set_op("XOR"); break;
      case 0x38: set_op("CMP"); break;
      }
    if(bitmod==32) {
      sprintf(args, "%s, 0x%08x",modbyte, *(uint32*)(opaddr+modsize));
      numbytes+=(4+modsize);
    }
    else
      RESERV();
}

static void BSH_C1_MRM(){
  char modbyte[256];

  int modsize=decode_mod(modbyte);

  switch(*(opaddr-1) & 0x38)
    {
    case 0x00: set_op("ROL"); break;
    case 0x08: set_op("ROR"); break;
    case 0x10: set_op("RCL"); break;
    case 0x18: set_op("RCR"); break;
    case 0x20: set_op("SHL"); break;
    case 0x28: set_op("SHR"); break;
    case 0x30: set_op("SAL"); break;
    case 0x38: set_op("SAR"); break;
    }
  sprintf(args, "%s, 0x%02x",modbyte,(*(opaddr+modsize)));
  numbytes+=1+modsize;
}

static void INC_DEC_CALL(){
  char modbyte[256];

  int modsize=decode_mod(modbyte);

  switch(*(opaddr-1) & 0x38)
    {
    case 0x10://Call Reg
      //      sprintf(args, "%s",regs[(*opaddr++)&0x7]);
      set_op("CALL");
      break;
    default:
      opaddr++;
      RESERV();
    }
  sprintf(args, "%s", modbyte);
  numbytes+=modsize;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[ DECODE_OP ]=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[//

decode_prefix_or_first()
{
    numbytes++;
    switch(*(opaddr++))
    {
    case 0x05: ADD_EAX_IMM();   break;
    case 0x0F: decode_second(); break;
    case 0x25: AND_EAX_IMM();   break;
    case 0x3B: CMP_REG_MRM();   break;
    case 0x3D: CMP_EAX_IMM();   break;
    case 0x48: //64 bit prefix op
        if(numbytes==1)
            {
            bitmod=64;
            decode_prefix_or_first();   
            }
        else
            RESERV();
        break;
    case 0x74: JZ_REL();        break;
    case 0x75: JNZ_REL();       break;
    case 0x81: LOGIC_MRM_IMM();     break;
    case 0x89: MOV_MRM_REG();   break;
    case 0x8B: MOV_MRM_MRM();   break;
    case 0xA1: MOV_EAX_MEM(); break;

    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF: MOV_REG_IMM();   break;
    case 0xC1: BSH_C1_MRM();    break;
    case 0xC7: MOV_MEM_IMM();   break;
    case 0xCD: INT_IMM8();      break;
    case 0xEB: JMP_REL();       break;
    case 0xFF: INC_DEC_CALL();  break;
    default:   RESERV();
    }
}


int host_decode_op( unsigned char* addr, char *opcode, char *arguments )
{
    char   buff[256], code[256];
    numbytes = 0;
    opaddr = addr;
    op = opcode;
    args = arguments;
    bitmod=32;
    int i;
    
    decode_prefix_or_first();

    return numbytes;
}
