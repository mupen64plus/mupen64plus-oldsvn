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

#include <QtGui>

#include "mainwindow.h"
#include "mainwidget.h"
#include "globals.h"
#include "rommodel.h"

#include "settingsdialog.h"

namespace core {
    extern "C" {
        #include "../main.h"
        #include "../plugin.h"
        #include "../savestates.h"
        #include "../rom.h"
        #include "../config.h"
    }
}

MainWindow::MainWindow() 
: QMainWindow(0)
, m_statusBarLabel(0)
{
    setupUi(this);
    setupActions();
    m_statusBarLabel = new QLabel;
    statusBar()->addPermanentWidget(m_statusBarLabel);

    connect(mainWidget, SIGNAL(itemCountChanged(int)),
             this, SLOT(itemCountUpdate(int)));
    connect(mainWidget, SIGNAL(romDoubleClicked(QUrl, unsigned int)),
             this, SLOT(romOpen(QUrl, unsigned int)));

    QSize size(core::config_get_number("MainWindowWidth",600),
               core::config_get_number("MainWindowHeight",400));
    QPoint position(core::config_get_number("MainWindowXPosition",0),
                    core::config_get_number("MainWindowYPosition",0));

    QDesktopWidget *d = QApplication::desktop();
    QSize desktop = d->size();

    if (position.x() > desktop.width()) {
        position.setX(0);
    }
    if (position.y() > desktop.height()) {
        position.setY(0);
    }

    if (size.width() > desktop.width()) {
        size.setWidth(600);
    }
    if (size.height() > desktop.height()) {
        size.setHeight(400);
    }

    if ((position.x() + size.width()) > desktop.width()) {
        position.setX(desktop.width() - size.width());
    }
    if ((position.y()+size.height())>desktop.height()) {
        position.setY(desktop.height() - size.height());
    }

    resize(size);
    move(position);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    core::config_put_number("MainWindowWidth", width());
    core::config_put_number("MainWindowHeight", height());
    core::config_put_number("MainWindowXPosition", x());
    core::config_put_number("MainWindowYPosition", y());
    core::config_put_bool("StatusbarVisible", statusBar()->isVisible());
}

void MainWindow::showInfoMessage(const QString& msg)
{
    statusBar()->showMessage(msg);
}

void MainWindow::showAlertMessage(const QString& msg)
{
    QMessageBox::critical(this, "Alert", msg);
}

bool MainWindow::confirmMessage(const QString& msg)
{
    QMessageBox::StandardButton res;
    res = QMessageBox::question(this,
                                 tr("Confirm"),
                                 msg,
                                 QMessageBox::Yes | QMessageBox::No);
    switch (res) {
        case QMessageBox::Yes:
            return true;
            break;
        default:
            return false;
            break;
    }
}

void MainWindow::romOpen()
{
    QString filter = RomExtensions.join(" ");
    QString filename = QFileDialog::getOpenFileName(this,
                                                     tr("Select Rom"),
                                                     QString(),
                                                     filter);
    if (!filename.isEmpty()) {
        romOpen(filename, 0);
    }
}

void MainWindow::romOpen(const QUrl& url)
{
    romOpen(url, 0);
}

void MainWindow::romOpen(const QUrl& url, unsigned int archivefile)
{
    QString path = url.path();
    if (core::open_rom(path.toLocal8Bit(), archivefile) == 0) {
        core::startEmulation();
    }
}

void MainWindow::romClose()
{
    core::close_rom();
}

void MainWindow::emulationStart()
{
    if(!core::rom) {
        QModelIndex index = mainWidget->getRomBrowserIndex();
        QString filename = index.data(RomModel::FullPath).toString();
        unsigned int archivefile = index.data(RomModel::ArchiveFile).toUInt();
        if (filename.isEmpty()) {
            if (confirmMessage(tr("There is no Rom loaded." \
                                  " Do you want to load one?"))) {
                romOpen();
            }
            return;
        } else {
            romOpen(filename, archivefile);
        }
    }
    else
        core::startEmulation();
}

void MainWindow::emulationPauseContinue()
{
    core::pauseContinueEmulation();
}

void MainWindow::emulationStop()
{
    core::stopEmulation();
}

void MainWindow::fullScreenToggle()
{
    core::changeWindow();
}

void MainWindow::saveStateSave()
{
    if (core::g_EmulationThread) {
        core::savestates_job |= SAVESTATE;
    }
}

void MainWindow::saveStateSaveAs()
{
    if (core::g_EmulationThread) {
        QString filename = QFileDialog::getSaveFileName(this);
        if (!filename.isEmpty()) {
            core::savestates_select_filename(filename.toLocal8Bit());
            core::savestates_job |= SAVESTATE;
        }
    } else {
        showAlertMessage(tr("Emulation not running!"));
    }
}

void MainWindow::saveStateLoad()
{
    if (core::g_EmulationThread) {
        core::savestates_job |= LOADSTATE;
    }
}

void MainWindow::saveStateLoadFrom()
{
    if (core::g_EmulationThread) {
        QString filename = QFileDialog::getOpenFileName(this);
        if (!filename.isEmpty()) {
            core::savestates_select_filename(filename.toLocal8Bit());
            core::savestates_job |= LOADSTATE;
        }
    } else {
        showAlertMessage(tr("Emulation not running!"));
    }
}

void MainWindow::savestateCheckSlot()
{
    int slot = core::savestates_get_slot();

    foreach(QAction* a, slotActions) {
        a->setChecked(false);
    }

    if (slot >= 0 && slot < slotActions.count()) {
        QAction* a = slotActions.at(slot);
        a->setChecked(true);
    }
}

void MainWindow::savestateSelectSlot(QAction* a)
{
    bool ok = false;
    int slot = a->data().toInt(&ok);
    if (ok) {
        core::savestates_select_slot(slot);
    }
}

//Slot should be renamed.
void MainWindow::configDialogShow()
{
    SettingsDialog* d = new SettingsDialog(this);
    d->show();
}

void MainWindow::itemCountUpdate(int count)
{
    m_statusBarLabel->setText(tr("%0 roms").arg(count));
}

void MainWindow::aboutDialogShow()
{
    QMessageBox::about(this,
        tr("About Mupen64Plus"),
        tr("<html><b>Mupen64Plus N64 Emulator</b><br/>\
            <a href=\"http://code.google.com/p/mupen64plus/\">Home...</a>\
            </html>"));
}

void MainWindow::customEvent(QEvent* event)
{
    switch (event->type()) {
        case InfoEventType:
            showInfoMessage(static_cast<InfoEvent*>(event)->message);
            break;
        case AlertEventType:
            showAlertMessage(static_cast<AlertEvent*>(event)->message);
            break;
        default:
            qDebug("Got unknown custom event of type %d!", event->type());
            break;
    }
}

void MainWindow::setupActions()
{
    //File Actions
    actionOpenRom->setIcon(icon("mupen64cart.png"));
    connect(actionOpenRom, SIGNAL(triggered()), this, SLOT(romOpen()));
    actionCloseRom->setIcon(icon("edit-delete.png"));
    connect(actionCloseRom, SIGNAL(triggered()), this, SLOT(romClose()));
    actionQuit->setIcon(icon("dialog-error.png"));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    //Emulation Actions
    actionStart->setIcon(icon("media-playback-start.png"));
    connect(actionStart, SIGNAL(triggered()), this, SLOT(emulationStart()));
    actionPause->setIcon(icon("media-playback-pause.png"));
    connect(actionPause, SIGNAL(triggered()),
            this, SLOT(emulationPauseContinue()));
    actionStop->setIcon(icon("media-playback-stop.png"));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(emulationStop()));
    actionSaveState->setIcon(icon("document-save.png"));
    connect(actionSaveState, SIGNAL(triggered()),
            this, SLOT(saveStateSave()));
    actionLoadState->setIcon(icon("document-open.png"));
    connect(actionLoadState, SIGNAL(triggered()), this, SLOT(saveStateLoad()));
    connect(actionSaveStateAs, SIGNAL(triggered()),
            this, SLOT(saveStateSaveAs()));
    connect(actionLoadStateFrom, SIGNAL(triggered()),
            this, SLOT(saveStateLoadFrom()));

    actionCurrentSaveStateSlot->setIcon(icon("star.png"));

    QMenu* slotMenu = new QMenu(this);
    QActionGroup* slotActionGroup = new QActionGroup(actionCurrentSaveStateSlot);
    for(int i = 0; i < 10; i++) {
        QAction* slot = slotMenu->addAction(tr("Slot &%1").arg(i));
        slot->setCheckable(true);
        slot->setData(i);
        slotActionGroup->addAction(slot);
        slotActions << slot;
    }
    actionCurrentSaveStateSlot->setMenu(slotMenu);
    connect(slotMenu, SIGNAL(aboutToShow()), this, SLOT(savestateCheckSlot()));
    connect(slotActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(savestateSelectSlot(QAction*)));

    //Settings Actions
    actionFullScreen->setIcon(icon("view-fullscreen.png"));
    connect(actionFullScreen, SIGNAL(triggered()),
            this, SLOT(fullScreenToggle()));
    actionConfigureMupen64Plus->setIcon(icon("preferences-system.png"));
    connect(actionConfigureMupen64Plus, SIGNAL(triggered()),
            this, SLOT(configDialogShow()));

    actionAbout->setIcon(icon("mupen64plus.png"));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(aboutDialogShow()));
    connect(actionAboutQt, SIGNAL(triggered()),
            QApplication::instance(), SLOT(aboutQt()));
}

#include "mainwindow.moc"
