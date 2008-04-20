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

/** function prototypes **/
static void *emulationThread( void *_arg );
static void sighandler( int signal, siginfo_t *info, void *context ); // signal handler

/** globals **/
pthread_t   g_EmulationThread = 0;      // core thread handle


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
    g_LimitFPS = config_get_bool("LimitFPS", TRUE);

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

