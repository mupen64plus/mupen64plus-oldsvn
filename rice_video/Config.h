/*
Copyright (C) 2002 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _RICE_CONFIG_H_
#define _RICE_CONFIG_H_

#include "winlnxdefs.h"

#ifndef bool
#define bool unsigned int
#endif

#define INI_FILE        "RiceVideoLinux.ini"
#define CONFIG_FILE     "RiceVideo.cfg"

enum DirectXCombinerType
{
    DX_DISABLE_COMBINER,
    DX_BEST_FIT,
    DX_LOW_END,
    DX_HIGH_END,
    DX_NVIDIA_TNT,
    DX_2_STAGES,
    DX_3_STAGES,
    DX_4_STAGES,
    DX_PIXEL_SHADER,
    DX_SEMI_PIXEL_SHADER,
};

enum {
    FRM_BUF_NONE,
    FRM_BUF_IGNORE,
    FRM_BUF_BASIC,
    FRM_BUF_BASIC_AND_WRITEBACK,
    FRM_BUF_WRITEBACK_AND_RELOAD,
    FRM_BUF_COMPLETE,
    FRM_BUF_WITH_EMULATOR,
    FRM_BUF_BASIC_AND_WITH_EMULATOR,
    FRM_BUF_WITH_EMULATOR_READ_ONLY,
    FRM_BUF_WITH_EMULATOR_WRITE_ONLY,
};

enum {
    FRM_BUF_WRITEBACK_NORMAL,
    FRM_BUF_WRITEBACK_1_2,
    FRM_BUF_WRITEBACK_1_3,
    FRM_BUF_WRITEBACK_1_4,
    FRM_BUF_WRITEBACK_1_5,
    FRM_BUF_WRITEBACK_1_6,
    FRM_BUF_WRITEBACK_1_7,
    FRM_BUF_WRITEBACK_1_8,
};

enum {
    TXT_BUF_NONE,
    TXT_BUF_IGNORE,
    TXT_BUF_NORMAL,
    TXT_BUF_WRITE_BACK,
    TXT_BUF_WRITE_BACK_AND_RELOAD   ,
};

enum {
    TXT_QUALITY_DEFAULT,
    TXT_QUALITY_32BIT,
    TXT_QUALITY_16BIT,
};

enum {
    FORCE_DEFAULT_FILTER,
    FORCE_POINT_FILTER,
    FORCE_LINEAR_FILTER,
    FORCE_BILINEAR_FILTER,
};

enum {
    TEXTURE_ENHANCEMENT_NORMAL,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

enum {
    SCREEN_UPDATE_DEFAULT = 0,
    SCREEN_UPDATE_AT_VI_UPDATE = 1,
    SCREEN_UPDATE_AT_VI_CHANGE = 2,
    SCREEN_UPDATE_AT_CI_CHANGE = 3,
    SCREEN_UPDATE_AT_1ST_CI_CHANGE = 4,
    SCREEN_UPDATE_AT_1ST_PRIMITIVE = 5,
    SCREEN_UPDATE_BEFORE_SCREEN_CLEAR = 6,
    SCREEN_UPDATE_AT_VI_UPDATE_AND_DRAWN = 7,   // Update screen at VI origin is updated and the screen has been drawn
};

enum HACK_FOR_GAMES
{
    NO_HACK_FOR_GAME,
    HACK_FOR_BANJO_TOOIE,
    HACK_FOR_DR_MARIO,
    HACK_FOR_ZELDA,
    HACK_FOR_MARIO_TENNIS,
    HACK_FOR_BANJO,
    HACK_FOR_PD,
    HACK_FOR_GE,
    HACK_FOR_PILOT_WINGS,
    HACK_FOR_YOSHI,
    HACK_FOR_NITRO,
    HACK_FOR_TONYHAWK,
    HACK_FOR_NASCAR,
    HACK_FOR_SUPER_BOWLING,
    HACK_FOR_CONKER,
    HACK_FOR_ALL_STAR_BASEBALL,
    HACK_FOR_TIGER_HONEY_HUNT,
    HACK_REVERSE_XY_COOR,
    HACK_REVERSE_Y_COOR,
    HACK_FOR_GOLDEN_EYE,
    HACK_FOR_FZERO,
    HACK_FOR_COMMANDCONQUER,
    HACK_FOR_RUMBLE,
    HACK_FOR_SOUTH_PARK_RALLY,
    HACK_FOR_BUST_A_MOVE,
    HACK_FOR_OGRE_BATTLE,
    HACK_FOR_TWINE,
    HACK_FOR_EXTREME_G2,
    HACK_FOR_ROGUE_SQUADRON,
    HACK_FOR_MARIO_GOLF,
    HACK_FOR_MLB,
    HACK_FOR_POLARISSNOCROSS,
    HACK_FOR_TOPGEARRALLY,
    HACK_FOR_DUKE_NUKEM,
    HACK_FOR_ZELDA_MM,
    HACK_FOR_MARIO_KART
};

enum {
    NOT_USE_CI_WIDTH_AND_RATIO,
    USE_CI_WIDTH_AND_RATIO_FOR_NTSC,
    USE_CI_WIDTH_AND_RATIO_FOR_PAL,
};

typedef struct {
    BOOL    bEnableHacks;
    BOOL    bEnableFog;
    BOOL    bWinFrameMode;
    BOOL    bForceSoftwareTnL;
    BOOL    bForceSoftwareClipper;
    BOOL    bOGLVertexClipper;
    BOOL    bEnableSSE;
    BOOL    bEnableVertexShader;
    BOOL    bSkipFrame;
    BOOL    bDisplayTooltip;
    BOOL    bHideAdvancedOptions;
    BOOL    bFullTMEM;
    BOOL    bUseFullTMEM;

    unsigned int  bDisplayOnscreenFPS;
    unsigned int  FPSColor;
    BOOL    bShowFPS;

    unsigned int  forceTextureFilter;
    unsigned int  textureEnhancement;
    unsigned int  textureEnhancementControl;
    unsigned int  textureQuality;
    BOOL    bTexRectOnly;
    BOOL    bSmallTextureOnly;
    BOOL    bDumpTexturesToFiles;
    BOOL    bLoadHiResTextures;

    int     RenderBufferSetting;

    int     DirectXDepthBufferSetting;
    unsigned int  DirectXAntiAliasingValue;
    unsigned int  DirectXAnisotropyValue;

    unsigned int  DirectXMaxFSAA;
    unsigned int  DirectXMaxAnisotropy;
    int     DirectXCombiner;
    int     DirectXDevice;

    int     OpenglDepthBufferSetting;
    int     OpenglRenderSetting;
    unsigned int  colorQuality;

    enum HACK_FOR_GAMES enableHackForGames;

    int     widescreenMode;
    BOOL    bWidescreenStretchBG;

} GlobalOptions;

enum WidescreenMode {
    WIDESCREEN_STRETCH,
    WIDESCREEN_PILLARBOX,
    WIDESCREEN_EXTEND
};

extern GlobalOptions options;

typedef struct {
    bool    bUpdateCIInfo;

    bool    bCheckBackBufs;         // Check texture again against the recent backbuffer addresses
    bool    bWriteBackBufToRDRAM;   // If a recent backbuffer is used, write its content back to RDRAM
    bool    bLoadBackBufFromRDRAM;  // Load content from RDRAM and draw into backbuffer
    bool    bIgnore;                // Ignore all rendering into texture buffers

    bool    bSupportRenderTextures;     // Support render-to-texture
    bool    bCheckRenderTextures;           // Check texture again against the the last render_texture addresses
    bool    bRenderTextureWriteBack;        // Write back render_texture into RDRAM
    bool    bLoadRDRAMIntoRenderTexture;    // Load RDRAM content and render into render_texture

    bool    bAtEachFrameUpdate;     // Reload and write back at each frame buffer and CI update

    bool    bProcessCPUWrite;
    bool    bProcessCPURead;

    bool    bFillRectNextTextureBuffer;
    bool    bIgnoreRenderTextureIfHeightUnknown;
    //bool  bFillColor;
} FrameBufferOptions;

extern FrameBufferOptions frameBufferOptions;

BOOL InitConfiguration(void);

typedef struct {
    unsigned int  N64FrameBufferEmuType;
    unsigned int  N64FrameBufferWriteBackControl;
    unsigned int  N64RenderToTextureEmuType;
    unsigned int  screenUpdateSetting;
    BOOL    bNormalCombiner;
    BOOL    bNormalBlender;
    BOOL    bFastTexCRC;
    BOOL    bAccurateTextureMapping;
    BOOL    bInN64Resolution;
    BOOL    bDoubleSizeForSmallTxtrBuf;
    BOOL    bSaveVRAM;
    BOOL    bOverlapAutoWriteBack;
} RomOptions;

extern RomOptions defaultRomOptions;
extern RomOptions currentRomOptions;

typedef struct IniSection
{
    bool    bOutput;
    char    crccheck[50];
    char    name[50];

    // Options with changeable default values
    unsigned int  dwNormalCombiner;
    unsigned int  dwNormalBlender;
    unsigned int  dwFastTextureCRC;
    unsigned int  dwAccurateTextureMapping;
    unsigned int  dwFrameBufferOption;
    unsigned int  dwRenderToTextureOption;
    unsigned int  dwScreenUpdateSetting;

    // Options with FALSE as default values
    BOOL    bDisableBlender;
    BOOL    bForceScreenClear;
    BOOL    bEmulateClear;
    BOOL    bForceDepthBuffer;

    // Less useful options
    BOOL    bDisableObjBG;
    BOOL    bDisableTextureCRC;
    BOOL    bIncTexRectEdge;
    BOOL    bZHack;
    BOOL    bTextureScaleHack;
    BOOL    bFastLoadTile;
    BOOL    bUseSmallerTexture;
    BOOL    bPrimaryDepthHack;
    BOOL    bTexture1Hack;
    BOOL    bDisableCulling;

    int     VIWidth;
    int     VIHeight;
    unsigned int  UseCIWidthAndRatio;

    unsigned int dwFullTMEM;
    BOOL    bTxtSizeMethod2;
    BOOL    bEnableTxtLOD;
} section;

extern bool bIniIsChanged;
extern char szIniFileName[300];

void WriteIniFile();
BOOL ReadIniFile();
void OutputSectionDetails(unsigned int i, FILE* fh);
int FindIniEntry(unsigned int dwCRC1, unsigned int dwCRC2, unsigned char nCountryID, char* szName);

typedef struct
{
    unsigned char  x1, x2, x3, x4;
    unsigned int dwClockRate;
    unsigned int dwBootAddressOffset;
    unsigned int dwRelease;
    unsigned int dwCRC1;
    unsigned int dwCRC2;
    unsigned long long qwUnknown1;
    char  szName[20];
    unsigned int dwUnknown2;
    unsigned short wUnknown3;
    unsigned char nUnknown4;
    unsigned char nManufacturer;
    unsigned short wCartID;
    char nCountryID;
    unsigned char nUnknown5;
} ROMHeader;

typedef struct
{
    // Other info from the rom. This is for convenience
    unsigned char szGameName[50+1];
    char nCountryID;

    // Copy of the ROM header
    ROMHeader   romheader;

    // With changeable default values
    unsigned int dwNormalCombiner;
    unsigned int dwNormalBlender;
    unsigned int dwAccurateTextureMapping;
    unsigned int dwFastTextureCRC;
    unsigned int dwFrameBufferOption;
    unsigned int dwRenderToTextureOption;
    unsigned int dwScreenUpdateSetting;

    // With FALSE as its default values
    BOOL    bForceScreenClear;
    BOOL    bEmulateClear;
    BOOL    bForceDepthBuffer;
    BOOL    bDisableBlender;

    // Less useful options
    BOOL    bDisableObjBG;
    BOOL    bDisableTextureCRC;
    BOOL    bIncTexRectEdge;
    BOOL    bZHack;
    BOOL    bTextureScaleHack;
    BOOL    bFastLoadTile;
    BOOL    bUseSmallerTexture;
    BOOL    bPrimaryDepthHack;
    BOOL    bTexture1Hack;
    BOOL    bDisableCulling;
    int     VIWidth;
    int     VIHeight;
    unsigned int UseCIWidthAndRatio;

    unsigned int dwFullTMEM;
    BOOL    bTxtSizeMethod2;
    BOOL    bEnableTxtLOD;
    BOOL    bTexturesInitialized;
} GameSetting, *LPGAMESETTING;

typedef struct
{
    char nCountryID;
    char* szName;
    unsigned int nTvType;
} CountryIDInfo;


extern const CountryIDInfo g_CountryCodeInfo[];

extern GameSetting g_curRomInfo;

void ROM_GetRomNameFromHeader(TCHAR * szName, ROMHeader * pHdr);

#define TV_SYSTEM_NTSC      1
#define TV_SYSTEM_PAL       0
unsigned int CountryCodeToTVSystem(unsigned int countryCode);

//#define COMPLETE_N64_TMEM

enum {
    PSH_OPTIONS,
    PSH_DIRECTX,
    PSH_OPENGL,
    PSH_TEXTURE,
    PSH_DEFAULTS,
    PSH_ROM_SETTINGS,
};

void CreateOptionsDialogs(HWND hParent);
void WriteConfiguration();

#endif

