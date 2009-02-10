/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cheatcheatdialog.cpp                                    *
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

#include "cheatcheatdialog.h"
#include "cheatdialog.h"    //Q_DECLARE_METATYPE(core::cheat_code_t*)

CheatCheatDialog::CheatCheatDialog(core::cheat_t *cheat, QWidget* parent) : QDialog(parent)
{
    setupUi(this);

    // Use this as our internal pointer to the cheat we are working on.
    _cheat = cheat;
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));
    connect(pushAdd, SIGNAL(clicked()), this, SLOT(onadd()));
    
    QString name, comment;
    core::list_t node = 0;
  
    name = QString(_cheat->name);
    comment = QString(_cheat->comment);
    if (_cheat->name) {
        lineEditName->setText(name);
        lineEditName->setEnabled(false);
    }
    if (_cheat->comment)
        textEditComment->setText(comment);

    tableModel = new QStandardItemModel();
    tableView->verticalHeader()->hide();
    tableView->setModel(tableModel);
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);
    QStandardItem *header0 = new QStandardItem(QString("Address"));
    QStandardItem *header1 = new QStandardItem(QString("Value"));
    tableModel->setHorizontalHeaderItem(0, header0);
    tableModel->setHorizontalHeaderItem(1, header1);
    
    list_foreach(_cheat->cheat_codes, node)
    {
        int row = tableModel->rowCount();
        core::cheat_code_t *code = static_cast<core::cheat_code_t*>(node->data);

        QStandardItem *row0 = new QStandardItem(QString("%1").arg(code->address, 8, 16, QChar('0')).toUpper());
        QStandardItem *row1 = new QStandardItem(QString("%1").arg(code->value, 4, 16, QChar('0')).toUpper());

        row0->setData(QVariant::fromValue(code), Qt::UserRole + 4);
        row1->setData(QVariant::fromValue(code), Qt::UserRole + 4);

        tableModel->setItem(row,0,row0);
        tableModel->setItem(row,1,row1);
        tableView->setRowHeight(row,15);
    }
    
    if (_cheat->always_enabled)
        radioAlwayEnabled->setChecked(true);
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);

    connect(tableModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(codeItemChanged(QStandardItem *)));
}

CheatCheatDialog::~CheatCheatDialog()
{
    // TODO: destructor
}

void CheatCheatDialog::onaccepted()
{
    int size;
    QByteArray arr = lineEditName->text().toLatin1();
    size = arr.size() + 1;
    _cheat->name = (char*) malloc(size);
    if (_cheat->name==NULL) return; // TODO: proper error handling
    memset(_cheat->name, '\0', size);
    strcpy(_cheat->name, arr.data());

    arr = textEditComment->toPlainText().toLatin1();
    size = arr.size() + 1;
    _cheat->comment = (char*) malloc(size);
    if (_cheat->comment==NULL) return; // TODO: proper error handling
    memset(_cheat->comment, '\0', size);
    strcpy(_cheat->comment, arr.data());

    if (radioAlwayEnabled->isChecked())
        _cheat->always_enabled = 1;
    if (radioEnabled->isChecked())
        _cheat->enabled = 1;
}

void CheatCheatDialog::onadd()
{
    core::cheat_code_t* code = cheat_new_cheat_code(_cheat);
    code->address = 0;
    code->value = 0;
    code->old_value = 0;
    int row = tableModel->rowCount();

    QStandardItem *row0 = new QStandardItem(QString("%1").arg(code->address, 8, 16, QChar('0')).toUpper());
    QStandardItem *row1 = new QStandardItem(QString("%1").arg(code->value, 4, 16, QChar('0')).toUpper());

    row0->setData(QVariant::fromValue(code), Qt::UserRole + 4);
    row1->setData(QVariant::fromValue(code), Qt::UserRole + 4);

    tableModel->setItem(row,0,row0);
    tableModel->setItem(row,1,row1);
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setRowHeight(row,15);
}

void CheatCheatDialog::codeItemChanged(QStandardItem* item)
{
    bool ok;
    core::cheat_code_t* code = 0;
    
    QString str = item->text();
    QModelIndex index = tableModel->indexFromItem(item);
    code = item->data(Qt::UserRole + 4).value<core::cheat_code_t*>();

    if (code) {
        if (index.column() == 0) { // address
            code->address = str.toUInt(&ok, 16);;
        } else { // value
            code->value = str.toUInt(&ok, 16);;
        }
        QStandardItem *row0 = new QStandardItem(QString("%1").arg(code->address, 8, 16, QChar('0')).toUpper());
        QStandardItem *row1 = new QStandardItem(QString("%1").arg(code->value, 4, 16, QChar('0')).toUpper());
        row0->setData(QVariant::fromValue(code));
        row1->setData(QVariant::fromValue(code));
        tableModel->setItem(index.row(),0,row0);
        tableModel->setItem(index.row(),1,row1);
    }
}

