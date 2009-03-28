/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - registerwidget.h                                        *
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

#ifndef __REGISTERWIDGET_H__
#define __REGISTERWIDGET_H__

#include <QtGui>

#include "ui_registerwidget.h"

class TableListModel;

namespace core {
    extern "C" {
        #include "../../../r4300/r4300.h"
        #include "../../../memory/memory.h"
    }
}

class RegisterWidget : public QWidget, private Ui_RegisterWidget
{
    Q_OBJECT
    public:
        RegisterWidget(QWidget *parent = 0);

    public slots:
        void update_registers(unsigned int);
        void radio_toggled();

    private:
        void init_registers();
        void init_gpr();
        void init_cop0();
        void init_special();
        void init_cop1();
        void init_ai();
        void init_vi();
        void init_pi();
        void init_ri();
        void init_si();
        void initTableView(QTableView *tableGpr, TableListModel *modelGpr);
        
        void update_gpr();
        void update_cop0();
        void update_special(unsigned int);
        void update_cop1();
        void update_ai();
        void update_vi();
        void update_pi();
        void update_ri();
        void update_si();
        
        QString getDouble(long long int val);
        QString getSingle(long long int val);
        QString getWord(long long int val);
        QString getLong(long long int val);
        QString getCop1String(long long int val);
        
        long long int gui_fantom_gpr_64[32];
        unsigned int gui_fantom_cop0_32[32];
        long long int gui_fantom_cop1_64[32];
        long long int gui_fantom_hi;
        long long int gui_fantom_lo;
        unsigned int gui_fantom_reg_Ai[6];
        unsigned int gui_fantom_reg_Vi[15];
        unsigned int gui_fantom_reg_Pi[13];
        unsigned int gui_fantom_reg_Ri[5];
        unsigned int gui_fantom_reg_Si[4];
        unsigned int gui_fantom_interrupt[11];
        unsigned int previous_pc;

        TableListModel *modelGpr;
        TableListModel *modelCop0;
        TableListModel *modelHiLo;
        TableListModel *modelInterrupt;
        TableListModel *modelCop1;
        TableListModel *modelAi;
        TableListModel *modelVi;
        TableListModel *modelPi;
        TableListModel *modelRi;
        TableListModel *modelSi;

        QStringList mnemonicGPR;
        QStringList mnemonicCop0;
        QStringList mnemonicHiLo;
        QStringList mnemonicInterrupt;
        QStringList mnemonicCop1;
        QStringList mnemonicAi;
        QStringList mnemonicVi;
        QStringList mnemonicPi;
        QStringList mnemonicRi;
        QStringList mnemonicSi;
        
        static const int HEIGHT = 15;   // row height in various QTableView
        static const int VI_INT      = 0x001;
        static const int COMPARE_INT = 0x002;
        static const int CHECK_INT   = 0x004;
        static const int SI_INT      = 0x008;
        static const int PI_INT      = 0x010;
        static const int SPECIAL_INT = 0x020;
        static const int AI_INT      = 0x040;
        static const int SP_INT      = 0x080;
        static const int DP_INT      = 0x100;
        static const int HW2_INT     = 0x200;
        static const int NMI_INT     = 0x400;

        QColor color_DEFAULT;
        QColor color_CHANGED;
        QColor color_PC;
        QColor color_BP;
};
#endif // __REGISTERWIDGET_H__

