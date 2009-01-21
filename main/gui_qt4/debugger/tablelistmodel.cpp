/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - tablelistmodel.cpp                                      *
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

#include "tablelistmodel.h"

TableListModel::TableListModel(QObject *parent)
 : QAbstractTableModel(parent) { }

int TableListModel::rowCount(const QModelIndex & /* parent */) const
{
    return 0; //TODO
//    return TableListModel.height();
}

int TableListModel::columnCount(const QModelIndex & /* parent */) const
{
    return 0; //TODO
//    return TableListModel.width();
}

void TableListModel::initStringIndex(const QStringList &str)
{
    stringIndex = str;
}

void TableListModel::initStringValue(const QStringList &str)
{
    stringValue = str;
}

QVariant TableListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    return QVariant(); //TODO: Don't get it ...
}

QVariant TableListModel::headerData(int /* section */,
                 Qt::Orientation /* orientation */,
                 int role) const
{
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}
