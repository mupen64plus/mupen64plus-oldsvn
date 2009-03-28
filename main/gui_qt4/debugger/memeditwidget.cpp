/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memeditwidget.cpp                                       *
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

#include "memeditwidget.h"
#include "memeditmodel.h"

namespace core {
    extern "C" {
        #include "../../gui.h"
    }
}

namespace debugger {
    extern "C" {
        #include "../../../debugger/debugger.h"
    }
}

MemEditWidget::MemEditWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    address = 0x80000000;
    ignore_data_changed = false;
    lineEdit->setInputMask("HHHHHHHH");    
    lineEdit->setText(QString("%1").arg(address, 8, 16, QChar('0')).toUpper());

    model = new MemEditModel();
    tableView->setModel(model);
    tableView->verticalHeader()->hide();

    connect( lineEdit, SIGNAL( textEdited (const QString &)), this, SLOT(text_edited(const QString &)));
    connect( model, SIGNAL( dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(data_changed(const QModelIndex &, const QModelIndex &)));
}

void MemEditWidget::update_memedit()
{
    bool ok;
    unsigned int i, j, addr = address;
    unsigned char byte;
    QStringList line;
    QModelIndex index;
    QString byteString, asciiString;

    // avoid any action on dataChanged signal when programatically updating the table
    ignore_data_changed = true;
    model->removeRows(0, model->rowCount());
    ok = model->insertRows(0, numlines, index);
    for(i = 0; i < numlines; i++) {
        index = model->index(i, AddressColumn, QModelIndex());
        byteString = QString("%1").arg(addr, 8, 16, QChar('0')).toUpper();
        ok = model->setData(index, byteString, Qt::EditRole);
        asciiString = "";
        for(j = 0; j < bytesperline; j++) {
            index = model->index(i, HexColumn + j, QModelIndex());
            byte = debugger::read_memory_8(addr + j);
            byteString = QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
            ok = model->setData(index, byteString, Qt::EditRole);
            asciiString += QString("%1").arg((QChar) byte);
        }
        index = model->index(i, AsciiColumn, QModelIndex());
        ok = model->setData(index, asciiString, Qt::EditRole);
        addr += bytesperline;
    }
    tableView->horizontalHeader()->setStretchLastSection(true);
    for (i = 0; i < numlines; i++) {
        tableView->setRowHeight(i, 15);
    }
    tableView->resizeColumnsToContents();
    ignore_data_changed = false;
}

void MemEditWidget::text_edited(const QString &text)
{
    bool ok;
    unsigned int addr;

    addr = text.toUInt( &ok, 16 );
    if (ok) {
        address = addr;
        update_memedit();
    }
}

void MemEditWidget::data_changed(const QModelIndex &first, const QModelIndex &last)
{
    Q_UNUSED(last);
    bool ok, ok1, ok2;
    unsigned char val;
    unsigned int addr;
    QString tmp, byteString;
    bool revert = false;
    unsigned char byte;

    // TODO: current implementation only handles "single edits"
    if (ignore_data_changed  == false) {
        QModelIndex valIndex = first;
        QModelIndex addrIndex = model->index(valIndex.row(), 0);
        tmp = model->data(valIndex,Qt::DisplayRole).toString();
        val = (unsigned char) tmp.toUInt( &ok1, 16 );
        tmp = model->data(addrIndex,Qt::DisplayRole).toString();
        addr = tmp.toUInt( &ok2, 16 );
        addr += first.column() - 1;
        if (ok1 && ok2) {
            debugger::write_memory_8(addr, val);
            core::debugger_update_desasm();
            byte = debugger::read_memory_8(addr);
            if (byte != val) {
                revert = true;
            }
        } else {
            byte = debugger::read_memory_8(addr);
            revert = true;
        }
    }
    if (revert) {
        byteString = QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
        ok = model->setData(first, byteString, Qt::EditRole);
    }
}

