/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cheatromdialog.cpp                                      *
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

#include "cheatromdialog.h"

CheatRomDialog::CheatRomDialog(core::rom_cheats_t *rom, QWidget* parent) : QDialog(parent)
{
    setupUi(this);
    // Use this as our internal pointer to the cheat we are working on.
    // To make it accessable for other than only constructor
    _rom = rom;
    if (_rom->rom_name) {
        lineEditName->setText(QString(_rom->rom_name));
        lineEditCRC1->setText(QString("%1").arg(_rom->crc1, 8, 16, QChar('0')).toUpper());
        lineEditCRC2->setText(QString("%1").arg(_rom->crc2, 8, 16, QChar('0')).toUpper());
    }
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));
}

CheatRomDialog::~CheatRomDialog()
{
    // TODO: destructor
}

void CheatRomDialog::onaccepted()
{
    bool *ok = NULL;
    int size;

    QByteArray arr = lineEditName->text().toLatin1();
    size = arr.size() + 1;
    _rom->rom_name = (char*) malloc(size);
    if (_rom->rom_name==NULL) return; // TODO: proper error handling?
    memset(_rom->rom_name, '\0', size);
    strcpy(_rom->rom_name, arr.data());
    _rom->crc1 = QString(lineEditCRC1->text()).toUInt(ok,16);
    _rom->crc2 = QString(lineEditCRC2->text()).toUInt(ok,16);
}

