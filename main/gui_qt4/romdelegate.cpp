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
{
    m_star = pixmap("star.png", "16x16");
}

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

                painter->save();
                painter->setCompositionMode(QPainter::CompositionMode_Xor);
                painter->setOpacity(0.2);
                drawStars(painter, r, MAX_STATUS);
                painter->restore();
                drawStars(painter, r, n);
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
            return QSize((m_star.width() + 2) * MAX_STATUS,m_star.height() + 2);
            break;
        default:
            return QItemDelegate::sizeHint(option, index);
            break;
    }
}

void RomDelegate::drawStars(QPainter* painter, const QRect& r, int n) const
{
    for (int i = 0; i < n; i++) {
        QPoint p = r.topLeft();
        p += QPoint(i * (m_star.width() + 2), 1);
        painter->drawPixmap(p, m_star);
    }
}

#include "romdelegate.moc"
