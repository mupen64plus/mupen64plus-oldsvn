/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - recordingwidget.cpp                                     *
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

#include "recordingwidget.h"
namespace core {
    extern "C" {
        #include "../main.h"
        #include "../rom.h"
        #include "../../memory/memory.h"
    }
}

recordingWidget::recordingWidget(QWidget *parent) : QWidget(parent)
{
    char tempbuf [MAX_PATH];

    setupUi(this);

    if (core::ROM_HEADER) {
        snprintf(tempbuf, "%s", core::ROM_HEADER->nom);
        labelName->setText(tempbuf);
		strcat(tempbuf, ".m64");
        lineMovieFile->setText(tempbuf);
        lineAuthor->setText("");
        lineDescription->setText("");
        snprintf(tempbuf, "C:%X", (core::ROM_HEADER->Country_code)&0xff);
        labelCountry->setText(tempbuf);
        snprintf(tempbuf, "%X-%X", core::ROM_HEADER->CRC1, core::ROM_HEADER->CRC2);
        labelCRC->setText(tempbuf);
        labelVideo->setText("<Video Plugin Name>");
        labelSound->setText("<Sound Plugin Name>");
        labelInput->setText("<Input Plugin Name>");
        labelRsp->setText("<RSP Plugin Name>");
        labelController1->setText("<undef>");
        labelController2->setText("<undef>");
        labelController3->setText("<undef>");
        labelController4->setText("<undef>");
        radioFromStart->setChecked(true);
    }
}

