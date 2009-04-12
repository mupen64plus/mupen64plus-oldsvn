/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Wiinput64 plugin : gui_none.c                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Lionel Fuentes (Funto)                             *
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

#ifndef USE_GTK

#include "wiinput64.h"
#include "gui.h"

void ConfigDialog()
{
	int nb_wiimotes=0;
	int i=0;
	char buffer[20] = "";

	do
	{
		puts("[Wiinput64] : number of wiimotes (max : 4) : ");
		scanf("%d", &nb_wiimotes);
	}
	while(nb_wiimotes < 0 || nb_wiimotes > 4);

	for(i=0 ; i<nb_wiimotes ; i++)
	{
		printf("[Wiinput64] wiimote %d address (get it with \"lswm\" ; currently : %s) : ", i, wiimote_addresses[i]);
		fgets(buffer, sizeof(buffer));

		// NB : fgets() may append an '\n', we get rid of it
		strncpy(wiimote_addresses[i], buffer, sizeof(bluetooth_addr));
	}
}

void AboutDialog()
{
	puts("[Wiinput64] : about : ");
	puts(PLUGIN_FULL_NAME);
	putchar('\n');
}

#endif // !defined USE_GTK
