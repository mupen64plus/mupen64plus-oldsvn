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

/* This is MUPEN64's main entry point. It contains code that is common
 * to both the gui and non-gui versions of mupen64. See
 * gui subdirectories for the gui-specific code.
 * if you want to implement an interface, you should look here
 */

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h> // POSIX Thread library
#include <signal.h> // signals
#include <getopt.h> // getopt_long
#include <libgen.h> // basename, dirname

#include "main.h"
#include "version.h"
#include "winlnxdefs.h"
#include "config.h"
#include "plugin.h"
#include "rom.h"
#include "mupenIniApi.h"
#include "../r4300/r4300.h"
#include "../r4300/recomph.h"
#include "../memory/memory.h"
#include "savestates.h"
#include "vcr.h"
#include "vcr_compress.h"
#include "guifuncs.h" // gui-specific functions
#include "util.h"
#include "translate.h"
#include "volume.h"
#include "cheat.h"
#include "osd.h"

#ifdef DBG
#include <glib.h>
#include "../debugger/debugger.h"
#endif

#ifdef WITH_LIRC
#include "lirc.h"
#endif //WITH_LIRC

#include <SDL.h>

/** function prototypes **/
static void parseCommandLine(int argc, char **argv);
static void *emulationThread( void *_arg );
static void sighandler( int signal, siginfo_t *info, void *context ); // signal handler

/** globals **/
// TODO: Improve the auto-incrementing savestate system.
int         autoinc_slot = 0;
int         *autoinc_save_slot = &autoinc_slot;
int         g_Noask = 0;                // don't ask to force load on bad dumps
int         g_NoaskParam = 0;           // was --noask passed at the commandline?
int         g_MemHasBeenBSwapped = 0;   // store byte-swapped flag so we don't swap twice when re-playing game
pthread_t   g_EmulationThread = 0;      // core thread handle
int         g_EmulatorRunning = 0;      // need separate boolean to tell if emulator is running, since --nogui doesn't use a thread
            
char        *g_GfxPlugin = NULL;        // pointer to graphics plugin specified at commandline (if any)
char        *g_AudioPlugin = NULL;      // pointer to audio plugin specified at commandline (if any)
char        *g_InputPlugin = NULL;      // pointer to input plugin specified at commandline (if any)
char        *g_RspPlugin = NULL;        // pointer to rsp plugin specified at commandline (if any)

#ifdef NO_GUI
static int  g_GuiEnabled = 0;           // GUI enabled?
#else
static int  g_GuiEnabled = 1;           // GUI enabled?
#endif

static char g_ConfigDir[PATH_MAX] = {0};
static char g_InstallDir[PATH_MAX] = {0};
#ifdef DBG
static int  g_DebuggerEnabled = 0;      // wether the debugger is enabled or not
#endif
static int  g_Fullscreen = 0;           // fullscreen enabled?
static int  g_EmuMode = 0;              // emumode specified at commandline?
static char g_SshotDir[PATH_MAX] = {0}; // pointer to screenshot dir specified at commandline (if any)
static char *g_Filename = NULL;         // filename to load & run at startup (if given at command line)
static int  g_SpeedFactor = 100;        // percentage of nominal game speed at which emulator is running
static int  g_FrameAdvance = 0;         // variable to check if we pause on next frame
/*********************************************************************************************************
* exported gui funcs
*/
char *get_configpath()
{
    return g_ConfigDir;
}

char *get_installpath()
{
    return g_InstallDir;
}

char *get_savespath()
{
    static char path[PATH_MAX];
    strcpy(path, get_configpath());
    strcat(path, "save/");
    return path;
}

char *get_iconspath()
{
    static char path[PATH_MAX];
    strcpy(path, get_installpath());
    strcat(path, "icons/");
    return path;
}

char *get_iconpath(char *iconfile)
{
    static char path[PATH_MAX];
    strcpy(path, get_iconspath());
    strcat(path, iconfile);
    return path;
}

int gui_enabled(void)
{
    return g_GuiEnabled;
}

/*********************************************************************************************************
* timer functions
*/
static float VILimit = 60.0;
static double VILimitMilliseconds = 1000.0/60.0;

static int GetVILimit(void)
{
    switch (ROM_HEADER->Country_code&0xFF)
    {
        // PAL codes
        case 0x44:
        case 0x46:
        case 0x49:
        case 0x50:
        case 0x53:
        case 0x55:
        case 0x58:
        case 0x59:
            return 50;
            break;

        // NTSC codes
        case 0x37:
        case 0x41:
        case 0x45:
        case 0x4a:
            return 60;
            break;

        // Fallback for unknown codes
        default:
            return 60;
    }
}

static void InitTimer(void)
{
    VILimit = GetVILimit();
    VILimitMilliseconds = (double) 1000.0/VILimit; 
    printf("init timer!\n");
}

static unsigned int gettimeofday_msec(void)
{
    struct timeval tv;
    unsigned int foo;
    
    gettimeofday(&tv, NULL);
    foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    return foo;
}

void new_frame(void)
{
}

void new_vi(void)
{
    int Dif;
    unsigned int CurrentFPSTime;
    static unsigned int LastFPSTime = 0;
    static unsigned int CounterTime = 0;
    static unsigned int CalculatedTime ;
    static int VI_Counter = 0;

    double AdjustedLimit = VILimitMilliseconds * 100.0 / g_SpeedFactor;  // adjust for selected emulator speed
    int time;

    start_section(IDLE_SECTION);
    VI_Counter++;

    if(LastFPSTime == 0)
    {
        LastFPSTime = gettimeofday_msec();
        CounterTime = gettimeofday_msec();
        return;
    }
    CurrentFPSTime = gettimeofday_msec();
    
    Dif = CurrentFPSTime - LastFPSTime;
    
    if (Dif < AdjustedLimit) 
    {
        CalculatedTime = CounterTime + AdjustedLimit * VI_Counter;
        time = (int)(CalculatedTime - CurrentFPSTime);
        if (time > 0)
        {
            usleep(time * 1000);
        }
        CurrentFPSTime = CurrentFPSTime + time;
    }

    if (CurrentFPSTime - CounterTime >= 1000.0 ) 
    {
        CounterTime = gettimeofday_msec();
        VI_Counter = 0 ;
    }
    
    LastFPSTime = CurrentFPSTime ;
    end_section(IDLE_SECTION);
    if (g_FrameAdvance) {
        rompause = 1;
        g_FrameAdvance = 0;
    }
}

int open_rom( const char *filename )
{
    int rc;

    if(g_EmulationThread)
    {
        if(!confirm_message(tr("Emulation is running. Do you want to\nstop it and load the selected rom?")))
        {
            return -1;
        }
        stopEmulation();
    }

    if(ROM_HEADER)
    {
        free(ROM_HEADER);
        ROM_HEADER = NULL;
    }

    if(rom)
    {
        free(rom);
        rom = NULL;
    }

    // clear Byte-swapped flag, since ROM is now deleted
    g_MemHasBeenBSwapped = 0;

    if(!fill_header(filename))
    {
        alert_message(tr("Couldn't load Rom!"));
        return -2;
    }

    if((rc = rom_read(filename)) != 0)
    {
        // rc of -3 means rom file was a hack or bad dump and the user did not want to load it.
        if(rc == -3)
            info_message(tr("Rom closed."));
        else
            alert_message(tr("Couldn't load Rom!"));

        return -3;
    }
    InitTimer();

    return 0;
}

int close_rom(void)
{
    if(g_EmulationThread)
    {
        if(!confirm_message(tr("Emulation is running. Do you want to\nstop it and load a rom?")))
        {
            return -1;
        }
        stopEmulation();
    }

    if(ROM_HEADER)
    {
        free(ROM_HEADER);
        ROM_HEADER = NULL;
    }

    if(rom)
    {
        free(rom);
        rom = NULL;
    }

    // clear Byte-swapped flag, since ROM is now deleted
    g_MemHasBeenBSwapped = 0;
    info_message(tr("Rom closed."));

    return 0;
}

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
        if(pthread_create(&g_EmulationThread, NULL, emulationThread, NULL) != 0)
        {
            g_EmulationThread = 0;
            alert_message(tr("Couldn't spawn core thread!"));
            return;
        }
        pthread_detach(g_EmulationThread);
        info_message(tr("Emulation started (PID: %d)"), g_EmulationThread);
    }
    // if emulation is already running, but it's paused, unpause it
    else if(rompause)
    {
        pauseContinueEmulation();
    }

}

void stopEmulation(void)
{
    if(g_EmulationThread || g_EmulatorRunning)
    {
        info_message(tr("Stopping emulation."));
        rompause = 0;
        stop_it();

        // wait until emulation thread is done before continuing
        if(g_EmulationThread)
            pthread_join(g_EmulationThread, NULL);

        g_EmulatorRunning = 0;

        info_message(tr("Emulation stopped."));
    }
}

int pauseContinueEmulation(void)
{
    static osd_message_t *msg = NULL;

    if (!g_EmulatorRunning)
        return 1;

    if (rompause)
    {
        info_message(tr("Emulation continued."));
        if(msg)
        {
            osd_delete_message(msg);
            msg = NULL;
        }
    }
    else
    {
        info_message(tr("Emulation paused."));

        if(msg)
            osd_delete_message(msg);

        msg = osd_new_message(tr("Pause"));
        osd_message_set_corner(msg, OSD_TOP_RIGHT);
        osd_message_set_static(msg);
    }
    
    rompause = !rompause;
    return rompause;
}

void screenshot(void)
{
    if(g_EmulationThread || g_EmulatorRunning)
        captureScreen(g_SshotDir);
}

/*********************************************************************************************************
* sdl event filter
*/
static int sdl_event_filter( const SDL_Event *event )
{
    static int SavedSpeedFactor = 100;
    char *event_str = NULL;

    switch( event->type )
    {
        // user clicked on window close button
        case SDL_QUIT:
            stopEmulation();
            break;
        case SDL_KEYDOWN:
            switch( event->key.keysym.sym )
            {
                case SDLK_F5:
                    savestates_job |= SAVESTATE;
                    break;

                case SDLK_F7:
                    savestates_job |= LOADSTATE;
                    break;

                case SDLK_ESCAPE:
                    stopEmulation();
                    break;

                case SDLK_RETURN:
                    // Alt+Enter toggles fullscreen
                    if(event->key.keysym.mod & (KMOD_LALT | KMOD_RALT))
                        changeWindow();
                    break;
                case SDLK_F10:
                    if (g_SpeedFactor > 10)
                    {
                        g_SpeedFactor -= 5;
                        printf("Emulator playback speed: %i%% \n", g_SpeedFactor);
                        setSpeedFactor(g_SpeedFactor);  // call to audio plugin
                    }
                    break;
                case SDLK_F11:
                    if (g_SpeedFactor < 300)
                    {
                        g_SpeedFactor += 5;
                        printf("Emulator playback speed: %i%% \n", g_SpeedFactor);
                        setSpeedFactor(g_SpeedFactor);  // call to audio plugin
                    }
                    break;
                case SDLK_F12:
                    screenshot();
                    break;

                default:
                    switch (event->key.keysym.unicode)
                    {
                        case '0':
                        //  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(slotDefaultItem), TRUE );
                            savestates_select_slot( 0 );
                            break;

                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            //gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(slotItem[event->key.keysym.unicode - '1']), TRUE );
                            savestates_select_slot( event->key.keysym.unicode - '0' );
                            break;
                        // Pause
                        case 'p':
                        case 'P':
                            pauseContinueEmulation();
                            g_FrameAdvance = 0;
                            break;
                        // volume mute/unmute
                        case 'm':
                        case 'M':
                            volMute();
                            break;
                        // increase volume
                        case ']':
                            volChange(2);
                            break;
                        // decrease volume
                        case '[':
                            volChange(-2);
                            break;
                        // fast-forward
                        case 'f':
                        case 'F':
                            SavedSpeedFactor = g_SpeedFactor;
                            g_SpeedFactor = 250;
                            setSpeedFactor(g_SpeedFactor);  // call to audio plugin
                            break;
                        // frame advance
                        case '/':
                        case '?':
                            g_FrameAdvance = 1;
                            rompause = 0;
                            break;
                        
                        // pass all other keypresses to the input plugin
                        
                        default:
                            keyDown( 0, event->key.keysym.sym );
                    }
            }
            return 0;
            break;

        case SDL_KEYUP:
            switch( event->key.keysym.sym )
            {
                case SDLK_ESCAPE:
                    break;
                case SDLK_f:
                    // cancel fast-forward
                    g_SpeedFactor = SavedSpeedFactor;
                    setSpeedFactor(g_SpeedFactor);  // call to audio plugin
                    break;
                default:
                    keyUp( 0, event->key.keysym.sym );
            }
            return 0;
            break;

        // if joystick action is detected, check if it's mapped to a special function
        case SDL_JOYAXISMOTION:
            // axis events have to be above a certain threshold to be valid
            if(event->jaxis.value > -15000 && event->jaxis.value < 15000)
                break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYHATMOTION:
            event_str = event_to_str(event);

            if(!event_str) return 0;

            if(strcmp(event_str, config_get_string("Joy Mapping Fullscreen", "")) == 0)
                changeWindow();
            else if(strcmp(event_str, config_get_string("Joy Mapping Stop", "")) == 0)
                stopEmulation();
            else if(strcmp(event_str, config_get_string("Joy Mapping Pause", "")) == 0)
                pauseContinueEmulation();
            else if(strcmp(event_str, config_get_string("Joy Mapping Save State", "")) == 0)
                savestates_job |= SAVESTATE;
            else if(strcmp(event_str, config_get_string("Joy Mapping Load State", "")) == 0)
                savestates_job |= LOADSTATE;
            else if(strcmp(event_str, config_get_string("Joy Mapping Increment Slot", "")) == 0)
                ;// TODO: Will add after reviewing statesave slot function (Issue 35)
            else if(strcmp(event_str, config_get_string("Joy Mapping Screenshot", "")) == 0)
                screenshot();
            else if(strcmp(event_str, config_get_string("Joy Mapping Mute", "")) == 0)
                volMute();
            else if(strcmp(event_str, config_get_string("Joy Mapping Decrease Volume", "")) == 0)
                volChange(-2);
            else if(strcmp(event_str, config_get_string("Joy Mapping Increase Volume", "")) == 0)
                volChange(2);

            free(event_str);
            return 0;
            break;
    }

    return 1;
}

/*********************************************************************************************************
* emulation thread - runs the core
*/
static void * emulationThread( void *_arg )
{
    const char *gfx_plugin = NULL,
               *audio_plugin = NULL,
           *input_plugin = NULL,
           *RSP_plugin = NULL;
    struct sigaction sa;

    // install signal handler
    memset( &sa, 0, sizeof( struct sigaction ) );
    sa.sa_sigaction = sighandler;
    sa.sa_flags = SA_SIGINFO;
    sigaction( SIGSEGV, &sa, NULL );
    sigaction( SIGILL, &sa, NULL );
    sigaction( SIGFPE, &sa, NULL );
    sigaction( SIGCHLD, &sa, NULL );

    g_EmulatorRunning = 1;

    // if emu mode wasn't specified at the commandline, set from config file
    if(!g_EmuMode)
        dynacore = config_get_number( "Core", CORE_DYNAREC );

    no_audio_delay = config_get_bool("NoAudioDelay", FALSE);
    no_compiled_jump = config_get_bool("NoCompiledJump", FALSE);

    // init sdl
    SDL_Init(SDL_INIT_VIDEO);
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
    romOpen_gfx();
    romOpen_audio();
    romOpen_input();

    // init on-screen display
    osd_init();

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

    osd_new_message("Starting Mupen64Plus...");
    go();   /* core func */

#ifdef WITH_LIRC
    lircStop();
#endif // WITH_LIRC

    osd_exit();

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
static void sighandler(int signal, siginfo_t *info, void *context)
{
    if( info->si_pid == g_EmulationThread )
    {
        switch( signal )
        {
            case SIGSEGV:
                alert_message(tr("The core thread recieved a SIGSEGV signal.\n"
                                "This means it tried to access protected memory.\n"
                                "Maybe you have set a wrong ucode for one of the plugins!"));
                printf( "SIGSEGV in core thread caught:\n" );
                printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
                printf( "\taddress = 0x%08lX\n", (unsigned long) info->si_addr );
#ifdef SEGV_MAPERR
                switch( info->si_code )
                {
                    case SEGV_MAPERR: printf( "                address not mapped to object\n" ); break;
                    case SEGV_ACCERR: printf( "                invalid permissions for mapped object\n" ); break;
                }
#endif
                break;
            case SIGILL:
                printf( "SIGILL in core thread caught:\n" );
                printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
                printf( "\taddress = 0x%08lX\n", (unsigned long) info->si_addr );
#ifdef ILL_ILLOPC
                switch( info->si_code )
                {
                    case ILL_ILLOPC: printf( "\tillegal opcode\n" ); break;
                    case ILL_ILLOPN: printf( "\tillegal operand\n" ); break;
                    case ILL_ILLADR: printf( "\tillegal addressing mode\n" ); break;
                    case ILL_ILLTRP: printf( "\tillegal trap\n" ); break;
                    case ILL_PRVOPC: printf( "\tprivileged opcode\n" ); break;
                    case ILL_PRVREG: printf( "\tprivileged register\n" ); break;
                    case ILL_COPROC: printf( "\tcoprocessor error\n" ); break;
                    case ILL_BADSTK: printf( "\tinternal stack error\n" ); break;
                }
#endif
                break;
            case SIGFPE:
                printf( "SIGFPE in core thread caught:\n" );
                printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
                printf( "\taddress = 0x%08lX\n", (unsigned long) info->si_addr );
                switch( info->si_code )
                {
                    case FPE_INTDIV: printf( "\tinteger divide by zero\n" ); break;
                    case FPE_INTOVF: printf( "\tinteger overflow\n" ); break;
                    case FPE_FLTDIV: printf( "\tfloating point divide by zero\n" ); break;
                    case FPE_FLTOVF: printf( "\tfloating point overflow\n" ); break;
                    case FPE_FLTUND: printf( "\tfloating point underflow\n" ); break;
                    case FPE_FLTRES: printf( "\tfloating point inexact result\n" ); break;
                    case FPE_FLTINV: printf( "\tfloating point invalid operation\n" ); break;
                    case FPE_FLTSUB: printf( "\tsubscript out of range\n" ); break;
                }
                break;
            default:
                printf( "Signal number %d in core thread caught:\n", signal );
                printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
        }
        pthread_cancel(g_EmulationThread);
        g_EmulationThread = 0;
        g_EmulatorRunning = 0;
    }
    else
    {
        printf( "Signal number %d caught:\n", signal );
        printf( "\terrno = %d (%s)\n", info->si_errno, strerror( info->si_errno ) );
        exit( EXIT_FAILURE );
    }
}

static void printUsage(const char *progname)
{
    char *str = strdup(progname);

    printf("Usage: %s [parameter(s)] rom\n"
           "\n"
           "Parameters:\n"
           "    --nogui         : do not display GUI\n"
           "    --fullscreen        : turn fullscreen mode on\n"
           "    --gfx (path)        : use gfx plugin given by (path)\n"
           "    --audio (path)      : use audio plugin given by (path)\n"
           "    --input (path)      : use input plugin given by (path)\n"
           "    --rsp (path)        : use rsp plugin given by (path)\n"
           "    --emumode (number)  : set emu mode to: 0=Interpreter 1=DynaRec 2=Pure Interpreter\n"
           "    --sshotdir (dir)    : set screenshot directory to (dir)\n"
           "    --configdir (dir)   : force config dir (must contain mupen64plus.conf)\n"
           "    --installdir (dir)  : force install dir (place to look for plugins, icons, lang, etc)\n"
           "    --noask         : don't ask to force load on bad dumps\n"
           "    -h, --help      : see this help message\n"
           "\n", basename(str));

    free(str);

    return;
}

/* parseCommandLine
 *  Parses commandline options and sets global variables accordingly
 */
void parseCommandLine(int argc, char **argv)
{
    int i;
    char *str = NULL;

    // option parsing vars
    int opt, option_index;
    enum
    {
        OPT_GFX = 1,
        OPT_AUDIO,
        OPT_INPUT,
        OPT_RSP,
        OPT_EMUMODE,
        OPT_SSHOTDIR,
        OPT_CONFIGDIR,
        OPT_INSTALLDIR,
        OPT_NOASK
    };
    struct option long_options[] =
    {
        {"nogui", no_argument, &g_GuiEnabled, FALSE},
        {"fullscreen", no_argument, &g_Fullscreen, TRUE},
        {"gfx", required_argument, NULL, OPT_GFX},
        {"audio", required_argument, NULL, OPT_AUDIO},
        {"input", required_argument, NULL, OPT_INPUT},
        {"rsp", required_argument, NULL, OPT_RSP},
        {"emumode", required_argument, NULL, OPT_EMUMODE},
        {"sshotdir", required_argument, NULL, OPT_SSHOTDIR},
        {"configdir", required_argument, NULL, OPT_CONFIGDIR},
        {"installdir", required_argument, NULL, OPT_INSTALLDIR},
        {"noask", no_argument, NULL, OPT_NOASK},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}    // last opt must be empty
    };
    char opt_str[] = "h";

    /* parse commandline options */
    while((opt = getopt_long(argc, argv, opt_str,
                 long_options, &option_index)) != -1)
    {
        switch(opt)
        {
            // if getopt_long returns 0, it already set the global for us, so do nothing
            case 0:
                break;
            case OPT_GFX:
                if(plugin_scan_file(optarg, PLUGIN_TYPE_GFX))
                {
                    g_GfxPlugin = optarg;
                }
                else
                {
                    printf("***Warning: GFX Plugin '%s' couldn't be loaded!\n", optarg);
                }
                break;
            case OPT_AUDIO:
                if(plugin_scan_file(optarg, PLUGIN_TYPE_AUDIO))
                {
                    g_AudioPlugin = optarg;
                }
                else
                {
                    printf("***Warning: Audio Plugin '%s' couldn't be loaded!\n", optarg);
                }
                break;
            case OPT_INPUT:
                if(plugin_scan_file(optarg, PLUGIN_TYPE_CONTROLLER))
                {
                    g_InputPlugin = optarg;
                }
                else
                {
                    printf("***Warning: Input Plugin '%s' couldn't be loaded!\n", optarg);
                }
                break;
            case OPT_RSP:
                if(plugin_scan_file(optarg, PLUGIN_TYPE_RSP))
                {
                    g_RspPlugin = optarg;
                }
                else
                {
                    printf("***Warning: RSP Plugin '%s' couldn't be loaded!\n", optarg);
                }
                break;
            case OPT_EMUMODE:
                i = atoi(optarg);
                if(i >= CORE_INTERPRETER && i <= CORE_PURE_INTERPRETER)
                {
                    g_EmuMode = TRUE;
                    dynacore = i;
                }
                else
                {
                    printf("***Warning: Invalid Emumode: %s\n", optarg);
                }
                break;
            case OPT_SSHOTDIR:
                if(isdir(optarg))
                    strncpy(g_SshotDir, optarg, PATH_MAX);
                else
                    printf("***Warning: Screen shot directory '%s' is not accessible or not a directory.\n", optarg);
                break;
            case OPT_CONFIGDIR:
                if(isdir(optarg))
                    strncpy(g_ConfigDir, optarg, PATH_MAX);
                else
                    printf("***Warning: Config directory '%s' is not accessible or not a directory.\n", optarg);
                break;
            case OPT_INSTALLDIR:
                if(isdir(optarg))
                    strncpy(g_InstallDir, optarg, PATH_MAX);
                else
                    printf("***Warning: Install directory '%s' is not accessible or not a directory.\n", optarg);
                break;
            case OPT_NOASK:
                g_Noask = g_NoaskParam = TRUE;
                break;
            // print help
            case 'h':
            case '?':
            default:
                printUsage(argv[0]);
                exit(1);
                break;
        }
    }

    // if there are still parameters left after option parsing, assume it's the rom filename
    if(optind < argc)
    {
        g_Filename = argv[optind];
    }

    // if executable name contains "_nogui", set g_GuiEnabled to FALSE.
    // This allows creation of a mupen64plus_nogui symlink to mupen64plus instead of passing --nogui
    // for backwards compatability with old mupen64_nogui program name.
    str = strdup(argv[0]);
    basename(str);
    if(strstr(str, "_nogui") != NULL)
    {
        g_GuiEnabled = FALSE;
    }
    free(str);
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

    // if the config dir was not specified at the commandline, look for ~/.mupen64plus dir
    if (strlen(g_ConfigDir) == 0)
    {
        strncpy(g_ConfigDir, getenv("HOME"), PATH_MAX);
        strncat(g_ConfigDir, "/.mupen64plus", PATH_MAX - strlen(g_ConfigDir));

        // if ~/.mupen64plus dir is not found, create it
        if(!isdir(g_ConfigDir))
        {
            printf("Creating %s to store user data\n", g_ConfigDir);
            if(mkdir(g_ConfigDir, (mode_t)0755) != 0)
            {
                printf("Error: Could not create %s: ", g_ConfigDir);
                perror(NULL);
                exit(errno);
            }

            // create save subdir
            strncpy(buf, g_ConfigDir, PATH_MAX);
            strncat(buf, "/save", PATH_MAX - strlen(buf));
            if(mkdir(buf, (mode_t)0755) != 0)
            {
                // report error, but don't exit
                printf("Warning: Could not create %s: %s", buf, strerror(errno));
            }

            // create screenshots subdir
            strncpy(buf, g_ConfigDir, PATH_MAX);
            strncat(buf, "/screenshots", PATH_MAX - strlen(buf));
            if(mkdir(buf, (mode_t)0755) != 0)
            {
                // report error, but don't exit
                printf("Warning: Could not create %s: %s", buf, strerror(errno));
            }
        }
    }

    // make sure config dir has a '/' on the end.
    if(g_ConfigDir[strlen(g_ConfigDir)-1] != '/')
        strncat(g_ConfigDir, "/", PATH_MAX - strlen(g_ConfigDir));

    // if install dir was not specified at the commandline, look for it in the default location
    if(strlen(g_InstallDir) == 0)
    {
        strncpy(g_InstallDir, PREFIX, PATH_MAX);
        strncat(g_InstallDir, "/share/mupen64plus/", PATH_MAX - strlen(g_InstallDir));
        
        // if install dir is not in the default location, try the same dir as the binary
        if(!isdir(g_InstallDir))
        {
            int n = readlink("/proc/self/exe", buf, PATH_MAX);

            if(n > 0)
            {
                buf[n] = '\0';
                dirname(buf);
                strncpy(g_InstallDir, buf, PATH_MAX);

                strncat(buf, "/config/mupen64plus.conf", PATH_MAX - strlen(buf));
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
    if(g_InstallDir[strlen(g_InstallDir)-1] != '/')
        strncat(g_InstallDir, "/", PATH_MAX - strlen(g_InstallDir));

    // make sure install dir is valid
    strncpy(buf, g_InstallDir, PATH_MAX);
    strncat(buf, "config/mupen64plus.conf", PATH_MAX - strlen(buf));
    if(!isfile(buf))
    {
        printf("Could not locate valid install directory\n");
        exit(1);
    }

    // check user config dir for mupen64plus.conf file. If it's not there, copy all
    // config files from install dir over to user dir.
    strncpy(buf, g_ConfigDir, PATH_MAX);
    strncat(buf, "mupen64plus.conf", PATH_MAX - strlen(buf));
    if(!isfile(buf))
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
            strncat(buf, "config/", PATH_MAX - strlen(buf));
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
        snprintf(g_SshotDir, PATH_MAX, "%sscreenshots/", g_ConfigDir);
        if(!isdir(g_SshotDir))
        {
            printf("Warning: Could not find screenshot dir: %s\n", g_SshotDir);
            g_SshotDir[0] = '\0';
        }
    }

    // make sure screenshots dir has a '/' on the end.
    if(g_SshotDir[strlen(g_SshotDir)-1] != '/')
        strncat(g_SshotDir, "/", PATH_MAX - strlen(g_SshotDir));
}

/*********************************************************************************************************
* main function
*/
int main(int argc, char *argv[])
{
#ifdef VCR_SUPPORT
    int i;
    const char *p;
#endif
    printf(" __  __                         __   _  _   ____  _             \n");  
    printf("|  \\/  |_   _ _ __   ___ _ __  / /_ | || | |  _ \\| |_   _ ___ \n");
    printf("| |\\/| | | | | '_ \\ / _ \\ '_ \\| '_ \\| || |_| |_) | | | | / __|  \n");
    printf("| |  | | |_| | |_) |  __/ | | | (_) |__   _|  __/| | |_| \\__ \\  \n");
    printf("|_|  |_|\\__,_| .__/ \\___|_| |_|\\___/   |_| |_|   |_|\\__,_|___/  \n");
    printf("             |_|         http://code.google.com/p/mupen64plus/  \n\n");                                              

    // allow gui subsystem to init and parse gui-specific commandline args first
    if(g_GuiEnabled)
        gui_init(&argc, &argv);

    parseCommandLine(argc, argv);
    setPaths();
    config_read();

#ifdef VCR_SUPPORT
    VCRComp_init();
    p = config_get_string( "VCR Video Codec", "XviD" );
    for (i = 0; i < VCRComp_numVideoCodecs(); i++)
    {
        if (!strcasecmp( VCRComp_videoCodecName( i ), p ))
        {
            VCRComp_selectVideoCodec( i );
            break;
        }
    }
    p = config_get_string( "VCR Audio Codec", VCRComp_audioCodecName( 0 ) );
    for (i = 0; i < VCRComp_numAudioCodecs(); i++)
    {
        if (!strcasecmp( VCRComp_audioCodecName( i ), p ))
        {
            VCRComp_selectAudioCodec( i );
            break;
        }
    }
#endif

    // init multi-language support
    tr_init();

    // look for plugins in the install dir and set plugin config dir
    plugin_scan_installdir();
    plugin_set_configdir(g_ConfigDir);

    /* TODO: autoinc_save_slot acts differently in the gui version than it does in the nogui version. Here, it's a bool, in nogui version, it's a pointer to the current autoinc_slot. Need to research this */
    *autoinc_save_slot = config_get_bool("AutoIncSaveSlot", FALSE);

    // if --noask was not specified at the commandline, try config file
    if(!g_NoaskParam)
        g_Noask = config_get_bool("No Ask", FALSE);

    /* TODO: nogui version does not use ini file */
    ini_openFile();

    cheat_read_config();

    // build gui, but do not display
    if(g_GuiEnabled)
        gui_build();

    // must be called after building gui
    info_message(tr("Config Dir: \"%s\", Install Dir: \"%s\""), g_ConfigDir, g_InstallDir);

    // only display gui if user wants it
    if(g_GuiEnabled)
        gui_display();

    // if rom file was specified, run it
    if (g_Filename)
    {
        if(open_rom(g_Filename) < 0 &&
           !g_GuiEnabled)
        {
            // cleanup and exit
            cheat_delete_all();
            ini_closeFile();
            plugin_delete_list();
            tr_delete_languages();
            config_delete();
            exit(1);
        }

        startEmulation();
    }
    // Rom file must be specified in nogui mode
    else if(!g_GuiEnabled)
    {
        alert_message("Rom file must be specified in nogui mode.");
        printUsage(argv[0]);

        // cleanup and exit
        cheat_delete_all();
        ini_closeFile();
        plugin_delete_list();
        tr_delete_languages();
        config_delete();
        exit(1);
    }

    // give control of this thread to the gui
    if(g_GuiEnabled)
        gui_main_loop();

    // cleanup and exit
    stopEmulation();
    config_write();
    ini_updateFile(1);
    cheat_write_config();
    cheat_delete_all();
    ini_closeFile();
    plugin_delete_list();
    tr_delete_languages();
    config_delete();

    return EXIT_SUCCESS;
}
