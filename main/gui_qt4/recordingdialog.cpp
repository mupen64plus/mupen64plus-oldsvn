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
        #include "../inputrecording.h"
    }
}

extern core::CONTROL Controls[4];

RecordingDialog::RecordingDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(pushBrowse , SIGNAL(clicked()), this, SLOT(browse()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));

    if (core::g_EmulatorRunning) {
        labelName->setText(QString((const char *) core::ROM_HEADER->nom));
        lineMovieFile->setText(QString((const char *) core::get_m64_filename()));
        lineAuthor->setText("");
        lineDescription->setText("");
        labelCountry->setText(QString("%1").arg(core::ROM_HEADER->Country_code & 0xff));
        labelCRC->setText(QString("%1").arg(core::ROM_HEADER->CRC1, 8, 16, QChar('0')).toUpper());
        labelVideo->setText(QString(QLatin1String(core::getGfxName())));
        labelSound->setText(QString(QLatin1String(core::getAudioName())));
        labelInput->setText(QString(QLatin1String(core::getInputName())));
        labelRsp->setText(QString(QLatin1String(core::getRspName())));

        labelController1->setText(core::getCtrlStrInternal(0));
        labelController2->setText(core::getCtrlStrInternal(1));
        labelController3->setText(core::getCtrlStrInternal(2));
        labelController4->setText(core::getCtrlStrInternal(3));
        radioFromStart->setChecked(true);
    }
}

void RecordingDialog::onaccepted()
{
    if (core::g_EmulatorRunning) {
        int fromSnapshot = 0;
        char *filename = NULL;
        QString aut, desc;
        
        if (radioFromSavestate->isChecked()) {
            fromSnapshot = 1;
        }
        if (lineAuthor->text() == "") {
            lineAuthor->setText(tr("(too lazy to write a name)"));
        }
        if (lineDescription->text() == "") {
            lineDescription->setText(tr("(no description entered)"));
        }
            
        const char *author = lineAuthor->text().toLatin1().data();
        const char *description = lineDescription->text().toLatin1().data();
        filename = lineMovieFile->text().toLatin1().data();
        core::BeginRecording(filename, fromSnapshot, author, description);
    }
}

void RecordingDialog::browse()
{
    QString filename = QFileDialog::getSaveFileName(
                        this,
                        tr("Select .m64 file for recording..."),
                        lineMovieFile->text(),
                        tr(".m64 files (*.m64)"));

    if (!filename.isEmpty()) {
        lineMovieFile->setText(filename);
    }

}

