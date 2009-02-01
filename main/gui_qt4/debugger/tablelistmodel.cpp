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

TableListModel::TableListModel(QStringList mnemonic, int size, QObject *parent)
    : QAbstractTableModel(parent)
{
    stringMnemonic = mnemonic;
    for (int i=0;i<mnemonic.size();i++) {
        stringValue.append(QString("%1").arg("", size, QChar('X')));
        bgColor.append(QColor(Qt::white));
    }
}
 
int TableListModel::rowCount(const QModelIndex & parent) const
{
    int rows = 0;
    if (!parent.isValid())
        rows = stringValue.size();
    return rows;
}

int TableListModel::columnCount(const QModelIndex & /* parent */) const
{
    return 3;
}

QVariant TableListModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    int row = index.row();

    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0:
                    result = index.row();
                    break;
                case 1:
                    result = stringValue.at(row);
                    break;
                case 2:
                    result = stringMnemonic.at(row);
                    break;
            }
        }
        else if (role == Qt::BackgroundRole) {
            result = bgColor[row];
        }
    }
    return result;
}

QVariant TableListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
         switch (section) {
             case 0:
                 return tr("#");
             case 1:
                 return tr("Value");
             case 2:
                 return tr("Mnemonic");
             default:
                 return QVariant();
         }
     }
     return QVariant();
}

bool TableListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool retval = false;
    int row = index.row();
    int stringCount = stringValue.count();

    if (index.isValid() && row >= 0 && row < stringCount) {
        if (index.column() == 1 && role == Qt::EditRole ) {
            QString val = value.toString();
            stringValue.replace(row, val);
            retval = true;
            emit(dataChanged(index, index));
            reset();
        }
        else if (Qt::BackgroundRole) {
            bgColor[row] = value.value<QColor>();
            retval = true;
            emit(dataChanged(index, index));
            reset();
        }
    }
    return retval;
}

Qt::ItemFlags TableListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);

    if (index.isValid()) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool TableListModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        QString empty = " ";
        stringValue.insert(position, empty);
        stringMnemonic.insert(position, empty);
    }
    
    endInsertRows();
    return true;
}

bool TableListModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; ++row) {
        stringValue.removeAt(position);
        stringMnemonic.removeAt(position);
    }

    endRemoveRows();
    return true;
}

