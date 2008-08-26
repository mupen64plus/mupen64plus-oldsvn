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

#include <QFileDialog>
#include "romdirectorieslistwidget.h"

RomDirectoriesListWidget::RomDirectoriesListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(addButton, SIGNAL(clicked()),
             this, SLOT(add()));
    connect(removeButton, SIGNAL(clicked()),
             this, SLOT(remove()));
    connect(removeAllButton, SIGNAL(clicked()),
             this, SLOT(removeAll()));
}

const QStringList& RomDirectoriesListWidget::directories() const
{
    return m_directories;
}

void RomDirectoriesListWidget::setDirectories(QStringList list)
{
    if (m_directories != list) {
        m_directories = list;
        updateListWidget();
        emit changed(m_directories);
    }
}

void RomDirectoriesListWidget::add()
{
    QString dir = QFileDialog::getExistingDirectory();
    if (!dir.isEmpty() && !m_directories.contains(dir)) {
        m_directories << dir;
        updateListWidget();
        emit changed(m_directories);
    }
}

void RomDirectoriesListWidget::remove()
{
    if (m_directories.count() > 0) {
        int index = romListWidget->currentRow();
        m_directories.removeAt(index);
        updateListWidget();
        emit changed(m_directories);
    }
}

void RomDirectoriesListWidget::removeAll()
{
    if (m_directories.count() > 0) {
        m_directories.clear();
        updateListWidget();
        emit changed(m_directories);
    }
}

void RomDirectoriesListWidget::updateListWidget()
{
    romListWidget->clear();
    romListWidget->addItems(m_directories);
}
