/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - mainwindow.cpp                                          *
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

#include <SDL.h>
#include <QtGui>

#include "mainwidget.h"
#include "globals.h"
#include "rommodel.h"
#include "settingsdialog.h"
#include "cheatdialog.h"
#include "aboutdialog.h"

#ifdef DBG
#include "debugger/debuggerwidget.h"
#include "debugger/registerwidget.h"
#include "debugger/breakpointswidget.h"
#endif

namespace core {
    extern "C" {
        #include "../gui.h"
        #include "../main.h"
        #include "../plugin.h"
        #include "../savestates.h"
        #include "../rom.h"
        #include "../config.h"
#ifdef DBG
        #include "../../debugger/debugger.h"
#endif
    }
}

#include "winuser.h"

#undef MB_ABORTRETRYIGNORE
#undef MB_CANCELTRYCONTINUE
#undef MB_OK
#undef MB_OKCANCEL
#undef MB_RETRYCANCEL
#undef MB_YESNO
#undef MB_YESNOCANCEL

#include "mainwindow.h"

MainWindow::MainWindow()
: QMainWindow(0)
, m_statusBarLabel(0)
, m_uiActions(0)
{
    setupUi(this);
    setupActions();
    setState(core::GUI_STATE_STOPPED);
    m_statusBarLabel = new QLabel;
    statusBar()->addPermanentWidget(m_statusBarLabel);

    connect(mainWidget, SIGNAL(itemCountChanged(int)),
             this, SLOT(itemCountUpdate(int)));
    connect(mainWidget, SIGNAL(romDoubleClicked(QString, unsigned int)),
             this, SLOT(romOpen(QString, unsigned int)));

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

MainWindow::~MainWindow()
{
#ifdef Q_WS_WIN
    delete m_renderWindow;
    m_renderWindow = 0;
#endif
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    core::config_put_number("MainWindowWidth", width());
    core::config_put_number("MainWindowHeight", height());
    core::config_put_number("MainWindowXPosition", x());
    core::config_put_number("MainWindowYPosition", y());
    core::config_put_bool("StatusBarVisible", actionShowStatusbar->isChecked());
    core::config_put_bool("ToolBarVisible", actionShowToolbar->isChecked());
    core::config_put_bool("FilterVisible", actionShowFilter->isChecked());

    if (m_renderWindow) {
        m_renderWindow->close();
        m_renderWindow->deleteLater();
    }
}

void MainWindow::pluginGuiQueryEvent(PluginGuiQueryEvent* event)
{
    QMessageBox mb(QWidget::find(event->window));
    mb.setWindowTitle(event->title);
    mb.setText(event->message);
    mb.setIconPixmap(QPixmap::fromImage(event->image));

    QAbstractButton *button1, *button2, *button3;
    button1 = button2 = button3 = 0;

    switch( event->flags & 0x000000FF )
    {
    case MB_ABORTRETRYIGNORE:
        button1 = mb.addButton(QWidget::tr("Abort"), QMessageBox::RejectRole);
        button2 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button3 = mb.addButton(QWidget::tr("Ignore"), QMessageBox::AcceptRole);
        break;

    case MB_CANCELTRYCONTINUE:
        button1 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        button2 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button3 = mb.addButton(QWidget::tr("Continue"), QMessageBox::AcceptRole);
        break;

    case MB_OKCANCEL:
        button1 = mb.addButton(QWidget::tr("OK"), QMessageBox::AcceptRole);
        button2 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_RETRYCANCEL:
        button1 = mb.addButton(QWidget::tr("Retry"), QMessageBox::AcceptRole);
        button2 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_YESNO:
        button1 = mb.addButton(QWidget::tr("Yes"), QMessageBox::YesRole);
        button2 = mb.addButton(QWidget::tr("No"), QMessageBox::NoRole);
        break;

    case MB_YESNOCANCEL:
        button1 = mb.addButton(QWidget::tr("Yes"), QMessageBox::YesRole);
        button2 = mb.addButton(QWidget::tr("No"), QMessageBox::NoRole);
        button3 = mb.addButton(QWidget::tr("Cancel"), QMessageBox::RejectRole);
        break;

    case MB_OK:
    default:
        button1 = mb.addButton(QWidget::tr("OK"), QMessageBox::AcceptRole);
    }

    mb.exec();

    if (button1 == mb.clickedButton()) {
        event->result = 1;
    } else if (button2 == mb.clickedButton()) {
        event->result = 2;
    } else if (button3 == mb.clickedButton()) {
        event->result = 3;
    }

    event->waitCondition->wakeAll();
    return;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_renderWindow) {
        switch (ev->type()) {
        case QEvent::Close:
            emulationStop();
            return false;
            break;
        case QEvent::MouseButtonDblClick:
            actionFullScreen->toggle();
            return true;
            break;
        case QEvent::KeyRelease:
            {
                QKeyEvent* qke = static_cast<QKeyEvent*>(ev);
                foreach (QObject* child, children()) {
                    if (QAction*a = qobject_cast<QAction*>(child)) {
                        QKeySequence seq(qke->key() + qke->modifiers());
                        if (seq == a->shortcut()) {
                            a->activate(QAction::Trigger);
                        }
                    }
                }
            }
            // fall through
        case QEvent::KeyPress:
            {
                QKeyEvent* qke = static_cast<QKeyEvent*>(ev);
                core::WPARAM wParam = 0;
                core::LPARAM lParam = 0;
                int key = qke->key();
                if ((key >= Qt::Key_0 && key <= Qt::Key_9)
                    || (key >= Qt::Key_A && key <= Qt::Key_Z)) {
                        // In these cases the Qt and windows definitions match
                    wParam = (core::WPARAM) key;
                } else if (key >= Qt::Key_Left && key <= Qt::Key_Down) {
                    wParam = (core::WPARAM) key - Qt::Key_Down + VK_DOWN;
                } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                    wParam = VK_RETURN;
                } else if (key == Qt::Key_Space) {
                    wParam = VK_SPACE;
                }
                if (ev->type() == QEvent::KeyRelease) {
                    core::keyUp(wParam, lParam);
                } else {
                    core::keyDown(wParam, lParam);
                }
            }
            return false;
            break;
        case QEvent::Move:
            {
                if (core::moveScreen) {
                    QMoveEvent* qme = static_cast<QMoveEvent*>(ev);
                    QPoint p = qme->pos();
                    core::moveScreen(p.x(), p.y());
                }
            }
            return false;
            break;
        default:
            return false;
            break;
        }
    } else {
        return QMainWindow::eventFilter(obj, ev);
    }
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

void MainWindow::romOpen(const QString& url)
{
    romOpen(url, 0);
}

void MainWindow::romOpen(const QString& url, unsigned int archivefile)
{
    if (core::open_rom(url.toLocal8Bit(), archivefile) == 0) {
        startEmulation();
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
        startEmulation();
}

void MainWindow::emulationPauseContinue()
{
    core::pauseContinueEmulation();
}

void MainWindow::emulationStop()
{
    core::stopEmulation();
    if (m_renderWindow) {
        m_renderWindow->close();
        m_renderWindow->deleteLater();
   }
#ifdef DBG
    core::debugger_close();
#endif
}

void MainWindow::showCheatDialog()
{
    CheatDialog* d = new CheatDialog(this);
    d->show();
}

void MainWindow::fullScreenToggle()
{
    if (core::g_EmulatorRunning) {
        core::changeWindow();
#ifdef Q_WS_WIN
        if (core::moveScreen && m_renderWindow) {
            QPoint p = m_renderWindow->pos();
            core::moveScreen(p.x(), p.y());
        }
#endif
    }
}

void MainWindow::saveStateSave()
{
    if (core::g_EmulatorRunning) {
        core::savestates_job |= SAVESTATE;
    }
}

void MainWindow::saveStateSaveAs()
{
    if (core::g_EmulatorRunning) {
        char* file = core::savestates_get_filename();
        QString filename = QFileDialog::getSaveFileName(
                            this,
                            tr("Save State as..."),
                            tr(file),
                            NULL);
        if (!filename.isEmpty()) {
            core::savestates_select_filename(filename.toLocal8Bit());
            core::savestates_job |= SAVESTATE;
        }
    }
}

void MainWindow::saveStateProject64SaveAs()
{
    if (core::g_EmulatorRunning) {
        char* file = core::savestates_get_pj64_filename();
        QString filename = QFileDialog::getSaveFileName(
                            this,
                            tr("Save Project64 State as..."),
                            tr(file),
                            tr(".zip files (*.zip); .pj files (*.pj)"));
        if (!filename.isEmpty()) {
            core::savestates_select_filename(filename.toLocal8Bit());
            core::savestates_job |= SAVESTATE;
            core::savestates_job |= SAVEPJ64STATE;
        }
    }
}

void MainWindow::saveStateLoad()
{
    if (core::g_EmulatorRunning) {
        core::savestates_job |= LOADSTATE;
    }
}

void MainWindow::saveStateLoadFrom()
{
    if (core::g_EmulatorRunning) {
        QString filename = QFileDialog::getOpenFileName(this);
        if (!filename.isEmpty()) {
            core::savestates_select_filename(filename.toLocal8Bit());
            core::savestates_job |= LOADSTATE;
        }
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
    d->exec();
}

void MainWindow::itemCountUpdate(int count)
{
    m_statusBarLabel->setText(tr("%n rom(s)", "", count));
}

void MainWindow::aboutDialogShow()
{
    AboutDialog d(this);
    d.exec();
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
        case ConfirmEventType:
            event->setAccepted(
                confirmMessage(static_cast<ConfirmEvent*>(event)->message)
            );
            break;
        case PluginGuiQueryEventType:
            pluginGuiQueryEvent(static_cast<PluginGuiQueryEvent*>(event));
            break;
        default:
            qWarning("Got unknown custom event of type %d!", event->type());
            break;
    }
}

void MainWindow::startEmulation()
{
#ifdef Q_WS_WIN
    m_renderWindow = new QWidget;
    m_renderWindow->addActions(actions());
    m_renderWindow->installEventFilter(this);
    m_renderWindow->show();
    core::g_RenderWindow = (core::HWND) m_renderWindow->winId();
    core::g_StatusBar = (core::HWND) statusBar()->winId();
#endif // Q_WS_WIN

    core::startEmulation();
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

    actionCheats->setIcon(icon("tools-wizard.png"));
    connect(actionCheats, SIGNAL(triggered()),
             this, SLOT(showCheatDialog()));

    actionSaveState->setIcon(icon("document-save.png"));
    connect(actionSaveState, SIGNAL(triggered()),
            this, SLOT(saveStateSave()));
    actionSaveStateAs->setIcon(icon("document-save-as.png"));
    actionSaveStateAs->setData(false);
    connect(actionSaveStateAs, SIGNAL(triggered()),
            this, SLOT(saveStateSaveAs()));
    actionSaveProject64StateAs->setIcon(icon("project64.png"));
    actionSaveProject64StateAs->setData(TRUE);
    connect(actionSaveProject64StateAs, SIGNAL(triggered()),
            this, SLOT(saveStateProject64SaveAs()));
    actionLoadState->setIcon(icon("document-revert.png"));
    connect(actionLoadState, SIGNAL(triggered()), this, SLOT(saveStateLoad()));
    actionLoadStateFrom->setIcon(icon("document-open.png"));
    connect(actionLoadStateFrom, SIGNAL(triggered()),
            this, SLOT(saveStateLoadFrom()));

    actionCurrentSaveStateSlot->setIcon(icon("star.png"));

    QMenu* slotMenu = new QMenu(this);
    QActionGroup* slotActionGroup = new QActionGroup(actionCurrentSaveStateSlot);
    for(int i = 0; i < 10; i++) {
        QAction* slot = slotMenu->addAction(tr("Slot &%1").arg(i));
        slot->setShortcut(QString("Ctrl+%1").arg(i));
        slot->setCheckable(true);
        slot->setData(i);
        slotActionGroup->addAction(slot);
        slotActions << slot;
    }
    actionCurrentSaveStateSlot->setMenu(slotMenu);
    connect(slotMenu, SIGNAL(aboutToShow()), this, SLOT(savestateCheckSlot()));
    connect(slotActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(savestateSelectSlot(QAction*)));

    if (QWidget* w = toolBar->widgetForAction(actionCurrentSaveStateSlot)) {
        if (QToolButton* tb = qobject_cast<QToolButton*>(w)) {
            tb->setPopupMode(QToolButton::InstantPopup);
        }
    }

#ifdef DBG
    //Debugger Actions
    connect(actionEnableDebugger, SIGNAL(triggered()), this, SLOT(debuggerToggle()));
    connect(actionDisassembler, SIGNAL(triggered()), this, SLOT(disassemblerShow()));
    connect(actionRegisters, SIGNAL(triggered()), this, SLOT(registersShow()));
    connect(actionBreakpoints, SIGNAL(triggered()), this, SLOT(breakpointsShow()));
    connect(actionMemory, SIGNAL(triggered()), this, SLOT(memeditShow()));
    menu_Debug->menuAction()->setVisible(true);
#else
    menu_Debug->menuAction()->setVisible(false);
#endif

    //Settings Actions
    connect(actionShowFilter, SIGNAL(toggled(bool)),
            mainWidget, SLOT(showFilter(bool)));
    actionFullScreen->setIcon(icon("view-fullscreen.png"));
    connect(actionFullScreen, SIGNAL(triggered()),
            this, SLOT(fullScreenToggle()));
    actionConfigureMupen64Plus->setIcon(icon("preferences-system.png"));
    connect(actionConfigureMupen64Plus, SIGNAL(triggered()),
            this, SLOT(configDialogShow()));

    // Help menu actions
    actionAbout->setIcon(icon("mupen64plus.png"));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(aboutDialogShow()));
    connect(actionAboutQt, SIGNAL(triggered()),
            QApplication::instance(), SLOT(aboutQt()));

    actionShowToolbar->setChecked(
        core::config_get_bool("ToolBarVisible", TRUE)
    );
    actionShowFilter->setChecked(
        core::config_get_bool("FilterVisible", TRUE)
    );
    actionShowStatusbar->setChecked(
        core::config_get_bool("StatusBarVisible", TRUE)
    );

    m_uiActions = new QActionGroup(this);
    m_uiActions->setExclusive(false);
    m_uiActions->addAction(actionCloseRom);
    m_uiActions->addAction(actionSaveState);
    m_uiActions->addAction(actionSaveStateAs);
    m_uiActions->addAction(actionSaveProject64StateAs);
    m_uiActions->addAction(actionLoadState);
    m_uiActions->addAction(actionLoadStateFrom);
    m_uiActions->addAction(actionFullScreen);
    m_uiActions->addAction(actionStop);
    m_uiActions->addAction(actionPause);
}

void MainWindow::setState(core::gui_state_t state)
{
    QMetaObject::invokeMethod(this, "setStateImplementation",
                              Qt::QueuedConnection, Q_ARG(int, state));
}

void MainWindow::setStateImplementation(int state)
{
    bool pause, stop, play;
    pause = stop = play = false;

    switch (state) {
        case core::GUI_STATE_RUNNING:
            play = true;
            break;
        case core::GUI_STATE_PAUSED:
            pause = true;
            break;
        case core::GUI_STATE_STOPPED:
            stop = true;
            break;
    }

    m_uiActions->setEnabled(!stop);

    actionStart->setChecked(play);
    actionPause->setChecked(pause);
    actionStop->setChecked(stop);

    if (stop && !m_renderWindow.isNull()) {
        m_renderWindow->close();
        m_renderWindow->deleteLater();
    }

    if (play && !m_renderWindow.isNull()) {
        if (SDL_Surface* s = SDL_GetVideoSurface()) {
            m_renderWindow->setFixedSize(s->w, s->h);
        }
    }
}

void MainWindow::debuggerToggle()
{
#ifdef DBG
    if (actionEnableDebugger->isChecked()) {
        actionDisassembler->setEnabled(true);
        actionRegisters->setEnabled(true);
        actionBreakpoints->setEnabled(true);
        actionMemory->setEnabled(true);
        core::g_DebuggerEnabled = 1;
    }
    else {
        core::g_DebuggerEnabled = 0;
        actionDisassembler->setEnabled(false);
        actionRegisters->setEnabled(false);
        actionBreakpoints->setEnabled(false);
        actionMemory->setEnabled(false);
    }
#endif
}

void MainWindow::disassemblerShow()
{
#ifdef DBG
    core::debugger_show_disassembler();
#endif
}

void MainWindow::registersShow()
{
#ifdef DBG
    core::debugger_show_registers();
#endif
}

void MainWindow::breakpointsShow()
{
#ifdef DBG
    core::debugger_show_breakpoints();
#endif
}

void MainWindow::memeditShow()
{
#ifdef DBG
    core::debugger_show_memedit();
#endif
}

