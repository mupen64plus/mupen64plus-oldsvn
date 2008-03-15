/**
 * Mupen64 - guifuncs.h
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

// parses AND REMOVES any gui-specific commandline arguments. This is called
// before mupen64 parses any of its commandline options.
void gui_parseArgs(int *argc, char ***argv);

// Build GUI components, but do not display
void gui_build(void);

// display GUI components to the screen
void gui_display(void);

// Give control of thread to the gui
void gui_main_loop(void);

// displays load rom dialog window
void gui_load_rom(void);

// prints informational message to user
void info_message(const char *fmt, ...);

// prints alert message to user (used for error messages that don't require
// feedback from user)
void alert_message(const char *fmt, ...);

// prints message to user that requires confirmation (yes/no)
//  Return codes:
//    0 - indicates user selected no
//    1 - indicates user selected yes
int confirm_message(const char *fmt, ...);
