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

#ifndef MUPEN64_KDE4_ROMDIRECTORIESLISTWIDGET_H
#define MUPEN64_KDE4_ROMDIRECTORIESLISTWIDGET_H

/*
* This is a small wrapper class so we get a nice QStringList property that
* KConfigXT can handle automatically. This could be made into a general class.
*/

#include <QWidget>
#include <QStringList>
#include "ui_romdirectorieslistwidget.h"

class RomDirectoriesListWidget
    : public QWidget, private Ui::RomDirectoriesListWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList directories READ directories WRITE setDirectories
                USER true)
    public:
        RomDirectoriesListWidget(QWidget* parent = 0);
        const QStringList& directories() const;

    signals:
        void changed(const QStringList& list);

    public slots:
        void setDirectories(QStringList list);

    private slots:
        void add();
        void remove();
        void removeAll();

    private:
        void updateListWidget();
        QStringList m_directories;
};

#endif // MUPEN64_KDE4_ROMDIRECTORIESLISTWIDGET_H

