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

#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "volume.h"
#include "../guifuncs.h"
#include "../translate.h"

static int g_VolMuted = 0; // volume muted?

/* volSet
 *  Sets volume of left and right PCM channels to given percentage (0-100) value.
 */
void volSet(int percent)
{
    int volume = (DWORD)(0xFFFF*1.0*percent/100.0);
    WAVEFORMATEX wfex;
    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.nChannels = 1;
    wfex.nSamplesPerSec = 8000*1000;
    wfex.wBitsPerSample = 8;
    wfex.nBlockAlign = wfex.nChannels*wfex.wBitsPerSample/8;
    wfex.nAvgBytesPerSec = wfex.nSamplesPerSec*wfex.nBlockAlign;
    wfex.cbSize = 0;
    HWAVEOUT waveout;
    int id;
    for (id=0;id<waveOutGetNumDevs();id++)
    {
        if (waveOutOpen(&waveout,id,&wfex,0,0,CALLBACK_NULL) == MMSYSERR_NOERROR)
        {
            waveOutSetVolume(waveout,volume); 
            waveOutClose(waveout);
            break;
        }
    }    
}

/* volGet
 *  Returns volume of PCM channel as a percentage (0-100).
 */
int volGet(void)
{
    int vol;

    // Open the audio device
    
    // Get the volume into vol

    // Close the audio device
    
    return vol & 0xff; // just return the left channel
}

/* volMute
 *  Mute/unmute volume.
 */
void volMute(void)
{
    static int saveVol = 0;

    if(g_VolMuted)
    {
        volSet(saveVol);
    }
    else
    {
        saveVol = volGet();
        volSet(0);
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
    if(g_VolMuted)
        volMute();

    volSet(volGet() + delta);
}

