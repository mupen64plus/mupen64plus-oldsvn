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

#include <QVBoxLayout>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QLabel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QApplication>
#include <Qt>
#include <QLabel>
#include <QLineEdit>

#include "mainwidget.h"
#include "rommodel.h"

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , m_proxyModel(0)
{
    setupUi(this);

    lineEdit->installEventFilter(this);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(RomModel::self());
    m_proxyModel->setFilterKeyColumn(-1); // search all columns
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSortRole(RomModel::Sort);

    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    treeView->setModel(m_proxyModel);
    treeView->sortByColumn(RomModel::GoodName, Qt::AscendingOrder);
    treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    treeView->setFocusProxy(lineEdit);

    m_timer.setSingleShot(true);

    connect(lineEdit, SIGNAL(textChanged(QString)),
             this, SLOT(lineEditTextChanged()));
    connect(&m_timer, SIGNAL(timeout()),
             this, SLOT(filter()));
    connect(m_proxyModel, SIGNAL(modelReset()),
             this, SLOT(resizeHeaderSections()));
    connect(m_proxyModel, SIGNAL(modelReset()),
             this, SLOT(filter()));
    connect(m_proxyModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
             this, SLOT(resizeHeaderSections()));
    connect(m_proxyModel, SIGNAL(layoutChanged()),
             this, SLOT(resizeHeaderSections()));
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(treeViewDoubleClicked(QModelIndex)));

    //lineEdit->setFocus();
    QTimer::singleShot(0, this, SLOT(filter())); // so we emit the base item count
}

QModelIndex MainWidget::getRomBrowserIndex()
{
    return treeView->currentIndex();
}

void MainWidget::toggleFilter()
{
    if(label->isVisible()) {
        label->hide();
        lineEdit->clear();
        lineEdit->hide();
        }
    else {
        label->show();
        lineEdit->show();
        }
}

void MainWidget::resizeHeaderSections()
{
    QString filter = lineEdit->text();
    m_proxyModel->setFilterFixedString("");
    treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    m_proxyModel->setFilterFixedString(filter);
    emit itemCountChanged(m_proxyModel->rowCount());
}

void MainWidget::lineEditTextChanged()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }
    m_timer.start(50);
}

void MainWidget::filter()
{
    m_proxyModel->setFilterFixedString(lineEdit->text());
    emit itemCountChanged(m_proxyModel->rowCount());
}

void MainWidget::treeViewDoubleClicked(const QModelIndex& index)
{
    load(index);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* event)
{
    bool filtered = false;

    if (obj == lineEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            switch(keyEvent->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                    QApplication::sendEvent(treeView, keyEvent);
                    filtered = true;
                    break;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    load(treeView->currentIndex());
                    filtered = true;
                    break;
            }
        }
    }

    return filtered;
}

void MainWidget::load(const QModelIndex& index)
{
    QString filename = index.data(RomModel::FullPath).toString();
    unsigned int archivefile = index.data(RomModel::ArchiveFile).toUInt();
    if (!filename.isEmpty()) {
        emit romDoubleClicked(filename, archivefile);
    }
}

#include "mainwidget.moc"
