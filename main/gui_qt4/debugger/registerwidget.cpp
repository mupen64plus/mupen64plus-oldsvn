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
    }
}

QColor color_DEFAULT = QColor(Qt::white);
QColor color_CHANGED = QColor(Qt::yellow);
QColor color_PC = QColor(Qt::red);
QColor color_BP = QColor(Qt::blue);


RegisterWidget::RegisterWidget(QWidget* parent) // : QWidget(parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    connect( radioRawhex, SIGNAL( toggled() ), this, SLOT( radio_toggled() ) );
    connect( radioSingle, SIGNAL( toggled() ), this, SLOT( radio_toggled() ) );
    connect( radioDouble, SIGNAL( toggled() ), this, SLOT( radio_toggled() ) );
    connect( radioWord,   SIGNAL( toggled() ), this, SLOT( radio_toggled() ) );
    connect( radioLong,   SIGNAL( toggled() ), this, SLOT( radio_toggled() ) );
    
    // Set window title
    setWindowTitle(tr("Mupen64Plus Register Viewer"));
    
    RegisterWidget::init_registers();
}

RegisterWidget::~RegisterWidget()
{
    //TODO: Implement destructor
}

void RegisterWidget::init_registers()
{
    RegisterWidget::init_gpr();
    RegisterWidget::init_cop0();
    RegisterWidget::init_special();
    RegisterWidget::init_cop1();
    RegisterWidget::init_ai();
    RegisterWidget::init_vi();
    RegisterWidget::init_pi();
    RegisterWidget::init_ri();
    RegisterWidget::init_si();
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
    tableGpr->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelGpr->rowCount();i++)
        tableGpr->setRowHeight(i,HEIGHT);
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
    tableCop0->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelCop0->rowCount();i++)
        tableCop0->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_special()
{
    linePC->setText(tr("XXXXXXXXXXXXXXXX: 0xXXXXXXXX"));
    linePrevPC->setText(tr("XXXXXXXXXXXXXXXX"));
 
    //TODO: interupt list

    //hi/lo
    gui_fantom_hi = 0x12345678;
    gui_fantom_lo = 0x12345678;
    mnemonicHiLo << "Hi" << "Lo";
    modelHiLo = new TableListModel(mnemonicHiLo, 16);
    tableHiLo->horizontalHeader()->hide();
    tableHiLo->verticalHeader()->hide();
    tableHiLo->setModel(modelHiLo);
    tableHiLo->resizeColumnsToContents();
    tableHiLo->horizontalHeader()->setStretchLastSection(true);
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
    tableCop1->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelCop1->rowCount();i++)
        tableCop1->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_ai()
{
    for(int i=0; i<6; i++)
        gui_fantom_reg_Ai[i] = 0x12345678;
        
    mnemonicAi
        << "AI_DRAM_ADDR_REG" << "AI_LEN_REG"
        << "AI_CONTROL_REG" << "AI_STATUS_REG"
        << "AI_DACRATE_REG" << "AI_BITRATE_REG"
        ;

    modelAi = new TableListModel(mnemonicAi, 10);
    tableAi->horizontalHeader()->hide();
    tableAi->verticalHeader()->hide();
    tableAi->setModel(modelAi);
    tableAi->resizeColumnsToContents();
    tableAi->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelAi->rowCount();i++)
        tableAi->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_vi()
{
    for(int i=0; i<15; i++)
        gui_fantom_reg_Vi[i] = 0x12345678;
        
    mnemonicVi
        << "VI_STATUS_REG" << "VI_DRAM_ADDR_REG"
        << "VI_WIDTH_REG" << "VI_INTR_REG"
        << "VI_CURRENT_LINE_REG" << "VI_TIMING_REG"
        << "VI_V_SYNC_REG" << "VI_H_SYNC_REG" << "VI_H_SYNC_LEAP_REG"
        << "VI_H_START_REG" << "VI_V_START_REG"
        << "VI_V_BURST_REG"
        << "VI_X_SCALE_REG" << "VI_Y_SCALE_REG"
        << "VI_DELAY" << "OsTvType"
        ;

    modelVi = new TableListModel(mnemonicVi, 10);
    tableVi->horizontalHeader()->hide();
    tableVi->verticalHeader()->hide();
    tableVi->setModel(modelVi);
    tableVi->resizeColumnsToContents();
    tableVi->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelVi->rowCount();i++)
        tableVi->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_pi()
{
    mnemonicPi
        << "PI_DRAM_ADDR_REG" << "PI_CART_ADDR_REG"
        << "PI_RD_LEN_REG" << "PI_WR_LEN_REG"
        << "PI_STATUS_REG"
        << "PI_BSD_DOM1_LAT_REG" << "PI_BSD_DOM1_PWD_REG"
        << "PI_BSD_DOM1_PGS_REG" << "PI_BSD_DOM1_RLS_REG"
        << "PI_BSD_DOM2_LAT_REG" << "PI_BSD_DOM2_PWD_REG"
        << "PI_BSD_DOM2_PGS_REG" << "PI_BSD_DOM2_RLS_REG"
        ;

    for(int i=0; i<mnemonicPi.size(); i++)
        gui_fantom_reg_Pi[i] = 0x12345678;
        

    modelPi = new TableListModel(mnemonicPi, 10);
    tablePi->horizontalHeader()->hide();
    tablePi->verticalHeader()->hide();
    tablePi->setModel(modelPi);
    tablePi->resizeColumnsToContents();
    tablePi->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelPi->rowCount();i++)
        tablePi->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_ri()
{
    mnemonicRi
        << "RI_MODE_REG" << "RI_CONFIG_REG"
        << "RI_CURRENT_LOAD_REG" << "RI_SELECT_REG"
        << "RI_REFRESH_REG"
        ;

    for(int i=0; i<mnemonicRi.size(); i++)
        gui_fantom_reg_Ri[i] = 0x12345678;
        

    modelRi = new TableListModel(mnemonicRi, 10);
    tableRi->horizontalHeader()->hide();
    tableRi->verticalHeader()->hide();
    tableRi->setModel(modelRi);
    tableRi->resizeColumnsToContents();
    tableRi->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelRi->rowCount();i++)
        tableRi->setRowHeight(i,HEIGHT);
}

void RegisterWidget::init_si()
{
    mnemonicSi
        << "SI_DRAM_ADDR_REG" << "SI_PIF_ADDR_RD64B_REG"
        << "SI_PIF_ADDR_WR64B_REG" << "SI_STATUS_REG"
        ;

    for(int i=0; i<mnemonicSi.size(); i++)
        gui_fantom_reg_Si[i] = 0x12345678;
        

    modelSi = new TableListModel(mnemonicSi, 10);
    tableSi->horizontalHeader()->hide();
    tableSi->verticalHeader()->hide();
    tableSi->setModel(modelSi);
    tableSi->resizeColumnsToContents();
    tableSi->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<modelSi->rowCount();i++)
        tableSi->setRowHeight(i,HEIGHT);
}

void RegisterWidget::update_registers()
{
    RegisterWidget::update_gpr();
    RegisterWidget::update_special();
    RegisterWidget::update_cop0();
    RegisterWidget::update_cop1();
    RegisterWidget::update_ai();
    RegisterWidget::update_vi();
    RegisterWidget::update_pi();
    RegisterWidget::update_ri();
    RegisterWidget::update_si();
}

void RegisterWidget::update_gpr()
{
    QModelIndex index;
    QString newstring;

    for(int i=0; i<32; i++) {
        index = modelCop1->index(i, 1, QModelIndex());
        if(gui_fantom_gpr_64[i]!=reg[i]) {
            gui_fantom_gpr_64[i] = reg[i];
            newstring = QString("%1").arg(gui_fantom_gpr_64[i], 16, 16, QChar('0')).toUpper();
            modelGpr->setData(index, newstring, Qt::EditRole);
            modelGpr->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelGpr->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
    tableGpr->update();
}

void RegisterWidget::update_cop0()
{
    QModelIndex index;
    QString newstring;

    for(int i=0; i<32; i++) {
        index = modelCop1->index(i, 1, QModelIndex());
        if(gui_fantom_cop0_32[i]!=reg_cop0[i]) {
            gui_fantom_cop0_32[i] = reg_cop0[i];
            newstring = QString("%1").arg(gui_fantom_cop0_32[i], 8, 16, QChar('0')).toUpper();
            modelCop0->setData(index, newstring, Qt::EditRole);
            modelCop0->setData(index, color_CHANGED, Qt::BackgroundRole);
            // TODO: change color
        }
        else {
            modelCop0->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_special()
{
    QModelIndex index;
    QString newstring;

    // TODO: PC
    // unsigned int instr;
    // unsigned int pc;
    // pc = debuggerWidget->get_current_pc();
    // instr = debugger::read_memory_32( pc );
    // newstring = QString("%1: 0x%2").arg(pc, 16, 16, QChar('0')).arg(instr, 8, 16, QChar('0')).toUpper();

    // Hi
    index = modelHiLo->index(0, 1, QModelIndex());
    if(gui_fantom_hi!=hi) {
        gui_fantom_hi=hi;
        newstring = QString("%1").arg(gui_fantom_hi, 16, 16, QChar('0')).toUpper();
        modelHiLo->setData(index, newstring, Qt::EditRole);
        modelHiLo->setData(index, color_CHANGED, Qt::BackgroundRole);
    }
    else {
        modelHiLo->setData(index, color_DEFAULT, Qt::BackgroundRole);
    }
    // Lo
    index = modelHiLo->index(1, 1, QModelIndex());
    if(gui_fantom_lo!=lo) {
        gui_fantom_lo=lo;
        newstring = QString("%1").arg(gui_fantom_lo, 16, 16, QChar('0')).toUpper();
        modelHiLo->setData(index, newstring, Qt::EditRole);
        modelHiLo->setData(index, color_CHANGED, Qt::BackgroundRole);
    }
    else {
        modelHiLo->setData(index, color_DEFAULT, Qt::BackgroundRole);
    }

    // TODO: Interupt queue
}

void RegisterWidget::update_cop1()
{
    QModelIndex index;
    QString newstring;
    
    for (int i=0;i<32;i++) {
        index = modelCop1->index(i, 1, QModelIndex());
        if (gui_fantom_cop1_64[i] != reg_cop1_fgr_64[i]) {
            gui_fantom_cop1_64[i] = reg_cop1_fgr_64[i];
            if (radioRawhex->isChecked()) {
                newstring = QString("%1").arg(gui_fantom_cop1_64[i], 16, 16, QChar('0')).toUpper();
            }
            else if (radioSingle->isChecked()) {
                newstring = QString("Not implemented!");
            }
            else if (radioDouble->isChecked()) {
                newstring = QString("Not implemented!");
            }
            else if (radioWord->isChecked()) {
                newstring = QString("Not implemented!");
            }
            else if (radioLong->isChecked()) {
                newstring = QString("Not implemented!");
            }
            else {
                qDebug() << "Strange. No radio buttons checked";
            }
            modelCop1->setData(index, newstring, Qt::EditRole);
            modelCop1->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelCop1->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::radio_toggled()
{
    QModelIndex index;
    QString newstring;

    for (int i=0;i<32;i++) {
        index = modelCop1->index(i, 1, QModelIndex());
        if (radioRawhex->isChecked()) {
            newstring = QString("%1").arg(gui_fantom_cop1_64[i], 16, 16, QChar('0')).toUpper();
        }
        else if (radioSingle->isChecked()) {
            newstring = QString("Not implemented!");
        }
        else if (radioDouble->isChecked()) {
            newstring = QString("Not implemented!");
        }
        else if (radioWord->isChecked()) {
            newstring = QString("Not implemented!");
        }
        else if (radioLong->isChecked()) {
            newstring = QString("Not implemented!");
        }
        else {
            qDebug() << "Strange. No radio buttons checked";
        }
        modelCop1->setData(index, newstring, Qt::EditRole);        
    }
}

void RegisterWidget::update_ai()
{
    QModelIndex index;
    QString newstring;
    extern unsigned int ai_register;
    unsigned int * regaiptr = &ai_register;

    for (int i=0;i<6;i++) {
        index = modelVi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Ai[i] != (unsigned int)(*(regaiptr+i))) {
            gui_fantom_reg_Ai[i] = (unsigned int)(*(regaiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Ai[i], 10, 16, QChar('0')).toUpper();
            modelAi->setData(index, newstring, Qt::EditRole);
            modelAi->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelAi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_vi()
{
    QModelIndex index;
    QString newstring;
    extern unsigned int vi_register;
    unsigned int * regviptr = &vi_register;

    // TODO: Last item #16, not implemented
    // OsTvType in GTK debugger frontend
    for (int i=0;i<15;i++) {
        index = modelVi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Vi[i] != (unsigned int)(*(regviptr+i))) {
            gui_fantom_reg_Vi[i] = (unsigned int)(*(regviptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Vi[i], 10, 16, QChar('0')).toUpper();
            modelVi->setData(index, newstring, Qt::EditRole);
            modelVi->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelVi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_pi()
{
    QModelIndex index;
    QString newstring;
    extern unsigned int pi_register;
    unsigned int * regpiptr = &pi_register;

    for (int i=0;i<13;i++) {
        index = modelPi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Pi[i] != (unsigned int)(*(regpiptr+i))) {
            gui_fantom_reg_Pi[i] = (unsigned int)(*(regpiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Pi[i], 10, 16, QChar('0')).toUpper();
            modelPi->setData(index, newstring, Qt::EditRole);
            modelPi->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelPi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_ri()
{
    QModelIndex index;
    QString newstring;
    extern unsigned int ri_register;
    unsigned int * regriptr = &ri_register;

    for (int i=0;i<5;i++) {
        index = modelRi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Ri[i] != (unsigned int)(*(regriptr+i))) {
            gui_fantom_reg_Ri[i] = (unsigned int)(*(regriptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Ri[i], 10, 16, QChar('0')).toUpper();
            modelRi->setData(index, newstring, Qt::EditRole);
            modelRi->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelRi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_si()
{
    QModelIndex index;
    QString newstring;
    extern unsigned int si_register;
    unsigned int * regsiptr = &si_register;

    for (int i=0;i<4;i++) {
        index = modelSi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Si[i] != (unsigned int)(*(regsiptr+i))) {
            gui_fantom_reg_Si[i] = (unsigned int)(*(regsiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Si[i], 10, 16, QChar('0')).toUpper();
            modelSi->setData(index, newstring, Qt::EditRole);
            modelSi->setData(index, color_CHANGED, Qt::BackgroundRole);
        }
        else {
            modelSi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

