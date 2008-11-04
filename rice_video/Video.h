/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Video.h                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Rice1964                                           *
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

#ifndef _DLLINTERFACE_H_
#define _DLLINTERFACE_H_

#include <limits.h> /* PATH_MAX */

typedef enum 
{
    PRIM_TRI1,
    PRIM_TRI2,
    PRIM_TRI3,
    PRIM_DMA_TRI,
    PRIM_LINE3D,
    PRIM_TEXTRECT,
    PRIM_TEXTRECTFLIP,
    PRIM_FILLRECT,
} PrimitiveType;

typedef enum 
{
    RSP_SCISSOR,
    RDP_SCISSOR,
    UNKNOWN_SCISSOR,
} CurScissorType;

enum {
    TEXTURE_NO_ENHANCEMENT,
    TEXTURE_2X_ENHANCEMENT,
    TEXTURE_2XSAI_ENHANCEMENT,
    TEXTURE_HQ2X_ENHANCEMENT,
    TEXTURE_LQ2X_ENHANCEMENT,
    TEXTURE_HQ4X_ENHANCEMENT,
    TEXTURE_SHARPEN_ENHANCEMENT,
    TEXTURE_SHARPEN_MORE_ENHANCEMENT,
    TEXTURE_EXTERNAL,
    TEXTURE_MIRRORED,
};

typedef struct {
    float   fViWidth, fViHeight;
    unsigned short uViWidth, uViHeight;
    unsigned short uDisplayWidth, uDisplayHeight;
    unsigned short uFullScreenDisplayWidth, uFullScreenDisplayHeight;
    unsigned short uWindowDisplayWidth, uWindowDisplayHeight;

    unsigned char bDisplayFullscreen;
    int uFullScreenRefreshRate;

    float fMultX, fMultY;
    int vpLeftW, vpTopW, vpRightW, vpBottomW, vpWidthW, vpHeightW;

    int statusBarHeight, statusBarHeightToUse, toolbarHeight, toolbarHeightToUse;
    unsigned char screenSaverStatus;

    struct {
        unsigned int left;
        unsigned int top;
        unsigned int right;
        unsigned int bottom;
        unsigned int width;
        unsigned int height;
        unsigned char needToClip;
    } clipping;

    int timer;
    float fps;    // frame per second
    float dps;    // dlist per second
    unsigned int lastSecFrameCount;
    unsigned int lastSecDlistCount;
} WindowSettingStruct;

extern WindowSettingStruct windowSetting;

typedef struct {
    unsigned char bGameIsRunning;
    unsigned int dwTvSystem;
    float   fRatio;

    unsigned char frameReadByCPU;
    unsigned char frameWriteByCPU;

    unsigned int SPCycleCount;       // Count how many CPU cycles SP used in this DLIST
    unsigned int DPCycleCount;       // Count how many CPU cycles DP used in this DLIST

    unsigned int dwNumTrisRendered;
    unsigned int dwNumDListsCulled;
    unsigned int dwNumTrisClipped;
    unsigned int dwNumVertices;
    unsigned int dwBiggestVertexIndex;

    unsigned int gDlistCount;
    unsigned int gFrameCount;
    unsigned int gUcodeCount;
    unsigned int gRDPTime;
    unsigned char ToToggleFullScreen;
    unsigned char bDisableFPS;

    unsigned char bUseModifiedUcodeMap;
    unsigned char ucodeHasBeenSet;
    unsigned char bUcodeIsKnown;

    unsigned int curRenderBuffer;
    unsigned int curDisplayBuffer;
    unsigned int curVIOriginReg;
    CurScissorType curScissor;

    PrimitiveType primitiveType;

    unsigned int lastPurgeTimeTime;      // Time textures were last purged

    unsigned char UseLargerTile[2];       // This is a speed up for large tile loading,
    unsigned int LargerTileRealLeft[2];  // works only for TexRect, LoadTile, large width, large pitch

    unsigned char bVIOriginIsUpdated;
    unsigned char bCIBufferIsRendered;
    int     leftRendered,topRendered,rightRendered,bottomRendered;

    unsigned char isMMXSupported;
    unsigned char isSSESupported;
    unsigned char isVertexShaderSupported;

    unsigned char isMMXEnabled;
    unsigned char isSSEEnabled;
    unsigned char isVertexShaderEnabled;
    unsigned char bUseHW_T_L;                 // Use hardware T&L, for debug purpose only

    unsigned char toShowCFB;
    unsigned char toCaptureScreen;
    char    screenCaptureFilename[MAX_PATH];

    char    CPUCoreMsgToDisplay[256];
    unsigned char CPUCoreMsgIsSet;

    unsigned char bAllowLoadFromTMEM;

    // Frame buffer simulation related status variables
    unsigned char bN64FrameBufferIsUsed;      // Frame buffer is used in the frame
    unsigned char bN64IsDrawingTextureBuffer; // The current N64 game is rendering into render_texture, to create self-rendering texture
    unsigned char bHandleN64RenderTexture;    // Do we need to handle of the N64 render_texture stuff?
    unsigned char bDirectWriteIntoRDRAM;      // When drawing into render_texture, this value =
                                        // = true   don't render, but write real N64 graphic value into RDRAM
                                        // = false  rendering into render_texture of OGL, the data will be copied into RDRAM at the end
    unsigned char bFrameBufferIsDrawn;        // flag to mark if the frame buffer is ever drawn
    unsigned char bFrameBufferDrawnByTriangles;   // flag to tell if the buffer is even drawn by Triangle cmds

    unsigned char bScreenIsDrawn;

    unsigned char bTexturesLoaded;
} PluginStatus;

extern PluginStatus status;
extern char generalText[];
extern void (*renderCallback)();
extern char g_ConfigDir[PATH_MAX];

void SetVIScales();

enum SupportedDeviceType
{
    OGL_DEVICE,
    OGL_1_1_DEVICE,
    OGL_1_2_DEVICE,
    OGL_1_3_DEVICE,
    OGL_1_4_DEVICE,
    OGL_1_4_V2_DEVICE,
    OGL_TNT2_DEVICE,
    NVIDIA_OGL_DEVICE,
    OGL_FRAGMENT_PROGRAM,
};

typedef struct
{
    const char* name;
    enum SupportedDeviceType type;
} RenderEngineSetting;

typedef struct
{
    const char* description;
    unsigned int setting;
} SettingInfo;

extern RenderEngineSetting OpenGLRenderSettings[];
extern SettingInfo openGLDepthBufferSettings[];
extern SettingInfo TextureQualitySettings[];
extern SettingInfo TextureEnhancementControlSettings[];
#endif

