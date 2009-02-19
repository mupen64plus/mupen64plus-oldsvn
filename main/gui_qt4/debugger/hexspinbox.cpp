/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - hexspinbox.cpp                                          *
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

#include "hexspinbox.h"

HexSpinBox::HexSpinBox(QWidget *parent) : QSpinBox(parent) { }

int HexSpinBox::valueFromText(const QString &text) const
{
    bool ok;
    int tmp_number, ret = 0;
    QString tmp = text;
    
    QStringList parts = QString(tmp).split("x", QString::SkipEmptyParts);
    tmp = parts.at(parts.size()-1); // Romoving possible 0x prefix
    tmp_number = tmp.toInt( &ok, 16 );
    if (ok) {
        ret = tmp_number;
    } else {    // Else try to return old value
        tmp_number = text.toInt( &ok, 16 );
        if (ok) {
            ret = tmp_number;
        }
    }
    return ret;
}

QString HexSpinBox::textFromValue(int value) const
{
    return "0x" + QString::number(value,16).toUpper();
}

