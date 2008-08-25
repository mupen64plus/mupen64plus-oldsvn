/*
* Copyright (C) 2008 Louai Al-Khanji
*
* This program is free software; you can redistribute it and/
* or modify it under the terms of the GNU General Public Li-
* cence as published by the Free Software Foundation; either
* version 2 of the Licence, or any later version.
*
* This program is distributed in the hope that it will be use-
* ful, but WITHOUT ANY WARRANTY; without even the implied war-
* ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public Licence for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
* USA.
*
*/

#include "rommodel.h"
#include "romdelegate.h"

RomDelegate::RomDelegate(QObject* parent)
: QItemDelegate(parent)
{}

void RomDelegate::paint(QPainter* painter,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) const
{
    switch (index.column()) {
        default:
            QItemDelegate::paint(painter, option, index);
            break;
    }
}

#include "romdelegate.moc"