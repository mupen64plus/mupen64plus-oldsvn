
#include <specific.h>
#include "glide.h"
#include "main.h"
#ifdef _WIN32
#include "resource.h"
#endif // _WIN32

typedef struct _wrapper_config
{
    int res;
    int filter;
    int disable_glsl;
    int disable_dithered_alpha;
  int FBO;
  int disable_auxbuf;
} wrapper_config;

#ifdef _WIN32
static HINSTANCE hinstance;
static HKEY Key = NULL;
static wrapper_config config;
static int valid_filter = 0;
static int valid_dithalpha = 0;

#ifdef WIN32
extern "C" {
BOOL WINAPI DllMain (HINSTANCE hinstDLL, 
                     DWORD fdwReason,
                     LPVOID lpReserved);
}
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved
)
{
    hinstance = hinstDLL;
    return TRUE;
}
#endif

static BOOL registry_open()
{
    DWORD Status;
    LONG Res;
    
    Res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GlideWrapper", 0, NULL, 
                         REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &Key, &Status);
    if(Res != ERROR_SUCCESS) return FALSE;
    return TRUE;
}

static BOOL registry_getvalue(char *name, char *buffer, unsigned long size)
{
    LONG Res = RegQueryValueEx(Key, name, NULL, NULL, (unsigned char *)buffer, &size);
    if(Res != ERROR_SUCCESS) return FALSE;
    return TRUE;
}

static BOOL registry_setvalue(char *name, DWORD type, char *buffer, unsigned long size)
{
    LONG Res = RegSetValueEx(Key, name, 0, type, (unsigned char *)buffer, size);
    if(Res != ERROR_SUCCESS) return FALSE;
    return TRUE;
}

static void registry_close()
{
    RegCloseKey(Key);
    Key = NULL;
}

static void readConfig()
{
    registry_open();
    if (registry_getvalue("res", (char*)&config.res, sizeof(int)) == FALSE)
        config.res = 7;
    if (registry_getvalue("filter", (char*)&config.filter, sizeof(int)) == FALSE)
        config.filter = 0;
    if (registry_getvalue("glsl", (char*)&config.disable_glsl, sizeof(int)) == FALSE)
        config.disable_glsl = 0;
    if (registry_getvalue("dithalpha", (char*)&config.disable_dithered_alpha, sizeof(int)) == FALSE)
        config.disable_dithered_alpha = 0;
    if (registry_getvalue("fbo", (char*)&config.FBO, sizeof(int)) == FALSE)
        config.FBO = 0;
    if (registry_getvalue("noauxbuf", (char*)&config.disable_auxbuf, sizeof(int)) == FALSE)
        config.disable_auxbuf = 0;
    registry_close();
}

static void saveConfig()
{
    registry_open();
    registry_setvalue("res", REG_DWORD, (char*)&config.res, sizeof(int));
    registry_setvalue("filter", REG_DWORD, (char*)&config.filter, sizeof(int));
    registry_setvalue("glsl", REG_DWORD, (char*)&config.disable_glsl, sizeof(int));
    registry_setvalue("dithalpha", REG_DWORD, (char*)&config.disable_dithered_alpha, sizeof(int));
    registry_setvalue("fbo", REG_DWORD, (char*)&config.FBO, sizeof(int));
    registry_setvalue("noauxbuf", REG_DWORD, (char*)&config.disable_auxbuf, sizeof(int));
    registry_close();
}

static BOOL CALLBACK ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"320*200");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"320*240");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"400*256");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"512*384");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"640*200");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"640*350");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"640*400");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"640*480");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"800*600");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"960*720");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"856*480");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"512*256");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"1024*768");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"1280*1024");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"1600*1200");
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_ADDSTRING, 0, (LPARAM)"400*300");

        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)"None");
        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)"Blur edges");
        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)"Super 2xSai");
        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)"Hq2x");
        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_ADDSTRING, 0, (LPARAM)"Hq4x");

        readConfig();
        SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_SETCURSEL, config.res, 0);
        SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_SETCURSEL, config.filter, 0);
        if(config.disable_glsl)
            SendDlgItemMessage(hwnd, IDC_GLSL, BM_SETCHECK, BST_CHECKED, 0);
        else
            SendDlgItemMessage(hwnd, IDC_GLSL, BM_SETCHECK, BST_UNCHECKED, 0);
        if(config.disable_dithered_alpha)
            SendDlgItemMessage(hwnd, IDC_DITHALPHA, BM_SETCHECK, BST_CHECKED, 0);
        else
            SendDlgItemMessage(hwnd, IDC_DITHALPHA, BM_SETCHECK, BST_UNCHECKED, 0);
        if(config.FBO)
            SendDlgItemMessage(hwnd, IDC_FBO, BM_SETCHECK, BST_CHECKED, 0);
        else
            SendDlgItemMessage(hwnd, IDC_FBO, BM_SETCHECK, BST_UNCHECKED, 0);
        if(config.disable_auxbuf)
            SendDlgItemMessage(hwnd, IDC_AUXBUF, BM_SETCHECK, BST_CHECKED, 0);
        else
            SendDlgItemMessage(hwnd, IDC_AUXBUF, BM_SETCHECK, BST_UNCHECKED, 0);
        return TRUE;
        break;

    case WM_CLOSE:
        EndDialog(hwnd, IDOK);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDOK);
            break;
        case IDOK:
            config.res = SendDlgItemMessage(hwnd, IDC_RES_COMBO, CB_GETCURSEL, 0, 0);
            config.filter = SendDlgItemMessage(hwnd, IDC_FILTER_COMBO, CB_GETCURSEL, 0, 0);
            config.disable_glsl = SendDlgItemMessage(hwnd, IDC_GLSL, BM_GETCHECK,0,0) == BST_CHECKED ? 1 : 0;
            config.disable_dithered_alpha = SendDlgItemMessage(hwnd, IDC_DITHALPHA, BM_GETCHECK,0,0) == BST_CHECKED ? 1 : 0;
            config.FBO = SendDlgItemMessage(hwnd, IDC_FBO, BM_GETCHECK,0,0) == BST_CHECKED ? 1 : 0;
            config.disable_auxbuf = SendDlgItemMessage(hwnd, IDC_AUXBUF, BM_GETCHECK,0,0) == BST_CHECKED ? 1 : 0;
            valid_filter = 0;
            saveConfig();
            EndDialog(hwnd, IDOK);
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}
#endif // _WIN32
FX_ENTRY void FX_CALL grConfigWrapperExt(HINSTANCE
instance, HWND hwnd)
{
#ifdef _WIN32
    DialogBox(hinstance, MAKEINTRESOURCE(IDD_CONFIG_DIAL), hwnd, ConfigDlgProc);
#endif // _WIN32
}

#ifdef _WIN32
int getFullScreenWidth()
{
    readConfig();
    switch(config.res)
    {
    case GR_RESOLUTION_320x200:
        return 320;
        break;
    case GR_RESOLUTION_320x240:
        return 320;
        break;
    case GR_RESOLUTION_400x256:
        return 400;
        break;
    case GR_RESOLUTION_512x384:
        return 512;
        break;
    case GR_RESOLUTION_640x200:
        return 640;
        break;
    case GR_RESOLUTION_640x350:
        return 640;
        break;
    case GR_RESOLUTION_640x400:
        return 640;
        break;
    case GR_RESOLUTION_640x480:
        return 640;
        break;
    case GR_RESOLUTION_800x600:
        return 800;
        break;
    case GR_RESOLUTION_960x720:
        return 960;
        break;
    case GR_RESOLUTION_856x480:
        return 856;
        break;
    case GR_RESOLUTION_512x256:
        return 512;
        break;
    case GR_RESOLUTION_1024x768:
        return 1024;
        break;
    case GR_RESOLUTION_1280x1024:
        return 1280;
        break;
    case GR_RESOLUTION_1600x1200:
        return 1600;
        break;
    case GR_RESOLUTION_400x300:
        return 400;
        break;
    }
    return 0;
}

int getFullScreenHeight()
{
    readConfig();
    switch(config.res)
    {
    case GR_RESOLUTION_320x200:
        return 200;
        break;
    case GR_RESOLUTION_320x240:
        return 240;
        break;
    case GR_RESOLUTION_400x256:
        return 256;
        break;
    case GR_RESOLUTION_512x384:
        return 384;
        break;
    case GR_RESOLUTION_640x200:
        return 200;
        break;
    case GR_RESOLUTION_640x350:
        return 350;
        break;
    case GR_RESOLUTION_640x400:
        return 400;
        break;
    case GR_RESOLUTION_640x480:
        return 480;
        break;
    case GR_RESOLUTION_800x600:
        return 600;
        break;
    case GR_RESOLUTION_960x720:
        return 720;
        break;
    case GR_RESOLUTION_856x480:
        return 480;
        break;
    case GR_RESOLUTION_512x256:
        return 250;
        break;
    case GR_RESOLUTION_1024x768:
        return 768;
        break;
    case GR_RESOLUTION_1280x1024:
        return 1024;
        break;
    case GR_RESOLUTION_1600x1200:
        return 1200;
        break;
    case GR_RESOLUTION_400x300:
        return 300;
        break;
    }
    return 0;
}

#else // _WIN32

#include "../rdp.h"

#endif // _WIN32

FX_ENTRY GrScreenResolution_t FX_CALL grWrapperFullScreenResolutionExt(void)
{
#ifdef _WIN32
    readConfig();
    return config.res;
#else // _WIN32
   return settings.full_res;
#endif // _WIN32
}

int getFilter()
{
#ifdef _WIN32
    if (!valid_filter)
    {
        readConfig();
        valid_filter = 1;
    }
    return config.filter;
#else // _WIN32
   return settings.tex_filter;
#endif // _WIN32
}

int getDisableDitheredAlpha()
{
#ifdef _WIN32
    if (!valid_dithalpha)
    {
        readConfig();
        valid_dithalpha = 1;
    }
    return config.disable_dithered_alpha;
#else // _WIN32
   return settings.noditheredalpha;
#endif // _WIN32
}

int getEnableFBO()
{
#ifdef _WIN32
    if (!valid_dithalpha)
    {
        readConfig();
        valid_dithalpha = 1;
    }
    return config.FBO;
#else // _WIN32
   return settings.FBO;
#endif // _WIN32
}

int getDisableAuxbuf()
{
#ifdef _WIN32
    if (!valid_dithalpha)
    {
        readConfig();
        valid_dithalpha = 1;
    }
    return config.disable_auxbuf;
#else // _WIN32
   return settings.disable_auxbuf;
#endif // _WIN32
}

int getDisableGLSL()
{
#ifdef _WIN32
    readConfig();
    return config.disable_glsl;
#else // _WIN32
   return settings.noglsl;
#endif // _WIN32
}
