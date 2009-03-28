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
    // To make it accessable for other than only constructor
    _cheat = cheat;
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));
    connect(pushAdd, SIGNAL(clicked()), this, SLOT(onadd()));
    connect(pushRemove, SIGNAL(clicked()), this, SLOT(onremove()));
    
    core::list_t node = 0;
  
    tableModel = new QStandardItemModel();
    tableView->verticalHeader()->hide();
    tableView->setModel(tableModel);
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);
    QStandardItem *header0 = new QStandardItem(QString("Address"));
    QStandardItem *header1 = new QStandardItem(QString("Value"));
    tableModel->setHorizontalHeaderItem(0, header0);
    tableModel->setHorizontalHeaderItem(1, header1);
    
    if (_cheat->name) {
        lineEditName->setText(tr("%1").arg(_cheat->name));
    }
    if (_cheat->comment) {
        textEditComment->setText(tr("%1").arg(_cheat->comment));
    }
    if (_cheat->always_enabled) {
        radioAlwayEnabled->setChecked(true);
    }
    list_foreach(_cheat->cheat_codes, node) {
        core::cheat_code_t *code = static_cast<core::cheat_code_t*>(node->data);
        addCheatCodeToTable(code);
    }
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);

    // Put this here to avoid signal while initializing tableView
    connect(tableModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(codeItemChanged(QStandardItem *)));
}

CheatCheatDialog::~CheatCheatDialog()
{
    // TODO: destructor
}

void CheatCheatDialog::onaccepted()
{
    QByteArray arr = lineEditName->text().toLatin1();
    int size = arr.size() + 1;
    _cheat->name = (char*) malloc(size);
    if (_cheat->name==NULL) { return; } // TODO: proper error handling?
    memset(_cheat->name, '\0', size);
    strcpy(_cheat->name, arr.data());

    arr = textEditComment->toPlainText().toLatin1();
    size = arr.size() + 1;
    _cheat->comment = (char*) malloc(size);
    if (_cheat->comment==NULL) { return; } // TODO: proper error handling?
    memset(_cheat->comment, '\0', size);
    strcpy(_cheat->comment, arr.data());

    if (radioAlwayEnabled->isChecked()) {
        _cheat->always_enabled = 1;
    }
    if (radioEnabled->isChecked()) {
        _cheat->enabled = 1;
    }
}

void CheatCheatDialog::onadd()
{
    core::cheat_code_t* code = cheat_new_cheat_code(_cheat);
    addCheatCodeToTable(code);
}

void CheatCheatDialog::onremove()
{
    const QModelIndex& index = tableView->selectionModel()->currentIndex();
    QStandardItem* item = 0;
    core::cheat_code_t* code;

    if (index.isValid()) {
        item = tableModel->itemFromIndex(index);
        code = item->data(CheatDialog::CheatCodeRole).value<core::cheat_code_t*>();
        tableModel->removeRow(index.row());
        core::cheat_delete_cheat_code(_cheat, code);
    }
}

void CheatCheatDialog::codeItemChanged(QStandardItem* item)
{
    bool ok;
    core::cheat_code_t* code = NULL;
    
    QModelIndex index = tableModel->indexFromItem(item);
    code = item->data(CheatDialog::CheatCodeRole).value<core::cheat_code_t*>();

    if (code) {
        if (index.column() == 0) { // address
            code->address = code->address, item->text().toUInt(&ok, 16);
            item->setText(tr("%1").arg(code->address, 8, 16, QChar('0')).toUpper());
        } else { // value
            code->value = item->text().toUInt(&ok, 16);;
            item->setText(tr("%1").arg(code->value, 4, 16, QChar('0')).toUpper());
        }
        tableView->resizeColumnsToContents();
        tableView->horizontalHeader()->setStretchLastSection(true);
    }
}

void CheatCheatDialog::addCheatCodeToTable(core::cheat_code_t *code) {
    QList<QStandardItem *> row;
    QStandardItem *address = new QStandardItem(QString("%1").arg(code->address, 8, 16, QChar('0')).toUpper());
    QStandardItem *value = new QStandardItem(QString("%1").arg(code->value, 4, 16, QChar('0')).toUpper());

    // Store the cheat in both rows. siplifies the 'codeItemChanged'
    address->setData(QVariant::fromValue(code), CheatDialog::CheatCodeRole);
    value->setData(QVariant::fromValue(code), CheatDialog::CheatCodeRole);
    row.append(address);
    row.append(value);

    tableModel->appendRow(row);
    tableView->setRowHeight(tableModel->rowCount() - 1,15);
}

