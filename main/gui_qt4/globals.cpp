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

#include <QIcon>
#include <QString>
#include <QFile>
#include <QPainter>
#include <QPaintEngine>

#include "globals.h"

namespace core {
    extern "C" {
        #include "../config.h"
        #include "../main.h"
    }
}

QStringList romDirectories()
{
    QStringList dirs;
    for(int i = 0; i < core::config_get_number("NumRomDirs", 0); i++) {
        char buf[31];
        snprintf(buf, 30, "RomDirectory[%d]", i);
        buf[30] = '\0';
        dirs << core::config_get_string(buf, "");
    }
    dirs.removeAll("");
    return dirs;
}

QIcon icon(QString name)
{
    QIcon icon;
    QStringList sizes;
    sizes << "16x16" << "22x22" << "32x32" << "";
    foreach (QString size, sizes) {
        QPixmap p = pixmap(name, size);
        if (!p.isNull()) {
            icon.addPixmap(p);
        }
    }
    return icon;
}

QPixmap pixmap(QString name, QString subdirectory)
{
    QString s;
    if (subdirectory.isEmpty()) {
        s = name;
    } else {
        s = QString("%1/%2").arg(subdirectory).arg(name);
    }
    QString filename = core::get_iconpath(qPrintable(s));
    return QPixmap(filename);
}

void drawStars(QPainter* painter,
               const QRect& r,
               int n,
               int max)
{
    static QPixmap enabled = icon("star.png").pixmap(16, 16);
    static QPixmap disabled = icon("star.png").pixmap(16, 16, QIcon::Disabled);
    for (int i = 0; i < n; i++) {
        QPoint p = r.topLeft();
        p += QPoint(i * (enabled.width() + 2), 1);
        painter->drawPixmap(p, enabled);
    }
    for (int i = n; i < max; i++) {
        QPoint p = r.topLeft();
        p += QPoint(i * (disabled.width() + 2), 1);
        painter->drawPixmap(p, disabled);
    }
}
