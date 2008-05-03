/**
 * Mupen64 - main.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
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
**/

#include <specific.h>
#include <stdlib.h>
HWND hwnd;

/* This is MUPEN64's main entry point. It contains code that is common
 * to both the gui and non-gui versions of mupen64. See
 * gui subdirectories for the gui-specific code.
 * if you want to implement an interface, you should look here
 */

/** function prototypes **/
static void *emulationThread( void *_arg );
static void sighandler( int signal, int signalfpe ); // signal handler

/** globals **/
HANDLE g_EmulationThread = 0;      // core thread handle


/** emulation **/
/* startEmulation
 *  Begins emulation thread
 */
void startEmulation(void)
{
    const char *gfx_plugin = NULL,
               *audio_plugin = NULL,
           *input_plugin = NULL,
           *RSP_plugin = NULL;

    // make sure rom is loaded before running
    if(!rom)
    {
        alert_message(tr("There is no Rom loaded."));
        return;
    }

    // make sure all plugins are specified before running
    if(g_GfxPlugin)
        gfx_plugin = plugin_name_by_filename(g_GfxPlugin);
    else
        gfx_plugin = plugin_name_by_filename(config_get_string("Gfx Plugin", ""));

    if(!gfx_plugin)
    {
        alert_message(tr("No graphics plugin specified."));
        return;
    }

    if(g_AudioPlugin)
        audio_plugin = plugin_name_by_filename(g_AudioPlugin);
    else
        audio_plugin = plugin_name_by_filename(config_get_string("Audio Plugin", ""));

    if(!audio_plugin)
    {
        alert_message(tr("No audio plugin specified."));
        return;
    }

    if(g_InputPlugin)
        input_plugin = plugin_name_by_filename(g_InputPlugin);
    else
        input_plugin = plugin_name_by_filename(config_get_string("Input Plugin", ""));

    if(!input_plugin)
    {
        alert_message(tr("No input plugin specified."));
        return;
    }

    if(g_RspPlugin)
        RSP_plugin = plugin_name_by_filename(g_RspPlugin);
    else
        RSP_plugin = plugin_name_by_filename(config_get_string("RSP Plugin", ""));

    if(!RSP_plugin)
    {
        alert_message(tr("No RSP plugin specified."));
        return;
    }

    // in nogui mode, just start the emulator in the main thread
    if(!g_GuiEnabled)
    {
        emulationThread(NULL);
    }
    else if(!g_EmulationThread)
    {
        // spawn emulation thread
        if((g_EmulationThread = CreateThread(NULL, 0, emulationThread, NULL, CREATE_SUSPENDED, NULL)) == 0)
        {
            g_EmulationThread = 0;
            alert_message(tr("Couldn't spawn core thread!"));
            return;
        }
        ResumeThread(g_EmulationThread);
        info_message(tr("Emulation started (PID: %d)"), g_EmulationThread);
    }

}

void stopEmulation(void)
{
    if(g_EmulationThread || g_EmulatorRunning)
    {
        info_message(tr("Stopping emulation."));
        rompause = 0;
        stop_it();

        Sleep(500);
        if(g_EmulationThread)
            TerminateThread(g_EmulationThread, 0);

        g_EmulatorRunning = 0;

        info_message(tr("Emulation stopped."));
    }
}


/*********************************************************************************************************
* emulation thread - runs the core
*/
static void * emulationThread( void *_arg )
{
    const char *gfx_plugin = NULL, *audio_plugin = NULL, *input_plugin = NULL, *RSP_plugin = NULL;

    signal( SIGSEGV, sighandler );
    signal( SIGILL, sighandler );
    signal( SIGFPE, sighandler );

    g_EmulatorRunning = 1;
    g_LimitFPS = config_get_bool("LimitFPS", TRUE);

    // if emu mode wasn't specified at the commandline, set from config file
    if(!g_EmuMode)
        dynacore = config_get_number( "Core", CORE_DYNAREC );

    no_audio_delay = config_get_bool("NoAudioDelay", FALSE);
    no_compiled_jump = config_get_bool("NoCompiledJump", FALSE);
    
    char *_putenv = (char *)malloc(256);
    snprintf( _putenv, 256, "SDL_WINDOWID=%i", (int)GetVideo());
    SDL_putenv(_putenv);

    // init sdl
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetVideoMode  (640, 480, 0, NULL);

    SDL_ShowCursor(0);
    SDL_EnableKeyRepeat(0, 0);

    SDL_SetEventFilter(sdl_event_filter);
    SDL_EnableUNICODE(1);

    /* Determine which plugins to use:
     *  -If valid plugin was specified at the commandline, use it
     *  -Else, get plugin from config. NOTE: gui code must change config if user switches plugin in the gui)
     */
    if(g_GfxPlugin)
        gfx_plugin = plugin_name_by_filename(g_GfxPlugin);
    else
        gfx_plugin = plugin_name_by_filename(config_get_string("Gfx Plugin", ""));

    if(g_AudioPlugin)
        audio_plugin = plugin_name_by_filename(g_AudioPlugin);
    else
        audio_plugin = plugin_name_by_filename(config_get_string("Audio Plugin", ""));

    if(g_InputPlugin)
        input_plugin = plugin_name_by_filename(g_InputPlugin);
    else
        input_plugin = plugin_name_by_filename(config_get_string("Input Plugin", ""));

    if(g_RspPlugin)
        RSP_plugin = plugin_name_by_filename(g_RspPlugin);
    else
        RSP_plugin = plugin_name_by_filename(config_get_string("RSP Plugin", ""));

    // initialize memory, and do byte-swapping if it's not been done yet
    if (g_MemHasBeenBSwapped == 0)
    {
        init_memory(1);
        g_MemHasBeenBSwapped = 1;
    }
    else
    {
        init_memory(0);
    }

    // load the plugins and attach the ROM to them
    plugin_load_plugins(gfx_plugin, audio_plugin, input_plugin, RSP_plugin);
    ShowVideo();
    romOpen_gfx();
    romOpen_audio();
    romOpen_input();

    if (g_Fullscreen)
        changeWindow();

#ifdef WITH_LIRC
    lircStart();
#endif // WITH_LIRC

#ifdef DBG
    if( g_DebuggerEnabled )
        init_debugger();
#endif
    // load cheats for the current rom
    cheat_load_current_rom();

    go();   /* core func */

#ifdef WITH_LIRC
    lircStop();
#endif // WITH_LIRC

    romClosed_RSP();
    romClosed_input();
    romClosed_audio();
    romClosed_gfx();
    closeDLL_RSP();
    closeDLL_input();
    closeDLL_audio();
    closeDLL_gfx();
    free_memory();

    // clean up
    g_EmulationThread = 0;
    
    HideVideo();
    KillWindow();
    InitWindow();

    SDL_Quit();

    if (g_Filename != 0)
    {
        // the following doesn't work - it wouldn't exit immediately but when the next event is
        // recieved (i.e. mouse movement)
/*      gdk_threads_enter();
        gtk_main_quit();
        gdk_threads_leave();*/
    }

    return NULL;
}

/*********************************************************************************************************
* signal handler
*/
static void sighandler(int signal, int fpesignal)
{
    switch( signal )
    {
        case SIGSEGV:
            alert_message(tr("The core thread recieved a SIGSEGV signal.\n"
                            "This means it tried to access protected memory.\n"
                            "This nearly always indicates a bug in the code.\n"
                            "Please report the bug at the Issue Tracker.\n"
                            "http://code.google.com/p/mupen64plus/issues"));
            printf( "SIGSEGV in core thread caught\n" );
            break;
        case SIGILL:
            printf( "SIGILL in core thread caught\n" );
            break;
        case SIGFPE:
            printf( "SIGFPE in core thread caught:\n" );
            switch( fpesignal )
            {
                case _FPE_INVALID:          printf( "\tfloating point invalid operation\n" ); break;
                case _FPE_DENORMAL:         printf( "\tfloating point denormal operation\n" ); break;
                case _FPE_ZERODIVIDE:       printf( "\tfloating point division by zero\n" ); break;
                case _FPE_OVERFLOW:         printf( "\tfloating point overflow\n" ); break;
                case _FPE_UNDERFLOW:        printf( "\tfloating point underflow\n" ); break;
                case _FPE_INEXACT:          printf( "\tfloating point inexact result\n" ); break;
                case _FPE_UNEMULATED:       printf( "\tfloating point unemulated operation\n" ); break;
                case _FPE_SQRTNEG:          printf( "\tfloating point negative square root\n" ); break;
                case _FPE_STACKOVERFLOW:    printf( "\tfloating point stack overflow\n" ); break;
                case _FPE_STACKUNDERFLOW:   printf( "\tfloating point stack udnerflow\n" ); break;
                case _FPE_EXPLICITGEN:      printf( "\tsomeone rose SIGFPE on explicitly\n" ); break;
            }
            break;
        default:
            printf( "Signal number %d in core thread caught:\n", signal );
    }
    g_EmulationThread = 0;
    g_EmulatorRunning = 0;
}

HINSTANCE hinst;
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* command_line, int show_command)
{
    if (!InitInstance()) 
        fprintf(stderr,"Win32 Error: Unable to initalize instance.\n");
 
    if (!InitWindow()) 
        fprintf(stderr,"Win32 Error: Unable to initalize instance.\n");
    
    char *_putenv = (char *)malloc(256);
    snprintf( _putenv, 256, "SDL_WINDOWID=%i", (int)GetVideo());
    SDL_putenv(_putenv);

    fflush(stderr);
    int    argc;
    char** argv;
    char*  arg;
    int    index;
    int    result;
    hinst = instance;
    
    argc = 1;
    arg  = command_line;
    
    while (arg[0] != 0) 
    {
        while (arg[0] != 0 && arg[0] == ' ') 
        {
            arg++;
        }
        if (arg[0] != 0) 
        {
            argc++;
            while (arg[0] != 0 && arg[0] != ' ') 
            {
                arg++;
            }
        }
    }    
    argv = (char**)malloc(argc * sizeof(char*));
    arg = command_line;
    index = 1;
    while (arg[0] != 0) 
    {
        while (arg[0] != 0 && arg[0] == ' ') 
        {
            arg++;
        }
        if (arg[0] != 0) 
        {
            argv[index] = arg;
            index++;
            while (arg[0] != 0 && arg[0] != ' ') 
            {
                arg++;
            }
            if (arg[0] != 0) 
            {
                arg[0] = 0;    
                arg++;
            }
        }
    }    

    // put the program name into argv[0]
    char filename[_MAX_PATH];
    GetModuleFileName(NULL, filename, _MAX_PATH);
    argv[0] = filename;

    // call the user specified main function
    result = main(argc, (const char **)argv);
    free(argv);
    return result;
}
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) 
    {
        default:
            return( DefWindowProc( hWnd, uMsg, wParam, lParam ));
    }
    return( DefWindowProc( hWnd, uMsg, wParam, lParam ));
}
void ErrorExit(char * lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code
    fprintf(stderr,"Reporting error in %s...\n", lpszFunction);
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL);

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    sprintf(lpDisplayBuf, "%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}


BOOL InitInstance() 
{ 
    WNDCLASSEX wcx;

    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = MainWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hinst;
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    if(!wcx.hIcon) ErrorExit("IDI_APPLICATION"); 
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    if(!wcx.hCursor) ErrorExit("LoadCursor"); 
    wcx.hbrBackground = GetStockObject(BLACK_BRUSH);
    if(!wcx.hbrBackground) ErrorExit("GetStockObject"); 
    wcx.lpszMenuName =  "MainMenu";
    wcx.lpszClassName = "MainWClass";
    //wcx.hIconSm = LoadImage(hinstance, MAKEINTRESOURCE(5), IMAGE_ICON,  GetSystemMetrics(SM_CXSMICON),  GetSystemMetrics(SM_CYSMICON),  LR_DEFAULTCOLOR); 
    wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if(!wcx.hIconSm) ErrorExit("LoadImage");
 
    if(!RegisterClassEx(&wcx)) ErrorExit("RegisterClassEx");
} 

HWND InitWindow() 
{ 
    hwnd = CreateWindow("MainWClass", "Mupen64Plus for Windows", WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, (HWND) NULL, (HMENU) NULL, hinst, (LPVOID) NULL);
 
    if (!hwnd) ErrorExit("CreateWindow");

    ShowWindow(hwnd, 0);
    UpdateWindow(hwnd);
    return hwnd;
 
}
BOOL KillWindow()
{
    DestroyWindow(hwnd);
}
HWND ShowVideo()
{
    ShowWindow(hwnd, 1);
    UpdateWindow(hwnd);
    return hwnd;
}
HWND HideVideo()
{
    ShowWindow(hwnd, 0);
    UpdateWindow(hwnd);
    return hwnd;
}
HWND GetVideo()
{
    return hwnd;
}

int file_exists(const char *fileName)
{
    DWORD fileAttr;

    fileAttr = GetFileAttributesA(fileName);
    if (0xFFFFFFFF == fileAttr)
        return FALSE;
    return TRUE;
}


/** setPaths
 *  setup paths to config/install/screenshot directories. The config dir is the dir where all
 *  user config information is stored, e.g. mupen64plus.conf, save files, and plugin conf files.
 *  The install dir is where mupen64plus looks for common files, e.g. plugins, icons, language
 *  translation files.
 */
static void setPaths(void)
{
    char buf[PATH_MAX], buf2[PATH_MAX];
    
    strncpy(g_ConfigDir, gethomedir(), PATH_MAX);
    
    fprintf(stderr, "config dir: %s\n", g_ConfigDir);
    
    if (strlen(g_ConfigDir) == 0)
    {
        strncpy(g_ConfigDir, getexedir(), PATH_MAX);
        strncat(g_ConfigDir, dirsep, PATH_MAX-strlen(g_ConfigDir));
        strncat(g_ConfigDir, "config", PATH_MAX-strlen(g_ConfigDir));
        strncat(g_ConfigDir, dirsep, PATH_MAX-strlen(g_ConfigDir));
    }

    // if the home dir is not found, create it
    if(!isdir(g_ConfigDir))
    {
        printf("Creating %s to store user data\n", g_ConfigDir);
        if(mkdirwp(g_ConfigDir, 0755) != 0)
        {
            printf("Error: Could not create %s: ", g_ConfigDir);
            perror(NULL);
            exit(errno);
        }

            // create save subdir
        strncpy(buf, g_ConfigDir, PATH_MAX);
        strncat(buf, dirsep, PATH_MAX - strlen(buf));
        strncat(buf, "save", PATH_MAX - strlen(buf));
        if(mkdirwp(buf, 0755) != 0)
        {
                // report error, but don't exit
            printf("Warning: Could not create %s: %s", buf, strerror(errno));
        }

            // create screenshots subdir
        strncpy(buf, g_ConfigDir, PATH_MAX);
        strncat(buf, dirsep, PATH_MAX - strlen(buf));
        strncat(buf, "screenshots", PATH_MAX - strlen(buf));
        if(mkdirwp(buf, 0755) != 0)
        {
                // report error, but don't exit
            printf("Warning: Could not create %s: %s", buf, strerror(errno));
        }
    }
    // make sure config dir has a '/' on the end.
    if(g_ConfigDir[strlen(g_ConfigDir)-1] != dirsep[0])
        strncat(g_ConfigDir, dirsep, 1);

    // if install dir was not specified at the commandline, look for it in the default location
    if(strlen(g_InstallDir) == 0)
    {
        memcpy(g_InstallDir, getexedir(), PATH_MAX);
        fprintf(stderr, "EXE dir: %s\n", buf);
        
        // if install dir is not in the default location, try the same dir as the binary
        if(!isdir(g_InstallDir))
        {
            memset(buf,0,PATH_MAX);
            strncpy(buf,getexedir(),PATH_MAX);

            if(buf)
            {
                dirname(buf);
                strncpy(g_InstallDir, buf, PATH_MAX);

                strncat(buf, dirsep, PATH_MAX - strlen(buf));
                strncat(buf, "config", PATH_MAX - strlen(buf));
                strncat(buf, dirsep, PATH_MAX - strlen(buf));
                strncat(buf, "mupen64plus.conf", PATH_MAX - strlen(buf));
                if(!isfile(buf))
                {
                    // try cwd as last resort
                    getcwd(g_InstallDir, PATH_MAX);
                }
            }
            else
            {
                // try cwd as last resort
                getcwd(g_InstallDir, PATH_MAX);
            }
        }
    }

    // make sure install dir has a '/' on the end.
    if(g_InstallDir[strlen(g_InstallDir)-1] != dirsep[0])
        strncat(g_InstallDir, dirsep, PATH_MAX - strlen(g_InstallDir));

    // make sure install dir is valid 
    strncpy(buf, g_InstallDir, PATH_MAX);
    strncat(buf, "config", PATH_MAX - strlen(buf));
    strncat(buf, dirsep, PATH_MAX - strlen(buf));
    strncat(buf, "mupen64plus.conf", PATH_MAX - strlen(buf));
    if(!file_exists(buf))
    {
        printf("Could not locate valid install directory\n");
    }

    // check user config dir for mupen64plus.conf file. If it's not there, copy all
    // config files from install dir over to user dir.
    strncpy(buf, g_ConfigDir, PATH_MAX);
    strncat(buf, "mupen64plus.conf", PATH_MAX - strlen(buf));
    if(!file_exists(buf))
    {
        DIR *dir;
        struct dirent *entry;

        strncpy(buf, g_InstallDir, PATH_MAX);
        strncat(buf, "config", PATH_MAX - strlen(buf));
        dir = opendir(buf);

        // should never hit this error because of previous checks
        if(!dir)
        {
            perror(buf);
            return;
        }

        while((entry = readdir(dir)) != NULL)
        {
            strncpy(buf, g_InstallDir, PATH_MAX);
            strncat(buf, "config", PATH_MAX - strlen(buf));
            strncat(buf, dirsep, PATH_MAX - strlen(buf));
            strncat(buf, entry->d_name, PATH_MAX - strlen(buf));

            // only copy regular files
            if(isfile(buf))
            {
                strncpy(buf2, g_ConfigDir, PATH_MAX);
                strncat(buf2, entry->d_name, PATH_MAX - strlen(buf2));
                                
                printf("Copying %s to %s\n", buf, g_ConfigDir);
                if(copyfile(buf, buf2) != 0)
                    printf("Error copying file\n");
            }
        }

        closedir(dir);
    }

    // set screenshot dir if it wasn't specified by the user
    if(strlen(g_SshotDir) == 0)
    {
        snprintf(g_SshotDir, PATH_MAX, "%sscreenshots%c", g_ConfigDir, dirsep[0]);
        if(!isdir(g_SshotDir))
        {
            printf("Warning: Could not find screenshot dir: %s\n", g_SshotDir);
            g_SshotDir[0] = '\0';
        }
    }

    // make sure screenshots dir has a '/' on the end.
    if(g_SshotDir[strlen(g_SshotDir)-1] != dirsep[0])
        strncat(g_SshotDir, dirsep, PATH_MAX - strlen(g_SshotDir));
}
