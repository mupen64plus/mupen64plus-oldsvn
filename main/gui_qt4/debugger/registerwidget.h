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

#include <QWidget>
#include <QStringListModel>
#include <QColor>

#include "ui_registerwidget.h"

class TableListModel;

// External variables from core
// used to check for changes
const extern long long int reg_cop1_fgr_64[32];
const extern long long int reg[32];
const extern long int reg_cop0[32];
const extern long long int hi;
const extern long long int lo;


class RegisterWidget : public QWidget, private Ui_RegisterWidget
{
    Q_OBJECT
    public:
        RegisterWidget(QWidget* parent = 0);
        virtual ~RegisterWidget();

    public slots:
        void update_registers();
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

        void update_gpr();
        void update_cop0();
        void update_special();
        void update_cop1();
        void update_ai();
        void update_vi();
        void update_pi();
        void update_ri();
        void update_si();

        long long int gui_fantom_gpr_64[32];
        long int gui_fantom_cop0_32[32];
        long long int gui_fantom_cop1_64[32];
        long long int gui_fantom_hi;
        long long int gui_fantom_lo;
        unsigned int gui_fantom_reg_Ai[6];
        unsigned int gui_fantom_reg_Vi[15];
        unsigned int gui_fantom_reg_Pi[13];
        unsigned int gui_fantom_reg_Ri[5];
        unsigned int gui_fantom_reg_Si[4];

        TableListModel *modelGpr;
        TableListModel *modelCop0;
        TableListModel *modelHiLo;
        TableListModel *modelCop1;
        TableListModel *modelAi;
        TableListModel *modelVi;
        TableListModel *modelPi;
        TableListModel *modelRi;
        TableListModel *modelSi;
        QStringListModel *modelInterupt;

        QStringList mnemonicGPR;
        QStringList mnemonicCop0;
        QStringList mnemonicHiLo;
        QStringList mnemonicCop1;
        QStringList mnemonicAi;
        QStringList mnemonicVi;
        QStringList mnemonicPi;
        QStringList mnemonicRi;
        QStringList mnemonicSi;
        
        static const int HEIGHT = 15;   // row height in various QTableView
};
#endif // __REGISTERWIDGET_H__

