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

int file_exists(const char *fileName)
{
    FILE *stream;
    
    stream = fopen (fileName, "r");
    if (stream == NULL)
    {
        return (0);
    }
    
    fclose (stream);
    return (1);

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

    fprintf(stderr, "configdir: %s\n", g_ConfigDir);

    // if the config dir was not specified at the commandline, look for ~/.mupen64plus dir
    if (strlen(g_ConfigDir) == 0)
    {
        strncpy(g_ConfigDir, gethomedir(), PATH_MAX);

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
    }

    // make sure config dir has a '/' on the end.
    if(g_ConfigDir[strlen(g_ConfigDir)-1] != dirsep[0])
        strncat(g_ConfigDir, dirsep, PATH_MAX - strlen(g_ConfigDir));

    // if install dir was not specified at the commandline, look for it in the default location
    if(strlen(g_InstallDir) == 0)
    {
        strncpy(g_InstallDir, PREFIX, PATH_MAX);
        strncat(g_InstallDir, dirsep, PATH_MAX - strlen(g_InstallDir));
        strncat(g_InstallDir, "share", PATH_MAX - strlen(g_InstallDir));
        strncat(g_InstallDir, dirsep, PATH_MAX - strlen(g_InstallDir));
        strncat(g_InstallDir, "mupen64plus", PATH_MAX - strlen(g_InstallDir));
        strncat(g_InstallDir, dirsep, PATH_MAX - strlen(g_InstallDir));
        
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

