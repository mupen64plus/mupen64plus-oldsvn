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

#include <QtGui>
#include <QDialog>

#include "debuggerwidget.h"
#include "registerwidget.h"
#include "hexspinbox.h"
#include "hexspinboxdialog.h"

// if we include <QtGui> there is no need to include every class used: <QString>, <QFileDialog>,...
// TODO: Figure out how to bring #ifdef DBG to the .ui file to #ifdef out debugger
namespace debugger {
    extern "C" {
        #include "../../../debugger/debugger.h"
        #include "../../../debugger/decoder.h"
    }
}

namespace core {
    extern "C" {
        #include "../../config.h"
        #include "../../version.h"
    }
}

class RegisterWidget;

unsigned int focused_pc, previous_pc;

DebuggerWidget::DebuggerWidget(QWidget* parent) // : QWidget(parent)
{
    setupUi(this); // this sets up GUI
 
    hexSpinBox = new HexSpinBox();
    previous_pc = 0x00000000;
    focused_pc = 0x00000000;
    hexSpinBox->setValue(focused_pc);
    hexSpinBox->setSingleStep(4);
    hexSpinBox->setMinimum(0);
    hexSpinBox->setMaximum(0xffffffff);
    formLayoutButtons->addWidget(hexSpinBox);


    // signals/slots mechanism in action
    connect( pushStep, SIGNAL( clicked() ), this, SLOT( onstep() ) ); 
    connect( pushRun, SIGNAL( clicked() ), this, SLOT( onrun() ) ); 
    connect( pushTrace, SIGNAL( clicked() ), this, SLOT( ontrace() ) ); 
    connect( pushBreak, SIGNAL( clicked() ), this, SLOT( onbreak() ) ); 
    connect( pushGoto, SIGNAL( clicked() ), this, SLOT( ongoto() ) );
    
    // Set window title
    char title[64];
    sprintf( title, "%s - %s", "Mupen64plus Debugger", PLUGIN_VERSION);
    setWindowTitle(title);

    treeDisasm->setColumnCount(3);
    treeDisasm->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QString str = "Address,Opcode,Args";
    QStringList list = str.split(",");
    treeDisasm->setHeaderLabels(list);
    
}

DebuggerWidget::~DebuggerWidget()
{
    //TODO: Implement destructor
}

void DebuggerWidget::update_desasm( unsigned int current_pc )
{
    char opcode[64];
    char args[128];
    unsigned int instr;
    unsigned int tmp_pc;

    treeDisasm->clear();
    items.clear();
    
    focused_pc = current_pc;
    hexSpinBox->setValue(focused_pc);
    for (int i = -4; i<28; i++)
    {
        tmp_pc = current_pc + (i*4);
        if((debugger::get_memory_flags((unsigned int)(long)current_pc) & MEM_FLAG_READABLE) != 0)
        {
            instr = debugger::read_memory_32((unsigned int)(long)tmp_pc);
            debugger::r4300_decode_op( instr, opcode, args, (long)tmp_pc );
        }
        else
        {
            strcpy( opcode, "X+X+X+X");
            sprintf( args, "UNREADABLE (%d)",debugger::get_memory_type((unsigned int)(long)tmp_pc));
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
    if(debugger::run == 0)
        debugger::debugger_step();
}

void DebuggerWidget::onrun()
{
    int oldrun = debugger::run;
    debugger::run = 2;
    if(oldrun == 0)
        debugger::debugger_step();
}

void DebuggerWidget::ontrace()
{
    int oldrun = debugger::run;
    debugger::run = 1;
    if(oldrun == 0)
        debugger::debugger_step();
}
 
void DebuggerWidget::onbreak()
{
    debugger::run = 0;
}

void DebuggerWidget::ongoto()
{
    unsigned int tmp_pc = 0;
  
    HexSpinBoxDialog* d = new HexSpinBoxDialog(focused_pc);
    if (d->exec())
    {
        // TODO: Get hold of the spinbox value ...
        // tmp_pc = (unsigned int) d->getAcceptedValue;
        // TODO: Should probably check if it is a valid PC.
        // Debugger API?
        DebuggerWidget::update_desasm( tmp_pc );
    }
}

