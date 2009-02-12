/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cheatcheatdialog.h                                      *
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

#ifndef CHEATCHEATDIALOG_H
#define CHEATCHEATDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "ui_cheatcheatdialog.h"

namespace core {
    extern "C" {
        #include "../cheat.h"
    }
}

class CheatCheatDialog : public QDialog, private Ui_CheatCheatDialog
{
    Q_OBJECT
    public:
        CheatCheatDialog(core::cheat_t *, QWidget* parent = 0);
        virtual ~CheatCheatDialog();

    private slots:
        void onaccepted();
        void onadd();
        void onremove();
        void codeItemChanged(QStandardItem *item);
        
    private:
        void addCheatCodeToTable(core::cheat_code_t *code);
        core::cheat_t *_cheat;
        QStandardItemModel *tableModel;
};

#endif // CHEATCHEATDIALOG_H
