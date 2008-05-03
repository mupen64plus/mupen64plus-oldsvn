// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(_STDAFX_H_)
#define _STDAFX_H_

#include <SDL/SDL.h>

#define EXPORT              __declspec(dllexport)

#include <stdio.h>
#include <specific.h>
#include "math.h"
#include "COLOR.h"

#include <math.h>           // For sqrt()
#include <iostream>
#include <fstream>
#include <istream>

#include <vector>

#include "gl/gl.h"
#include "gl/glu.h"
#include "glext.h"
#include "glh_extension_ati.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_CHECK
# define SAFE_CHECK(a)  if( (a) == NULL ) {ErrorMsg("Creater out of memory"); throw new std::exception();}
#endif

#include "typedefs.h"
#include "gfx.h"
#include "Video.h"
#include "Config.h"
#include "Debugger.h"
#include "RSP_S2DEX.h"
#include "RSP_Parser.h"

#include "TextureManager.h"
#include "ConvertImage.h"
#include "Texture.h"

#include "CombinerDefs.h"
#include "DecodedMux.h"
#include "DirectXDecodedMux.h"

#include "blender.h"


#include "Combiner.h"
#include "GeneralCombiner.h"

#include "RenderTexture.h"
#include "FrameBuffer.h"

#include "GraphicsContext.h"
#include "DeviceBuilder.h"

#include "RenderBase.h"
#include "ExtendedRender.h"
#include "Render.h"

#include "OGLTexture.h"
#include "OGLDecodedMux.h"
#include "CNvTNTCombiner.h"

#include "OGLCombiner.h"
#include "OGLExtCombiner.h"
#include "OGLCombinerNV.h"
#include "OGLCombinerTNT2.h"
#include "OGLFragmentShaders.h"

#include "OGLRender.h"
#include "OGLExtRender.h"
#include "OGLGraphicsContext.h"

#include "IColor.h"

#include "CSortedList.h"
#include "CritSect.h"
#include "Timing.h"


extern WindowSettingStruct windowSetting;

void __cdecl MsgInfo (char* Message, ...);
void __cdecl ErrorMsg (const char* Message, ...);

#define MI_INTR_DP          0x00000020  
#define MI_INTR_SP          0x00000001  

extern uint32 g_dwRamSize;

extern uint32 * g_pRDRAMu32;
extern signed char* g_pRDRAMs8;
extern unsigned char *g_pRDRAMu8;

extern GFX_INFO g_GraphicsInfo;

extern const char *project_name;
#endif

