/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - debuggerwidget.h                                        *
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

#ifndef __DEBUGGERWIDGET_H__
#define __DEBUGGERWIDGET_H__

#include <QtGui>

#include "ui_debuggerwidget.h"

class HexSpinBox;

class DebuggerWidget : public QWidget, private Ui_DebuggerWidget
{
    Q_OBJECT
    public:
        DebuggerWidget(QWidget *parent = 0);

    public slots:
        void update_desasm (unsigned int pc);

    private slots:
        void clicked(int value);
        void clicked(const QString text);
        void update (unsigned int pc);
        
    private:
        QList<QTreeWidgetItem *> items;
        HexSpinBox *hexSpinBox;
        unsigned int focused_pc, current_pc;
        QSignalMapper *smNavigate;
        QSignalMapper *smDebugger;
        enum { EmulatorStop = 0, EmulatorTrace, EmulatorRun };

        QColor color_PC;
        QColor color_BP;
};

#endif // __DEBUGGERWIDGET_H__

