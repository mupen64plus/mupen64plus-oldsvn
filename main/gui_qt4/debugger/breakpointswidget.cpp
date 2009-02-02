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

#include <QtGui>

#include "breakpointswidget.h"

namespace debugger {
    extern "C" {
        #include "../../../debugger/debugger.h"
    }
}

BreakpointsWidget::BreakpointsWidget(QWidget* parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
    connect( pushAdd, SIGNAL( clicked() ), this, SLOT( onadd() ) ); 
    connect( pushRemove, SIGNAL( clicked() ), this, SLOT( onremove() ) ); 
    connect( pushEnable, SIGNAL( clicked() ), this, SLOT( onenable() ) ); 
    connect( pushDisable, SIGNAL( clicked() ), this, SLOT( ondisable() ) ); 
    connect( pushToggle, SIGNAL( clicked() ), this, SLOT( ontoggle() ) );
    connect( pushEdit, SIGNAL( clicked() ), this, SLOT( onedit() ) );
//    listView->setModel(model);
    BreakpointsWidget::update_breakpoint();
}

BreakpointsWidget::~BreakpointsWidget()
{
//    delete model;
}

void BreakpointsWidget::update_breakpoint()
{   
    QStringList list;
    QStringListModel *model = new QStringListModel();
    for(int row=0; row < debugger::g_NumBreakpoints; row++ ) {
        list << get_breakpoint_display_string(row);
    }
    model->setStringList(list);
    listView->setModel(model);
}

void BreakpointsWidget::onadd()
{
    bool ok;
    debugger::breakpoint newbp;
    //Enabled by default
    newbp.flags = BPT_FLAG_ENABLED;
    newbp.address = 0;
    newbp.endaddr = 0;

    QString text = QInputDialog::getText(this, tr("New Breakpoint"),
                                      tr("Value:"), QLineEdit::Normal, "", &ok);
                                      
    if (text.contains("*", Qt::CaseInsensitive)) newbp.flags &= ~BPT_FLAG_ENABLED;
    else if(text.contains("r", Qt::CaseInsensitive)) newbp.flags |= BPT_FLAG_READ;
    else if(text.contains("w", Qt::CaseInsensitive)) newbp.flags |= BPT_FLAG_WRITE;
    else if(text.contains("x", Qt::CaseInsensitive)) newbp.flags |= BPT_FLAG_EXEC;
    else if(text.contains("l", Qt::CaseInsensitive)) newbp.flags |= BPT_FLAG_LOG;
    
    //If none of r/w/x specified, default to exec
    if(!(newbp.flags & (BPT_FLAG_EXEC | BPT_FLAG_READ | BPT_FLAG_WRITE)))
        BPT_SET_FLAG(newbp, BPT_FLAG_EXEC);

    int first = text.indexOf(QRegExp("[0123456789abcdefABCDEF]"));
    text.remove(0,first);
    int last = text.lastIndexOf(QRegExp("[0123456789abcdefABCDEF]"));
    text.truncate(last+1);

    QStringList parts = QString(text).split(" ", QString::SkipEmptyParts);
    if (parts.size() == 1) {
        newbp.address = parts.at(0).toULong( &ok, 16 );
        newbp.endaddr = newbp.address;
    }
    else if (parts.size() == 2) {
        newbp.address = parts.at(0).toULong( &ok, 16 );
        newbp.endaddr = parts.at(1).toULong( &ok, 16 );
    }
    else
        return;
    
    if(debugger::add_breakpoint_struct(&newbp) == -1)
    {
        QMessageBox::warning(this, tr("Warning"),
                tr("Cannot add any more breakpoints."),
                QMessageBox::Ok);
        return;
    }
    BreakpointsWidget::update_breakpoint();
}

void BreakpointsWidget::onremove()
{
    //TODO: Implement onremove()
}

void BreakpointsWidget::onenable()
{
    _toggle(1);
}

void BreakpointsWidget::ondisable()
{
    _toggle(0);
}
 
void BreakpointsWidget::ontoggle()
{
    _toggle(-1);
}

void BreakpointsWidget::onedit()
{
    //TODO: Implement onedit()
    bool ok;
    QString old = "old";
    QString text = QInputDialog::getText(this, tr("Edit Breakpoint"),
                                      tr("New Value:"), QLineEdit::Normal, old, &ok);    
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

    if(bpt->address == bpt->endaddr)
        address = QString("0x%1").arg(bpt->address, 8, 16, QChar('0')).toUpper();
    else
        address = QString("%1 - %2").arg(bpt->address, 8, 16, QChar('0')).arg(bpt->endaddr, 8, 16, QChar('0')).toUpper();

    str = QString("%1 / %2%3%4%5 %6").arg(status).arg(flagR).arg(flagW).arg(flagX).arg(flagL).arg(address);
    return str;
}

void BreakpointsWidget::_toggle(int flag)
{
    //flag is 1 for enable, 0 for disable, -1 for toggle
    QModelIndex index = listView->selectionModel()->currentIndex();
    int i = index.row();
    if (flag==1)
        debugger::enable_breakpoint(i);
    else if (flag == 0)
        debugger::disable_breakpoint(i);
    else if (BPT_CHECK_FLAG(debugger::g_Breakpoints[i], BPT_FLAG_ENABLED))
        debugger::disable_breakpoint(i);
    else
        debugger::enable_breakpoint(i);
    if(!debugger::g_NumBreakpoints) return;
    update_breakpoint();
}

