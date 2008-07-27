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

#ifndef MUPEN_KDE4_MAINWINDOW_H
#define MUPEN_KDE4_MAINWINDOW_H

#include <KMainWindow>
#include <QEvent>
#include <QActionGroup>
#include <QtGui>

class MainWidget;
class KRecentFilesAction;

enum CustomEventTypes
{
    InfoEventType = QEvent::User,
    AlertEventType
};

class InfoEvent : public QEvent
{
    public:
        InfoEvent() : QEvent(Type(InfoEventType)) {}
        QString message;
};

class AlertEvent : public QEvent
{
    public:
        AlertEvent() : QEvent(Type(AlertEventType)) {}
        QString message;
};

class MainWindow : public KMainWindow
{
    Q_OBJECT
    public:
        MainWindow();

        void showInfoMessage(const QString& msg);
        void showAlertMessage(const QString& msg);
        bool confirmMessage(const QString& msg);

    private slots:
        void romOpen();
        void romOpen(const KUrl& url);
        void romOpen(const KUrl& url, unsigned int archivefile);
        void romClose();
        void emulationStart();
        void emulationPauseContinue();
        void emulationStop();
        void viewFullScreen();
        void saveStateSave();
        void saveStateSaveAs();
        void saveStateLoad();
        void saveStateLoadAs();
        void savestateCheckSlot();
        void savestateSelectSlot(QAction* a);
        void configDialogShow();
        void updateItemCount(int count);

        //test junk
        void toolbarIconOnly();
        void toolbarTextOnly();
        void toolbarTextBeside();
        void toolbarTextUnder();

    protected:
        bool event(QEvent* event);
        void closeEvent(QCloseEvent* event);

    private:
        void createActions();
        void createMenus();
        void createToolBars();
        MainWidget* m_mainWidget;
        KRecentFilesAction* m_actionRecentFiles;

        //Menus
        QMenu* fileMenu;
        QMenu* emulationMenu;
        QMenu* settingsMenu;

        //Toolbar
        QToolBar* mainToolBar;

        //Actions
        QAction* rom_open;
        QAction* rom_close;
        QAction* application_close;
        QAction* emulation_start;
        QAction* emulation_pause;
        QAction* emulation_stop;
        QAction* emulation_load_state;
        QAction* emulation_save_state;
        QAction* emulation_load_state_as;
        QAction* emulation_save_state_as;
        QAction* emulation_current_slot;

        QList<QAction*> slotActions;

        //Test junk
        QAction* toolbar_icon_only;
        QAction* toolbar_text_only;
        QAction* toolbar_text_beside;
        QAction* toolbar_text_under;
};

#endif // MUPEN_KDE4_MAINWINDOW_H
