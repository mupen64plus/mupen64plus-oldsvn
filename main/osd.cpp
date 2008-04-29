/**
 * Mupen64 - osd.cpp
 * Copyright (C) 2008 nmn, ebenblues
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

// On-screen Display

#include <limits.h>

#include "../opengl/OGLFT.h"
#include "osd.h"

extern "C" {
    #include "main.h"
    #include "plugin.h"
    #include "util.h"
}

#define FONT_FILENAME "font.ttf"

typedef struct {
    char *text; /* Text that this object will have when displayed */
    int corner; /* One of the 9 corners */
    int xoffset; /* Relative X position */
    int yoffset; /* Relative Y position */
    float color[4]; /* Red, Green, Blue, Alpha values */
    int timeout; /* how many frames should this message be displayed for? */
} osd_message_t;

// globals
static list_t l_messageQueue = NULL;
static unsigned int l_defaultTimeout = 240;
static OGLFT::Monochrome *l_font;
static int l_fontsize;

// private functions

// public functions
extern "C"
void osd_init(void)
{
    char fontpath[PATH_MAX];

    snprintf(fontpath, PATH_MAX, "%s%s", get_configpath(), FONT_FILENAME);
    l_fontsize = 16;
    l_font = new OGLFT::Monochrome(fontpath, l_fontsize);

    if(!l_font || !l_font->isValid())
    {
        printf("Could not construct face from %s\n", fontpath);
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if defined(GL_RASTER_POSITION_UNCLIPPED_IBM)
    glEnable(GL_RASTER_POSITION_UNCLIPPED_IBM);
#endif

    // setup gfx plugin rendering callback
    setRenderingCallback(osd_render);
}

extern "C"
void osd_render(unsigned int width, unsigned int height)
{
    l_font->setForegroundColor(1., 0., 0., 1.);
    l_font->setHorizontalJustification(OGLFT::Face::CENTER);
    l_font->draw(100., 100., "Test String");
    glFinish();
}

void osd_set_default_timeout(int timeout)
{
    l_defaultTimeout = timeout;
}

void osd_new_message(const char *message)
{
    osd_message_t *msg = (osd_message_t *)malloc(sizeof(osd_message_t));

    memset(msg, 0, sizeof(osd_message_t));

    msg->text = strdup(message);
    msg->corner = OSD_BOTTOM_LEFT;
    msg->timeout = l_defaultTimeout;

    list_append(&l_messageQueue, msg);
}
