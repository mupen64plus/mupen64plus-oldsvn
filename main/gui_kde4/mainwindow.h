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

#include <KXmlGuiWindow>
#include <QEvent>

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

class MainWindow : public KXmlGuiWindow
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
        void romClose();
        void emulationStart();
        void emulationPauseContinue();
        void emulationStop();
        void viewFullScreen();
        void saveStateSave();
        void saveStateSaveAs();
        void saveStateRestore();
        void saveStateLoad();
        void saveStateSetCurrent(QAction* a);
        void configDialogShow();
        void updateItemCount(int count);

    protected:
        bool event(QEvent* event);
        void closeEvent(QCloseEvent* event);

    private:
        void createActions();
        MainWidget* m_mainWidget;
        KRecentFilesAction* m_actionRecentFiles;
};

#endif // MUPEN_KDE4_MAINWINDOW_H
