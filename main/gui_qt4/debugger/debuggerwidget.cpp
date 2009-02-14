/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - debuggerwidget.cpp                                      *
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

#include "debuggerwidget.h"
#include "hexspinboxdialog.h"

namespace debugger {
    extern "C" {
        #include "../../../debugger/debugger.h"
        #include "../../../debugger/decoder.h"
    }
}

namespace core {
    extern "C" {
        #include "../../version.h"
    }
}

DebuggerWidget::DebuggerWidget(QWidget* parent)
{
    setupUi(this);
 
    connect( pushStep,  SIGNAL( clicked() ), this, SLOT( onstep()  ));
    connect( pushRun,   SIGNAL( clicked() ), this, SLOT( onrun()   ));
    connect( pushTrace, SIGNAL( clicked() ), this, SLOT( ontrace() ));
    connect( pushBreak, SIGNAL( clicked() ), this, SLOT( onbreak() ));
    connect( pushGoto,  SIGNAL( clicked() ), this, SLOT( ongoto()  ));
    
    setWindowTitle(tr("Mupen64Plus Debugger - %1").arg(PLUGIN_VERSION));
    treeDisasm->setColumnCount(3);
    treeDisasm->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QStringList headerList;
    headerList << tr("Address") << tr("Opcode") << tr("Args");
    treeDisasm->setHeaderLabels(headerList);
}

DebuggerWidget::~DebuggerWidget()
{
    //TODO: Implement destructor
}

void DebuggerWidget::update_desasm( unsigned int pc )
{
    char opcode[64];
    char args[128];
    unsigned int instr;
    unsigned int tmp_pc;

    treeDisasm->clear();
    items.clear();
    
    current_pc = pc;
    focused_pc = pc;
    for (int i = -4; i<28; i++) {
        tmp_pc = current_pc + (i*4);
        if((debugger::get_memory_flags(current_pc) & MEM_FLAG_READABLE) != 0) {
            instr = debugger::read_memory_32(tmp_pc);
            debugger::r4300_decode_op(instr, opcode, args, tmp_pc );
        } else {
            strcpy( opcode, "X+X+X+X");
            sprintf( args, "UNREADABLE (%d)",debugger::get_memory_type(tmp_pc));
        }

        QTreeWidgetItem* item=new QTreeWidgetItem(0);
        item->setText(0,"0x" + QString::number(tmp_pc,16).toUpper());
        item->setText(1,opcode);
        item->setText(2,args);
        item->setFlags(Qt::ItemIsEnabled);
        if (i==0) {
            QColor red;
            red.setRed(255);
            item->setBackgroundColor(0, red);
            item->setBackgroundColor(1, red);
            item->setBackgroundColor(2, red);
        }
        if(debugger::check_breakpoints(tmp_pc) != -1) {
            QColor color;
            color.setBlue(255);
            item->setBackgroundColor(0, color);
            item->setBackgroundColor(1, color);
            item->setBackgroundColor(2, color);
        }
        items.append(item);
    }
    treeDisasm->insertTopLevelItems(0,items);
    treeDisasm->setItemSelected(items[4], true);
    treeDisasm->show();
    items.clear();

    previous_pc = focused_pc;
}

void DebuggerWidget::onstep()
{
    if(debugger::run == 0) {
        debugger::debugger_step();
    }
}

void DebuggerWidget::onrun()
{
    int oldrun = debugger::run;
    debugger::run = 2;
    if(oldrun == 0) {
        debugger::debugger_step();
    }
}

void DebuggerWidget::ontrace()
{
    int oldrun = debugger::run;
    debugger::run = 1;
    if(oldrun == 0) {
        debugger::debugger_step();
    }
}
 
void DebuggerWidget::onbreak()
{
    debugger::run = 0;
}

void DebuggerWidget::ongoto()
{
    HexSpinBoxDialog* d = new HexSpinBoxDialog(&focused_pc);
    if (d->exec()) {
        DebuggerWidget::update_desasm( focused_pc );
    }
}

