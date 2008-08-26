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

#include <QSize>
#include <QListWidgetItem>

#include "rommodel.h"
#include "settingsdialog.h"
#include "globals.h"
#include "ui_settingsdialog.h"

namespace core {
    extern "C" {
        #include "../config.h"
        #include "../main.h"
        #include "../plugin.h"
    }
}

SettingsDialog::SettingsDialog(QWidget* parent)
: QDialog(parent)
{
    setupUi(this);

    QList<QListWidgetItem*> items;
    items << new QListWidgetItem(QIcon(icon("preferences-system.png")),
                                 tr("Main Settings"),
                                 listWidget);
    items << new QListWidgetItem(QIcon(icon("applications-utilities.png")),
                                 tr("Plugins"),
                                 listWidget);
    items << new QListWidgetItem(QIcon(icon("preferences-system-network.png")),
                                 tr("Rom Browser"),
                                 listWidget);
    int i = 0;
    foreach (QListWidgetItem* item, items) {
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(110, 55));
        listWidget->insertItem(i, item);
    }

    connect(listWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(pageChanged(int)));
    listWidget->setCurrentRow(0);

    QSize labelPixmapSize(32, 32);
    rspPluginLabel->setPixmap(icon("cpu.png").pixmap(labelPixmapSize));
    inputPluginLabel->setPixmap(icon("input-gaming.png").pixmap(labelPixmapSize));
    audioPluginLabel->setPixmap(icon("audio-card.png").pixmap(labelPixmapSize));
    graphicsPluginLabel->setPixmap(icon("video-display.png").pixmap(labelPixmapSize));

    int core = core::config_get_number("Core", CORE_DYNAREC);
    switch (core) {
        case CORE_DYNAREC:
            dynamicRecompilerRadio->setChecked(true);
            break;
        case CORE_INTERPRETER:
            interpreterRadio->setChecked(true);
            break;
        case CORE_PURE_INTERPRETER:
            pureInterpreterRadio->setChecked(true);
            break;
    }

    disableCompiledJumpCheck->setChecked(
        core::config_get_bool("NoCompiledJump", FALSE)
    );

    disableMemoryExpansionCheck->setChecked(
        core::config_get_bool("NoMemoryExpansion", FALSE)
    );

    alwaysStartInFullScreenModeCheck->setChecked(
        core::config_get_bool("GuiStartFullscreen", FALSE)
    );

    askBeforeLoadingBadRomCheck->setChecked(
        core::config_get_bool("No Ask", !core::g_NoaskParam)
    );

    autoIncrementSaveSlotCheck->setChecked(
        core::config_get_bool("AutoIncSaveSlot", FALSE)
    );

    osdEnabledCheck->setChecked(core::g_OsdEnabled);

    core::list_node_t *node;
    core::plugin *p;
    list_foreach(core::g_PluginList, node) {
        p = static_cast<core::plugin*>(node->data);
        switch (p->type) {
            case PLUGIN_TYPE_GFX:
                if(!core::g_GfxPlugin ||
                        (core::g_GfxPlugin &&
                         (strcmp(core::g_GfxPlugin, p->file_name) == 0))) {
                    graphicsPluginCombo->addItem(p->plugin_name);
                }
                break;
            case PLUGIN_TYPE_AUDIO:
                if(!core::g_AudioPlugin ||
                    (core::g_AudioPlugin &&
                    (strcmp(core::g_AudioPlugin, p->file_name) == 0)))
                    audioPluginCombo->addItem(p->plugin_name);
                break;
            case PLUGIN_TYPE_CONTROLLER:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!core::g_InputPlugin ||
                    (core::g_InputPlugin &&
                    (strcmp(core::g_InputPlugin, p->file_name) == 0)))
                    inputPluginCombo->addItem(p->plugin_name);
                break;
            case PLUGIN_TYPE_RSP:
                // if plugin was specified at commandline, only add it to the combobox list
                if(!core::g_RspPlugin ||
                    (core::g_RspPlugin &&
                    (strcmp(core::g_RspPlugin, p->file_name) == 0)))
                    rspPluginCombo->addItem(p->plugin_name);
                break;
        }
    }

    romDirectoriesListWidget->setDirectories(romDirectories());

    scanDirectoriesRecursivelyCheck->setChecked(
        core::config_get_bool("RomDirsScanRecursive", FALSE)
    );

    showFullPathsInFilenamesCheck->setChecked(
        core::config_get_bool("RomBrowserShowFullPaths", FALSE)
    );
}

void SettingsDialog::on_dynamicRecompilerRadio_toggled(bool checked)
{
    if (checked) {
        core::config_put_number("Core", CORE_DYNAREC);
    }
}

void SettingsDialog::on_interpreterRadio_toggled(bool checked)
{
    if (checked) {
        core::config_put_number("Core", CORE_INTERPRETER);
    }
}

void SettingsDialog::on_pureInterpreterRadio_toggled(bool checked)
{
    if (checked) {
        core::config_put_number("Core", CORE_PURE_INTERPRETER);
    }
}

void SettingsDialog::on_disableCompiledJumpCheck_toggled(bool checked)
{
    core::config_put_bool("NoCompiledJump", checked);
}

void SettingsDialog::on_disableMemoryExpansionCheck_toggled(bool checked)
{
    core::config_put_bool("NoMemoryExpansion", checked);
}

void SettingsDialog::on_alwaysStartInFullScreenModeCheck_toggled(bool checked)
{
    core::config_put_bool("GuiStartFullscreen", checked);
}

void SettingsDialog::on_askBeforeLoadingBadRomCheck_toggled(bool checked)
{
    core::g_Noask = checked;
    core::config_put_bool("No Ask", checked);
}

void SettingsDialog::on_autoIncrementSaveSlotCheck_toggled(bool checked)
{
    core::config_put_bool("AutoIncSaveSlot", checked);
}

void SettingsDialog::on_osdEnabledCheck_toggled(bool checked)
{
    core::config_put_bool("OsdEnabled", checked);
}

void SettingsDialog::on_aboutAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::plugin_exec_about(qPrintable(text));
}

void SettingsDialog::on_configAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::plugin_exec_config(qPrintable(text));
}

void SettingsDialog::on_testAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::plugin_exec_test(qPrintable(text));
}

void SettingsDialog::on_aboutGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::plugin_exec_about(qPrintable(text));
}

void SettingsDialog::on_configGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::plugin_exec_config(qPrintable(text));
}

void SettingsDialog::on_testGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::plugin_exec_test(qPrintable(text));
}

void SettingsDialog::on_aboutRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::plugin_exec_about(qPrintable(text));
}

void SettingsDialog::on_configRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::plugin_exec_config(qPrintable(text));
}

void SettingsDialog::on_testRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::plugin_exec_test(qPrintable(text));
}

void SettingsDialog::on_aboutInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::plugin_exec_about(qPrintable(text));
}

void SettingsDialog::on_configInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::plugin_exec_config(qPrintable(text));
}

void SettingsDialog::on_testInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::plugin_exec_test(qPrintable(text));
}

void SettingsDialog::on_scanDirectoriesRecursivelyCheck_toggled(bool checked)
{
    core::config_put_bool("RomDirsScanRecursive", checked);
}

void SettingsDialog::on_showFullPathsInFilenamesCheck_toggled(bool checked)
{
    core::config_put_bool("RomBrowserShowFullPaths", checked);
}

void SettingsDialog::accept()
{
    QStringList romDirs = romDirectoriesListWidget->directories();
    core::config_put_number("NumRomDirs", romDirs.count());
    int i = 0;
    foreach(QString str, romDirs) {
        core::config_put_string(
            qPrintable(QString("RomDirectory[%1]").arg(i++)),
            qPrintable(str)
        );
    }
    RomModel::self()->settingsChanged();
    QDialog::accept();
}

void SettingsDialog::pageChanged(int page)
{
    QListWidgetItem* i = listWidget->item(page);
    imageLabel->setPixmap(i->icon().pixmap(32, 32));
    textLabel->setText(QString("<b>%1</b>").arg(i->text()));
}
