/**
 * Mupen64 - memedit.c
 * Copyright (C) 2002 DavFr - robind@esiee.fr
 *
 * If you want to contribute to this part of the project please
 * contact me (or Hacktarux) first.
 * 
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
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


#include "memedit.h"
#include <gdk/gdkkeysyms.h>

GtkTextBuffer *textbuf;
GtkTextTag *textfont;
GtkTextIter textstart, textend;

uint32 memedit_address, memedit_numlines;

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void on_close();

void update_memory_editor()
{
	int i, j, k;
	char line[75];
	char* buf;
	uint32 addr;
	uint8 byte[16];
	
	buf = (char*)malloc(memedit_numlines * sizeof(line) * sizeof(char));
	if(!buf)
	{
		printf("update_memory_editor: not enough memory\n");
		return;
	}

	
	//Read memory
	//todo: display 'XX' or something for unreadable memory,
	//maybe colour the text
	addr = memedit_address;
	buf[0] = 0;
	for(i=0; i<memedit_numlines; i++)
	{
		for(j=0; j<16; j++)
			byte[j] = read_memory_8(addr + j);
		
		sprintf(line, "%08X|%02X %02X %02X %02X|%02X %02X %02X %02X|%02X %02X %02X %02X|%02X %02X %02X %02X|",
			addr, byte[0], byte[1], byte[2], byte[3], byte[4], byte[5], byte[6], byte[7],
			byte[8], byte[9], byte[10], byte[11], byte[12], byte[13], byte[14], byte[15]);
		
		strcat(buf, line);
		for(j=0; j<16; j++)
		{
			if((byte[j] >= 0x20) && (byte[j] <= 0x7E))
				line[j] = byte[j];
			else line[j] = '.';
		}
		
		if(i < (memedit_numlines - 1))
		{
			line[16] = '\n';
			line[17] = 0;
		}
		else line[16] = 0;
		strcat(buf, line);
		addr += 16;
	}
	
	//Update textbox
	gtk_text_buffer_set_text(textbuf, buf, -1);
	gtk_text_buffer_get_bounds(textbuf, &textstart, &textend); //todo: there must be a better way to keep it in monospace
	gtk_text_buffer_apply_tag(textbuf, textfont, &textstart, &textend);
	
	free(buf);
}

//]=-=-=-=-=-=-=-=-=-=-=-=[ Memory Editor Initialisation ]=-=-=-=-=-=-=-=-=-=-=-=[

void init_memedit()
{
	int i;
	GtkWidget   *boxH1,
            	*scrolledwindow1,
            	*boxV1,
            	*textbox;
    
    memedit_opened = 1;
    memedit_address = 0x800F6A10; //0x800EB5D0;
    memedit_numlines = 16;

    //=== Creation of Memory Editor ===========/
    winMemEdit = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(winMemEdit), "Memory");
    gtk_window_set_default_size( GTK_WINDOW(winMemEdit), 500, 200);
    gtk_container_set_border_width( GTK_CONTAINER(winMemEdit), 2);
    
	textbox = gtk_text_view_new();
	textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textbox));
	
	textfont = gtk_text_buffer_create_tag(textbuf, "font", "font", "monospace", NULL);
	update_memory_editor();
	
	gtk_container_add(GTK_CONTAINER(winMemEdit), textbox);
    gtk_widget_show_all(winMemEdit);

    //=== Signal Connections ===========================/
    gtk_signal_connect( GTK_OBJECT(winMemEdit), "destroy", on_close, NULL );
    gtk_signal_connect( GTK_OBJECT(textbox), "key-press-event", on_key_press, NULL );
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	GtkTextMark* cursor;
	GtkTextIter cursor_iter;
	int cursorpos, linepos, linenum, linelen = 74;
	uint32 key, addr;
	uint8 byte;
	
	//Figure out what was typed.
	//todo: make this not suck
	switch(event->keyval)
	{
		case GDK_0:
		case GDK_1:
		case GDK_2:
		case GDK_3:
		case GDK_4:
		case GDK_5:
		case GDK_6:
		case GDK_7:
		case GDK_8:
		case GDK_9:
			key = event->keyval - GDK_0;
		break;
		
		case GDK_KP_0:
		case GDK_KP_1:
		case GDK_KP_2:
		case GDK_KP_3:
		case GDK_KP_4:
		case GDK_KP_5:
		case GDK_KP_6:
		case GDK_KP_7:
		case GDK_KP_8:
		case GDK_KP_9:
			key = event->keyval - GDK_KP_0;
		break;
		
		case GDK_A:
		case GDK_B:
		case GDK_C:
		case GDK_D:
		case GDK_E:
		case GDK_F:
			key = (event->keyval - GDK_A) + 10;
		break;
		
		case GDK_a:
		case GDK_b:
		case GDK_c:
		case GDK_d:
		case GDK_e:
		case GDK_f:
			key = (event->keyval - GDK_a) + 10;
		break;
		
		default:
			key = event->keyval;
		break;
	}
	
	//Get the cursor position.
	cursor = gtk_text_buffer_get_insert(textbuf);
	gtk_text_buffer_get_iter_at_mark(textbuf, &cursor_iter, cursor);
	cursorpos = gtk_text_iter_get_offset(&cursor_iter);
	
	
	//React to the keypress.
	//todo: skip between-bytes and separator areas when
	//navigating with arrow keys or mouse.
	if(key == GDK_Up)
	{
		cursorpos -= linelen;
		if(cursorpos < 0)
		{
			memedit_address -= 16;
			cursorpos += linelen;
		}
	}
	else if(key == GDK_Down)
	{
		cursorpos += linelen;
		if(cursorpos >= (linelen * memedit_numlines))
		{
			memedit_address += 16;
			cursorpos -= linelen;
		}
	}
	else if(key == GDK_Left)
	{
		if(cursorpos) cursorpos--;
	}
	else if(key == GDK_Right)
	{
		if(cursorpos) cursorpos++;
	}
	else
	{
		//Determine where we are.
		linenum = cursorpos / linelen;
		linepos = cursorpos % linelen;
		
		if((linepos == 8) || (linepos == 56)) //address/hex or hex/text separators
		{
			linepos++;
			cursorpos++;
		}
		
		//printf("line %u, pos %u: ", linenum, linepos);
		if(linepos >= (linelen - 1)); //end of line; do nothing
		
		//If cursor is in address
		else if(linepos < 8)
		{
			if(key < 16) //hex number entered
			{
				//yes, I probably _could_ have made this line uglier.
				memedit_address = (memedit_address & ~(0xF << (4 * (7 - linepos)))) | (key << (4 * (7 - linepos)));
				cursorpos++;
				if((cursorpos % linelen) == 8) cursorpos++; //skip address/hex separator
			}
			//todo: else, beep or something
		}
		
		//If cursor is in text
		else if(linepos >= 56)
		{
			//If a non-special key, except Enter, was pressed
			if((event->keyval <= 0xFF) || (event->keyval == GDK_Return) || (event->keyval == GDK_KP_Enter))
			{
				linepos -= 57; //Character index
				addr = memedit_address + (linenum * 16) + linepos; //Address to edit
				
				if(event->keyval <= 0xFF) byte = event->keyval;
				else if((event->keyval == GDK_Return) || (event->keyval == GDK_KP_Enter)) byte = 0x0A; //Enter inserts line break
				
				write_memory_8(addr, byte);
				cursorpos++;
				if((cursorpos % linelen) == 73) //wrote past last character
				{
					cursorpos += 58; //to first char on next line
					if(cursorpos >= (linelen * memedit_numlines)) //past end of box
					{
						memedit_address += 16;
						cursorpos -= linelen;
					}
				}
			}
			else if(event->keyval == GDK_BackSpace) //back one character
			{
				cursorpos--;
				if((cursorpos % linelen) < 57) //before first character
				{
					cursorpos -= 58; //to last char on prev line
					if(cursorpos < 0)
					{
						memedit_address -= 16;
						cursorpos += linelen;
					}
				}
			}
		}
		
		//If cursor is in hex
		else
		{
			linepos -= 8;
			if(!(linepos % 3)) //between bytes
			{
				cursorpos++;
				linepos++;
			}
			
			addr = memedit_address + (linenum * 16) + (linepos / 3);
			
			//printf("byte %u (%08X) nybble %u, ", linepos / 3, addr, (linepos % 3) - 1);
			if(key < 16) //hex number entered
			{
				byte = read_memory_8(addr);
				//printf("%02X -> ", byte);
				if((linepos % 3) - 1) //low nybble
				{
					byte = (byte & 0xF0) | (uint8)key;
					cursorpos++; //skip the between-bytes area
				}
				else byte = (byte & 0x0F) | ((uint8)key << 4);
				//printf("%02X\n", byte);
				write_memory_8(addr, byte);
				cursorpos++;
				
				if((cursorpos % linelen) == 57) //wrote past last byte
				{
					cursorpos += 26; //to first byte on next line
					if(cursorpos >= (linelen * memedit_numlines)) //past end of box
					{
						memedit_address += 16;
						cursorpos -= linelen;
					}
				}
			} //if hex number entered
		} //if in hex
	}
	
	//0.........1.........2.........3.........4.........5.........6.........7..
	//|00 01 02 03|04 05 06 07|08 09 0A 0B|0C 0D 0E 0F|0123456789ABCDEF
	//80000000|00 01 02 03|04 05 06 07|08 09 0A 0B|0C 0D 0E 0F|0123456789ABCDEF
	
	update_memory_editor();
	
	//Restore the cursor position.
	gtk_text_buffer_get_iter_at_offset(textbuf, &cursor_iter, cursorpos);
	gtk_text_buffer_place_cursor(textbuf, &cursor_iter);
	
	return TRUE;
}


static void on_close()
{
    memedit_opened = 0;
}
