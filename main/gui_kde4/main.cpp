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
}

#include <gtk/gtk.h>

#include <KApplication>
#include <KAboutData>
#include <KLocale>
#include <KCmdLineArgs>

#include <cstdio>
#include <cstdarg>

#include "rommodel.h"
#include "mainwindow.h"
#include "globals.h"
#include "plugins.h"

static KAboutData* aboutData = 0;
static MainWindow* mainWindow = 0;
static KApplication* application = 0;

// Initializes gui subsystem. Also parses AND REMOVES any gui-specific commandline
// arguments. This is called before mupen64plus parses any of its commandline options.
extern "C" {
void gui_init(int *argc, char ***argv)
{

    aboutData = new KAboutData(
        "mupen64plus", 0, ki18n("Mupen64Plus"),
        MUPEN_VERSION,
        ki18n("Cross-platform N64 emulator."),
        KAboutData::License_GPL,
        ki18n("Copyright (C) the Mupen64Plus team")
    );

    aboutData->addAuthor(ki18n("Hacktarux"), ki18n("Original Mupen64 Code"));
    aboutData->addAuthor(ki18n("Richard32"), ki18n("Developer"));
    aboutData->addAuthor(ki18n("DarkJezter"), ki18n("Developer"));
    aboutData->addAuthor(ki18n("Tillin9"), ki18n("Developer"));
    aboutData->addAuthor(ki18n("Gunther"), ki18n("Glide64 port"));
    aboutData->addAuthor(ki18n("slougi"), ki18n("KDE4 Interface"));
    aboutData->addAuthor(ki18n("Ebenblues"), ki18n("Fixes and Features"));
    aboutData->addAuthor(ki18n("nmn"), ki18n("Fixes and Features"));

    KCmdLineArgs::addStdCmdLineOptions();
    KCmdLineArgs::init(*argc, *argv, aboutData);

    gtk_init(argc, argv);

    QString file;
    char* iconpath = get_iconspath();
    // This is done here because above the mupen64 resource handling isn't
    // initialized properly yet...
    aboutData->setOtherText(ki18n("<html><img src=\"%1\"></img></html>").subs(file.sprintf("%s%s", iconpath, "logo.png")));
    application = new KApplication;
    mainWindow = new MainWindow;
}

// display GUI components to the screen
void gui_display(void)
{
    mainWindow->show();
    KApplication::instance()->processEvents();
    KApplication::instance()->sendPostedEvents();
}

// Give control of thread to the gui
void gui_main_loop(void)
{
    application->exec();
    KGlobal::config()->sync(); // Make sure we sync settings to disk on exit
}

int gui_message(unsigned char messagetype, const char *format, ...)
{
    if(!gui_enabled())
        return 0;

    va_list ap;
    char buffer[2049];
    pthread_t self = pthread_self();
    gint response = 0;

    va_start(ap, format);
    vsnprintf(buffer, 2048, format, ap);
    buffer[2048] = '\0';
    va_end(ap);

    if(messagetype==0)
        {
        InfoEvent* e = new InfoEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
        }
    else if(messagetype==1)
        {
        AlertEvent* e = new AlertEvent;
        e->message = buffer;
        application->postEvent(mainWindow, e);
        }
    else if(messagetype==2)
        {
        //0 - indicates user selected no
        //1 - indicates user selected yes
        return mainWindow->confirmMessage(buffer);
        }
}

void updaterombrowser(unsigned int roms, unsigned short clear)
{
    RomModel::self()->update(roms, clear);
}
}
