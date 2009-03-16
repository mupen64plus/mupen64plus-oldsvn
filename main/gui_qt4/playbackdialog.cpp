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
        #include "../inputrecording.h"
    }
}

PlaybackDialog::PlaybackDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(pushBrowse , SIGNAL(clicked()), this, SLOT(browse()));
    connect(checkReadOnly , SIGNAL(stateChanged(int)), this, SLOT(checkReadOnly_changed(int)));
    connect(checkResumeRecording , SIGNAL(stateChanged(int)), this, SLOT(checkResumeRecording_changed(int)));

    if (core::g_EmulatorRunning) {
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));
        filename = QString(core::get_m64_filename());
  
        labelName->setText(QString((const char *) core::ROM_HEADER->nom));
        lineMovieFile->setText(filename);
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
    }
    update_movieinfo();
}

void PlaybackDialog::onaccepted()
{
    if (!lineMovieFile->text().isEmpty()) {
        if (checkResumeRecording->checkState()) {
            core::ResumeRecording(lineMovieFile->text().toLocal8Bit());
        } else {
            core::BeginPlayback(lineMovieFile->text().toLocal8Bit());
        }
    }
}

void PlaybackDialog::update_movieinfo()
{
    // Update information for selected movie file
    char* file;
    file = lineMovieFile->text().toLatin1().data();
    PlaybackFile = fopen(file,"rb");
    core::m64_header header;
  	memset(&header, 0, sizeof(header));

    if (PlaybackFile && (fread(&header,sizeof(header),1,PlaybackFile) == 1)) {
        fclose(PlaybackFile);

        int min = 0, sec = 0;
        if ((header.total_vi > 0) && ((unsigned int) header.fps > 0)) {
            min = (header.total_vi / (unsigned int) header.fps) / 60;
            sec = (header.total_vi / (unsigned int) header.fps) % 60;
        }
        labelLength->setText(QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')));
        labelFrames->setText(QString("%1 (%2)").arg(header.total_vi).arg(header.input_samples));
        labelReRecords->setText(QString("%1").arg(header.rerecord_count));
        lineAuthor->setText(QString(QLatin1String(header.utf_authorname)));
        lineDescription->setText(QString(QLatin1String(header.utf_moviedesc)));
        labelName_2->setText(QString(QLatin1String(header.rom_name)));
        labelCountry_2->setText(QString("%1").arg(header.rom_cc));
        labelCRC_2->setText(QString("%1").arg(header.rom_crc, 8, 16, QChar('0')).toUpper());
        labelVideo_2->setText(QString(QLatin1String(header.video_plugin)));
        labelSound_2->setText(QString(QLatin1String(header.sound_plugin)));
        labelInput_2->setText(QString(QLatin1String(header.input_plugin)));
        labelRsp_2->setText(QString(QLatin1String(header.rsp_plugin)));

        if (header.start_type == MOVIE_START_FROM_SNAPSHOT) {
            labelFrom->setText("Savestate");
        } else {
            labelFrom->setText("Reset");
        }
        
        labelController1_2->setText(core::getCtrlStrHeader(0, header.controller_flags));
        labelController2_2->setText(core::getCtrlStrHeader(1, header.controller_flags));
        labelController3_2->setText(core::getCtrlStrHeader(2, header.controller_flags));
        labelController4_2->setText(core::getCtrlStrHeader(3, header.controller_flags));
        
        update_states();
    }

}

void PlaybackDialog::browse()
{
    QString filename = QFileDialog::getOpenFileName(
                        this,
                        tr("Open .m64 file for playback..."),
                        lineMovieFile->text(),
                        tr(".m64 files (*.m64)"));

    if (!filename.isEmpty()) {
        lineMovieFile->setText(filename);
        update_movieinfo();
    }

}

void PlaybackDialog::update_states()
{
    if (core::g_ReadOnlyPlayback) {
        checkReadOnly->setCheckState(Qt::Checked);
        lineAuthor->setEnabled(false);
        lineDescription->setEnabled(false);
    } else {
        checkReadOnly->setCheckState(Qt::Unchecked);
        lineAuthor->setEnabled(true);
        lineDescription->setEnabled(true);
    }
    if (core::g_ResumeRecording) {
        checkResumeRecording->setCheckState(Qt::Checked);
    } else {
        checkResumeRecording->setCheckState(Qt::Unchecked);
    }
}

void PlaybackDialog::checkReadOnly_changed(int state)
{
    if (state) {
        core::g_ReadOnlyPlayback = 1;
    } else {
        core::g_ReadOnlyPlayback = 0;
    }
    update_states();
}

void PlaybackDialog::checkResumeRecording_changed(int state)
{
    if (state) {
        core::g_ResumeRecording = 1;
    } else {
        core::g_ResumeRecording = 0;
    }
}

