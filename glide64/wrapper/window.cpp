#include <windows.h>

#ifdef GCC
#include <stdio.h>
#endif

#include "main.h"

static HDC          hDC=NULL;       // Private GDI Device Context
static HGLRC        hRC=NULL;       // Permanent Rendering Context
static HWND         hWnd=NULL;      // Holds Our Window Handle
static HINSTANCE    hInstance;      // Holds The Instance Of The Application

#ifdef DEBUG
void MSG_DBG(const char *szString)
{
    ::OutputDebugString(szString);
}
#else
#define MSG_DBG(x) 
#endif

static LRESULT CALLBACK WndProc(    HWND    hWnd,           // Handle For This Window
                            UINT    uMsg,           // Message For This Window
                            WPARAM  wParam,         // Additional Message Information
                            LPARAM  lParam)         // Additional Message Information
{
    switch (uMsg)                                   // Check For Windows Messages
    {
        case WM_CLOSE:                              // Did We Receive A Close Message?
        {
            PostQuitMessage(0);                     // Send A Quit Message
            return 0;                               // Jump Back
        }
    }
    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



GLvoid KillGLWindow(GLvoid)                             // Properly Kill The Window
{
    if (hRC)                                            // Do We Have A Rendering Context?
    {
        if (!wglMakeCurrent(NULL, NULL))                // Are We Able To Release The DC And RC Contexts?
        {
            MSG_DBG("Release Of DC And RC Failed.");
        }

        if (!wglDeleteContext(hRC))                         // Are We Able To Delete The RC?
        {
            MSG_DBG("Release Rendering Context Failed.");
        }
        hRC=NULL;                                       // Set RC To NULL
    }

    if (hDC && !ReleaseDC(hWnd, hDC))                   // Are We Able To Release The DC
    {
        MSG_DBG("Release Device Context Failed.");
        hDC=NULL;                                       // Set DC To NULL
    }

    if (hWnd && !DestroyWindow(hWnd))                   // Are We Able To Destroy The Window?
    {
        MSG_DBG("Could Not Release hWnd.");
        hWnd=NULL;                                      // Set hWnd To NULL
    }

    if (!UnregisterClass("OpenGL", hInstance))          // Are We Able To Unregister Class
    {
        MSG_DBG("Could Not Unregister Class.");
        hInstance=NULL;                                 // Set hInstance To NULL
    }
}


/*  This Code Creates Our OpenGL Window.  Parameters Are:                   *
 *  title           - Title To Appear At The Top Of The Window              *
 *  width           - Width Of The GL Window Or Fullscreen Mode             *
 *  height          - Height Of The GL Window Or Fullscreen Mode            *
 *  fullscreenflag  - Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)   */
 
BOOL CreateGLWindow(const char* title, int width, int height)
{
    static PIXELFORMATDESCRIPTOR pfd =              // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),              // Size Of This Pixel Format Descriptor
        1,                                          // Version Number
        PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
        PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                           // Must Support Double Buffering
        PFD_TYPE_RGBA,                              // Request An RGBA Format
        32,                                         // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                           // Color Bits Ignored
        0,                                          // No Alpha Buffer
        0,                                          // Shift Bit Ignored
        0,                                          // No Accumulation Buffer
        0, 0, 0, 0,                                 // Accumulation Bits Ignored
        24,                                         // 16Bit Z-Buffer (Depth Buffer)  
        0,                                          // No Stencil Buffer
        0,                                          // No Auxiliary Buffer
        PFD_MAIN_PLANE,                             // Main Drawing Layer
        0,                                          // Reserved
        0, 0, 0                                     // Layer Masks Ignored
    };

    GLuint      PixelFormat;                // Holds The Results After Searching For A Match
    WNDCLASS    wc;                         // Windows Class Structure
    DWORD       dwExStyle;                  // Window Extended Style
    DWORD       dwStyle;                    // Window Style
    RECT        WindowRect;                 // Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left     =   (long)0;        // Set Left Value To 0
    WindowRect.right    =   (long)width;    // Set Right Value To Requested Width
    WindowRect.top      =   (long)0;        // Set Top Value To 0
    WindowRect.bottom   =   (long)height;   // Set Bottom Value To Requested Height

    hInstance           = GetModuleHandle(NULL);                // Grab An Instance For Our Window
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc      = (WNDPROC) WndProc;                    // WndProc Handles Messages
    wc.cbClsExtra       = 0;                                    // No Extra Window Data
    wc.cbWndExtra       = 0;                                    // No Extra Window Data
    wc.hInstance        = hInstance;                            // Set The Instance
    wc.hIcon            = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
    wc.hbrBackground    = NULL;                                 // No Background Required For GL
    wc.lpszMenuName     = NULL;                                 // We Don't Want A Menu
    wc.lpszClassName    = "OpenGL";                                 // Set The Class Name

    if (!RegisterClass(&wc))                                    // Attempt To Register The Window Class
    {
        MSG_DBG("Failed To Register The Window Class.");
        return FALSE;                                           // Return FALSE
    }
    
    dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Window Extended Style
    dwStyle=WS_OVERLAPPEDWINDOW;                            // Windows Style
    

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);         // Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd=CreateWindowEx(  dwExStyle,                          // Extended Style For The Window
                                wc.lpszClassName,                       // Class Name
                                title,                              // Window Title
                                dwStyle |                           // Defined Window Style
                                WS_CLIPSIBLINGS |                   // Required Window Style
                                WS_CLIPCHILDREN,                    // Required Window Style
                                0, 0,                               // Window Position
                                WindowRect.right-WindowRect.left,   // Calculate Window Width
                                WindowRect.bottom-WindowRect.top,   // Calculate Window Height
                                NULL,                               // No Parent Window
                                NULL,                               // No Menu
                                hInstance,                          // Instance
                                NULL)))                             // Dont Pass Anything To WM_CREATE
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Window Creation Error.");
        return FALSE;                               // Return FALSE
    }
    
    if (!(hDC=GetDC(hWnd)))                         // Did We Get A Device Context?
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Can't Create A GL Device Context.");
        return FALSE;                               // Return FALSE
    }

    if (!(PixelFormat=ChoosePixelFormat(hDC, &pfd))) // Did Windows Find A Matching Pixel Format?
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Can't Find A Suitable PixelFormat.");
        return FALSE;                               // Return FALSE
    }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd))     // Are We Able To Set The Pixel Format?
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Can't Set The PixelFormat.");
        return FALSE;                               // Return FALSE
    }

    if (!(hRC=wglCreateContext(hDC)))               // Are We Able To Get A Rendering Context?
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Can't Create A GL Rendering Context.");
        return FALSE;                               // Return FALSE
    }

    if(!wglMakeCurrent(hDC, hRC))                   // Try To Activate The Rendering Context
    {
        KillGLWindow();                             // Reset The Display
        MSG_DBG("Can't Activate The GL Rendering Context.");
        return FALSE;                               // Return FALSE 
    }
    return TRUE;                                    // Success
}

