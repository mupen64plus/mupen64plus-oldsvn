/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - breakpointswidget.cpp                                   *
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

#include "breakpointswidget.h"

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

BreakpointsWidget::BreakpointsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    connect( pushAdd,     SIGNAL( clicked() ), this, SLOT( add()     )); 
    connect( pushRemove,  SIGNAL( clicked() ), this, SLOT( remove()  )); 
    connect( pushEnable,  SIGNAL( clicked() ), this, SLOT( enable()  )); 
    connect( pushDisable, SIGNAL( clicked() ), this, SLOT( disable() )); 
    connect( pushToggle,  SIGNAL( clicked() ), this, SLOT( toggle()  ));
    connect( pushEdit,    SIGNAL( clicked() ), this, SLOT( edit()    ));

    model = new QStringListModel();
    listView->setModel(model);
    update_breakpoint();
}

void BreakpointsWidget::update_breakpoint()
{   
    QStringList list;

    for(int row = 0; row < debugger::g_NumBreakpoints; row++ ) {
        list << get_breakpoint_display_string(row);
    }
    model->setStringList(list);
    core::debugger_update_desasm();
}

void BreakpointsWidget::add()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Breakpoint"),
                                      tr("Value:"), QLineEdit::Normal, "", &ok);
    if (ok) {
        breakpoint_parse(text);
        update_breakpoint();
    }
}

void BreakpointsWidget::remove()
{
    const QModelIndex& index = listView->selectionModel()->currentIndex();

    if (index.isValid()) {
        debugger::remove_breakpoint_by_num(index.row());
        update_breakpoint();
    }
}

void BreakpointsWidget::enable()
{
    ToggleBreakpoint(Enable);
}

void BreakpointsWidget::disable()
{
    ToggleBreakpoint(Disable);
}
 
void BreakpointsWidget::toggle()
{
    ToggleBreakpoint(Toggle);
}

void BreakpointsWidget::edit()
{
    const QModelIndex& index = listView->selectionModel()->currentIndex();
    QString old;
    bool ok;

    if (index.isValid()) {
        old = model->data(index,Qt::DisplayRole).toString();
        old.remove(0,4);
        QString text = QInputDialog::getText(this, tr("Edit Breakpoint"),
                                          tr("New Value:"), QLineEdit::Normal, old, &ok);
        if (ok) {
            breakpoint_parse(text, index.row());
            update_breakpoint();
        }
    }
}

QString BreakpointsWidget::get_breakpoint_display_string(int row)
{
    QString str;
    QString address;
    debugger::breakpoint *bpt = &debugger::g_Breakpoints[row];

    QString flagR = (bpt->flags & BPT_FLAG_READ) ? "R" : "_";
    QString flagW = (bpt->flags & BPT_FLAG_WRITE) ? "W" : "_";
    QString flagX = (bpt->flags & BPT_FLAG_EXEC) ? "X" : "_";
    QString flagL = (bpt->flags & BPT_FLAG_LOG) ? "L" : "_";
    QString status = (bpt->flags & BPT_FLAG_ENABLED) ? "e" : "d";

    if(bpt->address == bpt->endaddr) {
        address = QString("0x%1").arg(bpt->address, 8, 16, QChar('0')).toUpper();
    } else {
        address = QString("%1 - %2").arg(bpt->address, 8, 16, QChar('0')).arg(bpt->endaddr, 8, 16, QChar('0')).toUpper();
    }
    str = QString("%1 / %2%3%4%5 %6").arg(status).arg(flagR).arg(flagW).arg(flagX).arg(flagL).arg(address);
    return str;
}

void BreakpointsWidget::ToggleBreakpoint(int flag)
{
    QModelIndex index = listView->selectionModel()->currentIndex();
    int i = index.row();
    if (flag == Enable) {
        debugger::enable_breakpoint(i);
    } else if (flag == Disable) {
        debugger::disable_breakpoint(i);
    } else if (BPT_CHECK_FLAG(debugger::g_Breakpoints[i], BPT_FLAG_ENABLED)) {
        debugger::disable_breakpoint(i);
    } else {
        debugger::enable_breakpoint(i);
    }
    if(debugger::g_NumBreakpoints) {
        update_breakpoint();
    }    
}

void BreakpointsWidget::breakpoint_parse(QString text, int row)
{
    debugger::breakpoint bp;
    //Enabled by default
    bp.flags = BPT_FLAG_ENABLED;
    bp.address = 0;
    bp.endaddr = 0;
    bool ok;

    if (text.contains("*", Qt::CaseInsensitive)) {
        bp.flags &= ~BPT_FLAG_ENABLED;
    } else if(text.contains("r", Qt::CaseInsensitive)) {
        bp.flags |= BPT_FLAG_READ;
    } else if(text.contains("w", Qt::CaseInsensitive)) {
        bp.flags |= BPT_FLAG_WRITE;
    } else if(text.contains("x", Qt::CaseInsensitive)) {
        bp.flags |= BPT_FLAG_EXEC;
    } else if(text.contains("l", Qt::CaseInsensitive)) {
        bp.flags |= BPT_FLAG_LOG;
    }
    
    //If none of r/w/x specified, default to exec
    if(!(bp.flags & (BPT_FLAG_EXEC | BPT_FLAG_READ | BPT_FLAG_WRITE))) {
        BPT_SET_FLAG(bp, BPT_FLAG_EXEC);
    }

    int first = text.indexOf(QRegExp("[0123456789abcdefABCDEF]"));
    text.remove(0,first);
    int last = text.lastIndexOf(QRegExp("[0123456789abcdefABCDEF]"));
    text.truncate(last+1);

    QStringList parts = QString(text).split(" ", QString::SkipEmptyParts);
    if (parts.size() == 1) {
        bp.address = parts.at(0).toULong( &ok, 16 );
        bp.endaddr = bp.address;
    } else if (parts.size() == 2) {
        bp.address = parts.at(0).toULong( &ok, 16 );
        bp.endaddr = parts.at(1).toULong( &ok, 16 );
    } else {
        ok = false;
    }

    if (ok) {
        if (row < 0) {
            if(debugger::add_breakpoint_struct(&bp) == -1) {
                QMessageBox::warning(this, tr("Warning"),
                        tr("Cannot add any more breakpoints."),
                        QMessageBox::Ok);
            }
        } else {
            debugger::replace_breakpoint_num(row, &bp);
        }
    }
}

