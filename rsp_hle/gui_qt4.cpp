/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rsp plugin - gui_qt4.cpp                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Dylan Wagstaff (Pyromanik)                         *
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

#include <QMessageBox>
#include <QByteArray>
#include "ConfigDialog.h"

extern "C" {
#include <string.h>
#include "gui.h"
#include "config.h"
#include "extension.h"
#include "wintypes.h"

BOOL pseudospecifichle;
BOOL pseudoaudiohle;
BOOL pseudographicshle;
char pseudoaudioname[100];
}
#include <stdio.h>
#include "ui_config.h"

namespace llist {
    extern "C" {
        extern char* next_plugin();
    }
}

//Class functions
ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent)
{
    char* plugname = next_plugin();

    setupUi(this);

    if(pseudoaudiohle) //audiohle
    {
        radioA2->setChecked(true);
        if(pseudospecifichle) //specifichle
        {
            radioA3->setChecked(true);
        }
    }
    else
    {
        radioA1->setChecked(true);
    }

    //GUI explains opposite of bool, so we need to invert it.
    Gcheck->setChecked(!pseudographicshle);

    while(plugname != NULL)
    {
        dropbox->addItem(plugname);
        plugname = next_plugin();
    }
    dropbox->setCurrentIndex(dropbox->findText(pseudoaudioname));
}

void ConfigDialog::on_radioA1_toggled(bool checked)
{
    pseudoaudiohle = !checked;
}

void ConfigDialog::on_radioA2_toggled(bool checked)
{
    pseudoaudiohle = checked;
}

void ConfigDialog::on_radioA3_toggled(bool checked)
{
    pseudospecifichle = checked;
}

void ConfigDialog::on_Gcheck_toggled(bool checked)
{
    pseudographicshle = !checked;
}

void ConfigDialog::on_dropbox_activated(const QString &text)
{
    strcpy(pseudoaudioname, text.toAscii().constData());
}

// Actual functions.
void configDialog(HWND handle)
{
    startup();
    ConfigDialog dia(QWidget::find(handle));
    if(dia.exec() == QDialog::Accepted)
    {
        AudioHle = pseudoaudiohle;
        GraphicsHle = pseudographicshle;
        SpecificHle = pseudospecifichle;
        if(SpecificHle) { AudioHle = TRUE; }
        strcpy(audioname, pseudoaudioname);
        SaveConfig();
    }
}

/*Message box handler, to simplify info dialog calls.*/
void MessageBox(HWND handle, char* message, char* title, int flags)
{
    QMessageBox dia(QMessageBox::NoIcon, title, message);
    dia.setParent(QWidget::find(handle), Qt::Dialog);
    switch (flags)
    {
        case 1:
            dia.setIconPixmap(QPixmap(":/icon/dialog-question.png"));
        break;
        case 2:
            dia.setIconPixmap(QPixmap(":/icon/dialog-error.png"));
        break;
        case 3:
            dia.setIconPixmap(QPixmap(":/icon/dialog-warning.png"));
        break;
        default:
        break; //Leave default (no icon)
    }
    dia.exec();
}
