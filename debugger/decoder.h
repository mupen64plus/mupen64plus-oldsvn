/*
 * Mupen64Plus - debugger/decoder.h
 *
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64 homepage: http://code.google.com/p/mupen64plus/
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

 
#ifndef DECODER_H
#define DECODER_H

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "opprintf.h"

void r4300_decode_op( uint32 instr, char *op, char *args, int pc );
void mr4kd_disassemble ( uint32 instruction, uint32 counter, char * buffer );

#endif //DECODER_H
