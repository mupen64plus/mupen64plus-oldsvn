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

    connect(m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cheatItemChanged(QStandardItem*)));

    core::list_t node1 = 0;
    core::list_t node2 = 0;

    QMap<QString, QStandardItem*> itemMap;

    m_cheats = core::cheats_for_current_rom();
    if (m_cheats) {
        setWindowTitle(QString(core::ROM_SETTINGS.goodname) + " - Cheats");
        list_foreach(m_cheats, node1) {
            core::cheat_t* cheat = static_cast<core::cheat_t*>(node1->data);
            QStandardItem* entry = 0;
            if (QString(cheat->name).contains("\\")) {
                QStringList parts = QString(cheat->name).split("\\");
                QStandardItem* parent = 0;
                QStandardItem* child = 0;
                QStandardItem* gchild = 0;
                QStandardItem* ggchild = 0;
                int name_index = parts.size()-1;
                
                for (int lvl = 0;lvl < name_index;lvl++) {
                    if (lvl == 0) {
                        if (itemMap.contains(parts.at(lvl))) {
                            parent = itemMap.value(parts.at(lvl));
                        } else {
                            parent = new QStandardItem(parts.at(lvl));
                            m_model->appendRow(parent);
                            itemMap[parts.at(lvl)] = parent;
                        }
                    }
                    else if (lvl == 1) {
                        if (itemMap.contains(parts.at(lvl))) {
                            child = itemMap.value(parts.at(lvl));
                        } else {
                            child = new QStandardItem(parts.at(lvl));
                            parent->appendRow(child);
                            itemMap[parts.at(lvl)] = child;
                        }
                    }
                    else if (lvl == 2) {
                        if (itemMap.contains(parts.at(lvl))) {
                            gchild = itemMap.value(parts.at(lvl));
                        } else {
                            gchild = new QStandardItem(parts.at(lvl));
                            child->appendRow(gchild);
                            itemMap[parts.at(lvl)] = gchild;
                        }
                    }
                    else if (lvl == 3) {
                        if (itemMap.contains(parts.at(lvl))) {
                            ggchild = itemMap.value(parts.at(lvl));
                        } else {
                            ggchild = new QStandardItem(parts.at(lvl));
                            gchild->appendRow(ggchild);
                            itemMap[parts.at(lvl)] = ggchild;
                        }
                    }
                }                
                
                // The last string in the list should be the cheat name.
                entry = createItemForCheat(parts.at(name_index), cheat);
                switch (name_index) {
                    case 1:
                        parent->appendRow(entry);
                        break;
                    case 2:
                        child->appendRow(entry);
                        break;
                    case 3:
                        gchild->appendRow(entry);
                        break;
                    default:
                        qDebug("[Warning] Cheat: %i subcategories not supported!", name_index);
                        break;
                    
                }
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
    item->setData(QVariant::fromValue(cheat));
    item->setToolTip(QString(cheat->comment));
    return item;
}

void CheatDialog::cheatItemChanged(QStandardItem * item)
{
    // TODO
    core::cheat_t* cheat;
    cheat = item->data().value<core::cheat_t*>();
    core::cheat_enable_current_rom(cheat->number);
}

