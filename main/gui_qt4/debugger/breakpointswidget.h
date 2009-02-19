/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - breakpointswidget.h                                      *
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

#ifndef __BREAKPOINTSWIDGET_H__
#define __BREAKPOINTSWIDGET_H__

#include <QtGui>

#include "ui_breakpointswidget.h"

class BreakpointsWidget : public QWidget, private Ui_BreakpointsWidget
{
    Q_OBJECT
    public:
        BreakpointsWidget(QWidget *parent = 0);

    public slots:
        void update_breakpoint();

    private:
        QString get_breakpoint_display_string(int row);
        QStringListModel *model;
        void breakpoint_parse(QString, int row = -1); // -1 means insert new bp
        enum { Toggle = -1, Disable = 0, Enable = 1 };
        
    private slots:
        void add();
        void remove();
        void enable();
        void disable();
        void toggle();
        void edit();
        void ToggleBreakpoint(int flag);
};

#endif // __BREAKPOINTSWIDGET_H__

