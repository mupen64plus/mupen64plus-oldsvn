/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - configdialog_qt4.cpp                                    *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008 slougi                                             *
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

#ifndef CONFIGDIALOG_QT4_H
#define CONFIGDIALOG_QT4_H

#include <QDialog>
#include "ui_glide64config.h"

class ConfigDialog : public QDialog, private Ui_Glide64ConfigDialog
{
    Q_OBJECT
    public:
        ConfigDialog(QWidget* parent = 0);

    public slots:
        virtual void accept();
        void reset();
        void defaults();
        void buttonClicked(QAbstractButton* button);

    private:
        void readSettings();
        void writeSettings();
};

#endif // CONFIGDIALOG_QT4_H
