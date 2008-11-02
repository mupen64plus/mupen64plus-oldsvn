/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main_gtk2.h                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
 *   Copyright (C) 2003 Rice 1964                                          *
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

#include <gtk/gtk.h>

#ifndef __MAIN_GTK2_H__
#define __MAIN_GTK2_H__

typedef struct
{
GtkWidget* dialog;
GtkWidget* notebook;
GtkWidget* okButton;

/* General Options. */
GtkWidget *windowModeCombo;
GtkWidget *fullScreenCombo;
GtkWidget *colorBufferDepthCombo;
GtkWidget* enableSSECheck;
GtkWidget* wireframeCheck;
GtkWidget* skipFrameCheck;
GtkWidget* enableFogCheck;
GtkWidget* showFPSCheck;

/* OpenGL options. */
GtkWidget* combinerTypeCombo;
GtkWidget* depthBufferSettingCombo;

/* Texture options. */
GtkWidget* textureQualityCombo;
GtkWidget* textureFilterCombo;
GtkWidget* textureEnhancementCombo;
GtkWidget* enhancementControlCombo;
GtkWidget* fullTMEMCheck;
GtkWidget* onlyTexRectCheck;
GtkWidget* onlySmallTexturesCheck;
GtkWidget* loadHiResCheck;
GtkWidget* dumpTexturesCheck;

/* Default game Options. */
GtkWidget* renderToTextureCombo;
GtkWidget* normalBlenderCheck;
GtkWidget* normalCombinerCheck;
GtkWidget* accurateMappingCheck;
GtkWidget* fastTextureCheck;
GtkWidget* nativeResolutionCheck;
GtkWidget* saveVRAMCheck;
GtkWidget* doubleBufferSmallCheck;

/* Basic Game Options. */
GtkWidget *basicGameOptions;

GtkWidget *frameUpdateAtCombo;
GtkWidget *renderToTextureGameCombo;
GtkWidget* normalBlenderCombo;
GtkWidget* accurateMappingCombo;
GtkWidget* normalCombinerCombo;
GtkWidget* fastTextureCombo;
GtkWidget* forceBufferClearCheck;
GtkWidget* disableBlenderCheck;
GtkWidget* emulateClearCheck;
GtkWidget* forceDepthCompareCheck;

/* Advanced Game Options. */
GtkWidget *advancedGameOptions;

GtkWidget* tmemEmulationCombo;
GtkWidget* disableBigTexturesCheck;
GtkWidget* useSmallTexturesCheck;
GtkWidget* disableCullingCheck;
GtkWidget* textureScaleHackCheck;
GtkWidget* alternativeSizeCalcCheck;
GtkWidget* fasterLoadingTilesCheck;
GtkWidget* enableTextureLODCheck;
GtkWidget* texture1HackCheck;
GtkWidget* primaryDepthHackCheck;
GtkWidget* increaseTexRectEdgeCheck;
GtkWidget* nearPlaneZHackCheck;

GtkWidget *n64ScreenWidthHeightEntry1;
GtkWidget *n64ScreenWidthHeightEntry2;
GtkWidget *useCICombo;
} ConfigDialog;

char* get_iconpath(char* iconfile);

#endif /* __MAIN_GTK2_H_ */

