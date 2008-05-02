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

// globals
static list_t l_messageQueue = NULL;
static OGLFT::Monochrome *l_font;

static void animation_none(osd_message_t *);
static void animation_fade(osd_message_t *);

// animation handlers
static void (*l_animations[OSD_NUM_ANIM_TYPES])(osd_message_t *) = {
    animation_none, // animation handler for OSD_NONE
    animation_fade  // animation handler for OSD_FADE
};

// private functions
// draw message on screen
static void draw_message(osd_message_t *msg, int width, int height)
{
    float x = 0.,
          y = 0.;

    if(!l_font || !l_font->isValid())
        return;

    // set color. alpha is hard coded to 1. animation can change this
    l_font->setForegroundColor(msg->color[R], msg->color[G], msg->color[B], 1.);

    // set justification based on corner
    switch(msg->corner)
    {
        case OSD_TOP_LEFT:
            l_font->setVerticalJustification(OGLFT::Face::TOP);
            l_font->setHorizontalJustification(OGLFT::Face::LEFT);
            x = 0.;
            y = (float)height;
            break;
        case OSD_TOP_CENTER:
            l_font->setVerticalJustification(OGLFT::Face::TOP);
            l_font->setHorizontalJustification(OGLFT::Face::CENTER);
            x = ((float)width)/2.0;
            y = (float)height;
            break;
        case OSD_TOP_RIGHT:
            l_font->setVerticalJustification(OGLFT::Face::TOP);
            l_font->setHorizontalJustification(OGLFT::Face::RIGHT);
            x = (float)width;
            y = (float)height;
            break;
        case OSD_MIDDLE_LEFT:
            l_font->setVerticalJustification(OGLFT::Face::MIDDLE);
            l_font->setHorizontalJustification(OGLFT::Face::LEFT);
            x = 0.;
            y = ((float)height)/2.0;
            break;
        case OSD_MIDDLE_CENTER:
            l_font->setVerticalJustification(OGLFT::Face::MIDDLE);
            l_font->setHorizontalJustification(OGLFT::Face::CENTER);
            x = ((float)width)/2.0;
            y = ((float)height)/2.0;
            break;
        case OSD_MIDDLE_RIGHT:
            l_font->setVerticalJustification(OGLFT::Face::MIDDLE);
            l_font->setHorizontalJustification(OGLFT::Face::RIGHT);
            x = (float)width;
            y = ((float)height)/2.0;
            break;
        case OSD_BOTTOM_LEFT:
            l_font->setVerticalJustification(OGLFT::Face::BOTTOM);
            l_font->setHorizontalJustification(OGLFT::Face::LEFT);
            x = 0.;
            y = 0.;
            break;
        case OSD_BOTTOM_CENTER:
            l_font->setVerticalJustification(OGLFT::Face::BOTTOM);
            l_font->setHorizontalJustification(OGLFT::Face::CENTER);
            x = ((float)width)/2.0;
            y = 0.;
            break;
        case OSD_BOTTOM_RIGHT:
            l_font->setVerticalJustification(OGLFT::Face::BOTTOM);
            l_font->setHorizontalJustification(OGLFT::Face::RIGHT);
            x = (float)width;
            y = 0.;
            break;
        default:
            l_font->setVerticalJustification(OGLFT::Face::BOTTOM);
            l_font->setHorizontalJustification(OGLFT::Face::LEFT);
            x = 0.;
            y = 0.;
            break;
    }

    // apply animation for current message state
    (*l_animations[msg->animation[msg->state]])(msg);

    // xoffset moves message left
    x -= (float)msg->xoffset;
    // yoffset moves message up
    y += (float)msg->yoffset;

    l_font->draw(x, y, msg->text);
}

// null animation handler
static void animation_none(osd_message_t *msg) { }

// fade in/out animation handler
static void animation_fade(osd_message_t *msg)
{
    float alpha = 1.;
    float elapsed_frames;
    float total_frames = (float)msg->timeout[msg->state];

    switch(msg->state)
    {
        case OSD_DISAPPEAR:
            elapsed_frames = (float)(total_frames - msg->frames);
            break;
        case OSD_APPEAR:
        default:
            elapsed_frames = (float)msg->frames;
            break;
    }

    if(total_frames != 0.)
        alpha = elapsed_frames / total_frames;

    l_font->setForegroundColor(msg->color[R], msg->color[G], msg->color[B], alpha);
}

// remove message from message queue
static void delete_message(osd_message_t *msg)
{
    list_node_t *node;

    if(!msg) return;

    if(msg->text)
        free(msg->text);

    node = list_find_node(l_messageQueue, msg);
    free(msg);
    list_node_delete(&l_messageQueue, node);
}

// public functions
extern "C"
void osd_init(void)
{
    char fontpath[PATH_MAX];

    snprintf(fontpath, PATH_MAX, "%sfonts/%s", get_installpath(), FONT_FILENAME);
    l_font = new OGLFT::Monochrome(fontpath, 16);

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
void osd_exit(void)
{
    list_node_t *node;
    osd_message_t *msg;

    // delete font renderer
    if(l_font)
        delete l_font;

    // delete message queue
    list_foreach(l_messageQueue, node)
    {
        msg = (osd_message_t *)node->data;

        if(msg->text)
            free(msg->text);
        free(msg);
    }
    list_delete(&l_messageQueue);
}

// renders the current osd message queue to the screen
extern "C"
void osd_render(unsigned int width, unsigned int height)
{
    list_node_t *node;
    osd_message_t *msg, *msg_to_delete = NULL;

    list_foreach(l_messageQueue, node)
    {
        msg = (osd_message_t *)node->data;

        // if previous message was marked for deletion, delete it
        if(msg_to_delete)
        {
            delete_message(msg_to_delete);
            msg_to_delete = NULL;
        }

        // update message state
        msg->frames++;
        if(msg->frames >= msg->timeout[msg->state])
        {
            // if message is in last state, mark it for deletion and continue to the next message
            if(msg->state >= OSD_NUM_STATES - 1)
            {
                msg_to_delete = msg;
                continue;
            }

            // go to next state and reset frame count
            msg->state++;
            msg->frames = 0;
        }

        draw_message(msg, width, height);
    }

    // if last message was marked for deletion, delete it
    if(msg_to_delete)
        delete_message(msg_to_delete);

    glFinish();
}

// creates a new osd_message_t, adds it to the message queue and returns it in case
// the user wants to modify its parameters. Note, if the message can't be created,
// NULL is returned.
extern "C"
osd_message_t * osd_new_message(const char *fmt, ...)
{
    va_list ap;
    char buf[PATH_MAX];
    osd_message_t *msg = (osd_message_t *)malloc(sizeof(osd_message_t));

    if(!msg) return NULL;

    va_start(ap, fmt);
    vsnprintf(buf, PATH_MAX, fmt, ap);
    va_end(ap);

    // set default values
    memset(msg, 0, sizeof(osd_message_t));
    msg->text = strdup(buf);
    msg->corner = OSD_BOTTOM_LEFT;
    // default to white
    msg->color[R] = 1.;
    msg->color[G] = 1.;
    msg->color[B] = 1.;

    msg->state = OSD_APPEAR;

    msg->animation[OSD_APPEAR] = OSD_FADE;
    msg->animation[OSD_DISPLAY] = OSD_NONE;
    msg->animation[OSD_DISAPPEAR] = OSD_FADE;

    msg->timeout[OSD_APPEAR] = 20;
    msg->timeout[OSD_DISPLAY] = 180;
    msg->timeout[OSD_DISAPPEAR] = 40;

    // add to message queue
    list_append(&l_messageQueue, msg);

    return msg;
}
