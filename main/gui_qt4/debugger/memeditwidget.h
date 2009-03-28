/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memeditwidget.h                                         *
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

#ifndef __MEMEDITWIDGET_H__
#define __MEMEDITWIDGET_H__

#include <QtGui>

#include "ui_memeditwidget.h"

class MemEditModel;

class MemEditWidget : public QWidget, private Ui_MemEditWidget
{
    Q_OBJECT
    public:
        MemEditWidget(QWidget *parent = 0);

    public slots:
        void update_memedit();

    private slots:
        void text_edited(const QString &text);
        void data_changed(const QModelIndex &index1, const QModelIndex &index2);

    private:
        MemEditModel *model;
        unsigned int address;
        static const unsigned int numlines = 16;
        static const unsigned int bytesperline = 16;
        enum { AddressColumn = 0, HexColumn = 1, AsciiColumn = 17 };
        bool ignore_data_changed;
};

#endif // __MEMEDITWIDGET_H__

