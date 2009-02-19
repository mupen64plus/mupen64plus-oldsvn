/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - hexspinboxdialog.h                                        *
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

#ifndef __HEXSPINBOXDIALOG_H__
#define __HEXSPINBOXDIALOG_H__

#include <QDialog>
#include "ui_hexspinboxdialog.h"

class HexSpinBox;

class HexSpinBoxDialog : public QDialog, private Ui_HexSpinBoxDialog
{
    Q_OBJECT
    public:
        HexSpinBoxDialog(unsigned int *value, QDialog *parent = 0);

    private slots:
        void accepted();

    private:
        HexSpinBox *hexSpinBox;
        unsigned int *acceptedValue;
};

#endif // __HEXSPINBOXDIALOG_H__

