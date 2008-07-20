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

#ifndef MUPEN64_KDE4_PLUGINSCONFIG_H
#define MUPEN64_KDE4_PLUGINSCONFIG_H

#include <KConfigSkeleton>
#include <QList>

namespace core {
    extern "C" {
        #include "../plugin.h"
        #include "../main.h"
        #include "../config.h"
    }
}

class PluginList : public QList<core::plugin*>
{
    public:
        QStringList names() const
        {
            QStringList result;
            foreach(core::plugin* p, *this) {
                result << p->plugin_name;
            }
            return result;       
        }
};

class PluginConfig : public KConfigSkeleton
{
    Q_OBJECT
    public:
        PluginConfig();

        int graphicsPluginIndex;
        int audioPluginIndex;
        int inputPluginIndex;
        int rspPluginIndex;
    
        PluginList graphicsPlugins;
        PluginList audioPlugins;
        PluginList inputPlugins;
        PluginList rspPlugins;

    protected:
        int pluginIndex(char* mupenName, const char* mupenConfigString,
                            PluginList plugins);

        void writePlugin(const char* mupenConfigString, char* filename,
                          char* mupenName);

        virtual void usrReadConfig();
        virtual void usrWriteConfig();
};

#endif // MUPEN64_KDE4_PLUGINSCONFIG_H
