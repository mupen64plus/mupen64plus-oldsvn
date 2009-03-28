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

DebuggerWidget::DebuggerWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
 
    smDebugger = new QSignalMapper(this);
    connect(pushStep, SIGNAL(clicked()), smDebugger, SLOT(map()));
    connect(pushRun, SIGNAL(clicked()), smDebugger, SLOT(map()));
    connect(pushTrace, SIGNAL(clicked()), smDebugger, SLOT(map()));
    connect(pushBreak, SIGNAL(clicked()), smDebugger, SLOT(map()));
    connect(pushGoto, SIGNAL(clicked()), smDebugger, SLOT(map()));
    smDebugger->setMapping(pushStep, "Step");
    smDebugger->setMapping(pushRun, "Run");
    smDebugger->setMapping(pushTrace, "Trace");
    smDebugger->setMapping(pushBreak, "Break");
    smDebugger->setMapping(pushGoto, "Goto");
    connect(smDebugger, SIGNAL(mapped(const QString &)), this, SLOT(clicked(const QString &)));

    smNavigate = new QSignalMapper(this);
    connect(pushReduce1000,   SIGNAL(clicked()), smNavigate, SLOT(map()));
    connect(pushReduce100,    SIGNAL(clicked()), smNavigate, SLOT(map()));
    connect(pushReduce10,     SIGNAL(clicked()), smNavigate, SLOT(map()));
    connect(pushIncrease1000, SIGNAL(clicked()), smNavigate, SLOT(map()));
    connect(pushIncrease100,  SIGNAL(clicked()), smNavigate, SLOT(map()));
    connect(pushIncrease10,   SIGNAL(clicked()), smNavigate, SLOT(map()));
    smNavigate->setMapping(pushReduce1000,  -0x1000);
    smNavigate->setMapping(pushReduce100,    -0x100);
    smNavigate->setMapping(pushReduce10,      -0x10);
    smNavigate->setMapping(pushIncrease1000, 0x1000);
    smNavigate->setMapping(pushIncrease100,   0x100);
    smNavigate->setMapping(pushIncrease10,     0x10);
    connect(smNavigate, SIGNAL(mapped(const int)), this, SLOT(clicked(const int)));
    
    setWindowTitle(tr("Mupen64Plus Debugger - %1").arg(PLUGIN_VERSION));
    treeDisasm->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QStringList headerList;
    headerList << tr("Address") << tr("Opcode") << tr("Args");
    treeDisasm->setHeaderLabels(headerList);

    color_PC = QColor(Qt::red);
    color_BP = QColor(Qt::blue);
}

void DebuggerWidget::update_desasm( unsigned int pc )
{
    current_pc = pc;
    update(current_pc);
}

void DebuggerWidget::update( unsigned int pc )
{
    char opcode[64];
    char args[128];
    unsigned int instr;
    unsigned int tmp_pc;

    treeDisasm->clear();
    items.clear();
    
    focused_pc = pc;
    for (int i = -4; i < 28; i++) {
        tmp_pc = pc + (i * 4);
        if((debugger::get_memory_flags(tmp_pc) & MEM_FLAG_READABLE) != 0) {
            instr = debugger::read_memory_32(tmp_pc);
            debugger::r4300_decode_op(instr, opcode, args, tmp_pc );
        } else {
            strcpy( opcode, "X+X+X+X");
            sprintf( args, "UNREADABLE (%d)",debugger::get_memory_type(tmp_pc));
        }

        QTreeWidgetItem *item=new QTreeWidgetItem(0);
        item->setText(0,"0x" + QString::number(tmp_pc,16).toUpper());
        item->setText(1,opcode);
        item->setText(2,args);
        item->setFlags(Qt::ItemIsEnabled);
        if (tmp_pc==current_pc) {
            item->setBackgroundColor(0, color_PC);
            item->setBackgroundColor(1, color_PC);
            item->setBackgroundColor(2, color_PC);
        }
        if(debugger::check_breakpoints(tmp_pc) != -1) {
            item->setBackgroundColor(0, color_BP);
            item->setBackgroundColor(1, color_BP);
            item->setBackgroundColor(2, color_BP);
        }
        items.append(item);
    }
    treeDisasm->insertTopLevelItems(0,items);
    treeDisasm->setItemSelected(items[4], true);
    treeDisasm->show();
    items.clear();
}

void DebuggerWidget::clicked(int value)
{
    DebuggerWidget::update( focused_pc + value );
}

void DebuggerWidget::clicked(const QString text)
{
    if (text == "Step") {
        if(debugger::run == EmulatorStop) {
            debugger::debugger_step();
        }
    } else if (text == "Run") {
        int oldrun = debugger::run;
        debugger::run = EmulatorRun;
        if(oldrun == EmulatorStop) {
            debugger::debugger_step();
        }
    } else if (text == "Break") {
        debugger::run = EmulatorStop;
    } else if (text == "Goto") {
        HexSpinBoxDialog *d = new HexSpinBoxDialog(&focused_pc);
        if (d->exec()) {
            DebuggerWidget::update( focused_pc );
        }
    } else if (text == "Trace") {
        int oldrun = debugger::run;
        debugger::run = EmulatorTrace;
        if(oldrun == EmulatorStop) {
            debugger::debugger_step();
        }
    }
}

