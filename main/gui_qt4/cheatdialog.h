/**
* Mupen64 - cheatdialog.h
* Copyright (C) 2008 slougi
*
* Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
*
* This program is free software; you can redistribute it and/
* or modify it under the terms of the GNU General Public Li-
* cence as published by the Free Software Foundation; either
* version 2 of the Licence, or any later version.
*
* This program is distributed in the hope that it will be use-
* ful, but WITHOUT ANY WARRANTY; without even the implied war-
* ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public Licence for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
* USA.
**/

#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include <QDialog>
#include "ui_cheatdialog.h"

class QStandardItem;
class QStandardItemModel;
namespace core {
    extern "C" {
        #include "../cheat.h"
    }
}

class CheatDialog : public QDialog, private Ui_CheatDialog
{
    Q_OBJECT
    public:
        CheatDialog(QWidget* parent = 0);

    private:
        QStandardItemModel* m_model;
        QStandardItem* createItemForCheat(QString name, core::cheat_t* cheat);
};

#endif // CHEATDIALOG_H
