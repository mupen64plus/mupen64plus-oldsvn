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
        #include "../../../r4300/interupt.h"
    }
}

RegisterWidget::RegisterWidget(QWidget* parent) // : QWidget(parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    connect( radioRawhex, SIGNAL( toggled(bool) ), this, SLOT( radio_toggled() ));
    connect( radioSingle, SIGNAL( toggled(bool) ), this, SLOT( radio_toggled() ));
    connect( radioDouble, SIGNAL( toggled(bool) ), this, SLOT( radio_toggled() ));
    connect( radioWord,   SIGNAL( toggled(bool) ), this, SLOT( radio_toggled() ));
    connect( radioLong,   SIGNAL( toggled(bool) ), this, SLOT( radio_toggled() ));
    
    // Set window title
    setWindowTitle(tr("Mupen64Plus Register Viewer"));
    
    color_DEFAULT = QColor(Qt::white);
    color_CHANGED = QColor(Qt::yellow);
    color_PC = QColor(Qt::red);
    color_BP = QColor(Qt::blue);
    
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
    mnemonicGPR
        << "R0" << "AT" << "V0" << "V1" << "A0" << "A1" << "A2" << "A3" 
        << "T0" << "T1" << "T2" << "T3" << "T4" << "T5" << "T6" << "T7" 
        << "S0" << "S1" << "S2" << "S3" << "S4" << "S5" << "S6" << "S7" 
        << "T8" << "T9" << "K0" << "K1" << "GP" << "SP" << "S8" << "RA"
        ;

    // Not a fail safe way to do it,
    // but the size of the QStringList is required to be the same as
    // the size of the array (by design)
    for (int i=0;i<mnemonicGPR.size();i++) {
        gui_fantom_gpr_64[i] = 0x1234;
    }

    modelGpr = new TableListModel(mnemonicGPR, 16);
    initTableView(tableGpr, modelGpr);
}

void RegisterWidget::init_cop0()
{
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

    for (int i=0;i<mnemonicCop0.size();i++) {
        gui_fantom_cop0_32[i] = 0x1234;
    }
    modelCop0 = new TableListModel(mnemonicCop0, 8);
    initTableView(tableCop0, modelCop0);
}

void RegisterWidget::init_special()
{
    // PC
    linePC->setText(tr("XXXXXXXXXXXXXXXX: 0xXXXXXXXX"));
    linePrevPC->setText(tr("XXXXXXXXXXXXXXXX"));
 
    // hi/lo
    gui_fantom_hi = 0x12345678;
    gui_fantom_lo = 0x12345678;
    mnemonicHiLo << "Hi" << "Lo";
    modelHiLo = new TableListModel(mnemonicHiLo, 16);
    initTableView(tableHiLo, modelHiLo);

    // interupt
    mnemonicInterupt 
        << "VI_INT" << "COMPARE_INT" << "CHECK_INT" << "SI_INT" 
        << "PI_INT" << "SPECIAL_INT" << "AI_INT" << "SP_INT" 
        << "DP_INT" << "HW2_INT" << "NMI_INT"
        ;

    for (int i=0;i<mnemonicInterupt.size();i++) {
        gui_fantom_interupt[i] = 0x1234;
    }

    modelInterupt = new TableListModel(mnemonicInterupt, 8);
    initTableView(tableInterupt, modelInterupt);
}

void RegisterWidget::init_cop1()
{
    mnemonicCop1
        << "Cop1_0" << "Cop1_1" << "Cop1_2" << "Cop1_3" << "Cop1_4" << "Cop1_5" << "Cop1_6" << "Cop1_7" 
        << "Cop1_8" << "Cop1_9" << "Cop1_10" << "Cop1_11" << "Cop1_12" << "Cop1_13" << "Cop1_14" << "Cop1_15" 
        << "Cop1_16" << "Cop1_17" << "Cop1_18" << "Cop1_19" << "Cop1_20" << "Cop1_21" << "Cop1_22" << "Cop1_23" 
        << "Cop1_24" << "Cop1_25" << "Cop1_26" << "Cop1_27" << "Cop1_28" << "Cop1_29" << "Cop1_30" << "Cop1_31"
        ;

    for (int i=0;i<mnemonicCop1.size();i++) {
        gui_fantom_cop1_64[i] = 0x1234567890LL;
    }

    modelCop1 = new TableListModel(mnemonicCop0, 16);
    initTableView(tableCop1, modelCop1);
    tableCop1->hideColumn(2);
}

void RegisterWidget::init_ai()
{
    mnemonicAi
        << "AI_DRAM_ADDR_REG" << "AI_LEN_REG"
        << "AI_CONTROL_REG" << "AI_STATUS_REG"
        << "AI_DACRATE_REG" << "AI_BITRATE_REG"
        ;

    for (int i=0;i<mnemonicAi.size();i++) {
        gui_fantom_reg_Ai[i] = 0x12345678;
    }

    modelAi = new TableListModel(mnemonicAi, 10);
    initTableView(tableAi, modelAi);
}

void RegisterWidget::init_vi()
{
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

    for (int i=0;i<mnemonicVi.size();i++) {
        gui_fantom_reg_Vi[i] = 0x12345678;
    }

    modelVi = new TableListModel(mnemonicVi, 10);
    initTableView(tableVi, modelVi);
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

    for (int i=0;i<mnemonicPi.size();i++) {
        gui_fantom_reg_Pi[i] = 0x12345678;
    }

    modelPi = new TableListModel(mnemonicPi, 10);
    initTableView(tablePi, modelPi);
}

void RegisterWidget::init_ri()
{
    mnemonicRi
        << "RI_MODE_REG" << "RI_CONFIG_REG"
        << "RI_CURRENT_LOAD_REG" << "RI_SELECT_REG"
        << "RI_REFRESH_REG"
        ;

    for (int i=0;i<mnemonicRi.size();i++) {
        gui_fantom_reg_Ri[i] = 0x12345678;
    }

    modelRi = new TableListModel(mnemonicRi, 10);
    initTableView(tableRi, modelRi);
}

void RegisterWidget::init_si()
{
    mnemonicSi
        << "SI_DRAM_ADDR_REG" << "SI_PIF_ADDR_RD64B_REG"
        << "SI_PIF_ADDR_WR64B_REG" << "SI_STATUS_REG"
        ;

    for (int i=0;i<mnemonicSi.size();i++) {
        gui_fantom_reg_Si[i] = 0x12345678;
    }

    modelSi = new TableListModel(mnemonicSi, 10);
    initTableView(tableSi, modelSi);
}

void RegisterWidget::update_registers(unsigned int current_pc)
{
    RegisterWidget::update_gpr();
    RegisterWidget::update_special(current_pc);
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
        } else {
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
        } else {
            modelCop0->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_special(unsigned int current_pc)
{
    QModelIndex index;
    QString newstring;
    unsigned int count;

    // PC
    unsigned int instr;
    unsigned int pc;
    pc = current_pc;
    instr = debugger::read_memory_32( pc );
    newstring = QString("%1: 0x%2").arg(pc, 16, 16, QChar('0')).arg(instr, 8, 16, QChar('0')).toUpper();
    linePC->setText(newstring);
    newstring = QString("%1").arg(previous_pc, 16, 16, QChar('0')).toUpper();
    linePrevPC->setText(newstring);
    previous_pc = pc;

    // Hi
    index = modelHiLo->index(0, 1, QModelIndex());
    if(gui_fantom_hi!=hi) {
        gui_fantom_hi=hi;
        newstring = QString("%1").arg(gui_fantom_hi, 16, 16, QChar('0')).toUpper();
        modelHiLo->setData(index, newstring, Qt::EditRole);
        modelHiLo->setData(index, color_CHANGED, Qt::BackgroundRole);
    } else {
        modelHiLo->setData(index, color_DEFAULT, Qt::BackgroundRole);
    }
    // Lo
    index = modelHiLo->index(1, 1, QModelIndex());
    if(gui_fantom_lo!=lo) {
        gui_fantom_lo=lo;
        newstring = QString("%1").arg(gui_fantom_lo, 16, 16, QChar('0')).toUpper();
        modelHiLo->setData(index, newstring, Qt::EditRole);
        modelHiLo->setData(index, color_CHANGED, Qt::BackgroundRole);
    } else {
        modelHiLo->setData(index, color_DEFAULT, Qt::BackgroundRole);
    }

    // TODO: Interupt queue
    int j=1;
    for (int i=0;i<modelInterupt->rowCount();i++) {
        count = core::get_event(j);
        index = modelInterupt->index(i, 1, QModelIndex());
        if (gui_fantom_interupt[i] != count) {
            gui_fantom_interupt[i] = count;
            newstring = QString("%1").arg(gui_fantom_interupt[i], 8, 16, QChar('0')).toUpper();
            modelInterupt->setData(index, newstring, Qt::EditRole);
            modelInterupt->setData(index, color_CHANGED, Qt::BackgroundRole);
            if (gui_fantom_interupt[i] == 0) {
                tableInterupt->setRowHidden(i, true);
            } else {
                tableInterupt->setRowHidden(i, false);
            }
        } else {
            modelInterupt->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
        j=j<<1;
    }
}

void RegisterWidget::update_cop1()
{
    QModelIndex index;
    QString newstring;
    
    for (int i=0;i<32;i++) {
        index = modelCop1->index(i, 1, QModelIndex());
        if (gui_fantom_cop1_64[i] != reg_cop1_fgr_64[i]) {
            gui_fantom_cop1_64[i] = reg_cop1_fgr_64[i];
            
            newstring = getCop1String(gui_fantom_cop1_64[i]);
            modelCop1->setData(index, newstring, Qt::EditRole);
            modelCop1->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
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
        newstring = getCop1String(gui_fantom_cop1_64[i]);
        modelCop1->setData(index, newstring, Qt::EditRole);
    }
}

void RegisterWidget::update_ai()
{
    QModelIndex index;
    QString newstring;
    const unsigned int * regaiptr = &ai_register;

    for (int i=0;i<6;i++) {
        index = modelVi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Ai[i] != (unsigned int)(*(regaiptr+i))) {
            gui_fantom_reg_Ai[i] = (unsigned int)(*(regaiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Ai[i], 10, 16, QChar('0')).toUpper();
            modelAi->setData(index, newstring, Qt::EditRole);
            modelAi->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
            modelAi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_vi()
{
    QModelIndex index;
    QString newstring;
    const unsigned int * regviptr = &vi_register;

    // TODO: Last item #16, not implemented
    // OsTvType in GTK debugger frontend
    for (int i=0;i<15;i++) {
        index = modelVi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Vi[i] != (unsigned int)(*(regviptr+i))) {
            gui_fantom_reg_Vi[i] = (unsigned int)(*(regviptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Vi[i], 10, 16, QChar('0')).toUpper();
            modelVi->setData(index, newstring, Qt::EditRole);
            modelVi->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
            modelVi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_pi()
{
    QModelIndex index;
    QString newstring;
    const unsigned int * regpiptr = &pi_register;

    for (int i=0;i<13;i++) {
        index = modelPi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Pi[i] != (unsigned int)(*(regpiptr+i))) {
            gui_fantom_reg_Pi[i] = (unsigned int)(*(regpiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Pi[i], 10, 16, QChar('0')).toUpper();
            modelPi->setData(index, newstring, Qt::EditRole);
            modelPi->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
            modelPi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_ri()
{
    QModelIndex index;
    QString newstring;
    const unsigned int * regriptr = &ri_register;

    for (int i=0;i<5;i++) {
        index = modelRi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Ri[i] != (unsigned int)(*(regriptr+i))) {
            gui_fantom_reg_Ri[i] = (unsigned int)(*(regriptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Ri[i], 10, 16, QChar('0')).toUpper();
            modelRi->setData(index, newstring, Qt::EditRole);
            modelRi->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
            modelRi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

void RegisterWidget::update_si()
{
    QModelIndex index;
    QString newstring;
    const unsigned int * regsiptr = &si_register;

    for (int i=0;i<4;i++) {
        index = modelSi->index(i, 1, QModelIndex());
        if (gui_fantom_reg_Si[i] != (unsigned int)(*(regsiptr+i))) {
            gui_fantom_reg_Si[i] = (unsigned int)(*(regsiptr+i));
            newstring = QString("%1").arg(gui_fantom_reg_Si[i], 10, 16, QChar('0')).toUpper();
            modelSi->setData(index, newstring, Qt::EditRole);
            modelSi->setData(index, color_CHANGED, Qt::BackgroundRole);
        } else {
            modelSi->setData(index, color_DEFAULT, Qt::BackgroundRole);
        }
    }
}

QString RegisterWidget::getSingle(long long int val)
{
    QString str;
    float *flt;

    flt = (float *)&val;
    str.sprintf("%g", *flt);
    return str;
}

QString RegisterWidget::getDouble(long long int val)
{
    QString str;
    double *dbl;

    dbl = (double *)&val;
    str.sprintf("%g", *dbl);
    return str;
}

QString RegisterWidget::getWord(long long int val)
{
    QString str;
    float *flt;

    // TODO. Need verification
    //*reg_cop1_simple[cffd] = *((int*)reg_cop1_simple[cffs]);
    flt = (float *)&val;
    *flt = *((int*)flt);
    str.sprintf("%g", *flt);
    return str;
}

QString RegisterWidget::getLong(long long int val)
{
    QString str;
    double *dbl;

    // TODO. Need verification
    //*reg_cop1_double[cffd] = *((long long*)reg_cop1_double[cffs]);
    dbl = (double *)&val;
    *dbl = *((long long*)dbl);
    str.sprintf("%g", *dbl);
    return str;
}

QString RegisterWidget::getCop1String(long long int value)
{
    if (radioRawhex->isChecked()) {
        return QString("%1").arg(value, 16, 16, QChar('0')).toUpper();
    } else if (radioSingle->isChecked()) {
        return getSingle(value);
    } else if (radioDouble->isChecked()) {
        return getDouble(value);
    } else if (radioWord->isChecked()) {
        return getWord(value);
    } else if (radioLong->isChecked()) {
        return getLong(value);
    }
    return "";
}


void RegisterWidget::initTableView(QTableView* table, TableListModel* model)
{
    table->setModel(model);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i=0;i<model->rowCount();i++) {
        table->setRowHeight(i,HEIGHT);
    }
}

