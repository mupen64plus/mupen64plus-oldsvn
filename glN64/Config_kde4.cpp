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

#include <KConfigDialog>

#include "Config.h"
#include "OpenGL.h"

#include "gln64settings.h"
#include "ui_gln64config.h"

void Config_LoadConfig()
{
    glN64Settings* s = glN64Settings::self();

    int xres = 640, yres = 480;
    QStringList res = s->resolution().split("x");
    if (res.count() == 2) {
        bool x_ok, y_ok;
        int newxres = res[0].toInt(&x_ok);
        int newyres = res[1].toInt(&y_ok);
        if (x_ok && y_ok) {
            xres = newxres;
            yres = newyres;
        }
    }
    OGL.fullscreenWidth = xres;
    OGL.fullscreenHeight = yres;
    //  OGL.fullscreenBits = 0;
    OGL.windowedWidth = xres;
    OGL.windowedHeight = yres;
    //  OGL.windowedBits = 0;
    OGL.forceBilinear = s->forceBilinearFiltering();
    OGL.enable2xSaI = s->twoExSaiTextureScaling();
    OGL.enableAnisotropicFiltering = s->anisotropicFiltering();
    OGL.fog = s->drawFog();
    OGL.textureBitDepth = s->bitDepth(); // normal (16 & 32 bits)
    OGL.frameBufferTextures = s->hwFramebufferTextures();
    OGL.usePolygonStipple = s->ditheredAlphaTesting();
    cache.maxBytes = s->cacheSize() * 1024 * 1024;
}

void Config_DoConfig()
{
    if (KConfigDialog::showDialog("glN64 settings")) {
        return;
    }


    KConfigDialog* dialog = new KConfigDialog(0, "glN64 settings",
                                              glN64Settings::self());
    dialog->setFaceType(KPageDialog::Plain);
    dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);

    QWidget* settingsWidget = new QWidget(dialog);
    Ui::glN64ConfigWidget().setupUi(settingsWidget);
    dialog->addPage(settingsWidget, "")->setHeader("");

    dialog->show();
}
