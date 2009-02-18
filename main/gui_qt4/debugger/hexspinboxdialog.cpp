/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - hexspinboxdialog.cpp                                    *
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

#include "hexspinboxdialog.h"
#include "hexspinbox.h"

HexSpinBoxDialog::HexSpinBoxDialog(unsigned int* i, QDialog*)
{
    setupUi(this); // this sets up GUI
    
    acceptedValue = i;
 
    hexSpinBox = new HexSpinBox();
    hexSpinBox->setValue(*acceptedValue);
    hexSpinBox->setSingleStep(4);
    hexSpinBox->setMinimum(0);
    hexSpinBox->setMaximum(0x7fffffff);
    verticalLayout->addWidget(hexSpinBox);

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onaccepted() ));
}

HexSpinBoxDialog::~HexSpinBoxDialog()
{
    //TODO: Implement destructor
}

void HexSpinBoxDialog::onaccepted()
{
    *acceptedValue = (unsigned int) hexSpinBox->value();
}

