/**
 * Mupen64 - guifuncs.c
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

/* This file defines "gui" functions for the nogui-only build of mupen64
 */

#ifdef NOGUI_ONLY

#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "main.h"
#include "translate.h"
#include "../r4300/r4300.h"

void gui_parseArgs(int *argc, char ***argv) { }
void gui_build(void) { }
void gui_display(void) { }
void gui_main_loop(void) { }

// print info message to console
void info_message(const char *fmt, ...)
{
	va_list ap = {0};

	va_start(ap, fmt);
	printf(tr("Info"));
	printf(": ");
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}

// print alert message to console
void alert_message(const char *fmt, ...)
{
	va_list ap = {0};

	va_start(ap, fmt);
	printf(tr("Error"));
	printf(": ");
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}

// print message and ask for user confirmation (yes/no)
int confirm_message(const char *fmt, ...)
{
	va_list ap = {0};
	char c;

	while(1)
	{
		va_start(ap, fmt);
		printf(tr("Confirm"));
		printf(": ");
		vprintf(fmt, ap);
		printf("(y/n) ");
		va_end(ap);
        
		c = fgetc(stdin);
        
		if(tolower(c) == 'y') return 1;
		else if(tolower(c) == 'n') return 0;

		printf(tr("Please answer 'y' (%s) or 'n' (%s).\n"), tr("Yes"), tr("No"));
	}
}

#endif // NOGUI_ONLY
