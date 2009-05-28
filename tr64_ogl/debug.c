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

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

static char *Mode[] = { "G_CCMUX_COMBINED" ,          "G_CCMUX_TEXEL0" ,
                        "G_CCMUX_TEXEL1" ,        "G_CCMUX_PRIMITIVE" ,
                        "G_CCMUX_SHADE" ,             "G_CCMUX_ENVIRONMENT" ,
                        "G_CCMUX_CENTER" ,        "G_CCMUX_COMBINED_ALPHA" ,
                        "G_CCMUX_TEXEL0_ALPHA" ,    "G_CCMUX_TEXEL1_ALPHA" ,
                        "G_CCMUX_PRIMITIVE_ALPHA" , "G_CCMUX_SHADE_ALPHA" ,
                        "G_CCMUX_ENV_ALPHA" ,         "G_CCMUX_LOD_FRACTION" ,
                        "G_CCMUX_PRIM_LOD_FRAC" ,   "G_CCMUX_K5" ,
                        "G_CCMUX_UNDEFINED" ,         "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,         "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,         "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,       "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,       "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,       "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,       "G_CCMUX_UNDEFINED" ,
                        "G_CCMUX_UNDEFINED" ,       "G_CCMUX_0" };

static char *Alpha[] = { "G_ACMUX_COMBINED" ,      "G_ACMUX_TEXEL0" ,
                         "G_ACMUX_TEXEL1" ,        "G_ACMUX_PRIMITIVE" , 
                         "G_ACMUX_SHADE" ,          "G_ACMUX_ENVIRONMENT" , 
                         "G_ACMUX_PRIM_LOD_FRAC" , "G_ACMUX_0"};




#define F_NAME "C:\\TR64\\PerfectDark_CamSpy.txt"

void DebugBox(char *fmt, ...)
{
/*  va_list ap;
    char text[1024];

    va_start(ap, fmt);
    sprintf(text, fmt, ap);
    va_end(ap);
    MessageBox(NULL, text, "DebugBox", MB_OK|MB_ICONSTOP);
*/
}

BOOL FirstTime = TRUE;

#define DLIST_TEST ((DList_C < 500) || (DList_C > 510))

FILE *stream;

extern int DList_C;
void LOG_TO_FILE(char *fmt, ...)
{
#ifdef LOG_ON
//  FILE *stream;
    va_list ap;

    if DLIST_TEST 
    {
        if (stream) 
        {
            fclose(stream);
            stream = NULL;
        }
        return;
    }
    
    if (!stream) stream = fopen(F_NAME,"w");
    
    va_start(ap, fmt);
//  if (FirstTime)
//      stream = fopen(F_NAME,"w");
//  else
//      stream = fopen(F_NAME,"at");
    vfprintf(stream, fmt, ap);
    va_end(ap);

    fprintf(stream, "%s\n", "");

//  fclose(stream);/**/

    FirstTime = FALSE;
#endif
}

void PRINT_RDP_MNEMONIC(char *fmt, ...)
    {
#ifdef LOG_ON
//      FILE *stream;
        va_list ap;

    if DLIST_TEST 
    {
        if (stream) 
        {
            fclose(stream);
            stream = NULL;
        }
        return;
    }
    
    if (!stream) stream = fopen(F_NAME,"at");
    
        va_start(ap, fmt);
//      stream = fopen(F_NAME,"at");
        vfprintf(stream, fmt, ap);
        va_end(ap);

//      fclose(stream);
#endif
    }



void PRINT_RDP_INFO(char *text)
{
#ifdef LOG_ON
//  FILE *stream;

    if DLIST_TEST 
    {
        if (stream) 
        {
            fclose(stream);
            stream = NULL;
        }
        return;
    }
    
    if (!stream) stream = fopen(F_NAME,"at");
    fprintf(stream, "%s\n", text);
//  fclose(stream);
#endif 
}

