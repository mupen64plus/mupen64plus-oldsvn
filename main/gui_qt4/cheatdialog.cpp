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

CheatDialog::CheatDialog(QWidget* parent)
: QDialog(parent)
, m_cheats(0)
, m_model(new QStandardItemModel(this))
{
    setupUi(this);
    setWindowIcon(icon("tools-wizard.png"));

    connect(m_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(cheatItemChanged(QStandardItem *)));
    connect(pushNew, SIGNAL(clicked()), this, SLOT(onnew()));
    connect(pushEdit, SIGNAL(clicked()), this, SLOT(onedit()));
    connect(pushDelete, SIGNAL(clicked()), this, SLOT(ondelete()));
    connect(treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(treeViewClicked(const QModelIndex&)));

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
                int name_index = parts.size()-1;
                QString itemMapIndex = "";
                
                for (int lvl = 0;lvl < name_index;lvl++) {
                    itemMapIndex = QString(itemMapIndex + parts.at(lvl));
                    if (itemMap.contains(itemMapIndex))
                        parent = itemMap.value(itemMapIndex);
                    else {
                        entry = new QStandardItem(parts.at(lvl));
                        if (lvl == 0)
                            m_model->appendRow(entry);
                        else
                            parent->appendRow(entry);
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
                    optionItem = new QStandardItem(option->description + QString(" (Option)"));
                    optionItem->setEditable(false);
                    optionItem->setCheckable(true);
                    optionItem->setData(QVariant::fromValue(option->code), CheatOptionRole);
                    entry->appendRow(optionItem);
                }
            }
        }
    }

    m_model->sort(0);

    personal = new QStandardItem(QString("Personal Cheats"));
    m_model->appendRow(personal);

    // TODO: read .mupen64plus/cheats.cfg and add it here to parent
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
    item->setData(QVariant::fromValue(cheat), CheatCodeRole);
    item->setToolTip(QString(cheat->comment));
    return item;
}

void CheatDialog::cheatItemChanged(QStandardItem* item)
{
    core::cheat_t* cheat = 0;
    QVariant codeVariant = item->data(CheatOptionRole);
    int code = codeVariant.isValid() ? codeVariant.value<int>() : -1;

    if (code >= 0) {
        cheat = item->parent()->data(CheatCodeRole).value<core::cheat_t*>();
    } else {
        cheat = item->data(CheatCodeRole).value<core::cheat_t*>();
    }

    Q_ASSERT(item->checkState() != Qt::PartiallyChecked);

    if (item->checkState() == Qt::Checked) {
        if (code >= 0) {
            // Only one option can be selected at a time.
            // Deselect all other options and re-enable this one
            QStandardItem* parent = item->parent();
            for (int i = 0; i < parent->rowCount(); i++) {
                parent->child(i)->setCheckState(Qt::Unchecked);
            }
            item->setCheckState(Qt::Checked);
            cheat = parent->data(CheatCodeRole).value<core::cheat_t*>();
        }
        if (cheat) {
            core::cheat_enable_current_rom(cheat->number, code);
        }
    } else if (item->checkState() == Qt::Unchecked) {
        if (cheat) {
            core::cheat_disable_current_rom(cheat->number);
        }
    }
}

void CheatDialog::onnew()
{
    QStandardItem * item;
    QStandardItem * newitem;
    core::cheat_t *cheat = NULL;
    core::rom_cheats_t *romcheat = NULL;

    // If nothing is selected, exit.
    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    if (index.isValid()) {
        item = m_model->itemFromIndex(index);
        romcheat = item->data(RomRole).value<core::rom_cheats_t*>();
    }
    // If a rom is selected, make new cheat
    if (romcheat) {
        cheat = core::cheat_new_cheat(romcheat);
        cheat->name = NULL;
        cheat->comment = NULL;
        cheat->number = 0; //TODO
        cheat->enabled = 0;
        cheat->always_enabled = 0;
        cheat->was_enabled = 0;
        cheat->codes = NULL;
        cheat->options = NULL;
        CheatCheatDialog* d = new CheatCheatDialog(cheat,this);
        if (d->exec()) {
            newitem = new QStandardItem(cheat->name);
            newitem->setData(QVariant::fromValue(cheat), CheatCodeRole);
            item->appendRow(newitem);
        }
        else {
            cheat_delete_cheat(romcheat, cheat);
            delete cheat;
        }
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
            
            // select new rom
            // TODO
        }
        else {
            cheat_delete_rom(romcheat);
            delete romcheat;
        }
    }
}

void CheatDialog::onedit()
{
    core::cheat_t *cheat = NULL;
    core::rom_cheats_t *romcheat = NULL;
    QStandardItem * item;
    
    // If nothing is selected, exit.
    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    if (!index.isValid())
        return;
        
    item = m_model->itemFromIndex(index);
    romcheat = item->data(RomRole).value<core::rom_cheats_t*>();
    cheat = item->data(CheatCodeRole).value<core::cheat_t*>();
    if (romcheat) {
        CheatRomDialog* d = new CheatRomDialog(romcheat,this);
        if (d->exec()) {
            ; //TODO: change name if changed        
            // TODO: select rom?
        }
    }
    else if (cheat) {
        CheatCheatDialog* d = new CheatCheatDialog(cheat,this);
        if (d->exec()) {
            ; //TODO: rename if name changed
        }
    }
}

    void CheatDialog::ondelete()
{
    const QModelIndex& index = treeView->selectionModel()->currentIndex();
    const QModelIndex& parent = index.parent();
    if (parent.isValid())
        m_model->removeRow(index.row(), parent);
}

void CheatDialog::treeViewClicked(const QModelIndex& index)
{
    QStandardItem *item;
    core::cheat_t* cheat;

    item = m_model->itemFromIndex(index);
    cheat = item->data(CheatCodeRole).value<core::cheat_t*>();
    if (cheat)
        textDescription->setPlainText(QString(cheat->comment));
}

