/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - playbackdialog.cpp                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 olejl                                              *
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

#include <QtGui>

#include "playbackdialog.h"

namespace core {
    extern "C" {
        #include "../main.h"
        #include "../rom.h"
        #include "../config.h"
        #include "../plugin.h"
        #include "../inputrecording.h"
        #include "../../memory/memory.h"
    }
}

PlaybackDialog::PlaybackDialog(QWidget *parent) : QDialog(parent)
{
    char tempbuf[255];

    setupUi(this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));

    // Settings from current setup
    sprintf(tempbuf, "%s", core::ROM_HEADER->nom);
    labelName->setText(tempbuf);
    
    char* file = core::get_m64_filename();
    filename = QString(file);
  
    lineMovieFile->setText(filename);
    lineAuthor->setText("");
    lineDescription->setText("");
    
    sprintf(tempbuf, "%X", (core::ROM_HEADER->Country_code)&0xff);
    labelCountry->setText(tempbuf);
    sprintf(tempbuf, "%X", core::ROM_HEADER->CRC1);
    labelCRC->setText(tempbuf);
    
    gfx_name = core::config_get_string("Gfx Plugin", "");
    input_name = core::config_get_string("Input Plugin", "");
    sound_name = core::config_get_string("Audio Plugin", "");
    rsp_name = core::config_get_string("RSP Plugin", "");
    labelVideo->setText(gfx_name);
    labelSound->setText(input_name);
    labelInput->setText(sound_name);
    labelRsp->setText(rsp_name);


    strcpy(tempbuf, core::Controls[0].Present ? "Present" : "Disconnected");
    if(core::Controls[0].Present && core::Controls[0].Plugin == PLUGIN_MEMPAK) {
        strcat(tempbuf, " with mempak");
    }
    if(core::Controls[0].Present && core::Controls[0].Plugin == PLUGIN_RUMBLE_PAK) {
        strcat(tempbuf, " with rumble");
    }
    labelController1->setText(tempbuf);

    strcpy(tempbuf, core::Controls[1].Present ? "Present" : "Disconnected");
    if(core::Controls[1].Present && core::Controls[1].Plugin == PLUGIN_MEMPAK) {
        strcat(tempbuf, " with mempak");
    }
    if(core::Controls[1].Present && core::Controls[1].Plugin == PLUGIN_RUMBLE_PAK) {
        strcat(tempbuf, " with rumble pak");
    }
    labelController2->setText(tempbuf);
    
    
    if(core::Controls[2].Present && core::Controls[2].Plugin == PLUGIN_MEMPAK) {
        strcat(tempbuf, " with mempak");
    }
    if(core::Controls[2].Present && core::Controls[2].Plugin == PLUGIN_RUMBLE_PAK) {
        strcat(tempbuf, " with rumble pak");
    }
    labelController3->setText(tempbuf);

    if(core::Controls[3].Present && core::Controls[3].Plugin == PLUGIN_MEMPAK) {
        strcat(tempbuf, " with mempak");
    }
    if(core::Controls[3].Present && core::Controls[3].Plugin == PLUGIN_RUMBLE_PAK) {
        strcat(tempbuf, " with rumble pak");
    }
    labelController4->setText(tempbuf);
    
    // Update information for selected movie file
    PlaybackFile = fopen(file,"rb");
    core::m64_header header;
    if (PlaybackFile) {
    	memset(&header, 0, sizeof(header));
        fread(&header,sizeof(header),1,PlaybackFile);
        fclose(PlaybackFile);

        int min = 0;
        int sec = 0;
        int fps = (unsigned int) header.fps;
        if ((header.total_vi > 0) && (fps > 0)) {
            min = (header.total_vi / fps) / 60;
            sec = (header.total_vi / fps) % 60;
            qDebug() << "header.total_vi: " << header.total_vi << " - header.fps: " << fps;
            qDebug() << min << ":" << sec;
        }
        sprintf(tempbuf, "%.2u:%.2u", min, sec);
        labelLength->setText(tempbuf);
        sprintf(tempbuf, "%u (%u)", header.total_vi, header.input_samples);
        labelFrames->setText(tempbuf);
        sprintf(tempbuf, "%u", header.rerecord_count);
        labelReRecords->setText(tempbuf);
        if (header.start_type == MOVIE_START_FROM_SNAPSHOT) {
            labelFrom->setText("Savestate");
        } else {
            labelFrom->setText("Reset");
        }
       
        lineAuthor->setText(QString(QLatin1String(header.utf_authorname)));
        lineDescription->setText(QString(QLatin1String(header.utf_moviedesc)));
        labelName_2->setText(QString(QLatin1String(header.rom_name)));
        sprintf(tempbuf, "%X", header.rom_cc);
        labelCountry_2->setText(tempbuf);
        snprintf(tempbuf, 9, "%X", header.rom_crc);
        labelCRC_2->setText(tempbuf);

        labelVideo_2->setText(QString(QLatin1String(header.video_plugin)));
        labelSound_2->setText(QString(QLatin1String(header.sound_plugin)));
        labelInput_2->setText(QString(QLatin1String(header.input_plugin)));
        labelRsp_2->setText(QString(QLatin1String(header.rsp_plugin)));
        
        int ctrl_flags = header.controller_flags;
        QString ctrl1 = "Disconnected";
        QString ctrl2 = "Disconnected";
        QString ctrl3 = "Disconnected";
        QString ctrl4 = "Disconnected";
        if (ctrl_flags & 0x001) { 
            ctrl1 = "Present";
            if (ctrl_flags & 0x010) { ctrl1 += " with mempak"; }
            if (ctrl_flags & 0x100) { ctrl1 += " with rumble pak"; }
        }
        if (ctrl_flags & 0x002) {
            ctrl2 = "Present";
            if (ctrl_flags & 0x020) { ctrl2 += " with mempak"; }
            if (ctrl_flags & 0x200) { ctrl2 += " with rumble pak"; }
        }
        if (ctrl_flags & 0x004) {
            ctrl3 = "Present";
            if (ctrl_flags & 0x040) { ctrl3 += " with mempak"; }
            if (ctrl_flags & 0x400) { ctrl3 += " with rumble pak"; }
        }
        if (ctrl_flags & 0x008) {
            ctrl4 = "Present";
            if (ctrl_flags & 0x080) { ctrl4 += " with mempak"; }
            if (ctrl_flags & 0x800) { ctrl4 += " with rumble pak"; }
        }
        
        labelController1_2->setText(ctrl1);
        labelController2_2->setText(ctrl2);
        labelController3_2->setText(ctrl3);
        labelController4_2->setText(ctrl4);
    }
}

void PlaybackDialog::onaccepted()
{
    if (!filename.isEmpty()) {
        core::BeginPlayback((char *) filename.toLatin1().data());
    }
}

