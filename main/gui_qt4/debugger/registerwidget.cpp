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
#include <QTableView>

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
        extern long long int reg_cop1_fgr_64[32];
    }
}

RegisterWidget::RegisterWidget(QWidget* parent) // : QWidget(parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    // connect( pushSomething, SIGNAL( clicked() ), this, SLOT( on_something() ) ); 
    
    // Set window title
    setWindowTitle(tr("Mupen64Plus Register Viewer"));
    
    RegisterWidget::init_registers();
    
    registers_opened = 1;
}

RegisterWidget::~RegisterWidget()
{
    //TODO: Implement destructor
    registers_opened = 0;
}

void RegisterWidget::init_registers()
{
    RegisterWidget::init_gpr();
    RegisterWidget::init_cop0();
    RegisterWidget::init_special();
    RegisterWidget::init_cop1();
}

void RegisterWidget::init_gpr()
{
    for (int i=0;i<32;i++)
        gui_fantom_gpr_64[i] = 0x1234567890LL;  // Unlikely value
        
    mnemonicGPR 
        << "R0" << "AT" << "V0" << "V1" << "A0" << "A1" << "A2" << "A3" 
        << "T0" << "T1" << "T2" << "T3" << "T4" << "T5" << "T6" << "T7" 
        << "S0" << "S1" << "S2" << "S3" << "S4" << "S5" << "S6" << "S7" 
        << "T8" << "T9" << "K0" << "K1" << "GP" << "SP" << "S8" << "RA"
        ;

    modelGpr = new TableListModel(mnemonicGPR, 16);
    tableGpr->horizontalHeader()->hide();
    tableGpr->verticalHeader()->hide();
    tableGpr->setModel(modelGpr);
    tableGpr->resizeColumnsToContents();
    for (int i=0;i<32;i++)
        tableGpr->setRowHeight(i,15);
}

void RegisterWidget::init_cop0()
{
    for (int i=0;i<32;i++)
        gui_fantom_cop0_32[i] = 0x1234;

    mnemonicCop0
        << "Index"    << "Random"   << "EntryLo0" << "EntryLo1"
        << "Context"  << "PageMask" << "Wired"    << "----"
        << "BadVAddr" << "Count"    << "EntryHi"  << "Compare"
        << "SR"       << "Cause"    << "EPC"      << "PRid"
        << "Config"   << "LLAddr"   << "WatchLo"  << "WatchHi"
        << "Xcontext" << "----"     << "----"     << "----"
        << "----"     << "----"     << "PErr"     << "CacheErr"
        << "TagLo"    << "TagHi"    << "ErrorEPC" << "----"
        ;

    modelCop0 = new TableListModel(mnemonicCop0, 8);
    tableCop0->horizontalHeader()->hide();
    tableCop0->verticalHeader()->hide();
    tableCop0->setModel(modelCop0);
    tableCop0->resizeColumnsToContents();
    for (int i=0;i<32;i++)
        tableCop0->setRowHeight(i,15);
}

void RegisterWidget::init_special()
{
    //TODO: RegisterWidget::init_special()
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

void RegisterWidget::init_cop1()
{
    for (int i=0;i<32;i++)
        gui_fantom_cop1_64[i] = 0x1234567890LL;
    
    mnemonicCop1
        << "Cop1_0" << "Cop1_1" << "Cop1_2" << "Cop1_3" << "Cop1_4" << "Cop1_5" << "Cop1_6" << "Cop1_7" 
        << "Cop1_8" << "Cop1_9" << "Cop1_10" << "Cop1_11" << "Cop1_12" << "Cop1_13" << "Cop1_14" << "Cop1_15" 
        << "Cop1_16" << "Cop1_17" << "Cop1_18" << "Cop1_19" << "Cop1_20" << "Cop1_21" << "Cop1_22" << "Cop1_23" 
        << "Cop1_24" << "Cop1_25" << "Cop1_26" << "Cop1_27" << "Cop1_28" << "Cop1_29" << "Cop1_30" << "Cop1_31"
        ;

    modelCop1 = new TableListModel(mnemonicCop0, 16);
    tableCop1->horizontalHeader()->hide();
    tableCop1->verticalHeader()->hide();
    tableCop1->setModel(modelCop1);
    tableCop1->hideColumn(2);
    tableCop1->resizeColumnsToContents();
    for (int i=0;i<32;i++)
        tableCop1->setRowHeight(i,15);
    
    //TODO: I can manage to edit individual index'es from here.
    QModelIndex index;
    QString tmp = "XXpos4 testXXXXX";
    index = modelCop1->index(4, 1, QModelIndex());
    modelCop1->setData(index, tmp, Qt::EditRole);
}

void RegisterWidget::update_registers()
{
    RegisterWidget::update_gpr();
    RegisterWidget::update_special();
    RegisterWidget::update_cop0();
    RegisterWidget::update_cop1();
}

void RegisterWidget::update_gpr()
{
    //TODO: update_gpr
}

void RegisterWidget::update_cop0()
{
    //TODO: update_cop0
}

void RegisterWidget::update_special()
{
    //TODO: update_special
}

void RegisterWidget::update_cop1()
{
    //TODO: I can manage to edit individual index'es from here.
    QModelIndex index;
    QString tmp = "XXpos8 testXXXXX";
    index = modelCop1->index(8, 1, QModelIndex());
    modelCop1->setData(index, tmp, Qt::EditRole);
    //TODO: update_cop1
}

