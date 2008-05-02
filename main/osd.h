/**
 * Mupen64 - osd.h
 * Copyright (C) 2008 nmn, ebenblues
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
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

#ifndef __OSD_H__
#define __OSD_H__

/******************************************************************
   osd_corner
   0    1    2 |
    \ __|__/   | Offset always effects the same:
     |     |   |  +X = Leftward   +Y = Upward
   3-|  4  |-5 |  With no offset, the text will touch the border.
     |_____|   |
    /   |   \  |
   6    7    8 |
*******************************************************************/
enum osd_corner {
    OSD_TOP_LEFT,       // 0 in the picture above
    OSD_TOP_CENTER,     // 1 in the picture above
    OSD_TOP_RIGHT,      // 2 in the picture above

    OSD_MIDDLE_LEFT,    // 3 in the picture above
    OSD_MIDDLE_CENTER,  // 4 in the picture above
    OSD_MIDDLE_RIGHT,   // 5 in the picture above

    OSD_BOTTOM_LEFT,    // 6 in the picture above
    OSD_BOTTOM_CENTER,  // 7 in the picture above
    OSD_BOTTOM_RIGHT,   // 8 in the picture above

    OSD_NUM_CORNERS
};

enum osd_message_state {
    OSD_APPEAR,     // OSD message is appearing on the screen
    OSD_DISPLAY,    // OSD message is being displayed on the screen
    OSD_DISAPPEAR,  // OSD message is disappearing from the screen

    OSD_NUM_STATES
};

enum osd_animation_type {
    OSD_NONE,
    OSD_FADE,

    OSD_NUM_ANIM_TYPES
};

typedef struct {
    char *text; // Text that this object will have when displayed
    enum osd_corner corner; // One of the 9 corners
    int xoffset; // Relative X position
    int yoffset; // Relative Y position
    float color[3]; // Red, Green, Blue values
    int state; // display state of current message
    enum osd_animation_type animation[OSD_NUM_STATES]; // animations for each display state
    int timeout[OSD_NUM_STATES]; // timeouts for each display state
    int frames; // number of frames in this state
} osd_message_t;

enum { R, G, B }; // for referencing color array

#ifdef __cplusplus
extern "C" {
#endif

void osd_init(void);
void osd_exit(void);
void osd_render(unsigned int width, unsigned int height);
osd_message_t * osd_new_message(const char *, ...);

#ifdef __cplusplus
}
#endif

#endif // __OSD_H__
