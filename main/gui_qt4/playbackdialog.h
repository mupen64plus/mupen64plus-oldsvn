/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - playbackdialog.h                                       *
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

#ifndef __PLAYBACKDIALOG_H__
#define __PLAYBACKDIALOG_H__

#include <QDialog>

#include "ui_playbackdialog.h"

class PlaybackDialog : public QDialog, private Ui_PlaybackDialog
{
    Q_OBJECT
    public:
        PlaybackDialog(QWidget *parent = 0);
    private:
        QString filename;
        FILE *PlaybackFile;
        void update_movieinfo();
        void update_states();
    private slots:
        void onaccepted();
        void browse();
        void checkReadOnly_changed(int state);
        void checkResumeEof_changed(int state);
};

#endif // __PLAYBACKDIALOG_H__

