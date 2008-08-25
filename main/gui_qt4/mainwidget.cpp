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
#include <QSettings>
#include <QMenu>
#include <QActionGroup>
#include <QDialog>

#include "mainwidget.h"
#include "rommodel.h"
#include "romdelegate.h"
#include "globals.h"

#include "ui_rominfodialog.h"

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
    treeView->setItemDelegate(new RomDelegate(this));

    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(treeViewContextMenuRequested(QPoint)));

    QSettings s;
    QByteArray headerState = s.value("RomBrowserHeadersState").toByteArray();
    if (!headerState.isEmpty()) {
        treeView->header()->restoreState(headerState);
    } else {
        for (int i = 0; i < treeView->header()->count(); i++) {
            treeView->header()->hideSection(i);
        }
        for (int i = 0; i <= RomModel::LAST_VISIBLE_COLUMN; i++) {
            treeView->header()->showSection(i);
        }
        resizeHeaderSections();
    }

    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(headerContextMenuRequested(QPoint)));

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

MainWidget::~MainWidget()
{
    QSettings s;
    QByteArray headerState = treeView->header()->saveState();
    s.setValue("RomBrowserHeadersState", headerState);
}

QModelIndex MainWidget::getRomBrowserIndex()
{
    return treeView->currentIndex();
}

void MainWidget::showFilter(bool show)
{
    label->setVisible(show);
    lineEdit->setVisible(show);
    lineEdit->clear();
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

void MainWidget::headerContextMenuRequested(const QPoint& pos)
{
    QHeaderView* header = treeView->header();
    QAbstractItemModel* model = header->model();
    Qt::Orientation o = header->orientation();

    QMenu menu;
    QActionGroup group(this);

    group.setExclusive(false);
    connect(&group, SIGNAL(triggered(QAction*)),
            this, SLOT(hideHeaderSection(QAction*)));

    for (int i = 0; i < header->count(); i++) {
        QString title = model->headerData(i, o, Qt::DisplayRole).toString();
        QAction* a = menu.addAction(title);
        a->setCheckable(true);
        a->setChecked(!header->isSectionHidden(i));
        a->setData(i);
        group.addAction(a);
    }

    menu.exec(header->mapToGlobal(pos));
}

void MainWidget::hideHeaderSection(QAction* a)
{
    int section = a->data().toInt();
    treeView->header()->setSectionHidden(section, !a->isChecked());
    resizeHeaderSections();
}

void MainWidget::treeViewContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = treeView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    QMenu m;
    QAction* propertiesAction = m.addAction(tr("Properties..."));
    propertiesAction->setIcon(icon("mupen64cart.png"));
    QAction* a = m.exec(treeView->mapToGlobal(pos));
    if (a == propertiesAction) {
        int row = index.row();
        QDialog* d = new QDialog(this);
        Ui_RomInfoDialog ui;
        ui.setupUi(d);
        ui.statusLabel->setMax(5);
        ui.flagLabel->setPixmap(index.sibling(row, RomModel::Country).data(Qt::DecorationRole).value<QPixmap>());
        ui.goodNameLabel->setText(index.sibling(row, RomModel::GoodName).data().toString());
        ui.statusLabel->setText(index.sibling(row, RomModel::Status).data().toString());
        ui.countryLabel->setText(index.sibling(row, RomModel::Country).data().toString());
        ui.sizeLabel->setText(index.sibling(row, RomModel::Size).data().toString());
        ui.fileNameLabel->setText(index.sibling(row, RomModel::FileName).data().toString());
        ui.fullPathLabel->setText(index.data(RomModel::FullPath).toString());
        ui.internalNameLabel->setText(index.sibling(row, RomModel::InternalName).data().toString());
        ui.md5HashLabel->setText(index.sibling(row, RomModel::MD5).data().toString());
        ui.crc1Label->setText(index.sibling(row, RomModel::CRC1).data().toString());
        ui.crc2Label->setText(index.sibling(row, RomModel::CRC2).data().toString());
        ui.playersLabel->setText(index.sibling(row, RomModel::Players).data().toString());
        ui.rumbleLabel->setText(index.sibling(row, RomModel::Rumble).data().toString());
        ui.saveTypeLabel->setText(index.sibling(row, RomModel::SaveType).data().toString());
        ui.sizeLabel->setText(index.sibling(row, RomModel::Size).data().toString());
        ui.cicChipLabel->setText(index.sibling(row, RomModel::CIC).data().toString());
        ui.compressionLabel->setText(index.sibling(row, RomModel::CompressionType).data().toString());
        ui.imageTypeLabel->setText(index.sibling(row, RomModel::ImageType).data().toString());
        ui.rumbleLabel->setText(index.sibling(row, RomModel::Rumble).data().toString());
        d->show();
    }
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
