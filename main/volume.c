/**
 * Mupen64 - volume.c
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

/* Sound volume functions.
 */

#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "volume.h"
#include "guifuncs.h"
#include "translate.h"
#include "../opengl/osd.h"

static int g_VolMuted = 0; // volume muted?
static osd_message_t *g_volMsg = NULL;

/* volSet
 *  Sets volume of left and right PCM channels to given percentage (0-100) value.
 */
void volSet(int percent)
{
    int ret, vol;
    int mixerfd = open("/dev/mixer", O_RDONLY);

    if(mixerfd < 0)
    {
        perror("/dev/mixer: ");
        return;
    }

    if(percent > 100)
        percent = 100;
    else if(percent < 0)
        percent = 0;

    // if we had a volume message, make sure that it's still in the OSD list, or set it to NULL
    if (g_volMsg != NULL)
        g_volMsg = osd_message_valid(g_volMsg);

    // create a new message or update an existing one
    if (g_volMsg != NULL)
        osd_update_message(g_volMsg, tr("Volume: %i%%"), percent);
    else
        g_volMsg = osd_new_message(OSD_MIDDLE_CENTER, tr("Volume: %i%%"), percent);

    vol = (percent << 8) + percent; // set both left/right channels to same vol
    ret = ioctl(mixerfd, MIXER_WRITE(SOUND_MIXER_PCM), &vol);

    if(ret < 0)
        perror("Setting PCM volume: ");

    close(mixerfd);
}

/* volGet
 *  Returns volume of PCM channel as a percentage (0-100).
 */
int volGet(void)
{
    int vol, ret;
    int mixerfd = open("/dev/mixer", O_RDONLY);

    if(mixerfd < 0)
    {
        perror("/dev/mixer: ");
        return;
    }

    ret = ioctl(mixerfd, MIXER_READ(SOUND_MIXER_PCM), &vol);

    if(ret < 0)
        perror("Reading PCM volume: ");

    close(mixerfd);

    return vol & 0xff; // just return the left channel
}

/* volMute
 *  Mute/unmute volume.
 */
void volMute(void)
{
    static int saveVol = 0;

    if (volIsMuted())
    {
        volSet(saveVol);
        osd_update_message(g_volMsg, tr("Mute Off"));
    }
    else
    {
        saveVol = volGet();
        volSet(0);
        osd_update_message(g_volMsg, tr("Mute On"));
    }

    g_VolMuted = !g_VolMuted;
}

/* volIsMuted
 *  Returns 1 if volume is muted, else 0.
 */
int volIsMuted(void)
{
    return g_VolMuted;
}

/* volChange
 *  Increase/decrease volume by the given percentage.
 */
void volChange(int delta)
{
    // if we're muted, unmute before changing volume
    if (volIsMuted())
        volMute();

    volSet(volGet() + delta);
}

