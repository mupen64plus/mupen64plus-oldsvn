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

#include <QPainter>

#include "globals.h"
#include "rommodel.h"
#include "romdelegate.h"

static const int MAX_STATUS = 5;

RomDelegate::RomDelegate(QObject* parent)
: QItemDelegate(parent)
{}

void RomDelegate::paint(QPainter* painter,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) const
{
    switch (index.column()) {
        case RomModel::Status:
            {
                drawBackground(painter, option, index);
                int n = index.data(Qt::DisplayRole).toInt();
                const QRect& r = option.rect;

                drawStars(painter, r, n, MAX_STATUS);

                drawFocus(painter, option, option.rect);
            }
            break;
        default:
            QItemDelegate::paint(painter, option, index);
            break;
    }
}

QSize RomDelegate::sizeHint(const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    switch (index.column()) {
        case RomModel::Status:
            return QSize((16 + 2) * MAX_STATUS, 16 + 2);
            break;
        default:
            return QItemDelegate::sizeHint(option, index);
            break;
    }
}

