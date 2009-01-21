/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - registerwidget.cpp                                      *
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

#include "registerwidget.h"
#include "tablelistmodel.h"

namespace debugger {
    extern "C" {
        #include "../../../debugger/debugger.h"
        #include "../../../debugger/decoder.h"
    }
}

namespace core {
    extern "C" {
        #include "../../config.h"
        #include "../../version.h"
        long long int reg_cop1_fgr_64[32];
    }
}

RegisterWidget::RegisterWidget(QWidget *parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    // connect( pushSomething, SIGNAL( clicked() ), this, SLOT( on_something() ) ); 
    
    // Set window title
    setWindowTitle(tr("Mupen64Plus Register Viewer"));
    
    // Init GPR
    modelGpr = new TableListModel(this);
//    tableGpr->setShowGrid(false);
//    tableGpr->horizontalHeader()->hide();
//    tableGpr->verticalHeader()->hide();
//    tableGpr->horizontalHeader()->setMinimumSectionSize(1);
//    tableGpr->verticalHeader()->setMinimumSectionSize(1);
    QStringList str1;
    QStringList str2;
    str1 << "r0" << "r1";
    str1 << "XXXXXXXXXXXXXXXX" << "XXXXXXXXXXXXXXXX";
    modelGpr->initStringIndex(str1);
    modelGpr->initStringValue(str2);
    tableGpr->update();
    tableGpr->setModel(modelGpr);
    
    
    // Init Special Registers
    linePC->setText(tr("XXXXXXXXXXXXXXXX: 0xXXXXXXXX"));
    linePrevPC->setText(tr("XXXXXXXXXXXXXXXX"));
 
    stringlistHiLo << "XXXXXXXXXXXXXXXX" << "XXXXXXXXXXXXXXXX";
    modelHiLo = new QStringListModel();
    modelHiLo->setStringList(stringlistHiLo);
    listHiLo->setModel(modelHiLo);

    stringlistInterupt << "XXXXXXXXXXXXXXXX";
    modelInterupt = new QStringListModel();
    modelInterupt->setStringList(stringlistInterupt);
    listInterupt->setModel(modelInterupt);

}

RegisterWidget::~RegisterWidget()
{
    //TODO: Implement destructor
    delete modelHiLo;
    delete modelInterupt;
}

void RegisterWidget::init_registers()
{
    //TODO?: init_registers
    
}
void RegisterWidget::update_registers()
{
    RegisterWidget::update_fgr();  // cop1 (floating points registers)
}

void RegisterWidget::update_fgr()
{
    char txt[24];
    for(int i=0; i<32; i++) {
        if( gui_fantom_fgr_64[i]!= core::reg_cop1_fgr_64[i] )
        {
            gui_fantom_fgr_64[i] = core::reg_cop1_fgr_64[i];
            
            sprintf(txt, "%llX", gui_fantom_fgr_64[i]);
            //TODO: write txt to correct column
            //TODO: change background color
            
        } else {
            ;//TODO: reset highlight
        }
    }    
}

