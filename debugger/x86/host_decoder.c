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

static char regs[][4] = {"EAX","EBX","ECX","EDX","EDI","ESI","EBP","ESP"};

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
	else
	  {
	    switch(disp)
	      {
	      case 0:
		sprintf(buff, "[%s]", regs[*opaddr & 0x7]);
		break;
	      case 1:
		sprintf(buff, "[%s+0x%02x]", regs[*opaddr & 0x7], *(opaddr + 1));
		break;
	      case 4:
		sprintf(buff, "[%s+0x%08x]", regs[*opaddr & 0x7], *((uint32*)(opaddr+1)));
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

static void MOV_EAX_MEM64(){
    set_op("MOV");
    numbytes+=8;
    sprintf(args, "EAX [0x%08x]", *((uint64*)opaddr));
}

static void MOV_REG_IMM(){
    set_op("MOV");
    switch(bitmod)
    {
    case 32:
        numbytes+=4;
        sprintf(args, "%s [0x%08x]", regs[(*(opaddr-1))&0x7],*((uint32*)opaddr));
        break;
    case 64:
        numbytes+=8;
        sprintf(args, "%s [0x%016x]", regs[(*(opaddr-1))&0x7],*((uint64*)opaddr));
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
        sprintf(args, "%s, 0x%08x", modbyte, *((uint32*)opaddr));
        break;
    case 64:
        numbytes+=8;
        sprintf(args, "%s, 0x%016x", modbyte, *((uint64*)opaddr));
        break;
    }
}

static void INT_IMM8(){
    numbytes++;
    set_op("INT");
    sprintf(args, "imm0x%02x", *opaddr);
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

static void LOGIC_MRM() {
    numbytes++;
    switch(*opaddr & 0xF8)
    {
    case 0xC0: // ADD with register
        if(bitmod==32)
	  {
            sprintf(args, "%s, %08x",regs[(*opaddr)&0x7], *(uint32*)(opaddr+1));
            opaddr++;
            set_op("ADD");
            numbytes+=4;
	  }
	else
	  {
	    RESERV();
	  }
        break;
    default:
        RESERV();
    }
}

static void INC_DEC_CALL(){
    numbytes++;
    switch(*opaddr & 0xF8)
    {
    case 0xD0://Call Reg
        set_op("CALL");
        sprintf(args, "%s",regs[(*opaddr++)&0x7]);
        break;
    default:
        RESERV();
    }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[ DECODE_OP ]=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[//

decode_prefix_or_first()
{
    numbytes++;
    switch(*(opaddr++))
    {
    case 0x0F: decode_second(); break;
    case 0x48: //64 bit prefix op
        if(numbytes==1)
            {
            bitmod=64;
            decode_prefix_or_first();   
            }
        else
            RESERV();
        break;
    case 0x81: LOGIC_MRM();     break;
    case 0x89: MOV_MRM_REG();   break;
    case 0xA1: MOV_EAX_MEM64(); break;

    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF: MOV_REG_IMM();   break;
    case 0xC7: MOV_MEM_IMM();   break;
    case 0xCD: INT_IMM8();      break;
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
