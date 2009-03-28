/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memeditmodel.cpp                                        *
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

#include "memeditmodel.h"

MemEditModel::MemEditModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int MemEditModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;
    if (!parent.isValid()) {
        result = values.size();
    }
    return result;
}

int MemEditModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMNS;
}

QVariant MemEditModel::data(const QModelIndex &index, int role) const
{   // Create an empty QVariant to be returned if index not valid
    QVariant result;

    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            result = values.at(index.row())[index.column()];
        }
    }
    return result;
}

QVariant MemEditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    QVariant result;
    
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    result = tr("Address");
                    break;
                case  1:
                case  2:
                case  3:
                case  4:
                case  5:
                case  6:
                case  7:
                case  8:
                case  9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                    result = tr("#");
                    break;
                case 17:
                    result = tr("Ascii");
                    break;
                default:
                    result = tr("");
                    break;
            }
        }
    }
    return result;
}

bool MemEditModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (index.isValid()) {
        if (role == Qt::EditRole ) {
            QStringList tmp = values.at(index.row());
            QString val = value.toString();
            tmp.replace(index.column(), val);
            values.replace(index.row(), tmp);
            emit(dataChanged(index, index));
//            reset();
            result = true;
        }
    }
    return result;
}

Qt::ItemFlags MemEditModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = QAbstractTableModel::flags(index);

    if (index.isValid()) {
        // Make address and ascii string read only
        if ((index.column() > 0) && (index.column() < (COLUMNS - 1))) {
            result |= Qt::ItemIsEditable;
        }
    }

    return result;
}

bool MemEditModel::insertRows(int position, int rows, const QModelIndex &index)
{
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; row++) {
        QStringList empty;
        for (int i = 0; i < COLUMNS; i++) {
            empty << "";
        }
        values.insert(position, empty);
        emit(dataChanged(index, index));
    }
    endInsertRows();
    return true;
}

bool MemEditModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        values.removeAt(position);
    }

    endRemoveRows();
    return true;
}

QModelIndex MemEditModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

