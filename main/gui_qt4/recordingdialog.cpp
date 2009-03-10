/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - recordingdialog.cpp                                     *
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

#include "recordingdialog.h"

namespace core {
    extern "C" {
        #include "../main.h"
        #include "../rom.h"
        #include "../config.h"
        #include "../plugin.h"
        #include "../../memory/memory.h"
    }
}

extern core::CONTROL Controls[4];

RecordingDialog::RecordingDialog(QWidget *parent) : QDialog(parent)
{
    char tempbuf[255];

    setupUi(this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));

    if (core::ROM_HEADER) {
        sprintf(tempbuf, "%s", core::ROM_HEADER->nom);
        labelName->setText(tempbuf);
        strcat(tempbuf, ".m64");
        lineMovieFile->setText(tempbuf);
        lineAuthor->setText("");
        lineDescription->setText("");
        
        sprintf(tempbuf, "%X", (core::ROM_HEADER->Country_code)&0xff);
        labelCountry->setText(tempbuf);
        snprintf(tempbuf, 18, "%X-%X", core::ROM_HEADER->CRC1, core::ROM_HEADER->CRC2);
        labelCRC->setText(tempbuf);
        
        gfx_name = core::config_get_string("Gfx Plugin", "");
        input_name = core::config_get_string("Input Plugin", "");
        sound_name = core::config_get_string("Audio Plugin", "");
        rsp_name = core::config_get_string("RSP Plugin", "");
        labelVideo->setText(gfx_name);
        labelSound->setText(input_name);
        labelInput->setText(sound_name);
        labelRsp->setText(rsp_name);


        strcpy(tempbuf, Controls[0].Present ? "Present" : "Disconnected");
        if(Controls[0].Present && Controls[0].Plugin == PLUGIN_MEMPAK) {
            strcat(tempbuf, " with mempak");
        }
        if(Controls[0].Present && Controls[0].Plugin == PLUGIN_RUMBLE_PAK) {
            strcat(tempbuf, " with rumble");
        }
        labelController1->setText(tempbuf);

        strcpy(tempbuf, Controls[1].Present ? "Present" : "Disconnected");
        if(Controls[1].Present && Controls[1].Plugin == PLUGIN_MEMPAK) {
            strcat(tempbuf, " with mempak");
        }
        if(Controls[1].Present && Controls[1].Plugin == PLUGIN_RUMBLE_PAK) {
            strcat(tempbuf, " with rumble pak");
        }
        labelController2->setText(tempbuf);
        
        
        if(Controls[2].Present && Controls[1].Plugin == PLUGIN_MEMPAK) {
            strcat(tempbuf, " with mempak");
        }
        if(Controls[2].Present && Controls[1].Plugin == PLUGIN_RUMBLE_PAK) {
            strcat(tempbuf, " with rumble pak");
        }
        labelController3->setText(tempbuf);

        if(Controls[3].Present && Controls[1].Plugin == PLUGIN_MEMPAK) {
            strcat(tempbuf, " with mempak");
        }
        if(Controls[3].Present && Controls[1].Plugin == PLUGIN_RUMBLE_PAK) {
            strcat(tempbuf, " with rumble pak");
        }
        labelController4->setText(tempbuf);
        radioFromStart->setChecked(true);
    }
}

void RecordingDialog::onaccepted()
{
}
