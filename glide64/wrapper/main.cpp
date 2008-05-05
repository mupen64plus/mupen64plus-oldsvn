#define SAVE_CBUFFER

#ifndef _WIN32
#include "../winlnxdefs.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <SDL/SDL.h>
#endif
#include "glide.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
// #include <gl/gl.h>
// #include <gl/glu.h>
#endif // _WIN32
#include "main.h"
#ifdef _WIN32
#include "wglext.h"
#endif // _WIN32

#ifdef VPDEBUG
#include <IL/il.h>
#endif

int screen_width, screen_height;

static inline void opt_glCopyTexImage2D( GLenum target,
                                         GLint level,
                                         GLenum internalFormat,
                                         GLint x,
                                         GLint y,
                                         GLsizei width,
                                         GLsizei height,
                                         GLint border )

{
    GLsizei w, h;
    GLenum  fmt;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint *) &fmt);
    //printf("copyteximage %dx%d fmt %x oldfmt %x\n", width, height, internalFormat, fmt);
    if (w == width && h == height && fmt == internalFormat) {
      if (x+width >= screen_width) {
        width = screen_width - x;
        //printf("resizing w --> %d\n", width);
      }
      if (y+height >= screen_height+viewport_offset) {
        height = screen_height+viewport_offset - y;
        //printf("resizing h --> %d\n", height);
      }
      glCopyTexSubImage2D(target, level, 0, 0, x, y, width, height);
    } else {
      printf("copyteximage %dx%d fmt %x old %dx%d oldfmt %x\n", width, height, internalFormat, w, h, fmt);
//       glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, 0);
//       glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
//       printf("--> %dx%d newfmt %x\n", width, height, fmt);
      glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
    }
}
#define glCopyTexImage2D opt_glCopyTexImage2D

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLFOGCOORDFPROC glFogCoordfEXT;
#ifdef _WIN32
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
#endif // _WIN32

PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;

PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC glUniform1iARB;
PFNGLUNIFORM4IARBPROC glUniform4iARB;
PFNGLUNIFORM4FARBPROC glUniform4fARB;
PFNGLUNIFORM1FARBPROC glUniform1fARB;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
PFNGLSECONDARYCOLOR3FPROC glSecondaryColor3f;

typedef struct
{
    unsigned int address;
    int width;
    int height;
  unsigned int fbid;
  unsigned int zbid;
  unsigned int texid;
  int buff_clear;
} fb;

int nbTextureUnits;
int nbAuxBuffers, current_buffer;
int width, widtho, heighto, height;
int saved_width, saved_height;
int blend_func_separate_support;
int npot_support;
int fog_coord_support;
int render_to_texture = 0;
int texture_unit;
int use_fbo;
int buffer_cleared;
// ZIGGY
// to allocate a new static texture name, take the value (free_texture++)
int free_texture;
int default_texture; // the infamous "32*1024*1024" is now configurable
int current_texture;
int depth_texture, color_texture;
int glsl_support = 0;
int viewport_width, viewport_height, viewport_offset = 0;
int save_w, save_h;
int lfb_color_fmt;
float invtex[2];

#ifdef _WIN32
static HDC hDC = NULL;
static HGLRC hGLRC = NULL;
static HWND hToolBar = NULL;
static HWND hwnd_win = NULL;
static unsigned long windowedExStyle, windowedStyle;
#endif // _WIN32
static unsigned long fullscreen;
#ifdef _WIN32
static RECT windowedRect;
static HMENU windowedMenu;
#endif // _WIN32

static int savedWidtho, savedHeighto;
static int savedWidth, savedHeight;
unsigned int pBufferAddress;
static int pBufferFmt;
static int pBufferWidth, pBufferHeight;
static fb fbs[100];
static int nb_fb = 0;
static unsigned int curBufferAddr = 0;

struct s_usage { unsigned int min, max; }; struct s_usage tmu_usage[2] = { {0xfffffff, 0}, {0xfffffff, 0} };

struct texbuf_t {
  DWORD start, end;
  int fmt;
};
#define NB_TEXBUFS 128 // MUST be a power of two
static texbuf_t texbufs[NB_TEXBUFS];
static int texbuf_i;

#ifndef _WIN32
static SDL_Surface *m_pScreen;
#endif // _WIN32

// unsigned short * frameBuffer = NULL;
// unsigned short * depthBuffer = NULL;
unsigned short frameBuffer[2048*2048];
unsigned short depthBuffer[2048*2048];

//#define VOODOO1

void display_warning(const char *text, ...)
{
    static int first_message = 100;
    if (first_message)
    {
        unsigned char buf[1000];
        
        va_list ap;
    
        va_start(ap, text);
        vsprintf((char*)buf, (char*)text, ap);
        va_end(ap);

// #ifdef _WIN32
//      MessageBox(NULL, (LPCTSTR)buf, "Glide3x warning : ", MB_OK);
// #else // _WIN32
       printf("Glide3x warning : %s\n", buf);
// #endif // _WIN32
        first_message--;
    }
}

#ifdef _WIN32
void display_error()
{
    LPVOID lpMsgBuf;
    if (!FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL ))
    {
    // Handle the error.
    return;
    }
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

    // Free the buffer.
    LocalFree( lpMsgBuf );
}
#endif // _WIN32

#ifdef LOGGING
FILE *log_file = NULL;

void OPEN_LOG()
{
    log_file = fopen("wrapper_log.txt", "wb+");
}

void CLOSE_LOG()
{
    if(log_file == NULL) return;
    fclose(log_file);
    log_file = NULL;
}

void LOG(char *text, ...)
{
#ifdef VPDEBUG
  if (!dumping) return;
#endif
    va_list ap;
    
    if(log_file == NULL) return;
    va_start(ap, text);
    vfprintf(log_file, text, ap);
    vfprintf(stderr, text, ap);
    va_end(ap);
}
#endif // LOGGING

FX_ENTRY void FX_CALL
grSstOrigin(GrOriginLocation_t  origin)
{
    LOG("grSstOrigin(%d)\r\n", origin);
    if (origin != GR_ORIGIN_UPPER_LEFT)
        display_warning("grSstOrigin : %x", origin);
}

FX_ENTRY void FX_CALL 
grClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy )
{
    LOG("grClipWindow(%d,%d,%d,%d)\r\n", minx, miny, maxx, maxy);

  if (use_fbo && render_to_texture) {
    glScissor(minx, miny, maxx - minx, maxy - miny);
    glEnable(GL_SCISSOR_TEST);
    return;
  }

  if (!use_fbo) {
    int th = height;
    if (!use_fbo && th > screen_height) th = screen_height;
    maxy = th - maxy;
    miny = th - miny;
    FxU32 tmp = maxy; maxy = miny; miny = tmp;
    //   if (minx < 0) minx = 0;
    //   if (miny < 0) miny = 0;
    if (maxx > (unsigned int) width) maxx = width;
    if (maxy > (unsigned int) height) maxy = height;
    glScissor(minx, miny+viewport_offset, maxx - minx, maxy - miny);
    //printf("gl scissor %d %d %d %d\n", minx, miny, maxx, maxy);
    //glScissor(minx, (viewport_offset)+height-maxy, maxx - minx, maxy - miny);
  } else {
    glScissor(minx, (viewport_offset)+height-maxy, maxx - minx, maxy - miny);
  }
    glEnable(GL_SCISSOR_TEST);
}

FX_ENTRY void FX_CALL
grColorMask( FxBool rgb, FxBool a )
{
    LOG("grColorMask(%d, %d)\r\n", rgb, a);
    glColorMask(rgb, rgb, rgb, a);
}

FX_ENTRY void FX_CALL
grGlideInit( void )
{
    OPEN_LOG();
    LOG("grGlideInit()\r\n");
}

FX_ENTRY void FX_CALL 
grSstSelect( int which_sst )
{
    LOG("grSstSelect(%d)\r\n", which_sst);
}

BOOL isExtensionSupported(const char *extension)
{
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    where = (GLubyte *)strchr(extension, ' ');
    if (where || *extension == '\0')
        return 0;

    extensions = glGetString(GL_EXTENSIONS);

    start = extensions;
    for (;;)
    {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;

        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return TRUE;

        start = terminator;
    }

    return FALSE;
}

#ifdef _WIN32
BOOL isWglExtensionSupported(const char *extension)
{
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    where = (GLubyte *)strchr(extension, ' ');
    if (where || *extension == '\0')
        return 0;

    extensions = (GLubyte*)wglGetExtensionsStringARB(wglGetCurrentDC());

    start = extensions;
    for (;;)
    {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;

        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return TRUE;

        start = terminator;
    }

    return FALSE;
}

BOOL CALLBACK FindToolBarProc(HWND hWnd, LPARAM lParam)
{
    if (GetWindowLong(hWnd, GWL_STYLE) & RBS_VARHEIGHT)
    {
        hToolBar = hWnd;
        return FALSE;
    }
    return TRUE;
}
#endif // _WIN32

#define GrPixelFormat_t int

FX_ENTRY GrContext_t FX_CALL 
grSstWinOpenExt(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          GrPixelFormat_t      pixelformat,
          int                  nColBuffers,
          int                  nAuxBuffers)
{
    LOG("grSstWinOpenExt(%d, %d, %d, %d, %d, %d %d)\r\n", hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);
    return grSstWinOpen(hWnd, screen_resolution, refresh_rate, color_format, 
                        origin_location, nColBuffers, nAuxBuffers);
}

#ifdef WIN32
# include <fcntl.h>
# ifndef ATTACH_PARENT_PROCESS
#  define ATTACH_PARENT_PROCESS ((DWORD)-1)
# endif
#endif

FX_ENTRY GrContext_t FX_CALL 
grSstWinOpen(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers)
{
  static int show_warning = 1;

//   {
//     static int inidebug;
//     if (!inidebug) {
//       inidebug = 1;
//       FILE * newstdout = freopen("wrapper-debug.txt", "w", stdout);
//       _dup2(_fileno(stdout), _fileno(stderr));
//     }
//   }

  // ZIGGY
  // allocate static texture names
  // the initial value should be big enough to support the maximal resolution
  free_texture = 32*2048*2048;
  default_texture = free_texture++;
  color_texture = free_texture++;
  depth_texture = free_texture++;
  
#ifdef _WIN32
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                       // version number
        PFD_DRAW_TO_WINDOW |     // support window
        PFD_SUPPORT_OPENGL |     // support OpenGL
        PFD_GENERIC_ACCELERATED | //PFD_SWAP_COPY | PFD_SWAP_EXCHANGE |
        PFD_DOUBLEBUFFER,        // double buffered
        PFD_TYPE_RGBA,           // RGBA type
        32,
        0, 0, 0, 0, 0, 0,        // color bits ignored
        0,                       // no alpha buffer
        0,                       // shift bit ignored
        0,                       // no accumulation buffer
        0, 0, 0, 0,              // accum bits ignored
        24,        // z-buffer      
        0,                       // no stencil buffer
        1,                       // no auxiliary buffer ZIGGY: added 1 auxiliary buffer
        PFD_MAIN_PLANE,          // main layer
        0,                       // reserved
        0, 0, 0};                // layer masks ignored
    int pfm;
    RECT windowRect, toolRect;
    int pc_width, pc_height;
#endif // _WIN32

    LOG("grSstWinOpen(%d, %d, %d, %d, %d, %d %d)\r\n", hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);

#ifdef _WIN32
    if ((HWND)hWnd == NULL) hWnd = (FxU32)GetActiveWindow();
    hwnd_win = (HWND)hWnd;
#endif // _WIN32
    switch ((screen_resolution & ~0x80)&0xFF)
    {
    case GR_RESOLUTION_320x200:
        width = 320;
        height = 200;
        break;
    case GR_RESOLUTION_320x240:
        width = 320;
        height = 240;
        break;
    case GR_RESOLUTION_400x256:
        width = 400;
        height = 256;
        break;
    case GR_RESOLUTION_512x384:
        width = 512;
        height = 384;
        break;
    case GR_RESOLUTION_640x200:
        width = 640;
        height = 200;
        break;
    case GR_RESOLUTION_640x350:
        width = 640;
        height = 350;
        break;
    case GR_RESOLUTION_640x400:
        width = 640;
        height = 400;
        break;
    case GR_RESOLUTION_640x480:
        width = 640;
        height = 480;
        break;
    case GR_RESOLUTION_800x600:
        width = 800;
        height = 600;
        break;
    case GR_RESOLUTION_960x720:
        width = 960;
        height = 720;
        break;
    case GR_RESOLUTION_856x480:
        width = 856;
        height = 480;
        break;
    case GR_RESOLUTION_512x256:
        width = 512;
        height = 256;
        break;
    case GR_RESOLUTION_1024x768:
        width = 1024;
        height = 768;
        break;
    case GR_RESOLUTION_1280x1024:
        width = 1280;
        height = 1024;
        break;
    case GR_RESOLUTION_1600x1200:
        width = 1600;
        height = 1200;
        break;
    case GR_RESOLUTION_400x300:
        width = 400;
        height = 300;
        break;
    default:
        display_warning("unknown SstWinOpen resolution : %x", screen_resolution);
    }

#ifdef _WIN32
    if (screen_resolution & 0x80)
    {
        viewport_offset = max(25, screen_resolution >> 8);
        ChangeDisplaySettings(NULL, 0);
        GetClientRect(hwnd_win, &windowRect);
        EnumChildWindows(hwnd_win, FindToolBarProc, 0);

        if (hToolBar)
            GetWindowRect(hToolBar, &toolRect);
        else
            toolRect.bottom = toolRect.top = 0;

        windowRect.right = windowRect.left + width - 1;
        windowRect.bottom = windowRect.top + height - 1 + 40;
        AdjustWindowRect(&windowRect, GetWindowLong(hwnd_win, GWL_STYLE), GetMenu(hwnd_win) != NULL);

        SetWindowPos(hwnd_win, NULL, 0, 0, windowRect.right - windowRect.left + 1,
                    windowRect.bottom - windowRect.top + 1 /*+ toolRect.bottom - toolRect.top + 1*/, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    fullscreen = 0;
    }
    else
    {
        DEVMODE fullscreenMode;
        DEVMODE currentMode;
        
        viewport_offset = 0;
        pc_width = getFullScreenWidth();
        pc_height = getFullScreenHeight();
        if (pc_width == 0 || pc_height == 0)
        {
            pc_width = width;
            pc_height = height;
        }

        memset(&fullscreenMode, 0, sizeof(DEVMODE));
        fullscreenMode.dmSize = sizeof(DEVMODE);
        fullscreenMode.dmPelsWidth= pc_width;
        fullscreenMode.dmPelsHeight= pc_height;
        fullscreenMode.dmBitsPerPel= 32;
        fullscreenMode.dmDisplayFrequency= 60;
        fullscreenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &currentMode);
        fullscreenMode.dmDisplayFrequency = currentMode.dmDisplayFrequency;

        if (ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL)
        {
            display_warning("can't change to fullscreen mode");
        }
        ShowCursor(FALSE);

        windowedExStyle = GetWindowLong(hwnd_win, GWL_EXSTYLE);
        windowedStyle = GetWindowLong(hwnd_win, GWL_STYLE);

        SetWindowLong(hwnd_win, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
        SetWindowLong(hwnd_win, GWL_STYLE, WS_POPUP);

        GetWindowRect(hwnd_win, &windowedRect);
        windowedMenu = GetMenu(hwnd_win);
        if (windowedMenu) SetMenu(hwnd_win, NULL);

        fullscreen = 1;

        SetWindowPos(hwnd_win, NULL, 0, 0, pc_width, pc_height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    
    if ((hDC = GetDC(hwnd_win)) == NULL)
    {
        display_warning("GetDC on main window failed");
        return FXFALSE;
    }
    SetViewportExtEx(hDC, width, height, NULL);
    SetWindowExtEx(hDC, width, height, NULL);

    if ((pfm = ChoosePixelFormat(hDC, &pfd)) == 0) {
    printf("disabling auxiliary buffers\n");
    pfd.cAuxBuffers = 0;
    pfm = ChoosePixelFormat(hDC, &pfd);
  }
    if (pfm == 0)
    {
        display_warning("ChoosePixelFormat failed");
        return FXFALSE;
    }
    if (SetPixelFormat(hDC, pfm, &pfd) == FALSE)
    {
        display_warning("SetPixelFormat failed");
        return FXFALSE;
    }

    DescribePixelFormat(hDC, pfm, sizeof(pfd), &pfd);
    
    if ((hGLRC = wglCreateContext(hDC)) == 0)
    {
        display_warning("wglCreateContext failed!");
        grSstWinClose(0);
        return FXFALSE;
    }

    if (!wglMakeCurrent(hDC, hGLRC))
    {
        display_warning("wglMakeCurrent failed!");
        grSstWinClose(0);
        return FXFALSE;
    }
#else // _WIN32
   // init sdl & gl
   const SDL_VideoInfo *videoInfo;
   Uint32 videoFlags = 0;
   fullscreen = 0;
   
   /* Initialize SDL */
   printf("(II) Initializing SDL video subsystem...\n");
   if(SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
   {
     printf("(EE) Error initializing SDL video subsystem: %s\n", SDL_GetError());
     return false;
   }
   
   /* Video Info */
   printf("(II) Getting video info...\n");
   if(!(videoInfo = SDL_GetVideoInfo()))
   {
     printf("(EE) Video query failed: %s\n", SDL_GetError());
     SDL_QuitSubSystem(SDL_INIT_VIDEO);
     return false;
   }
   
   /* Setting the video mode */
   videoFlags |= SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;
   
   if(videoInfo->hw_available)
     videoFlags |= SDL_HWSURFACE;
   else
     videoFlags |= SDL_SWSURFACE;
   
   if(videoInfo->blit_hw)
     videoFlags |= SDL_HWACCEL;
   
    if(screen_resolution & 0x80)
      ;
    else
    {
        viewport_offset = 0;
        videoFlags |= SDL_FULLSCREEN;
    }
    viewport_offset = ((screen_resolution>>2) > 20) ? screen_resolution >> 2 : 20;

  // ZIGGY viewport_offset is WIN32 specific, with SDL just set it to zero
    viewport_offset = 0; //-10 //-20;

  // ZIGGY not sure, but it might be better to let the system choose
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16);
//   SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
//   SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
//   SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
//   SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
//   SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
   
   printf("(II) Setting video mode %dx%d...\n", width, height);
   if(!(m_pScreen = SDL_SetVideoMode(width, height, 0, videoFlags)))
     {
    printf("(EE) Error setting videomode %dx%d: %s\n", width, height, SDL_GetError());
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return false;
     }
   
   char caption[500];
# ifdef _DEBUG
   sprintf(caption, "Glide64 Debug");
# else // _DEBUG
   sprintf(caption, "Glide64");
# endif // _DEBUG
   SDL_WM_SetCaption(caption, caption);
   glViewport(0, viewport_offset, width, height);
#endif // _WIN32
    
    //if (color_format !=   GR_COLORFORMAT_ARGB) display_warning("color format is not ARGB");
    lfb_color_fmt = color_format;
    if (origin_location != GR_ORIGIN_UPPER_LEFT) display_warning("origin must be in upper left corner");
    if (nColBuffers != 2) display_warning("number of color buffer is not 2");
    if (nAuxBuffers != 1) display_warning("number of auxiliary buffer is not 1");

    if (isExtensionSupported("GL_ARB_texture_env_combine") == FALSE &&
        isExtensionSupported("GL_EXT_texture_env_combine") == FALSE &&
    show_warning)
        display_warning("Your video card doesn't support GL_ARB_texture_env_combine extension");
    if (isExtensionSupported("GL_ARB_multitexture") == FALSE && show_warning)
        display_warning("Your video card doesn't support GL_ARB_multitexture extension");
    if (isExtensionSupported("GL_ARB_texture_mirrored_repeat") == FALSE && show_warning)
        display_warning("Your video card doesn't support GL_ARB_texture_mirrored_repeat extension");
  show_warning = 0;

#ifdef _WIN32
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
#else // _WIN32
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
    glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
#endif // _WIN32
  
  nbTextureUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nbTextureUnits);
    if (nbTextureUnits == 1) display_warning("You need a video card that has at least 2 texture units");
  
  nbAuxBuffers = 0;
  int getDisableAuxbuf();
  if (!getDisableAuxbuf())
    glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &nbAuxBuffers);
  if (nbAuxBuffers > 0)
    printf("Congratulations, you have %d auxilliary buffers, we'll use them wisely !\n", nbAuxBuffers);

#ifdef VOODOO1
    nbTextureUnits = 2;
#endif

    if (isExtensionSupported("GL_EXT_blend_func_separate") == FALSE)
        blend_func_separate_support = 0;
    else
        blend_func_separate_support = 1;

    if (isExtensionSupported("GL_EXT_packed_pixels") == FALSE)
        packed_pixels_support = 0;
    else {
      printf("packed pixels extension used\n");
        packed_pixels_support = 1;
    }

    if (isExtensionSupported("GL_ARB_texture_non_power_of_two") == FALSE)
        npot_support = 0;
    else {
      printf("NPOT extension used\n");
        npot_support = 1;
    }

#ifdef _WIN32
    glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)wglGetProcAddress("glBlendFuncSeparateEXT");
#else // _WIN32
    glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)SDL_GL_GetProcAddress("glBlendFuncSeparateEXT");
#endif // _WIN32

    if (isExtensionSupported("GL_EXT_fog_coord") == FALSE)
        fog_coord_support = 0;
    else
        fog_coord_support = 1;

#ifdef _WIN32
    glFogCoordfEXT = (PFNGLFOGCOORDFPROC)wglGetProcAddress("glFogCoordfEXT");
#else // _WIN32
    glFogCoordfEXT = (PFNGLFOGCOORDFPROC)SDL_GL_GetProcAddress("glFogCoordfEXT");
#endif // _WIN32

#ifdef _WIN32
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
#endif // _WIN32

#ifdef _WIN32
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");

  glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
  glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
#else // _WIN32
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");

  glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
  glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
#endif // _WIN32

  int getEnableFBO();
  use_fbo = getEnableFBO() && glFramebufferRenderbufferEXT;

  printf("use_fbo %d\n", use_fbo);

    if (isExtensionSupported("GL_ARB_shading_language_100") &&
        isExtensionSupported("GL_ARB_shader_objects") &&
        isExtensionSupported("GL_ARB_fragment_shader") &&
        isExtensionSupported("GL_ARB_vertex_shader") && !getDisableGLSL())
    {
        glsl_support = 1;

#ifdef _WIN32
        glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
        glUniform4iARB = (PFNGLUNIFORM4IARBPROC)wglGetProcAddress("glUniform4iARB");
        glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
        glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
        glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");

        glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)wglGetProcAddress("glSecondaryColor3f");
#else // _WIN32
        glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
        glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
        glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
        glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
        glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
        glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
        glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
        glUniform1iARB = (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
        glUniform4iARB = (PFNGLUNIFORM4IARBPROC)SDL_GL_GetProcAddress("glUniform4iARB");
        glUniform4fARB = (PFNGLUNIFORM4FARBPROC)SDL_GL_GetProcAddress("glUniform4fARB");
        glUniform1fARB = (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");
        glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)SDL_GL_GetProcAddress("glDeleteObjectARB");
        glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");

        glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)SDL_GL_GetProcAddress("glSecondaryColor3f");
#endif // _WIN32
    }
    else
        glsl_support = 0;

#ifdef _WIN32
    if (screen_resolution & 0x80)
    {
        glViewport(0, viewport_offset, width, height);
        viewport_width = width;
        viewport_height = height;
    }
    else
    {
        glViewport(0, 0, pc_width, pc_height);
        viewport_width = pc_width;
        viewport_height = pc_height;
    viewport_offset = 0;
    }
#else
    glViewport(0, viewport_offset, width, height);
    viewport_width = width;
    viewport_height = height;
#endif // _WIN32

//   void do_benchmarks();
//   do_benchmarks();
    
    // VP try to resolve z precision issues
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, 1-zscale);
    glScalef(1, 1, zscale);

//  glAlphaFunc(GL_GREATER, 0.5);
//   glEnable(GL_ALPHA_TEST);

    widtho = width/2;
  heighto = height/2;

    pBufferWidth = pBufferHeight = -1;

  current_buffer = GL_BACK;

  if(!glsl_support)
  {
    switch(nbTextureUnits)
    {
            case 2:
                texture_unit = GL_TEXTURE1_ARB;
                break;
            case 3:
                texture_unit = GL_TEXTURE2_ARB;
                break;
            default:
                texture_unit = GL_TEXTURE3_ARB;
    }
  }
  else texture_unit = GL_TEXTURE0_ARB;

//   frameBuffer = (unsigned short *) calloc(2048, 2048*sizeof(unsigned short));
//   depthBuffer = (unsigned short *) calloc(2048, 2048*sizeof(unsigned short));

  screen_width = width;
  screen_height = height;

  {
    int i;
    for (i=0; i<NB_TEXBUFS; i++)
      texbufs[i].start = texbufs[i].end = 0xffffffff;
  }

  if (!use_fbo && nbAuxBuffers == 0) {
    // create the framebuffer saving texture
    int w = width, h = height;
    glBindTexture(GL_TEXTURE_2D, color_texture);
    if (!npot_support) {
      w = h = 1;
      while (w<width) w*=2;
      while (h<height) h*=2;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    save_w = save_h = 0;
  }

  void FindBestDepthBias();
  FindBestDepthBias();

    init_geometry();
    init_textures();
    init_combiner();
  
    return 1;
}

FX_ENTRY void FX_CALL
grGlideShutdown( void )
{
    LOG("grGlideShutdown\r\n");
    CLOSE_LOG();
}

FX_ENTRY FxBool FX_CALL
grSstWinClose( GrContext_t context )
{
  int i, clear_texbuff = use_fbo;
    LOG("grSstWinClose(%d)\r\n", context);

//   void remove_all_tex();
//   remove_all_tex();

  for (i=0; i<2; i++) {
    tmu_usage[i].min = 0xfffffff;
    tmu_usage[i].max = 0;
    invtex[i] = 0;
  }

//   if (frameBuffer)
//     free(frameBuffer);
//   if (depthBuffer)
//     free(depthBuffer);
//   frameBuffer = depthBuffer = NULL;
  
    free_combiners();
#ifndef WIN32
#ifndef GCC
  __try // I don't know why, but opengl can be killed before this function call when emulator is closed (Gonetz).
    // ZIGGY : I found the problem : it is a function pointer, when the extension isn't supported , it is then zero, so just need to check the pointer prior to do the call.
#endif
  {
    if (use_fbo && glBindFramebufferEXT)
      glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  }
#ifndef GCC
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    clear_texbuff = 0;
  }
#endif

  if (clear_texbuff)
  {
    for (i=0; i<nb_fb; i++)
    {
      glDeleteTextures( 1, &(fbs[i].texid) );
      glDeleteFramebuffersEXT( 1, &(fbs[i].fbid) );
      glDeleteRenderbuffersEXT( 1, &(fbs[i].zbid) );
    }
  }
#endif
  nb_fb = 0;
  
    //free_textures();
#ifndef WIN32
  // ZIGGY for some reasons, Pj64 doesn't like remove_tex on exit
  remove_tex(0, 0xfffffff);
#endif

  //*/
#ifdef _WIN32
    if (hGLRC)
    {
        wglMakeCurrent(NULL,NULL);
        wglDeleteContext(hGLRC);
        hGLRC = NULL;
    }
  /*
  if (hDC != NULL) 
  {
      ReleaseDC(hwnd_win,hDC);
    hDC = NULL;
  }
    //*/
    if (fullscreen)
    {
        ShowCursor(TRUE);
        ChangeDisplaySettings(NULL, 0);
        SetWindowLong(hwnd_win, GWL_STYLE, windowedStyle);
        SetWindowLong(hwnd_win, GWL_EXSTYLE, windowedExStyle);
        SetWindowPos(hwnd_win, NULL, windowedRect.left, windowedRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        if (windowedMenu) SetMenu(hwnd_win, windowedMenu);
        fullscreen = 0;
    }
#else
   //SDL_QuitSubSystem(SDL_INIT_VIDEO);
   //sleep(2);
   m_pScreen = NULL;
#endif
    return FXTRUE;
}

FX_ENTRY void FX_CALL grTextureBufferExt( GrChipID_t        tmu, 
                                          FxU32                 startAddress, 
                                          GrLOD_t           lodmin, 
                                          GrLOD_t           lodmax, 
                                          GrAspectRatio_t   aspect, 
                                          GrTextureFormat_t     fmt, 
                                          FxU32                 evenOdd)
{
    int i;
    static int fbs_init = 0;
  
    //printf("grTextureBufferExt(%d, %d, %d, %d, %d, %d, %d)\r\n", tmu, startAddress, lodmin, lodmax, aspect, fmt, evenOdd);
    LOG("grTextureBufferExt(%d, %d, %d, %d %d, %d, %d)\r\n", tmu, startAddress, lodmin, lodmax, aspect, fmt, evenOdd);
    if (lodmin != lodmax) display_warning("grTextureBufferExt : loading more than one LOD");
  if (!use_fbo) {

    if (!render_to_texture) { //initialization
      return;
    }

    render_to_texture = 2;
      
    if (aspect < 0)
    {
      pBufferHeight = 1 << lodmin;
      pBufferWidth = pBufferHeight >> -aspect;
    }
    else
    {
      pBufferWidth = 1 << lodmin;
      pBufferHeight = pBufferWidth >> aspect;
    }

    if (curBufferAddr && startAddress+1 != curBufferAddr)
      updateTexture();
#ifdef SAVE_CBUFFER
    //printf("saving %dx%d\n", pBufferWidth, pBufferHeight);
    // save color buffer
    if (nbAuxBuffers > 0) {
      glDrawBuffer(GL_AUX0);
      current_buffer = GL_AUX0;
    } else {
      int tw, th;
      if (pBufferWidth < screen_width)
        tw = pBufferWidth;
      else
        tw = screen_width;
      if (pBufferHeight < screen_height)
        th = pBufferHeight;
      else
        th = screen_height;
      glReadBuffer(GL_BACK);
      glActiveTextureARB(texture_unit);
      glBindTexture(GL_TEXTURE_2D, color_texture);
      // save incrementally the framebuffer
      if (save_w) {
        if (tw > save_w && th > save_h) {
          glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, save_h,
                              0, viewport_offset+save_h, tw, th-save_h);
          glCopyTexSubImage2D(GL_TEXTURE_2D, 0, save_w, 0,
                              save_w, viewport_offset, tw-save_w, save_h);
          save_w = tw;
          save_h = th;
        } else if (tw > save_w) {
          glCopyTexSubImage2D(GL_TEXTURE_2D, 0, save_w, 0,
                              save_w, viewport_offset, tw-save_w, save_h);
          save_w = tw;
        } else if (th > save_h) {
          glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, save_h,
                              0, viewport_offset+save_h, save_w, th-save_h);
          save_h = th;
        }
      } else {
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                            0, viewport_offset, tw, th);
        save_w = tw;
        save_h = th;
      }
      glBindTexture(GL_TEXTURE_2D, default_texture);
    }
#endif
    
    if (startAddress+1 != curBufferAddr ||
        (curBufferAddr == 0L && nbAuxBuffers == 0))
      buffer_cleared = FALSE;
      
    curBufferAddr = pBufferAddress = startAddress+1;
    pBufferFmt = fmt;
    
    int rtmu = startAddress < grTexMinAddress(GR_TMU1)? 0 : 1;
    int size = pBufferWidth*pBufferHeight*2; //grTexFormatSize(fmt);
    if (tmu_usage[rtmu].min > pBufferAddress)
      tmu_usage[rtmu].min = pBufferAddress;
    if (tmu_usage[rtmu].max < pBufferAddress+size)
      tmu_usage[rtmu].max = pBufferAddress+size;
    //   printf("tmu %d usage now %gMb - %gMb\n",
    //          rtmu, tmu_usage[rtmu].min/1024.0f, tmu_usage[rtmu].max/1024.0f);
    
    
    width = pBufferWidth;
    height = pBufferHeight;
    
    widtho = width/2;
    heighto = height/2;

    // this could be improved, but might be enough as long as the set of
    // texture buffer addresses stay small
    for (i=(texbuf_i-1)&(NB_TEXBUFS-1) ; i!=texbuf_i; i=(i-1)&(NB_TEXBUFS-1))
      if (texbufs[i].start == pBufferAddress)
        break;
    texbufs[i].start = pBufferAddress;
    texbufs[i].end = pBufferAddress + size;
    texbufs[i].fmt = fmt;
    if (i == texbuf_i)
      texbuf_i = (texbuf_i+1)&(NB_TEXBUFS-1);
    //printf("texbuf %x fmt %x\n", pBufferAddress, fmt);
    
    // ZIGGY it speeds things up to not delete the buffers
    // a better thing would be to delete them *sometimes*
    //   remove_tex(pBufferAddress+1, pBufferAddress + size);
    add_tex(pBufferAddress);
    
    //printf("viewport %dx%d\n", width, height);
    if (height > screen_height) {
      glViewport( 0, viewport_offset + screen_height - height, width, height);
    } else
      glViewport( 0, viewport_offset, width, height);
    
    glScissor(0, viewport_offset, width, height);

//   glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
//   glClear( GL_COLOR_BUFFER_BIT );
//   glClear( GL_DEPTH_BUFFER_BIT );
  

  } else {
    if (!render_to_texture) //initialization
    {
      if(!fbs_init)
      {
        for(i=0; i<100; i++) fbs[i].address = 0;
        fbs_init = 1;
        nb_fb = 0;
      }
      return; //no need to allocate FBO if render buffer is not texture buffer
    }
    
    render_to_texture = 2;
      
    if (aspect < 0)
    {
      pBufferHeight = 1 << lodmin;
      pBufferWidth = pBufferHeight >> -aspect;
    }
    else
    {
      pBufferWidth = 1 << lodmin;
      pBufferHeight = pBufferWidth >> aspect;
    }
    pBufferAddress = startAddress+1;
    
    width = pBufferWidth;
    height = pBufferHeight;
    
    widtho = width/2;
    heighto = height/2;
    
    //glScissor(0, 0, width, height);
    //glEnable(GL_SCISSOR_TEST);

    for (i=0; i<nb_fb; i++)
    {
      if (fbs[i].address == pBufferAddress)
      {
        if (fbs[i].width == width && fbs[i].height == height) //select already allocated FBO
        {
          glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
          glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbs[i].fbid );
          glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbs[i].texid, 0 );
          glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, fbs[i].zbid );
          glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbs[i].zbid );
          glViewport( 0, 0, width, height);
          glScissor( 0, 0, width, height);
          if (fbs[i].buff_clear)
          {
            glDepthMask(TRUE);
            glClear( GL_DEPTH_BUFFER_BIT ); //clear z-buffer only. we may need content, stored in the frame buffer
            fbs[i].buff_clear = 0;
          }
          CHECK_FRAMEBUFFER_STATUS();
          curBufferAddr = pBufferAddress;
          return;
        }
        else //create new FBO at the same address, delete old one
        {
          glDeleteFramebuffersEXT( 1, &(fbs[i].fbid) );
          glDeleteRenderbuffersEXT( 1, &(fbs[i].zbid) );
          if (nb_fb > 1)
            memmove(&(fbs[i]), &(fbs[i+1]), sizeof(fb)*(nb_fb-i));
          nb_fb--;
          break;
        }
      }
    }
    
    remove_tex(pBufferAddress, pBufferAddress + width*height*2/*grTexFormatSize(fmt)*/);
    //create new FBO 
    glGenFramebuffersEXT( 1, &(fbs[nb_fb].fbid) );
    glGenRenderbuffersEXT( 1, &(fbs[nb_fb].zbid) );
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, fbs[nb_fb].zbid );
    // VP ported from mudlord
    glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
    //glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
    fbs[nb_fb].address = pBufferAddress;  
    fbs[nb_fb].width = width;  
    fbs[nb_fb].height = height;  
    fbs[nb_fb].texid = pBufferAddress;  
    fbs[nb_fb].buff_clear = 0;
    add_tex(fbs[nb_fb].texid);
    glBindTexture(GL_TEXTURE_2D, fbs[nb_fb].texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbs[nb_fb].fbid);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbs[nb_fb].texid, 0);
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbs[nb_fb].zbid );
    glViewport(0,0,width,height);
    glScissor(0,0,width,height);
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glDepthMask(TRUE);
    glClear( GL_DEPTH_BUFFER_BIT );
    CHECK_FRAMEBUFFER_STATUS();
    curBufferAddr = pBufferAddress;
    nb_fb++;
  }
}

int CheckTextureBufferFormat(GrChipID_t tmu, FxU32 startAddress, GrTexInfo *info )
{
  int found, i;
  if (!use_fbo) {
    for (found=i=0; i<2; i++)
      if (tmu_usage[i].min <= startAddress && tmu_usage[i].max > startAddress) {
        //printf("tmu %d == framebuffer %x\n", tmu, startAddress);
        found = 1;
        break;
      }

//     if (found && info->format == GR_TEXFMT_ALPHA_INTENSITY_88) {
//       // now check the original buffer format
//       // if it was 565, then we are dealing with a b&w conversion hack
//       // so use special shader for it
//       for (i=(texbuf_i-1)&(NB_TEXBUFS-1); i!=texbuf_i; i = (i-1)&(NB_TEXBUFS-1))
//         if (texbufs[i].start == startAddress) {
//           if (texbufs[i].fmt != GR_TEXFMT_ALPHA_INTENSITY_88)
//             found = 2;
//           if (found == 2)
//             printf("texbuf %x fmt now %x\n", startAddress, info->format);
//           break;
//         }
//       if (i == texbuf_i)
//         display_warning("Couldn't find texbuf %x !\n", startAddress);
//     }
  } else {
    found = i = 0;
    while (i < nb_fb)
    {
      unsigned int end = fbs[i].address + fbs[i].width*fbs[i].height*2;
      if (startAddress >= fbs[i].address &&  startAddress < end)
      {
        found = 1;
        break;
      }
      i++;
    }
  }
    
  if (!use_fbo && found) {
    int tw, th, rh, cw, ch;
    if (info->aspectRatioLog2 < 0)
    {
      th = 1 << info->largeLodLog2;
      tw = th >> -info->aspectRatioLog2;
    }
    else
    {
      tw = 1 << info->largeLodLog2;
      th = tw >> info->aspectRatioLog2;
    }
    
    if (info->aspectRatioLog2 < 0)
    {
      ch = 256;
      cw = ch >> -info->aspectRatioLog2;
    }
    else
    {
      cw = 256;
      ch = cw >> info->aspectRatioLog2;
    }
    
    if (use_fbo || th < screen_height)
      rh = th;
    else
      rh = screen_height;
    
    //printf("th %d rh %d ch %d\n", th, rh, ch);
    
    invtex[tmu] = 1.0f - (th - rh) / (float)th;
  } else
    invtex[tmu] = 0;
    
  if (info->format == GR_TEXFMT_ALPHA_INTENSITY_88 ) {
    if (!found) {
      return 0;
    }
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, fbs[i].width, fbs[i].height, 0, GL_LUMINANCE8_ALPHA8, GL_UNSIGNED_BYTE, NULL);
    if(tmu == 0)
    {
      if(blackandwhite1 != found)
      {
        blackandwhite1 = found;
        need_to_compile = 1;
      }
    }
    else
    {
      if(blackandwhite0 != found)
      {
        blackandwhite0 = found;
        need_to_compile = 1;
      }
    }
    return 1;
  }
  return 0;
  
}


FX_ENTRY void FX_CALL 
grTextureAuxBufferExt( GrChipID_t tmu, 
                                        FxU32      startAddress,
                                        GrLOD_t    thisLOD,
                                        GrLOD_t    largeLOD,
                                        GrAspectRatio_t aspectRatio,
                                        GrTextureFormat_t format,
                                        FxU32      odd_even_mask )
{
    LOG("grTextureAuxBufferExt(%d, %d, %d, %d %d, %d, %d)\r\n", tmu, startAddress, thisLOD, largeLOD, aspectRatio, format, odd_even_mask);
    //display_warning("grTextureAuxBufferExt");
}

FX_ENTRY void FX_CALL grAuxBufferExt( GrBuffer_t buffer );

FX_ENTRY GrProc FX_CALL
grGetProcAddress( const char *procName )
{
    LOG("grGetProcAddress(%s)\r\n", procName);
    if(!strcmp(procName, "grSstWinOpenExt"))
        return (GrProc)grSstWinOpenExt;
    if(!strcmp(procName, "grTextureBufferExt"))
        return (GrProc)grTextureBufferExt;
    if(!strcmp(procName, "grChromaRangeExt"))
        return (GrProc)grChromaRangeExt;
    if(!strcmp(procName, "grChromaRangeModeExt"))
        return (GrProc)grChromaRangeModeExt;
    if(!strcmp(procName, "grTexChromaRangeExt"))
        return (GrProc)grTexChromaRangeExt;
    if(!strcmp(procName, "grTexChromaModeExt"))
        return (GrProc)grTexChromaModeExt;
    if(!strcmp(procName, "grConfigWrapperExt"))
        return (GrProc)grConfigWrapperExt;
  // ZIGGY framebuffer copy extension
    if(/*glsl_support && */!strcmp(procName, "grFramebufferCopyExt"))
        return (GrProc)grFramebufferCopyExt;
    if(!strcmp(procName, "grWrapperFullScreenResolutionExt"))
        return (GrProc)grWrapperFullScreenResolutionExt;
    if(!strcmp(procName, "grColorCombineExt"))
        return (GrProc)grColorCombineExt;
    if(!strcmp(procName, "grAlphaCombineExt"))
        return (GrProc)grAlphaCombineExt;
    if(!strcmp(procName, "grTexColorCombineExt"))
        return (GrProc)grTexColorCombineExt;
    if(!strcmp(procName, "grTexAlphaCombineExt"))
        return (GrProc)grTexAlphaCombineExt;
    if(!strcmp(procName, "grConstantColorValueExt"))
        return (GrProc)grConstantColorValueExt;
    if(!strcmp(procName, "grTextureAuxBufferExt"))
        return (GrProc)grTextureAuxBufferExt;
    if(!strcmp(procName, "grAuxBufferExt"))
        return (GrProc)grAuxBufferExt;
    display_warning("grGetProcAddress : %s", procName);
    return 0;
}

FX_ENTRY FxU32 FX_CALL 
grGet( FxU32 pname, FxU32 plength, FxI32 *params )
{
    LOG("grGet(%d,%d)\r\n", pname, plength);
    switch(pname)
    {
    case GR_MAX_TEXTURE_SIZE:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 2048;
        return 4;
        break;
    case GR_NUM_TMU:
        if (plength < 4 || params == NULL) return 0;
        if (!nbTextureUnits)
        {
            grSstWinOpen((unsigned long)NULL, GR_RESOLUTION_640x480 | 0x80, 0, GR_COLORFORMAT_ARGB,
                GR_ORIGIN_UPPER_LEFT, 2, 1);
            grSstWinClose(0);
        }
#ifdef VOODOO1
        params[0] = 1;
#else
        if (nbTextureUnits > 2)
            params[0] = 2;
        else
            params[0] = 1;
#endif
        return 4;
        break;
    case GR_NUM_BOARDS:
    case GR_NUM_FB:
    case GR_REVISION_FB:
    case GR_REVISION_TMU:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 1;
        return 4;
        break;
    case GR_MEMORY_FB:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 16*1024*1024;
        return 4;
        break;
    case GR_MEMORY_TMU:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 16*1024*1024;
        return 4;
        break;
    case GR_MEMORY_UMA:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 16*1024*1024*nbTextureUnits;
        return 4;
        break;
    case GR_BITS_RGBA:
        if (plength < 16 || params == NULL) return 0;
        params[0] = 8;
        params[1] = 8;
        params[2] = 8;
        params[3] = 8;
        return 16;
        break;
    case GR_BITS_DEPTH:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 16;
        return 4;
        break;
    case GR_BITS_GAMMA:
    case GR_GAMMA_TABLE_ENTRIES:
        return 0;
        break;
    case GR_FOG_TABLE_ENTRIES:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 64;
        return 4;
        break;
    case GR_WDEPTH_MIN_MAX:
        if (plength < 8 || params == NULL) return 0;
        params[0] = 0;
        params[1] = 65528;
        return 8;
        break;
    case GR_ZDEPTH_MIN_MAX:
        if (plength < 8 || params == NULL) return 0;
        params[0] = 0;
        params[1] = 65535;
        return 8;
        break;
    case GR_LFB_PIXEL_PIPE:
        if (plength < 4 || params == NULL) return 0;
        params[0] = FXFALSE;
        return 4;
        break;
    case GR_MAX_TEXTURE_ASPECT_RATIO:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 3;
        return 4;
        break;
    case GR_NON_POWER_OF_TWO_TEXTURES:
        if (plength < 4 || params == NULL) return 0;
        params[0] = FXFALSE;
        return 4;
        break;
    case GR_TEXTURE_ALIGN:
        if (plength < 4 || params == NULL) return 0;
        params[0] = 0;
        return 4;
        break;
    default:
        display_warning("unknown pname in grGet : %x", pname);
    }
    return 0;
}

FX_ENTRY const char * FX_CALL 
grGetString( FxU32 pname )
{
    LOG("grGetString(%d)\r\n", pname);
    switch(pname)
    {
    case GR_EXTENSION:
        {
            static int glsl_combiner = -1;
            static char extension1[] = "CHROMARANGE TEXCHROMA TEXMIRROR PALETTE6666 FOGCOORD EVOODOO TEXTUREBUFFER TEXFMT COMBINE";
            static char extension2[] = "CHROMARANGE TEXCHROMA TEXMIRROR PALETTE6666 FOGCOORD EVOODOO TEXTUREBUFFER TEXFMT";
            if(glsl_combiner == -1)
            {
/* JOSH FIXME: hack to avoid implementing CreateGLWindow and KillGLWindow
 * Rather than calling glGetString to check for the appropriate extensions,
 * just let getDisableGLSL() decide. */
#ifdef _WIN32
                int openglinit = (hGLRC == NULL);
        //if (glGetString(GL_EXTENSIONS) == NULL) openglinit = 1;
                if(openglinit)
                {
          printf("Creating gl window\n");
                    CreateGLWindow("Opengl window", 640, 480);
                }
                if (isExtensionSupported("GL_ARB_shading_language_100") &&
                    isExtensionSupported("GL_ARB_shader_objects") &&
                    isExtensionSupported("GL_ARB_fragment_shader") &&
                    isExtensionSupported("GL_ARB_vertex_shader") && !getDisableGLSL())
                {
                    glsl_combiner = 1;
                }
                else
                {
                    glsl_combiner = 0;
                }
        printf("glsl_combiner %d\n", glsl_combiner);
                if(openglinit)
                {
                    KillGLWindow();
                }
#else // _WIN32
        glsl_combiner = 1; /* Just use the disable flag */
#endif // _WIN32
            }
            if(glsl_combiner == 1 && !getDisableGLSL())
                return extension1;
            else
                return extension2;
        }
        break;
    case GR_HARDWARE:
        {
            static char hardware[] = "Voodoo5 (tm)";
            return hardware;
        }
        break;
    case GR_VENDOR:
        {
            static char vendor[] = "3Dfx Interactive";
            return vendor;
        }
        break;
    case GR_RENDERER:
        {
            static char renderer[] = "Glide";
            return renderer;
        }
        break;
    case GR_VERSION:
        {
            static char version[] = "3.0";
            return version;
        }
        break;
    default:
        display_warning("unknown grGetString selector : %x", pname);
    }
    return NULL;
}

static void render_rectangle(int texture_number,
                             int dst_x, int dst_y,
                             int src_width, int src_height,
                             int tex_width, int tex_height, int invert)
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBegin(GL_QUADS);
  glMultiTexCoord2fARB(texture_number, 0.0f, 0.0f);
  glVertex2f(((int)dst_x - widtho) / (float)(width/2),
             invert*-((int)dst_y - heighto) / (float)(height/2));
  glMultiTexCoord2fARB(texture_number, 0.0f, (float)src_height / (float)tex_height);
  glVertex2f(((int)dst_x - widtho) / (float)(width/2),
             invert*-((int)dst_y + (int)src_height - heighto) / (float)(height/2));
  glMultiTexCoord2fARB(texture_number, (float)src_width / (float)tex_width, (float)src_height / (float)tex_height);
  glVertex2f(((int)dst_x + (int)src_width - widtho) / (float)(width/2),
             invert*-((int)dst_y + (int)src_height - heighto) / (float)(height/2));
  glMultiTexCoord2fARB(texture_number, (float)src_width / (float)tex_width, 0.0f);
  glVertex2f(((int)dst_x + (int)src_width - widtho) / (float)(width/2),
             invert*-((int)dst_y - heighto) / (float)(height/2));
  glMultiTexCoord2fARB(texture_number, 0.0f, 0.0f);
  glVertex2f(((int)dst_x - widtho) / (float)(width/2),
             invert*-((int)dst_y - heighto) / (float)(height/2));
  glEnd();

  if(!glsl_support)
  {
    switch(nbTextureUnits)
    {
            case 2:
                updateCombiner(1);
                updateCombiner(1);
                break;
            case 3:
                updateCombiner(2);
                updateCombiner(2);
                break;
            default:
                updateCombiner(3);
                updateCombiner(3);
    }
  }
  else compile_shader();
    
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
}

void reloadTexture()
{
  if (use_fbo || !render_to_texture || buffer_cleared)
    return;

  LOG("reload texture %dx%d\n", width, height);
  printf("reload texture %dx%d\n", width, height);
  
  buffer_cleared = TRUE;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glActiveTextureARB(texture_unit);
  glBindTexture(GL_TEXTURE_2D, pBufferAddress);
  glDisable(GL_ALPHA_TEST);
  glDrawBuffer(current_buffer);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  set_copy_shader();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  int w = 0, h = 0;
  //if (width > screen_width) w = screen_width - width;
  if (height > screen_height) h = screen_height - height;
  render_rectangle(texture_unit,
                   -w, -h,
                   width,  height,
                   width, height, -1);
  glBindTexture(GL_TEXTURE_2D, default_texture);
  glPopAttrib();
}

void updateTexture()
{
  if (!use_fbo && render_to_texture == 2) {
    LOG("update texture %x\n", pBufferAddress);
    //printf("update texture %x\n", pBufferAddress);

    // nothing changed, don't update the texture
    if (!buffer_cleared) {
      LOG("update cancelled\n", pBufferAddress);
      return;
    }
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // save result of render to texture into actual texture
    glReadBuffer(current_buffer);
    glActiveTextureARB(texture_unit);
    // ZIGGY
    // deleting the texture before resampling it increases speed on certain old
    // nvidia cards (geforce 2 for example), unfortunatly it slows down a lot
    // on newer cards.
    //glDeleteTextures( 1, &pBufferAddress );
    glBindTexture(GL_TEXTURE_2D, pBufferAddress);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, (!glsl_support && pBufferFmt == GR_TEXFMT_ALPHA_INTENSITY_88)? GL_INTENSITY : GL_RGB,
                     0, viewport_offset, width, height, 0);

    glBindTexture(GL_TEXTURE_2D, default_texture);
    glPopAttrib();
  }
}

FX_ENTRY void FX_CALL grFramebufferCopyExt(int x, int y, int w, int h,
                                           int from, int to, int mode)
{
  if (mode == GR_FBCOPY_MODE_DEPTH) {
    if(!glsl_support) {
      return;
    }
    
    int tw = 1, th = 1;
    if (npot_support) {
      tw = width; th = height;
    } else {
      while (tw < width) tw <<= 1;
      while (th < height) th <<= 1;
    }
    
    if (from == GR_FBCOPY_BUFFER_BACK && to == GR_FBCOPY_BUFFER_FRONT) {
      printf("save depth buffer %d\n", render_to_texture);
      // save the depth image in a texture
      //glDisable(GL_ALPHA_TEST);
      glReadBuffer(current_buffer);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                       0, viewport_offset, tw, th, 0);
      glBindTexture(GL_TEXTURE_2D, default_texture);
      return;
    }
    if (from == GR_FBCOPY_BUFFER_FRONT && to == GR_FBCOPY_BUFFER_BACK) {
      printf("writing to depth buffer %d\n", render_to_texture);
      
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_ALPHA_TEST);
      glDrawBuffer(current_buffer);
      glActiveTextureARB(texture_unit);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      set_depth_shader();
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_ALWAYS);
      glDisable(GL_CULL_FACE);
      render_rectangle(texture_unit,
                       0, 0,
                       width,  height,
                       tw, th, -1);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glBindTexture(GL_TEXTURE_2D, default_texture);
      glPopAttrib();
      return;
    }
    
  }    
}

FX_ENTRY void FX_CALL
grRenderBuffer( GrBuffer_t buffer )
{
#ifdef _WIN32
    static HANDLE region = NULL;
    int realWidth = pBufferWidth, realHeight = pBufferHeight;
#endif // _WIN32
    LOG("grRenderBuffer(%d)\r\n", buffer);
    //printf("grRenderBuffer(%d)\n", buffer);

    switch(buffer)
    {
    case GR_BUFFER_BACKBUFFER:
        if(render_to_texture)
        {
      updateTexture();
      
      // VP z fix
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0, 0, 1-zscale);
            glScalef(1, 1, zscale);
            inverted_culling = 0;
            grCullMode(culling_mode);

            width = savedWidth;
            height = savedHeight;
            widtho = savedWidtho;
            heighto = savedHeighto;
      if (use_fbo) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );
      }
      curBufferAddr = 0;

            glViewport(0, viewport_offset, width, viewport_height);
      glScissor(0, viewport_offset, width, height);
      
#ifdef SAVE_CBUFFER
      if (!use_fbo && render_to_texture == 2) {
        // restore color buffer
        if (nbAuxBuffers > 0) {
          glDrawBuffer(GL_BACK);
          current_buffer = GL_BACK;
        } else if (save_w) {
          int tw = 1, th = 1;
          //printf("restore %dx%d\n", save_w, save_h);
          if (npot_support) {
            tw = screen_width;
            th = screen_height;
          } else {
            while (tw < screen_width) tw <<= 1;
            while (th < screen_height) th <<= 1;
          }
          
          glPushAttrib(GL_ALL_ATTRIB_BITS);
          glDisable(GL_ALPHA_TEST);
          glDrawBuffer(GL_BACK);
          glActiveTextureARB(texture_unit);
          glBindTexture(GL_TEXTURE_2D, color_texture);
          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          set_copy_shader();
          glDisable(GL_DEPTH_TEST);
          glDisable(GL_CULL_FACE);
          render_rectangle(texture_unit,
                           0, 0,
                           save_w,  save_h,
                           tw, th, -1);
          glBindTexture(GL_TEXTURE_2D, default_texture);
          glPopAttrib();

          save_w = save_h = 0;
        }
      }  
#endif

      // ZIGGY
      // restore depth buffer
//       grFramebufferCopyExt(0, 0, width, height,
//                            GR_FBCOPY_BUFFER_FRONT, GR_FBCOPY_BUFFER_BACK,
//                            GR_FBCOPY_MODE_DEPTH);
      
            render_to_texture = 0;
        }
        glDrawBuffer(GL_BACK);
        break;
    case 6: // RENDER TO TEXTURE
        if(!render_to_texture)
        {

      
      // ZIGGY
      // save depth buffer
//       grFramebufferCopyExt(0, 0, width, height,
//                            GR_FBCOPY_BUFFER_BACK, GR_FBCOPY_BUFFER_FRONT,
//                            GR_FBCOPY_MODE_DEPTH);
            savedWidth = width;
            savedHeight = height;
            savedWidtho = widtho;
            savedHeighto = heighto;
        }

        {
      if (!use_fbo) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, 1-zscale);
        glScalef(1, 1, zscale);
        inverted_culling = 0;
      } else {
        float m[4*4] = {1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f,-1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f};
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m);
        // VP z fix
        glTranslatef(0, 0, 1-zscale);
        glScalef(1, 1*1, zscale);
        inverted_culling = 1;
        grCullMode(culling_mode);
      }
        }
        render_to_texture = 1;
        break;
    default:
        display_warning("grRenderBuffer : unknown buffer : %x", buffer);
    }
}

FX_ENTRY void FX_CALL
grAuxBufferExt( GrBuffer_t buffer )
{
    LOG("grAuxBufferExt(%d)\r\n", buffer);
    //display_warning("grAuxBufferExt");

  if (glsl_support && buffer == GR_BUFFER_AUXBUFFER) {
    invtex[0] = 0;
    invtex[1] = 0;
    need_to_compile = 0;
    set_depth_shader();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_TRUE);
    grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
//     glActiveTextureARB(texture_unit);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    need_to_compile = 1;
  }
}

FX_ENTRY void FX_CALL
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU32 depth )
{
    LOG("grBufferClear(%d,%d,%d)\r\n", color, alpha, depth);
    switch(lfb_color_fmt)
    {
    case GR_COLORFORMAT_ARGB:
        glClearColor(((color >> 16) & 0xFF) / 255.0f,
                    ((color >>  8) & 0xFF) / 255.0f,
                    ( color        & 0xFF) / 255.0f,
                    alpha / 255.0f);
        break;
    case GR_COLORFORMAT_RGBA:
        glClearColor(((color >> 24) & 0xFF) / 255.0f,
                    ((color >> 16) & 0xFF) / 255.0f,
                    (color         & 0xFF) / 255.0f,
                    alpha / 255.0f);
        break;
    default:
        display_warning("grBufferClear: unknown color format : %x", lfb_color_fmt);
    }

    if (w_buffer_mode)
        glClearDepth(1.0f - ((1.0f + (depth >> 4) / 4096.0f) * (1 << (depth & 0xF))) / 65528.0);
    else
        glClearDepth(depth / 65535.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // ZIGGY TODO check that color mask is on
  buffer_cleared = TRUE;

}


extern void (*renderCallback)();

// #include <unistd.h>
FX_ENTRY void FX_CALL
grBufferSwap( FxU32 swap_interval )
{
  if(renderCallback)
    (*renderCallback)();
  int i;
    LOG("grBufferSwap(%d)\r\n", swap_interval);
  //printf("swap\n");
  if (render_to_texture) {
    display_warning("swap while render_to_texture\n");
    return;
  }

#ifdef _WIN32
    SwapBuffers(wglGetCurrentDC());
#else // _WIN32
  SDL_GL_SwapBuffers();
#endif // _WIN32
  for (i = 0; i < nb_fb; i++)
    fbs[i].buff_clear = 1;

  // VP debugging
#ifdef VPDEBUG
  dump_stop();
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
        case 'd':
          printf("Dumping !\n");
          dump_start();
          break;
        case 'w': {
          static int wireframe;
          wireframe = !wireframe;
          glPolygonMode(GL_FRONT_AND_BACK, wireframe? GL_LINE : GL_FILL);
          break;
        default:
          break;
        }
      }
      break;
    }
  }
#endif
}

// frame buffer

FX_ENTRY FxBool FX_CALL
grLfbLock( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode,
           GrOriginLocation_t origin, FxBool pixelPipeline, 
           GrLfbInfo_t *info )
{
    LOG("grLfbLock(%d,%d,%d,%d,%d)\r\n", type, buffer, writeMode, origin, pixelPipeline);
    if (type == GR_LFB_WRITE_ONLY)
    {
        display_warning("grLfbLock : write only");
    }
    else
    {
        unsigned char *buf;
        int i,j;

        switch(buffer)
        {
        case GR_BUFFER_FRONTBUFFER:
            glReadBuffer(GL_FRONT);
            break;
        case GR_BUFFER_BACKBUFFER:
            glReadBuffer(GL_BACK);
            break;
        /*case GR_BUFFER_AUXBUFFER:
            glReadBuffer(current_buffer);
            break;*/
        default:
            display_warning("grLfbLock : unknown buffer : %x", buffer);
        }
    
        if(buffer != GR_BUFFER_AUXBUFFER)
        {
          if (writeMode == GR_LFBWRITEMODE_888) {
        printf("LfbLock GR_LFBWRITEMODE_888\n");
        info->lfbPtr = frameBuffer;
        info->strideInBytes = width*4;
        info->writeMode = GR_LFBWRITEMODE_888;
        info->origin = origin;
        glReadPixels(0, viewport_offset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, frameBuffer);
          } else {
        buf = (unsigned char*)malloc(width*height*4);
        
        info->lfbPtr = frameBuffer;
        info->strideInBytes = width*2;
        info->writeMode = GR_LFBWRITEMODE_565;
        info->origin = origin;
        glReadPixels(0, viewport_offset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        
        for (j=0; j<height; j++)
        {
          for (i=0; i<width; i++)
          {
            frameBuffer[(height-j-1)*width+i] =
              ((buf[j*width*4+i*4+0] >> 3) << 11) |
              ((buf[j*width*4+i*4+1] >> 2) <<  5) |
              (buf[j*width*4+i*4+2] >> 3);
          }
        }
        //adler32b = adler32(0, (const unsigned char*)frameBuffer, width*height*2);
        free(buf);
          }
        }
        else
        {
            info->lfbPtr = depthBuffer;
            info->strideInBytes = width*2;
            info->writeMode = GR_LFBWRITEMODE_ZA16;
            info->origin = origin;
            glReadPixels(0, viewport_offset, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, depthBuffer);
        }
    }

    return FXTRUE;
}

FX_ENTRY FxBool FX_CALL
grLfbUnlock( GrLock_t type, GrBuffer_t buffer )
{
    LOG("grLfbUnlock(%d,%d)\r\n", type, buffer);
    if (type == GR_LFB_WRITE_ONLY)
    {
        display_warning("grLfbUnlock : write only");
    }
    return FXTRUE;
}

FX_ENTRY FxBool FX_CALL
grLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data )
{
    unsigned char *buf;
    unsigned int i,j;
    unsigned short *frameBuffer = (unsigned short*)dst_data;
    unsigned short *depthBuffer = (unsigned short*)dst_data;
    LOG("grLfbReadRegion(%d,%d,%d,%d,%d,%d)\r\n", src_buffer, src_x, src_y, src_width, src_height, dst_stride);

    switch(src_buffer)
    {
    case GR_BUFFER_FRONTBUFFER:
        glReadBuffer(GL_FRONT);
        break;
    case GR_BUFFER_BACKBUFFER:
        glReadBuffer(GL_BACK);
        break;
    /*case GR_BUFFER_AUXBUFFER:
        glReadBuffer(current_buffer);
        break;*/
    default:
        display_warning("grReadRegion : unknown buffer : %x", src_buffer);
    }

    if(src_buffer != GR_BUFFER_AUXBUFFER)
    {
        buf = (unsigned char*)malloc(src_width*src_height*4);

        glReadPixels(src_x, (viewport_offset)+height-src_y-src_height, src_width, src_height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        
        for (j=0; j<src_height; j++)
        {
            for (i=0; i<src_width; i++)
            {
                frameBuffer[j*(dst_stride/2)+i] =
                    ((buf[(src_height-j-1)*src_width*4+i*4+0] >> 3) << 11) |
                    ((buf[(src_height-j-1)*src_width*4+i*4+1] >> 2) <<  5) |
                    (buf[(src_height-j-1)*src_width*4+i*4+2] >> 3);
                }
            }
        free(buf);
    }
    else
    {
        buf = (unsigned char*)malloc(src_width*src_height*2);
        
        glReadPixels(src_x, (viewport_offset)+height-src_y-src_height, src_width, src_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, depthBuffer);

        for (j=0;j<src_height; j++)
        {
            for (i=0; i<src_width; i++)
            {
                depthBuffer[j*(dst_stride/2)+i] =
                    ((unsigned short*)buf)[(src_height-j-1)*src_width*4+i*4];
            }
        }
        free(buf);
    }
    
    return FXTRUE;
}

FX_ENTRY FxBool FX_CALL
grLfbWriteRegion( GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxBool pixelPipeline,
                  FxI32 src_stride, void *src_data )
{
    unsigned char *buf;
    unsigned int i,j;
    unsigned short *frameBuffer = (unsigned short*)src_data;
    int texture_number;
    unsigned int tex_width = 1, tex_height = 1;
    LOG("grLfbWriteRegion(%d,%d,%d,%d,%d,%d,%d,%d)\r\n",dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride);

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  
  while (tex_width < src_width) tex_width <<= 1;
  while (tex_height < src_height) tex_height <<= 1;

    switch(dst_buffer)
    {
    case GR_BUFFER_BACKBUFFER:
        glDrawBuffer(GL_BACK);
        break;
    case GR_BUFFER_AUXBUFFER:
        glDrawBuffer(current_buffer);
        break;
    default:
        display_warning("grLfbWriteRegion : unknown buffer : %x", dst_buffer);
    }

    if(dst_buffer != GR_BUFFER_AUXBUFFER)
    {
        buf = (unsigned char*)malloc(tex_width*tex_height*4);

        if(!glsl_support)
        {
            switch(nbTextureUnits)
            {
            case 2:
                texture_number = GL_TEXTURE1_ARB;
                break;
            case 3:
                texture_number = GL_TEXTURE2_ARB;
                break;
            default:
                texture_number = GL_TEXTURE3_ARB;
            }
        }
        else texture_number = GL_TEXTURE0_ARB;
        glActiveTextureARB(texture_number);

        switch(src_format)
        {
        case GR_LFB_SRC_FMT_1555:
            for (j=0; j<src_height; j++)
            {
                for (i=0; i<src_width; i++)
                {
                    buf[j*tex_width*4+i*4+0]=((frameBuffer[j*(src_stride/2)+i]>>10)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+1]=((frameBuffer[j*(src_stride/2)+i]>> 5)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+2]=((frameBuffer[j*(src_stride/2)+i]>> 0)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+3]=(frameBuffer[j*(src_stride/2)+i]>>15)?0xFF:0;
                }
            }
            break;
        case GR_LFBWRITEMODE_555:
            for (j=0; j<src_height; j++)
            {
                for (i=0; i<src_width; i++)
                {
                    buf[j*tex_width*4+i*4+0]=((frameBuffer[j*(src_stride/2)+i]>>10)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+1]=((frameBuffer[j*(src_stride/2)+i]>> 5)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+2]=((frameBuffer[j*(src_stride/2)+i]>> 0)&0x1F)<<3;
                    buf[j*tex_width*4+i*4+3]=0xFF;
                }
            }
            break;
        default:
            display_warning("grLfbWriteRegion : unknown format : %d", src_format);
        }

#ifdef VPDEBUG
    if (dumping) {
      ilTexImage(tex_width, tex_height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, buf);
      char name[128];
      static int id;
      sprintf(name, "dump/writecolor%d.png", id++);
      ilSaveImage(name);
      printf("dumped gdLfbWriteRegion %s\n", name);
    }
#endif

        glBindTexture(GL_TEXTURE_2D, default_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        free(buf);

    set_copy_shader();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    render_rectangle(texture_number,
                     dst_x, dst_y,
                     src_width,  src_height,
                     tex_width,  tex_height, +1);

    }
    else
    {
        float *buf = (float*)malloc(src_width*(src_height+(viewport_offset))*sizeof(float));

        if (src_format != GR_LFBWRITEMODE_ZA16)
            display_warning("unknown depth buffer write format:%x", src_format);

        if(dst_x || dst_y)
            display_warning("dst_x:%d, dst_y:%d\n",dst_x, dst_y);

        for (j=0; j<src_height; j++)
        {
            for (i=0; i<src_width; i++)
            {
                buf[(j+(viewport_offset))*src_width+i] = 
                    (frameBuffer[(src_height-j-1)*(src_stride/2)+i]/(65536.0f*(2.0f/zscale)))+1-zscale/2.0f;
        //(frameBuffer[(src_height-j-1)*(src_stride/2)+i]/(65536.0f));
            }
        }

#ifdef VPDEBUG
    if (dumping) {
      unsigned char * buf2 = (unsigned char *)malloc(src_width*(src_height+(viewport_offset)));
      for (i=0; i<src_width*src_height ; i++)
        buf2[i] = (unsigned char) (buf[i] * 255.0f);
      ilTexImage(src_width, src_height, 1, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, buf2);
      char name[128];
      static int id;
      sprintf(name, "dump/writedepth%d.png", id++);
      ilSaveImage(name);
      printf("dumped gdLfbWriteRegion %s\n", name);
      free(buf2);
    }
#endif

    //static int num;
    //printf("zrite %d, back %d\n", num++, dst_buffer == GR_BUFFER_BACKBUFFER);
        glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

        glDrawBuffer(GL_BACK);
    glClear( GL_DEPTH_BUFFER_BIT );

        //glDisable(GL_DEPTH_TEST);
        glDepthMask(1);
        glDrawPixels(src_width, src_height+(viewport_offset), GL_DEPTH_COMPONENT, GL_FLOAT, buf);

        free(buf);

        //glDepthMask(0);
    }
    glDrawBuffer(current_buffer);
  glPopAttrib();
    return FXTRUE;
}

// unused by glide64

FX_ENTRY FxI32 FX_CALL 
grQueryResolutions( const GrResolution *resTemplate, GrResolution *output )
{
    int res_inf = 0;
    int res_sup = 0xf;
    int i;
    int n=0;
    display_warning("grQueryResolutions");
    if ((unsigned int)resTemplate->resolution != GR_QUERY_ANY)
    {
        res_inf = res_sup = resTemplate->resolution;
    }
    if ((unsigned int)resTemplate->refresh == GR_QUERY_ANY) display_warning("querying any refresh rate");
    if ((unsigned int)resTemplate->numAuxBuffers == GR_QUERY_ANY) display_warning("querying any numAuxBuffers");
    if ((unsigned int)resTemplate->numColorBuffers == GR_QUERY_ANY) display_warning("querying any numColorBuffers");

    if (output == NULL) return res_sup - res_inf + 1;
    for (i=res_inf; i<=res_sup; i++)
    {
        output[n].resolution = i;
        output[n].refresh = resTemplate->refresh;
        output[n].numAuxBuffers = resTemplate->numAuxBuffers;
        output[n].numColorBuffers = resTemplate->numColorBuffers;
        n++;
    }
    return res_sup - res_inf + 1;
}

FX_ENTRY FxBool FX_CALL 
grReset( FxU32 what )
{
    display_warning("grReset");
    return 1;
}

FX_ENTRY void FX_CALL 
grEnable( GrEnableMode_t mode )
{
    display_warning("grEnable");
}

FX_ENTRY void FX_CALL 
grDisable( GrEnableMode_t mode )
{
    display_warning("grDisable");
}

FX_ENTRY void FX_CALL 
grDisableAllEffects( void )
{
    display_warning("grDisableAllEffects");
}

FX_ENTRY void FX_CALL 
grErrorSetCallback( GrErrorCallbackFnc_t fnc )
{
    display_warning("grErrorSetCallback");
}

FX_ENTRY void FX_CALL 
grFinish(void)
{
    display_warning("grFinish");
}

FX_ENTRY void FX_CALL 
grFlush(void)
{
    display_warning("grFlush");
}

FX_ENTRY void FX_CALL 
grTexMultibase( GrChipID_t tmu,
                FxBool     enable )
{
    display_warning("grTexMultibase");
}

FX_ENTRY void FX_CALL 
grTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend )
{
    display_warning("grTexMipMapMode");
}

FX_ENTRY void FX_CALL
grTexDownloadTablePartial( GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end )
{
    display_warning("grTexDownloadTablePartial");
}

FX_ENTRY void FX_CALL
grTexDownloadTable( GrTexTable_t type, 
                    void         *data )
{
    display_warning("grTexDownloadTable");
}

FX_ENTRY FxBool FX_CALL 
grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end )
{
    display_warning("grTexDownloadMipMapLevelPartial");
    return 1;
}

FX_ENTRY void FX_CALL 
grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data )
{
    display_warning("grTexDownloadMipMapLevel");
}

FX_ENTRY void FX_CALL 
grTexNCCTable( GrNCCTable_t table )
{
    display_warning("grTexNCCTable");
}

FX_ENTRY void FX_CALL 
grViewport( FxI32 x, FxI32 y, FxI32 width, FxI32 height )
{
    display_warning("grViewport");
}

FX_ENTRY void FX_CALL 
grDepthRange( FxFloat n, FxFloat f )
{
    display_warning("grDepthRange");
}

FX_ENTRY void FX_CALL
grSplash(float x, float y, float width, float height, FxU32 frame)
{
    display_warning("grSplash");
}

FX_ENTRY FxBool FX_CALL
grSelectContext( GrContext_t context )
{
    display_warning("grSelectContext");
    return 1;
}

FX_ENTRY void FX_CALL
grAADrawTriangle(
                 const void *a, const void *b, const void *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 )
{
    display_warning("grAADrawTriangle");
}

FX_ENTRY void FX_CALL
grAlphaControlsITRGBLighting( FxBool enable )
{
    display_warning("grAlphaControlsITRGBLighting");
}

FX_ENTRY void FX_CALL
grGlideSetVertexLayout( const void *layout )
{
    display_warning("grGlideSetVertexLayout");
}

FX_ENTRY void FX_CALL
grGlideGetVertexLayout( void *layout )
{
    display_warning("grGlideGetVertexLayout");
}

FX_ENTRY void FX_CALL
grGlideSetState( const void *state )
{
    display_warning("grGlideSetState");
}

FX_ENTRY void FX_CALL
grGlideGetState( void *state )
{
    display_warning("grGlideGetState");
}

FX_ENTRY void FX_CALL
grLfbWriteColorFormat(GrColorFormat_t colorFormat)
{
    display_warning("grLfbWriteColorFormat");
}

FX_ENTRY void FX_CALL 
grLfbWriteColorSwizzle(FxBool swizzleBytes, FxBool swapWords)
{
    display_warning("grLfbWriteColorSwizzle");
}

FX_ENTRY void FX_CALL 
grLfbConstantDepth( FxU32 depth )
{
    display_warning("grLfbConstantDepth");
}

FX_ENTRY void FX_CALL 
grLfbConstantAlpha( GrAlpha_t alpha )
{
    display_warning("grLfbConstantAlpha");
}

FX_ENTRY void FX_CALL
grTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info )
{
    display_warning("grTexMultibaseAddress");
}

FX_ENTRY void FX_CALL 
grLoadGammaTable( FxU32 nentries, FxU32 *red, FxU32 *green, FxU32 *blue)
{
    display_warning("grLoadGammaTable");
}

FX_ENTRY void FX_CALL 
grDitherMode( GrDitherMode_t mode )
{
    display_warning("grDitherMode");
}

void grChromaRangeExt(GrColor_t color0, GrColor_t color1, FxU32 mode)
{
    display_warning("grChromaRangeExt");
}

void grChromaRangeModeExt(GrChromakeyMode_t mode)
{
    display_warning("grChromaRangeModeExt");
}

void grTexChromaRangeExt(GrChipID_t tmu, GrColor_t color0, GrColor_t color1, GrTexChromakeyMode_t mode)
{
    display_warning("grTexChromaRangeExt");
}

void grTexChromaModeExt(GrChipID_t tmu, GrChromakeyMode_t mode)
{
    display_warning("grTexChromaRangeModeExt");
}



// VP debug
int dumping;
#ifdef VPDEBUG
static int tl_i;
static int tl[10240];

void dump_start()
{
  static int init;
  if (!init) {
    init = 1;
    ilInit();
    ilEnable(IL_FILE_OVERWRITE);
  }
  dumping = 1;
  tl_i = 0;
}

void dump_stop()
{
  if (!dumping) return;

  int i, j;
  for (i=0; i<nb_fb; i++) {
    dump_tex(fbs[i].texid);
  }
  dump_tex(default_texture);
  dump_tex(depth_texture);

  dumping = 0;

  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer);
  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, frameBuffer);
  ilSaveImage("dump/framecolor.png");
  glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, depthBuffer);
//   FILE * fp = fopen("glide_depth1.bin", "rb");
//   fread(depthBuffer, 2, width*height, fp);
//   fclose(fp);
  unsigned char *frameByte = (unsigned char *) frameBuffer;
  for (j=0; j<height; j++) {
    for (i=0; i<width; i++) {
      //uint16_t d = ( (uint16_t *)depthBuffer )[i+(height-1-j)*width]/2 + 0x8000;
      uint16_t      d = ( (uint16_t *)depthBuffer )[i+j*width];
      unsigned char c = frameByte[(i+j*width)*4];
      frameByte[(i+j*width)*3] = d&0xff;
      frameByte[(i+j*width)*3+1] = d>>8;
      frameByte[(i+j*width)*3+2] = c&0xff;
    }
  }
  ilTexImage(width, height, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, frameBuffer);
  ilSaveImage("dump/framedepth.png");

  for (i=0; i<tl_i; i++) {
    glBindTexture(GL_TEXTURE_2D, tl[i]);
    GLint w, h, fmt;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
    fprintf(stderr, "Texture %d %dx%d fmt %x\n", tl[i], (int)w, (int)h, (int) fmt);

    uint32_t * pixels = (uint32_t *) malloc(w*h*4);
    // 0x1902 is another constant meaning GL_DEPTH_COMPONENT
    // (but isn't defined in gl's headers !!)
    if (fmt != GL_DEPTH_COMPONENT && fmt != 0x1902) {
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      ilTexImage(w, h, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels);
    } else {
      glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, pixels);
      int i;
      for (i=0; i<w*h; i++) 
        ((unsigned char *)frameBuffer)[i] = ((unsigned short *)pixels)[i]/256;
      ilTexImage(w, h, 1, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, frameBuffer);
    }
    char name[128];
//     sprintf(name, "mkdir -p dump ; rm -f dump/tex%04d.png", i);
//     system(name);
    sprintf(name, "dump/tex%04d.png", i);
    fprintf(stderr, "Writing '%s'\n", name);
    ilSaveImage(name);

//     SDL_FreeSurface(surf);
    free(pixels);
  }
  glBindTexture(GL_TEXTURE_2D, default_texture);
}

void dump_tex(int id)
{
  if (!dumping) return;

  int n;
  // yes, it's inefficient
  for (n=0; n<tl_i; n++)
    if (tl[n] == id)
      return;

  tl[tl_i++] = id;
}

#endif