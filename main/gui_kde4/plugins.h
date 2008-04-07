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

#ifndef MUPEN64_KDE4_PLUGINS_H
#define MUPEN64_KDE4_PLUGINS_H

#include <QStringList>
#include <KConfigSkeleton>

namespace core {
    struct _plugin;
    typedef _plugin plugin;
}

class PluginList : public QList<core::plugin*>
{
    public:
        QStringList names() const;
};

class Plugins : public KConfigSkeleton
{
    Q_OBJECT
    public:
        Plugins();
        static Plugins* self();

        enum PluginType { Graphics, Audio, Input, Rsp };
        QStringList plugins(PluginType);

    // this sucks!
    public slots:
        void aboutGraphicsPlugin();
        void configureGraphicsPlugin();
        void testGraphicsPlugin();

        void aboutInputPlugin();
        void configureInputPlugin();
        void testInputPlugin();

        void aboutAudioPlugin();
        void configureAudioPlugin();
        void testAudioPlugin();

        void aboutRspPlugin();
        void configureRspPlugin();
        void testRspPlugin();

    protected:
        virtual void usrReadConfig();
        virtual void usrWriteConfig();

    private:
        int pluginIndex(char* mupenName, char* mupenConfigString,
                         PluginList plugins);
        void writePlugin(char* mupenConfigString, char* filename,
                          char* mupenName);

        int graphicsPluginIndex;
        int audioPluginIndex;
        int inputPluginIndex;
        int rspPluginIndex;
        
        PluginList graphicsPlugins;
        PluginList audioPlugins;
        PluginList inputPlugins;
        PluginList rspPlugins;
};

#endif // MUPEN64_KDE4_PLUGINS_H
