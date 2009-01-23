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

TableListModel::TableListModel(QStringList value, QStringList mnemonic, QObject *parent)
    : QAbstractTableModel(parent)
{
    stringMnemonic = mnemonic;
    stringValue = value;
}

TableListModel::TableListModel(QStringList mnemonic, int size, QObject *parent)
    : QAbstractTableModel(parent)
{
    stringMnemonic = mnemonic;
    for (int i=0;i<mnemonic.size();i++)
        if (size == 16)
            stringValue << "XXXXXXXXXXXXXXXX";
        else if (size == 8)
            stringValue << "XXXXXXXX";
        else
            stringValue << "Not Valid";
}
 
int TableListModel::rowCount(const QModelIndex & /* parent */) const
{
    return stringValue.size();
}

int TableListModel::columnCount(const QModelIndex & /* parent */) const
{
    return 3;
}

QVariant TableListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    if (index.column() == 0)
        return index.row();
    else if (index.column() == 1)
        return stringValue.at(index.row());
    else
        return stringMnemonic.at(index.row());
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
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();
        QString val;

        if (index.column() == 0)
            return false;
        else if (index.column() == 1)
            val = value.toString();
        else
            return false;

        stringValue.replace(row, val);
        emit(dataChanged(index, index));

        return true;
    }
    return false;
}

Qt::ItemFlags TableListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
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

