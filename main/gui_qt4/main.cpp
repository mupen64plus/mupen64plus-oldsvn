/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main.cpp                                                *
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

#include <QtGlobal>

#ifdef Q_WS_X11
# include <gtk/gtk.h>
#endif

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>

#include <cstdio>
#include <cstdarg>

#include "rommodel.h"
#include "mainwindow.h"
#include "globals.h"

#ifdef DBG
#include "debugger/debuggerwidget.h"
#include "debugger/registerwidget.h"
#include "debugger/breakpointswidget.h"
#include "debugger/memeditwidget.h"
#endif

// ugly globals
static MainWindow* mainWindow = 0;
static QApplication* application = 0;
static QTranslator* translator = 0;
#ifdef DBG
void update_debugger_frontend( unsigned int );

static DebuggerWidget* debuggerWidget = 0;
static RegisterWidget* registerWidget = 0;
static BreakpointsWidget* breakpointsWidget = 0;
static MemEditWidget* memeditWidget = 0;

unsigned int _pc = 0;
#endif
namespace core {
extern "C" {

#include "../version.h"
#include "../main.h"
#include "../config.h"
#include "../gui.h"

// Initializes gui subsystem. Also parses AND REMOVES any gui-specific commandline
// arguments. This is called before mupen64plus parses any of its commandline options.
void gui_init(int *argc, char ***argv)
{
#ifdef Q_WS_X11
    // We always want the glib-based event loop - make sure it is not disabled by
    // the user.
    unsetenv("QT_NO_GLIB");
#endif

    application = new QApplication(*argc, *argv);

    QString locale = QLocale::system().name();
    QString translation = QString("mupen64plus_%1").arg(locale);
    QString path = QString("%1translations").arg(get_installpath());
    translator = new QTranslator;
    if (!translator->load(translation, path)) {
        qDebug("Could not load translation %s. Looked in %s.",
                  qPrintable(translation), qPrintable(path));
    }

    application->installTranslator(translator);

#ifdef Q_WS_X11
    if (QAbstractEventDispatcher::instance()->inherits("QEventDispatcherGlib")) {
        delete application;
        application = 0;
        gtk_init(argc, argv);
        application = new QApplication(*argc, *argv);
        application->installTranslator(translator);
    } else {
        QMessageBox::warning(0,
            QObject::tr("No Glib Integration"),
            QObject::tr("<html><p>Your Qt library was compiled without glib "
                        "mainloop integration. Plugins that use Gtk+ "
                        "<b>will</b> crash the emulator!</p>"
                        "<p>To fix this, install a Qt version with glib "
                        "main loop support. Most distributions provide this "
                        "by default.</p></html>"));
    }
#endif

    application->setOrganizationName("Mupen64Plus");
    application->setApplicationName("Mupen64Plus");
    application->setWindowIcon(icon("mupen64plus.png"));

#if QT_VERSION >= 0x040400
    application->setAttribute(Qt::AA_NativeWindows);
#endif

    mainWindow = new MainWindow;

#ifdef DBG
    debuggerWidget = new DebuggerWidget;
    registerWidget = new RegisterWidget;
    breakpointsWidget = new BreakpointsWidget;
    memeditWidget = new MemEditWidget;
#endif
}

// display GUI components to the screen
void gui_display(void)
{
    mainWindow->show();
    QApplication::instance()->processEvents();
    QApplication::instance()->sendPostedEvents();
}

// Give control of thread to the gui
void gui_main_loop(void)
{
    application->exec();
    config_write();
    delete mainWindow;
    mainWindow = 0;
    delete application;
    application = 0;
    delete translator;
    translator = 0;
#ifdef DBG
    debuggerWidget = 0;
    registerWidget = 0;
    breakpointsWidget = 0;
    memeditWidget = 0;
#endif
}

int gui_message(gui_message_t messagetype, const char *format, ...)
{
    if (!gui_enabled())
        return 0;

    va_list ap;
    char buffer[2049];
    int response = 0;

    va_start(ap, format);
    vsnprintf(buffer, 2048, format, ap);
    buffer[2048] = '\0';
    va_end(ap);

    if (messagetype == GUI_MESSAGE_INFO) {
        InfoEvent* e = new InfoEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
    } else if (messagetype == GUI_MESSAGE_CONFIRM) {
        //0 - indicates user selected no
        //1 - indicates user selected yes
        ConfirmEvent e;
        e.message = buffer;
        application->sendEvent(mainWindow, &e);
        response = e.isAccepted();
    } else if (messagetype == GUI_MESSAGE_ERROR) {
        AlertEvent* e = new AlertEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
    }
    return response;
}

void gui_update_rombrowser(unsigned int roms, unsigned short clear)
{
    RomModel::self()->update(roms, clear);
}

void gui_set_state(gui_state_t state)
{
     if (!gui_enabled())
        return;

    mainWindow->setState(state);
}

#ifdef DBG
void init_debugger_frontend()
{
    QMetaObject::invokeMethod(registerWidget, "show", Qt::QueuedConnection);
    QMetaObject::invokeMethod(debuggerWidget, "show", Qt::QueuedConnection);
    debuggerWidget->setFocus();
}
void update_debugger_frontend( unsigned int pc )
{
    _pc = pc;
    if (debuggerWidget->isVisible()) {
        QMetaObject::invokeMethod(debuggerWidget, "update_desasm",
                                   Qt::QueuedConnection,
                                   Q_ARG(unsigned int, pc));
    }
    if (debuggerWidget->isVisible()) {
        QMetaObject::invokeMethod(registerWidget, "update_registers",
                                   Qt::QueuedConnection,
                                   Q_ARG(unsigned int, pc));
    }    
}

//Runs each VI for auto-updating views
void debugger_frontend_vi()
{
    //TODO: Implement debugger_frontend_vi
}
void switch_button_to_run()
{
    //TODO: Implement switch_button_to_run
}

void debugger_show_disassembler( )
{
    QMetaObject::invokeMethod(debuggerWidget, "show", Qt::QueuedConnection);
    debuggerWidget->setFocus();
}

void debugger_show_registers( )
{
    QMetaObject::invokeMethod(registerWidget, "show", Qt::QueuedConnection);
    registerWidget->setFocus();
}

void debugger_show_breakpoints( )
{
    QMetaObject::invokeMethod(breakpointsWidget, "show", Qt::QueuedConnection);
    breakpointsWidget->setFocus();
}

void debugger_show_memedit( )
{
    QMetaObject::invokeMethod(memeditWidget, "show", Qt::QueuedConnection);
    memeditWidget->setFocus();
    QMetaObject::invokeMethod(memeditWidget, "update_memedit",
                               Qt::QueuedConnection);
}

void debugger_update_desasm()
{
    if (debuggerWidget->isVisible()) {
        QMetaObject::invokeMethod(debuggerWidget, "update_desasm",
                                   Qt::QueuedConnection,
                                   Q_ARG(unsigned int, _pc));
    }
}

void debugger_close()
{
    debuggerWidget->setVisible(false);
    registerWidget->setVisible(false);
    breakpointsWidget->setVisible(false);
    memeditWidget->setVisible(false);
}
#endif

} // extern "C"
} // namespace core

