/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - settingsdialog.cpp                                      *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Slougi                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QSize>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QDebug>
#include <SDL.h>

#include "rommodel.h"
#include "settingsdialog.h"
#include "globals.h"
#include "ui_settingsdialog.h"
#include "getinput.h"

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
    items << new QListWidgetItem(QIcon(icon("video-display.png")), //TODO: Find better icon
                                 tr("Hotkeys"),
                                 listWidget);
    foreach (QListWidgetItem* item, items) {
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(110, 55));
        listWidget->insertItem(0, item);
    }

    connect(listWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(pageChanged(int)));
    listWidget->setCurrentRow(0);

    QSize labelPixmapSize(32, 32);
    rspPluginLabel->setPixmap(icon("cpu.png").pixmap(labelPixmapSize));
    inputPluginLabel->setPixmap(icon("input-gaming.png").pixmap(labelPixmapSize));
    audioPluginLabel->setPixmap(icon("audio-card.png").pixmap(labelPixmapSize));
    graphicsPluginLabel->setPixmap(icon("video-display.png").pixmap(labelPixmapSize));

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(buttonClicked(QAbstractButton*)));

    // Input Mappings
    smInput = new QSignalMapper(this);
    connect(pushFullscreen, SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushStop,       SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushPause,      SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushSave,       SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushLoad,       SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushIncrement,  SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushReset,      SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushSpeeddown,  SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushSpeedup,    SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushScreenshot, SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushMute,       SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushIncrease,   SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushDecrease,   SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushForward,    SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushAdvance,    SIGNAL(clicked()), smInput, SLOT(map()));
    connect(pushGameshark,  SIGNAL(clicked()), smInput, SLOT(map()));
    smInput->setMapping(pushFullscreen, kbdFullscreen);
    smInput->setMapping(pushStop,       kbdStop);
    smInput->setMapping(pushPause,      kbdPause);
    smInput->setMapping(pushSave,       kbdSave);
    smInput->setMapping(pushLoad,       kbdLoad);
    smInput->setMapping(pushIncrement,  kbdIncrement);
    smInput->setMapping(pushReset,      kbdReset);
    smInput->setMapping(pushSpeeddown,  kbdSpeeddown);
    smInput->setMapping(pushSpeedup,    kbdSpeedup);
    smInput->setMapping(pushScreenshot, kbdScreenshot);
    smInput->setMapping(pushMute,       kbdMute);
    smInput->setMapping(pushIncrease,   kbdIncrease);
    smInput->setMapping(pushDecrease,   kbdDecrease);
    smInput->setMapping(pushForward,    kbdForward);
    smInput->setMapping(pushAdvance,    kbdAdvance);
    smInput->setMapping(pushGameshark,  kbdGameshark);
    connect(smInput, SIGNAL(mapped(const QString &)), this, SLOT(hotkeyClicked(const QString &)));

    readSettings();
}

void SettingsDialog::on_aboutAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_about_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_configAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_config_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_testAudioPluginButton_clicked()
{
    QString text = audioPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_test_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_aboutGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_about_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_configGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_config_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_testGraphicsPluginButton_clicked()
{
    QString text = graphicsPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_test_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_aboutRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_about_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_configRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_config_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_testRspPluginButton_clicked()
{
    QString text = rspPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_test_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_aboutInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_about_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_configInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_config_with_wid(qPrintable(text), wid);
}

void SettingsDialog::on_testInputPluginButton_clicked()
{
    QString text = inputPluginCombo->currentText();
    core::HWND wid = (core::HWND) winId();
    core::plugin_exec_test_with_wid(qPrintable(text), wid);
}

void SettingsDialog::accept()
{
    writeSettings();
    QDialog::accept();
}

void SettingsDialog::buttonClicked(QAbstractButton* button)
{
    switch (buttonBox->standardButton(button)) {
        case QDialogButtonBox::Reset:
            readSettings();
            break;
        default: // we really only care about a few buttons here
            break;
    }
}

void SettingsDialog::pageChanged(int page)
{
    QListWidgetItem* i = listWidget->item(page);
    imageLabel->setPixmap(i->icon().pixmap(32, 32));
    textLabel->setText(QString("<b>%1</b>").arg(i->text()));
}

void SettingsDialog::readSettings()
{
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
        !core::config_get_bool("No Ask", FALSE)
    );

    autoIncrementSaveSlotCheck->setChecked(
        core::config_get_bool("AutoIncSaveSlot", FALSE)
    );

    osdEnabledCheck->setChecked(core::g_OsdEnabled);

    QString inputPlugin = core::config_get_string("Input Plugin", "");
    QString gfxPlugin = core::config_get_string("Gfx Plugin", "");
    QString audioPlugin = core::config_get_string("Audio Plugin", "");
    QString rspPlugin = core::config_get_string("RSP Plugin", "");

    core::list_node_t *node;
    core::plugin *p;
    list_foreach(core::g_PluginList, node) {
        p = reinterpret_cast<core::plugin*>(node->data);
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

    int i = 0;

    inputPlugin = core::plugin_name_by_filename(qPrintable(inputPlugin));
    if (!inputPlugin.isEmpty() &&
        ((i = inputPluginCombo->findText(inputPlugin)) != -1)) {
        inputPluginCombo->setCurrentIndex(i);
    }

    audioPlugin = core::plugin_name_by_filename(qPrintable(audioPlugin));
    if (!audioPlugin.isEmpty() &&
        ((i = audioPluginCombo->findText(audioPlugin)) != -1)) {
        audioPluginCombo->setCurrentIndex(i);
    }

    rspPlugin = core::plugin_name_by_filename(qPrintable(rspPlugin));
    if (!rspPlugin.isEmpty() &&
        ((i = rspPluginCombo->findText(rspPlugin)) != -1)) {
        rspPluginCombo->setCurrentIndex(i);
    }

    gfxPlugin = core::plugin_name_by_filename(qPrintable(gfxPlugin));
    if (!gfxPlugin.isEmpty() &&
        ((i = graphicsPluginCombo->findText(gfxPlugin)) != -1)) {
        graphicsPluginCombo->setCurrentIndex(i);
    }

    romDirectoriesListWidget->setDirectories(romDirectories());

    scanDirectoriesRecursivelyCheck->setChecked(
        core::config_get_bool("RomDirsScanRecursive", FALSE)
    );

    showFullPathsInFilenamesCheck->setChecked(
        core::config_get_bool("RomBrowserShowFullPaths", FALSE)
    );
    
    initializeSdlKeyMap();
    
    pushFullscreen->setText(getSdlKeyString(kbdFullscreen, SDLK_LAST));
    pushStop->setText(getSdlKeyString(kbdStop, SDLK_ESCAPE));
    pushPause->setText(getSdlKeyString(kbdPause, SDLK_p));
    pushSave->setText(getSdlKeyString(kbdSave, SDLK_F5));
    pushLoad->setText(getSdlKeyString(kbdLoad, SDLK_F7));
    pushIncrement->setText(getSdlKeyString(kbdIncrement, SDLK_LAST));
    pushReset->setText(getSdlKeyString(kbdReset, SDLK_F9));
    pushSpeeddown->setText(getSdlKeyString(kbdSpeeddown, SDLK_F10));
    pushSpeedup->setText(getSdlKeyString(kbdSpeedup, SDLK_F11));
    pushScreenshot->setText(getSdlKeyString(kbdScreenshot, SDLK_F12));
    pushMute->setText(getSdlKeyString(kbdMute, SDLK_m));
    pushIncrease->setText(getSdlKeyString(kbdIncrease, SDLK_RIGHTBRACKET));
    pushDecrease->setText(getSdlKeyString(kbdDecrease, SDLK_LEFTBRACKET));
    pushForward->setText(getSdlKeyString(kbdForward, SDLK_f));
    pushAdvance->setText(getSdlKeyString(kbdAdvance, SDLK_SLASH));
    pushGameshark->setText(getSdlKeyString(kbdGameshark, SDLK_g));
}

void SettingsDialog::writeSettings()
{
    const char* p = 0;

    p = qPrintable(audioPluginCombo->currentText());
    p = core::plugin_filename_by_name(p);
    core::config_put_string("Audio Plugin", p);

    p = qPrintable(graphicsPluginCombo->currentText());
    p = core::plugin_filename_by_name(p);
    core::config_put_string("Gfx Plugin", p);

    p = qPrintable(rspPluginCombo->currentText());
    p = core::plugin_filename_by_name(p);
    core::config_put_string("RSP Plugin", p);

    p = qPrintable(inputPluginCombo->currentText());
    p = core::plugin_filename_by_name(p);
    core::config_put_string("Input Plugin", p);

    if (dynamicRecompilerRadio->isChecked()) {
        core::config_put_number("Core", CORE_DYNAREC);
    }

    if (interpreterRadio->isChecked()) {
        core::config_put_number("Core", CORE_INTERPRETER);
    }

    if (pureInterpreterRadio->isChecked()) {
        core::config_put_number("Core", CORE_PURE_INTERPRETER);
    }

    core::config_put_bool(
        "NoCompiledJump",
        disableCompiledJumpCheck->isChecked()
    );

    core::config_put_bool(
        "NoMemoryExpansion",
        disableMemoryExpansionCheck->isChecked()
    );

    core::config_put_bool(
        "GuiStartFullscreen",
        alwaysStartInFullScreenModeCheck->isChecked()
    );

    core::g_Noask = !askBeforeLoadingBadRomCheck->isChecked();
    core::config_put_bool("No Ask", !askBeforeLoadingBadRomCheck->isChecked());

    core::g_OsdEnabled = osdEnabledCheck->isChecked();
    core::config_put_bool("OsdEnabled", osdEnabledCheck->isChecked());

    core::config_put_bool(
        "AutoIncSaveSlot",
        autoIncrementSaveSlotCheck->isChecked()
    );

    core::config_put_bool(
        "RomDirsScanRecursive",
        scanDirectoriesRecursivelyCheck->isChecked()
    );

    core::config_put_bool(
        "RomBrowserShowFullPaths",
        showFullPathsInFilenamesCheck->isChecked()
    );

    QStringList romDirs = romDirectoriesListWidget->directories();
    core::config_put_number("NumRomDirs", romDirs.count());
    int i = 0;
    foreach(QString str, romDirs) {
        if (!str.endsWith("/")) {
            str.append("/");
        }
        core::config_put_string(
            qPrintable(QString("RomDirectory[%1]").arg(i++)),
            qPrintable(str)
        );
    }
    RomModel::self()->settingsChanged();
    core::config_write();
}

void SettingsDialog::hotkeyClicked(const QString text)
{
    QByteArray ba = text.toLatin1();
    char *c_str = ba.data();
    int key;

    GetInput d(&key);

    if (key > 0) {
        core::config_put_number(c_str, key);
        if (text == kbdFullscreen) {
            pushFullscreen->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdStop) {
            pushStop->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdPause) {
            pushPause->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdSave) {
            pushSave->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdLoad) {
            pushLoad->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdIncrement) {
            pushIncrement->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdReset) {
            pushReset->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdSpeeddown) {
            pushSpeeddown->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdSpeedup) {
            pushSpeedup->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdScreenshot) {
            pushScreenshot->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdMute) {
            pushMute->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdIncrease) {
            pushIncrease->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdDecrease) {
            pushDecrease->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdForward) {
            pushForward->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdAdvance) {
            pushAdvance->setText(getSdlKeyString(c_str, key));
        } else if (text == kbdGameshark) {
            pushGameshark->setText(getSdlKeyString(c_str, key));
        }
    }
}

QString SettingsDialog::getSdlKeyString(const char *name, int key)
{
    QString result;
    int val;
    val = core::config_get_number(name, key);
    if (sdlKeyMap.contains(val)) {
        result = sdlKeyMap[val];
    } else {
        result = QString("%1").arg(val);
    }
    return result;
}

void SettingsDialog::initializeSdlKeyMap()
{
    sdlKeyMap.insert(  0, "SDLK_UNKNOWN");    sdlKeyMap.insert(  8, "SDLK_BACKSPACE"); sdlKeyMap.insert(  9, "SDLK_TAB");
    sdlKeyMap.insert( 12, "SDLK_CLEAR");      sdlKeyMap.insert( 13, "SDLK_RETURN");    sdlKeyMap.insert( 19, "SDLK_PAUSE");
    sdlKeyMap.insert( 27, "SDLK_ESCAPE");     sdlKeyMap.insert( 32, "SDLK_SPACE");     sdlKeyMap.insert( 33, "SDLK_EXCLAIM");
    sdlKeyMap.insert( 34, "SDLK_QUOTEDBL");   sdlKeyMap.insert( 35, "SDLK_HASH");      sdlKeyMap.insert( 36, "SDLK_DOLLAR");
    sdlKeyMap.insert( 38, "SDLK_AMPERSAND");  sdlKeyMap.insert( 39, "SDLK_QUOTE");     sdlKeyMap.insert( 40, "SDLK_LEFTPAREN");
    sdlKeyMap.insert( 41, "SDLK_RIGHTPAREN"); sdlKeyMap.insert( 42, "SDLK_ASTERISK");  sdlKeyMap.insert( 43, "SDLK_PLUS");
    sdlKeyMap.insert( 44, "SDLK_COMMA");      sdlKeyMap.insert( 45, "SDLK_MINUS");     sdlKeyMap.insert( 46, "SDLK_PERIOD");
    sdlKeyMap.insert( 47, "SDLK_SLASH");      sdlKeyMap.insert( 48, "SDLK_0");         sdlKeyMap.insert( 49, "SDLK_1");
    sdlKeyMap.insert( 50, "SDLK_2");          sdlKeyMap.insert( 51, "SDLK_3");         sdlKeyMap.insert( 52, "SDLK_4");
    sdlKeyMap.insert( 53, "SDLK_5");          sdlKeyMap.insert( 54, "SDLK_6");         sdlKeyMap.insert( 55, "SDLK_7");
    sdlKeyMap.insert( 56, "SDLK_8");          sdlKeyMap.insert( 57, "SDLK_9");         sdlKeyMap.insert( 58, "SDLK_COLON");
    sdlKeyMap.insert( 59, "SDLK_SEMICOLON");  sdlKeyMap.insert( 60, "SDLK_LESS");      sdlKeyMap.insert( 61, "SDLK_EQUALS");
    sdlKeyMap.insert( 62, "SDLK_GREATER");    sdlKeyMap.insert( 63, "SDLK_QUESTION");  sdlKeyMap.insert( 64, "SDLK_AT");
    sdlKeyMap.insert( 91, "SDLK_LEFTBRACKET");sdlKeyMap.insert( 92, "SDLK_BACKSLASH"); sdlKeyMap.insert( 93, "SDLK_RIGHTBRACKET");
    sdlKeyMap.insert( 94, "SDLK_CARET");      sdlKeyMap.insert( 95, "SDLK_UNDERSCORE");sdlKeyMap.insert( 96, "SDLK_BACKQUOTE");
    sdlKeyMap.insert( 97, "SDLK_a");          sdlKeyMap.insert( 98, "SDLK_b");         sdlKeyMap.insert( 99, "SDLK_c");
    sdlKeyMap.insert(100, "SDLK_d");          sdlKeyMap.insert(101, "SDLK_e");         sdlKeyMap.insert(102, "SDLK_f");
    sdlKeyMap.insert(103, "SDLK_g");          sdlKeyMap.insert(104, "SDLK_h");         sdlKeyMap.insert(105, "SDLK_i");
    sdlKeyMap.insert(106, "SDLK_j");          sdlKeyMap.insert(107, "SDLK_k");         sdlKeyMap.insert(108, "SDLK_l");
    sdlKeyMap.insert(109, "SDLK_m");          sdlKeyMap.insert(110, "SDLK_n");         sdlKeyMap.insert(111, "SDLK_o");
    sdlKeyMap.insert(112, "SDLK_p");          sdlKeyMap.insert(113, "SDLK_q");         sdlKeyMap.insert(114, "SDLK_r");
    sdlKeyMap.insert(115, "SDLK_s");          sdlKeyMap.insert(116, "SDLK_t");         sdlKeyMap.insert(117, "SDLK_u");
    sdlKeyMap.insert(118, "SDLK_v");          sdlKeyMap.insert(119, "SDLK_w");         sdlKeyMap.insert(120, "SDLK_x");
    sdlKeyMap.insert(121, "SDLK_y");          sdlKeyMap.insert(122, "SDLK_z");         sdlKeyMap.insert(127, "SDLK_DELETE");
    sdlKeyMap.insert(273, "SDLK_UP");         sdlKeyMap.insert(274, "SDLK_DOWN");      sdlKeyMap.insert(275, "SDLK_RIGHT");
    sdlKeyMap.insert(276, "SDLK_LEFT");       sdlKeyMap.insert(277, "SDLK_INSERT");    sdlKeyMap.insert(278, "SDLK_HOME");
    sdlKeyMap.insert(279, "SDLK_END");        sdlKeyMap.insert(280, "SDLK_PAGEUP");    sdlKeyMap.insert(281, "SDLK_PAGEDOWN");
    sdlKeyMap.insert(282, "SDLK_F1");         sdlKeyMap.insert(283, "SDLK_F2");        sdlKeyMap.insert(284, "SDLK_F3");
    sdlKeyMap.insert(285, "SDLK_F4");         sdlKeyMap.insert(286, "SDLK_F5");        sdlKeyMap.insert(287, "SDLK_F6");
    sdlKeyMap.insert(288, "SDLK_F7");         sdlKeyMap.insert(289, "SDLK_F8");        sdlKeyMap.insert(290, "SDLK_F9");
    sdlKeyMap.insert(291, "SDLK_F10");        sdlKeyMap.insert(292, "SDLK_F11");       sdlKeyMap.insert(293, "SDLK_F12");
    sdlKeyMap.insert(294, "SDLK_F13");        sdlKeyMap.insert(295, "SDLK_F14");       sdlKeyMap.insert(296, "SDLK_F15");
    sdlKeyMap.insert(300, "SDLK_NUMLOCK");    sdlKeyMap.insert(301, "SDLK_CAPSLOCK");  sdlKeyMap.insert(302, "SDLK_SCROLLOCK");
    sdlKeyMap.insert(303, "SDLK_RSHIFT");     sdlKeyMap.insert(304, "SDLK_LSHIFT");    sdlKeyMap.insert(305, "SDLK_RCTRL");
    sdlKeyMap.insert(306, "SDLK_LCTRL");      sdlKeyMap.insert(307, "SDLK_RALT");      sdlKeyMap.insert(308, "SDLK_LALT");
    sdlKeyMap.insert(309, "SDLK_RMETA");
}

