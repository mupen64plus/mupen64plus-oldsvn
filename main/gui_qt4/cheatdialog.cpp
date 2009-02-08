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

CheatDialog::CheatDialog(QWidget* parent)
: QDialog(parent)
, m_cheats(0)
, m_model(new QStandardItemModel(this))
{
    setupUi(this);
    setWindowIcon(icon("tools-wizard.png"));

    connect(m_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(cheatItemChanged(QStandardItem *)));
    connect(pushAddCheat, SIGNAL(clicked()), this, SLOT(onaddcheat()));
    connect(pushAddOption, SIGNAL(clicked()), this, SLOT(onaddoption()));
    connect(pushRemove, SIGNAL(clicked()), this, SLOT(onremove()));
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

void CheatDialog::cheatItemChanged(QStandardItem * item)
{
    bool success = true;
    core::cheat_t* cheat;
    int code;
    QStandardItem * parent;
    cheat = item->data(CheatCodeRole).value<core::cheat_t*>();
    code = item->data(CheatOptionRole).value<int>();
    
    if (item->checkState() == Qt::Checked) {
        if (cheat)
            core::cheat_enable_current_rom(cheat->number, -1);
        else if (code) {
            parent = item->parent();
            // Only one option can be selected at the time
            // TODO: Probably not the best way to do it ...
            for (int i = 0;i<parent->rowCount();i++) {
                if ((parent->child(i)->checkState() == Qt::Checked) &&
                    (item->row() != i)) {
                    item->setCheckState(Qt::Unchecked);
                    QMessageBox::warning(this, tr("Warning"),
                        tr("Only one option can be selected for each cheat."),
                        QMessageBox::Ok);
                    return;
                }
            }
            cheat = parent->data(CheatCodeRole).value<core::cheat_t*>();
            if (cheat)
                core::cheat_enable_current_rom(cheat->number, code);
            else
                success = false;
        }
        else
            success = false;

        if (!success)
            QMessageBox::warning(this, tr("Warning"),
                tr("Failed to apply cheat."),
                QMessageBox::Ok);
    }
    else if (item->checkState() == Qt::Unchecked) {
        cheat = item->data(CheatCodeRole).value<core::cheat_t*>();
        code = item->data(CheatOptionRole).value<int>();
        if (cheat)
            core::cheat_disable_current_rom(cheat->number);
        else if (code) {
            parent = item->parent();
            cheat = parent->data(CheatCodeRole).value<core::cheat_t*>();
            if (cheat)
                core::cheat_disable_current_rom(cheat->number);
            else
                success = false;
        }
        else
            success = false;
        if (!success)
            QMessageBox::warning(this, tr("Warning"),
                tr("Failed to un-apply cheat."),
                QMessageBox::Ok);
    }
}

void CheatDialog::onaddcheat()
{
/*
    core::cheat_t* cheat;
    QStandardItem* entry = 0;

    // TODO: Get some cheat user input
    entry = createItemForCheat("Cheat Name", cheat);
    personal->appendRow(entry);
*/
}

void CheatDialog::onaddoption()
{
/*
    QStandardItem * item;
    QModelIndex index = treeView->selectionModel()->currentIndex();
    item = m_model->itemFromIndex(index);
    
    // Only add option if selected is a cheat
    if (item->data(CheatCodeRole).value<core::cheat_t*>()) {
        core::cheat_option_t* option = 0;
        QStandardItem* optionItem = 0;
        
        //TODO: Get some user input      
        option =  new core::cheat_option_t;
        option->code = 0;
        char *buf = "Description";
        option->description = buf;
        optionItem = new QStandardItem(option->description + QString(" (Option)"));
        optionItem->setEditable(false);
        optionItem->setCheckable(true);
        optionItem->setData(QVariant::fromValue(option->code), CheatOptionRole);
        item->appendRow(optionItem);
    }
    else {
        QMessageBox::warning(this, tr("Warning"),
            tr("Options must be added to a cheat."),
            QMessageBox::Ok);
    }
*/
}

void CheatDialog::onremove()
{
/*
    QStandardItem *item;
    
    QModelIndex index = treeView->selectionModel()->currentIndex();
    item = m_model->itemFromIndex(index);
    m_model->removeRow(item->row());
*/
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

