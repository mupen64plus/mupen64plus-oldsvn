/*
* Copyright (C) 2008 Louai Al-Khanji
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
*/

#ifndef MUPEN_KDE4_SETTINGS_DIALOG_H
#define MUPEN_KDE4_SETTINGS_DIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog, public Ui_SettingsDialog
{
    Q_OBJECT
    public:
        SettingsDialog(QWidget* parent = 0);

    public slots:
        void accept();

    private slots:
        void on_aboutAudioPluginButton_clicked();
        void on_configAudioPluginButton_clicked();
        void on_testAudioPluginButton_clicked();
        void on_aboutInputPluginButton_clicked();
        void on_configInputPluginButton_clicked();
        void on_testInputPluginButton_clicked();
        void on_aboutRspPluginButton_clicked();
        void on_configRspPluginButton_clicked();
        void on_testRspPluginButton_clicked();
        void on_aboutGraphicsPluginButton_clicked();
        void on_configGraphicsPluginButton_clicked();
        void on_testGraphicsPluginButton_clicked();

        void pageChanged(int);
};

#endif // MUPEN_KDE4_SETTINGS_DIALOG_H