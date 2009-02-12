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

#include <QtGui>

#include "globals.h"
#include "cheatdialog.h"
#include "cheatromdialog.h"
#include "cheatcheatdialog.h"

CheatDialog::CheatDialog(QWidget* parent) : QDialog(parent)
, m_cheats(0), m_model(new QStandardItemModel(this))
{
    setupUi(this);
    setWindowIcon(icon("tools-wizard.png"));

    connect(m_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(cheatItemChanged(QStandardItem *)));
    connect(pushNew, SIGNAL(clicked()), this, SLOT(onnew()));
    connect(pushEdit, SIGNAL(clicked()), this, SLOT(onedit()));
    connect(pushDelete, SIGNAL(clicked()), this, SLOT(ondelete()));
    connect(treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(treeViewClicked(const QModelIndex&)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onaccepted()));

    // If a rom is loaded, prevent editing of cheats (causes a SIGSEGV)
    if (core::ROM_HEADER) {
        pushNew->setEnabled(false);
        pushEdit->setEnabled(false);
        pushDelete->setEnabled(false);
    } else {
        pushNew->setEnabled(true);
        pushEdit->setEnabled(true);
        pushDelete->setEnabled(true);
    }
    
    core::list_t node1 = 0;
    core::list_t node2 = 0;

    QMap<QString, QStandardItem*> itemMap;

    m_cheats = core::cheats_for_current_rom();
    if (m_cheats) {
        setWindowTitle(tr("%1 - Cheats").arg(core::ROM_SETTINGS.goodname));
        list_foreach(m_cheats, node1) {
            core::cheat_t* cheat = static_cast<core::cheat_t*>(node1->data);
            QStandardItem* entry = 0;
            if (QString(cheat->name).contains("\\")) {
                QStringList parts = QString(cheat->name).split("\\");
                QStandardItem* parent = 0;
                int name_index = parts.size()-1;
                QString itemMapIndex = "";
                
                for (int lvl = 0;lvl < name_index;lvl++) {
                    itemMapIndex = QString(itemMapIndex + parts.at(lvl));
                    if (itemMap.contains(itemMapIndex)) {
                        parent = itemMap.value(itemMapIndex);
                    } else {
                        entry = new QStandardItem(parts.at(lvl));
                        if (lvl == 0) {
                            m_model->appendRow(entry);
                        } else {
                            parent->appendRow(entry);
                        }
                        parent = entry;
                        itemMap[itemMapIndex] = parent;
                    }
                }                
                
                // The last string in the list should be the cheat name.
                entry = createItemForCheat(parts.at(name_index), cheat);
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
                    optionItem = new QStandardItem(tr("%1 (Option)").arg(option->description));
                    optionItem->setEditable(false);
                    optionItem->setCheckable(true);
                    optionItem->setData(QVariant::fromValue(option->code), CheatOptionRole);
                    entry->appendRow(optionItem);
                }
            }
        }
    }

    // Adding cheats from cheats.cfg.
    // Only add if emulator is not running. If emulator is running they will be
    // added to the 'cheat_load_current_rom'. Only for editing!
    if (!core::ROM_HEADER) {
        personal = new QStandardItem(tr("Personal Cheats"));
        m_model->appendRow(personal);

        // populate model
        core::list_node_t*  romnode;
        core::list_node_t*  cheatnode;
        core::rom_cheats_t* romcheat;
        core::cheat_t*      cheat;

        list_foreach(core::g_Cheats, romnode) {
            QStandardItem* newrom = 0;

            romcheat = (core::rom_cheats_t *)romnode->data;
            newrom = new QStandardItem(romcheat->rom_name);
            newrom->setData(QVariant::fromValue(romcheat), RomRole);
            personal->appendRow(newrom);

            list_foreach(romcheat->cheats, cheatnode)
                {
                cheat = (core::cheat_t *)cheatnode->data;

                QStandardItem * newitem;
                newitem = createItemForCheat(cheat->name, cheat);
                newitem->setData(QVariant::fromValue(cheat), CheatRole);
                newrom->appendRow(newitem);
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
    item->setData(QVariant::fromValue(cheat), CheatRole);
    item->setToolTip(QString(cheat->comment));
    return item;
}

void CheatDialog::cheatItemChanged(QStandardItem* item)
{
    core::cheat_t* cheat = 0;
    core::cheat_code_t* cheatcode = 0;
    core::list_t node = 0;
    QVariant codeVariant = item->data(CheatOptionRole);
    int code = codeVariant.isValid() ? codeVariant.value<int>() : -1;

    if (code >= 0) {
        cheat = item->parent()->data(CheatRole).value<core::cheat_t*>();
    } else {
        cheat = item->data(CheatRole).value<core::cheat_t*>();
    }

    Q_ASSERT(item->checkState() != Qt::PartiallyChecked);

    if (item->checkState() == Qt::Checked) {
        if (code >= 0) {
            // Only one option can be selected at a time.
            // Deselect all other options
            QStandardItem* parent = item->parent();
            for (int i = 0; i < parent->rowCount(); i++) {
                if (parent->child(i) != item) {
                    parent->child(i)->setCheckState(Qt::Unchecked);
                }
            }
        }
        if (cheat) {
            cheat->enabled = 1;
            if (code>=0) {
                list_foreach(cheat->cheat_codes, node) {
                    cheatcode = (core::cheat_code_t *)node->data;
                    // If this is an option to set, search for a code which is supposed
                    // to be patched with the new codes
                    if (cheatcode->option) {
                        cheatcode->value = code;
                        break;
                    }
                }
            }
        }
    } else if (item->checkState() == Qt::Unchecked) {
        if (cheat) {
            cheat->enabled = 0;
        }
    }
}

void CheatDialog::onnew()
{
    QStandardItem* item = 0;
    QStandardItem* newitem = 0;
    core::cheat_t* cheat = NULL;
    core::rom_cheats_t* romcheat = NULL;

    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    if (index.isValid()) {
        item = m_model->itemFromIndex(index);
        romcheat = item->data(RomRole).value<core::rom_cheats_t*>();
    }

    // If a rom is selected, make new cheat
    if (romcheat) {
        cheat = core::cheat_new_cheat(romcheat);
        CheatCheatDialog* d = new CheatCheatDialog(cheat,this);
        if (d->exec()) {
            newitem = createItemForCheat(cheat->name, cheat);
            newitem->setData(QVariant::fromValue(cheat), CheatRole);
            item->appendRow(newitem);
        }
        else {
            // delete newly created cheat if the user click cancel
            cheat_delete_cheat(romcheat, cheat);
        }
        delete d;
    }
    // otherwise add a new rom
    else {
        romcheat = core::cheat_new_rom();
        romcheat->rom_name = NULL;
        CheatRomDialog* d = new CheatRomDialog(romcheat,this);
        if (d->exec()) {
            // add new rom to tree model
            QStandardItem* newrom = 0;
            newrom = new QStandardItem(romcheat->rom_name);
            newrom->setData(QVariant::fromValue(romcheat), RomRole);
            personal->appendRow(newrom);
        }
        else {
            cheat_delete_rom(romcheat);
        }
        delete d;
    }
}

void CheatDialog::onedit()
{
    core::cheat_t* cheat = NULL;
    core::rom_cheats_t* romcheat = NULL;
    QStandardItem* item;
    
    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    if (index.isValid()) {
        item = m_model->itemFromIndex(index);
        romcheat = item->data(RomRole).value<core::rom_cheats_t*>();
        cheat = item->data(CheatRole).value<core::cheat_t*>();
        if (romcheat) {
            CheatRomDialog* d = new CheatRomDialog(romcheat,this);
            if (d->exec()) {
                // Reset the rom name in case it was changed
                item->setText(tr("%1").arg(romcheat->rom_name));
            }
        } else if (cheat) {
            CheatCheatDialog* d = new CheatCheatDialog(cheat,this);
            if (d->exec()) {
                // Reset the cheat name in case it was changed
                item->setText(tr("%1").arg(cheat->name));
            }
        }
    }
}

void CheatDialog::ondelete()
{
    core::rom_cheats_t* romcheat = NULL;
    core::cheat_t* cheat = NULL;
    QStandardItem* romitem = 0;
    QStandardItem* cheatitem = 0;
    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    const QModelIndex& parent = index.parent();

    if (parent.isValid()) {
        if (index.isValid()) {
            romitem = m_model->itemFromIndex(index);
            romcheat = romitem->data(RomRole).value<core::rom_cheats_t*>();
            if (romcheat) {
                core::cheat_delete_rom(romcheat);
                m_model->removeRow(index.row(), parent);
            } else {
                romitem = m_model->itemFromIndex(parent);
                cheatitem = m_model->itemFromIndex(index);
                romcheat = romitem->data(RomRole).value<core::rom_cheats_t*>();
                cheat = cheatitem->data(CheatRole).value<core::cheat_t*>();
                if (romcheat && cheat) {
                    core::cheat_delete_cheat(romcheat, cheat);
                    m_model->removeRow(index.row(), parent);
                }
            }
        }
    }
}

void CheatDialog::treeViewClicked(const QModelIndex& index)
{
    core::cheat_t* cheat;

    cheat = m_model->itemFromIndex(index)->data(CheatRole).value<core::cheat_t*>();
    if (cheat) {
        textDescription->setPlainText(QString(cheat->comment));
    }
}

void CheatDialog::onaccepted()
{
    core::cheat_write_config();
}

