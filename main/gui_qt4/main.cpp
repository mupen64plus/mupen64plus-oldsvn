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

extern "C" {
    #include "../version.h"
    #include "../main.h"
    #include "../config.h"
}

#ifndef __WIN32__
# include <gtk/gtk.h>
#endif

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QMessageBox>

#include <cstdio>
#include <cstdarg>

#include "rommodel.h"
#include "mainwindow.h"
#include "globals.h"

static MainWindow* mainWindow = 0;
static QApplication* application = 0;

#include <QDebug>

extern "C" {

// Initializes gui subsystem. Also parses AND REMOVES any gui-specific commandline
// arguments. This is called before mupen64plus parses any of its commandline options.
void gui_init(int *argc, char ***argv)
{
    application = new QApplication(*argc, *argv);
    application->setOrganizationName("Mupen64Plus");
    application->setApplicationName("Mupen64Plus");
    application->setWindowIcon(icon("mupen64plus.png"));
    mainWindow = new MainWindow;

#ifndef __WIN32__
    if (QAbstractEventDispatcher::instance()->inherits("QEventDispatcherGlib"))
        gtk_init(argc, argv);
    else
        QMessageBox::warning(0,
            QObject::tr("No Glib Integration"),
            QObject::tr("<html><p>Your Qt library was compiled without glib \
                         mainloop integration. Plugins that use Gtk+ \
                         <b>will</b> crash the emulator!</p>\
                         <p>To fix this, install a Qt version with glib \
                         main loop support. Most distributions provide this \
                         by default.</p></html>"));
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
    delete mainWindow;
    mainWindow = 0;
    delete application;
    application = 0;
    config_write();
}

int gui_message(unsigned char messagetype, const char *format, ...)
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

    if (messagetype == 0) {
        InfoEvent* e = new InfoEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
    } else if (messagetype == 1) {
        AlertEvent* e = new AlertEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
    } else if (messagetype == 2) {
        //0 - indicates user selected no
        //1 - indicates user selected yes
        response = mainWindow->confirmMessage(buffer);
    }

    return response;
}

void updaterombrowser(unsigned int roms, unsigned short clear)
{
    RomModel::self()->update(roms, clear);
}

} // extern "C"
