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

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

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
    x -= msg->xoffset;
    // yoffset moves message up
    y += msg->yoffset;

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

// sets message Y offset depending on where they are in the message queue
static float get_message_offset(osd_message_t *msg, int i)
{
    float offset = l_font->height() * (float)i;

    switch(msg->corner)
    {
        case OSD_TOP_LEFT:
        case OSD_TOP_CENTER:
        case OSD_TOP_RIGHT:
            return -offset;
            break;
        default:
            return offset;
            break;
    }
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
void osd_render()
{
    list_node_t *node;
    osd_message_t *msg, *msg_to_delete = NULL;
    // keeps track of how many messages are in each corner
    int corner_ctr[OSD_NUM_CORNERS] = {0};

    // if list is empty, then just skip it all
    if (l_messageQueue == NULL)
        return;

    // get the viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // save all the attributes
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    bool bFragmentProg = glIsEnabled(GL_FRAGMENT_PROGRAM_ARB);
    bool bColorArray = glIsEnabled(GL_COLOR_ARRAY);
    bool bTexCoordArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    bool bSecColorArray = glIsEnabled(GL_SECONDARY_COLOR_ARRAY);

    // deactivate all the texturing units
    int  iActiveTex;
    bool bTexture2D[8];
    glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &iActiveTex);
    for (int i = 0; i < 8; i++)
    {
        glActiveTexture(GL_TEXTURE0_ARB + i);
        bTexture2D[i] = glIsEnabled(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_2D);
    }

    // save the matrices and set up new ones
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // setup for drawing text
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_REGISTER_COMBINERS_NV);
    glDisable(GL_COLOR_MATERIAL);

    glDisable(GL_BLEND);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    glShadeModel(GL_FLAT);

    list_foreach(l_messageQueue, node)
    {
        msg = (osd_message_t *)node->data;

        // if previous message was marked for deletion, delete it
        if(msg_to_delete)
        {
            osd_delete_message(msg_to_delete);
            msg_to_delete = NULL;
        }

        // update message state
        if(msg->timeout[msg->state] != OSD_INFINITE_TIMEOUT &&
           ++msg->frames >= msg->timeout[msg->state])
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

        // offset y depending on how many other messages are in the same corner
        msg->yoffset += get_message_offset(msg, corner_ctr[msg->corner]);

        draw_message(msg, viewport[2], viewport[2]);

        msg->yoffset -= get_message_offset(msg, corner_ctr[msg->corner]);
        corner_ctr[msg->corner]++;
    }

    // if last message was marked for deletion, delete it
    if(msg_to_delete)
        osd_delete_message(msg_to_delete);

    // restore the matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // restore the attributes
    for (int i = 0; i < 8; i++)
    {
        glActiveTexture(GL_TEXTURE0_ARB + i);
        if (bTexture2D[i])
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(iActiveTex);
    glPopAttrib();
    if (bFragmentProg)
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    if (bColorArray)
        glEnableClientState(GL_COLOR_ARRAY);
    if (bTexCoordArray)
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (bSecColorArray)
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);

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

// update message string
extern "C"
void osd_update_message(osd_message_t *msg, const char *fmt, ...)
{
    va_list ap;
    char buf[PATH_MAX];

    if(!msg) return;

    va_start(ap, fmt);
    vsnprintf(buf, PATH_MAX, fmt, ap);
    va_end(ap);

    if(msg->text)
        free(msg->text);

    msg->text = strdup(buf);
}

// remove message from message queue
extern "C"
void osd_delete_message(osd_message_t *msg)
{
    list_node_t *node;

    if(!msg) return;

    if(msg->text)
        free(msg->text);

    node = list_find_node(l_messageQueue, msg);
    free(msg);
    list_node_delete(&l_messageQueue, node);
}

// set "corner" of the screen that message appears in.
extern "C"
void osd_message_set_corner(osd_message_t *msg, enum osd_corner corner)
{
        msg->corner = corner;
}

// set message so it doesn't automatically expire in a certain number of frames.
extern "C"
void osd_message_set_static(osd_message_t *msg)
{
        msg->timeout[OSD_DISPLAY] = OSD_INFINITE_TIMEOUT;
        msg->state = OSD_DISPLAY;
        msg->frames = 0;
}
