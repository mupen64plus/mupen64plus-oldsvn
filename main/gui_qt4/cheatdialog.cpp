/**
* Mupen64 - cheatdialog.h
* Copyright (C) 2008 slougi
*
* Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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
**/

#include <QDebug>
#include <QStandardItemModel>
#include <QMap>

#include "globals.h"
#include "cheatdialog.h"

CheatDialog::CheatDialog(QWidget* parent)
: QDialog(parent)
, m_cheats(0)
, m_model(new QStandardItemModel(this))
{
    setupUi(this);
    setWindowIcon(icon("tools-wizard.png"));

    core::list_t node1 = 0;
    core::list_t node2 = 0;

    QMap<QString, QStandardItem*> itemMap;

    m_cheats = core::cheats_for_current_rom();
    if (m_cheats) {
        list_foreach(m_cheats, node1) {
            core::cheat_t* cheat = static_cast<core::cheat_t*>(node1->data);
            QStandardItem* entry = 0;
            if (QString(cheat->name).contains("\\")) {
                QStringList parts = QString(cheat->name).split("\\");
                QStandardItem* parent = 0;
                if (itemMap.contains(parts.at(0))) {
                    parent = itemMap.value(parts.at(0));
                } else {
                    parent = new QStandardItem(parts.at(0));
                    m_model->appendRow(parent);
                    itemMap[parts.at(0)] = parent;
                }
                entry = createItemForCheat(parts.at(1), cheat);
                parent->appendRow(entry);
            } else {
                entry = createItemForCheat(cheat->name, cheat);
                m_model->appendRow(entry);
            }
            if (entry && cheat->options) {
                list_foreach(cheat->options, node2) {
                    core::cheat_option_t* option = 0;
                    QStandardItem* optionItem = 0;
                    
                    option = static_cast<core::cheat_option_t*>(node2->data);
                    optionItem = new QStandardItem(option->description);
                    optionItem->setEditable(false);
                    optionItem->setCheckable(true);
                    optionItem->setData(option->code);
                    entry->appendRow(optionItem);
                }
            }
        }
    }

    m_model->sort(0);
    treeView->setModel(m_model);
}

CheatDialog::~CheatDialog()
{
    cheats_free(&m_cheats);
}

QStandardItem* CheatDialog::createItemForCheat(QString name,
                                               core::cheat_t* cheat)
{
    QStandardItem* item = new QStandardItem(name);
    if (!cheat->options) {
        item->setCheckable(true);
    }
    item->setEditable(false);
    item->setData(static_cast<void*>(cheat));
    return item;
}
