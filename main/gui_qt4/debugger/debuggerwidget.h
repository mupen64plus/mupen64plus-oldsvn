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
        DebuggerWidget(QWidget* parent = 0);
        virtual ~DebuggerWidget();

    public slots:
        void update_desasm ( unsigned int );

    private slots:
        void onstep();
        void onrun();
        void ontrace();
        void onbreak();
        void ongoto();

        void onreduce1000();
        void onreduce100();
        void onreduce10();
        void onincrease1000();
        void onincrease100();
        void onincrease10();

    private:
        QList<QTreeWidgetItem *> items;
        HexSpinBox *hexSpinBox;
        unsigned int current_pc, focused_pc;
};

#endif // __DEBUGGERWIDGET_H__

