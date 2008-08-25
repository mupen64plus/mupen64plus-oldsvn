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

#include <QPaintEvent>
#include <QPainter>

#include "starlabel.h"
#include "globals.h"

StarLabel::StarLabel(QWidget* parent)
: QLabel(parent), m_max(-1)
{}

int StarLabel::max() const
{
    return m_max;
}

void StarLabel::setMax(int max)
{
    if (max != m_max) {
        m_max = max;
        update();
    }
}

void StarLabel::paintEvent(QPaintEvent* event)
{
    bool ok;
    int n = text().toInt(&ok);
    if (ok) {
        QPainter p(this);
        drawStars(&p, event->rect(), n, qMax(0, m_max));
    } else {
        QLabel::paintEvent(event);
    }
}

#include "starlabel.moc"
