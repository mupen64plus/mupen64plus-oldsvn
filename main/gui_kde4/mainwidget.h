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

#ifndef MUPEN_KDE4_MAINWIDGET_H
#define MUPEN_KDE4_MAINWIDGET_H

#include <QWidget>
#include <QTimer>

class QVBoxLayout;
class QTreeView;
class KLineEdit;
class QSortFilterProxyModel;

class MainWidget : public QWidget
{
    Q_OBJECT
    public:
        MainWidget(QWidget* parent = 0);
        QModelIndex getRomBrowserIndex();

    private slots:
        void resizeHeaderSections();
        void lineEditTextChanged();
        void filter();
        void treeViewDoubleClicked(const QModelIndex& index);

    signals:
        void itemCountChanged(int count);
        void romDoubleClicked(const KUrl& filename, unsigned int archivefile);

    protected:
        virtual bool eventFilter(QObject* obj, QEvent* event);

    private:
        QTreeView* m_treeView;
        KLineEdit* m_lineEdit;
        QSortFilterProxyModel* m_proxyModel;
        QTimer m_timer;
};

#endif // MUPEN_KDE4_MAINWIDGET_H
