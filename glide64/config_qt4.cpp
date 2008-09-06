/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - config_qt4.cpp                                          *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008 slougi                                             *
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

#include <QString>
#include <string.h>

#include "Gfx1.3.h"
#include "ui_glide64config.h"

extern "C" {

static void setUiSettings(Ui_Glide64ConfigDialog ui)
{
    ui.autodetectMicrocodeCheck->setChecked(settings.autodetect_ucode);
    ui.forceMicrocodeCombo->setCurrentIndex(settings.ucode);
    ui.windowResolutionCombo->setCurrentIndex(settings.res_data);
    ui.fullscreenResolutionCombo->setCurrentIndex(settings.full_res);
    ui.textureFilterCombo->setCurrentIndex(settings.tex_filter);
    ui.filteringModeCombo->setCurrentIndex(settings.filtering);
    ui.lodCalculationCombo->setCurrentIndex(settings.lodmode);
    ui.bufferSwappingMethodCombo->setCurrentIndex(settings.swapmode);
    ui.fogEnabledCheck->setChecked(settings.fog);
    ui.bufferClearOnEveryFrameCheck->setChecked(settings.buff_clear);
    ui.verticalSyncCheck->setChecked(settings.vsync);
    ui.fastCrcCheck->setChecked(settings.fast_crc);
    ui.hiresFramebufferCheck->setChecked(settings.fb_hires);
    ui.bufferSwappingMethodCombo->setCurrentIndex(settings.swapmode);
    ui.disableDitheredAlphaCheck->setChecked(settings.noditheredalpha);
    ui.disableGlslCombinersCheck->setChecked(settings.noglsl);
    ui.useFramebufferObjectsCheck->setChecked(settings.FBO);
    ui.customIniSettingsCheck->setChecked(!settings.custom_ini);
    ui.wrapTexturesCheck->setChecked(settings.wrap_big_tex);
    ui.zeldaCoronaFixCheck->setChecked(settings.flame_corona);
    ui.readEveryFrameCheck->setChecked(settings.fb_read_always);
    ui.detectCpuWritesCheck->setChecked(settings.cpu_write_hack);
    ui.getFramebufferInfoCheck->setChecked(settings.fb_get_info);
    ui.depthBufferRendererCheck->setChecked(settings.fb_depth_render);
    ui.fpsCounterCheck->setChecked(settings.show_fps & 1);
    ui.viCounterCheck->setChecked(settings.show_fps & 2);
    ui.percentSpeedCheck->setChecked(settings.show_fps & 4);
    ui.fpsTransparentCheck->setChecked(settings.show_fps & 8);
    ui.clockEnabledCheck->setChecked(settings.clock);
    ui.clockIs24HourCheck->setChecked(settings.clock_24_hr);
}

static void setSettings(Ui_Glide64ConfigDialog& ui)
{
    settings.autodetect_ucode = ui.autodetectMicrocodeCheck->isChecked();
    settings.ucode = ui.forceMicrocodeCombo->currentIndex();
    settings.res_data = ui.windowResolutionCombo->currentIndex();
    settings.full_res = ui.fullscreenResolutionCombo->currentIndex();
    settings.tex_filter = ui.textureFilterCombo->currentIndex();
    settings.filtering = ui.filteringModeCombo->currentIndex();
    settings.lodmode = ui.lodCalculationCombo->currentIndex();
    settings.swapmode = ui.bufferSwappingMethodCombo->currentIndex();
    settings.fog = ui.fogEnabledCheck->isChecked();
    settings.buff_clear = ui.bufferClearOnEveryFrameCheck->isChecked();
    settings.vsync = ui.verticalSyncCheck->isChecked();
    settings.fast_crc = ui.fastCrcCheck->isChecked();
    settings.fb_hires = ui.hiresFramebufferCheck->isChecked();
    settings.swapmode = ui.bufferSwappingMethodCombo->currentIndex();
    settings.noditheredalpha = ui.disableDitheredAlphaCheck->isChecked();
    settings.noglsl = ui.disableGlslCombinersCheck->isChecked();
    settings.FBO = ui.useFramebufferObjectsCheck->isChecked();
    settings.custom_ini = !ui.customIniSettingsCheck->isChecked();
    settings.wrap_big_tex = ui.wrapTexturesCheck->isChecked();
    settings.flame_corona = ui.zeldaCoronaFixCheck->isChecked();
    settings.fb_read_always = ui.readEveryFrameCheck->isChecked();
    settings.cpu_write_hack = ui.detectCpuWritesCheck->isChecked();
    settings.fb_get_info = ui.getFramebufferInfoCheck->isChecked();
    settings.fb_depth_render = ui.depthBufferRendererCheck->isChecked();
    settings.show_fps = (
        ui.fpsCounterCheck->isChecked() ? 1 : 0 |
        ui.viCounterCheck->isChecked() ? 2 : 0 |
        ui.percentSpeedCheck->isChecked() ? 4 : 0 |
        ui.fpsTransparentCheck->isChecked() ? 8 : 0
    );
    settings.clock = ui.clockEnabledCheck->isChecked();
    settings.clock_24_hr = ui.clockIs24HourCheck->isChecked();

    WriteSettings();

    // re-init evoodoo graphics to resize window
    if (evoodoo && fullscreen && !ev_fullscreen) {
        ReleaseGfx();
        InitGfx(TRUE);
    }
}

void CALL DllConfig(HWND hParent)
{
    ReadSettings();

    char name[21] = "DEFAULT";
    ReadSpecialSettings(name);

    if (gfx.HEADER) {
        for (int i = 0; i < 20; i++) {
            name[i] = gfx.HEADER[(32+i)^3];
        }
        name[20] = '\0';

        ReadSpecialSettings(qPrintable(QString(name).trimmed()));
    }

    QDialog d;
    Ui_Glide64ConfigDialog ui;
    ui.setupUi(&d);
    setUiSettings(ui);
    if (d.exec() == QDialog::Accepted) {
        setSettings(ui);
    }
}

} // extern "C"
