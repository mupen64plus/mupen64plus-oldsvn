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

#include "pluginconfig.h"

PluginConfig::PluginConfig()
{
    core::list_node_t* node = 0;
    list_foreach(core::g_PluginList, node) {
        core::plugin* plugin = static_cast<core::plugin*>(node->data);
        switch(plugin->type) {
            case PLUGIN_TYPE_RSP:
                rspPlugins << plugin;
                break;
            case PLUGIN_TYPE_GFX:
                graphicsPlugins << plugin;
                break;
            case PLUGIN_TYPE_AUDIO:
                audioPlugins << plugin;
                break;
            case PLUGIN_TYPE_CONTROLLER:
                inputPlugins << plugin;
                break;
        }
    }

    setCurrentGroup("Plugins");
    addItemInt("GraphicsPlugin", graphicsPluginIndex, 0);
    addItemInt("AudioPlugin", audioPluginIndex, 0);
    addItemInt("InputPlugin", inputPluginIndex, 0);
    addItemInt("RspPlugin", rspPluginIndex, 0);

    readConfig();
    writeConfig();
}

int PluginConfig::pluginIndex(char* mupenName, char* mupenConfigString,
                    PluginList plugins)
{
    QString name;
    QStringList available;
    int index = 0;

    if (mupenName) {
        name = core::plugin_name_by_filename(mupenName);
    } else {
        name = core::plugin_name_by_filename(
            core::config_get_string(mupenConfigString, "")
        );
    }
    
    if (!name.isEmpty()) {
        index = plugins.names().indexOf(name);
    }

    return index;
}

void PluginConfig::writePlugin(char* mupenConfigString, char* filename,
                    char* mupenName)
{
    if (!mupenName) {
        core::config_put_string(mupenConfigString, filename);
    }
}

void PluginConfig::usrReadConfig()
{
    graphicsPluginIndex = pluginIndex(core::g_GfxPlugin, "Gfx Plugin",
                                        graphicsPlugins);
    audioPluginIndex = pluginIndex(core::g_AudioPlugin, "Audio Plugin",
                                    audioPlugins);
    inputPluginIndex = pluginIndex(core::g_InputPlugin, "Input Plugin",
                                    inputPlugins);
    rspPluginIndex = pluginIndex(core::g_RspPlugin, "RSP Plugin",
                                    rspPlugins);
}

void PluginConfig::usrWriteConfig()
{
    writePlugin("Gfx Plugin",
                    graphicsPlugins[graphicsPluginIndex]->file_name,
                    core::g_GfxPlugin);
    writePlugin("Audio Plugin", 
                    audioPlugins[audioPluginIndex]->file_name,
                    core::g_AudioPlugin);
    writePlugin("Input Plugin",
                    inputPlugins[inputPluginIndex]->file_name,
                    core::g_InputPlugin);
    writePlugin("RSP Plugin",
                    rspPlugins[rspPluginIndex]->file_name,
                    core::g_RspPlugin);
    core::config_write();
}

#include "pluginconfig.moc"
