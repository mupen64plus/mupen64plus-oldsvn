/***************************************************************************
                          plugin.c  -  description
                             -------------------
    begin                : Fri Oct 18 2002
    copyright            : (C) 2002 by blight
    email                : blight@fuckmicrosoft.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>

#include "plugin.h"

#include <specific.h>

#ifdef GUI_SDL
# include "configdialog_sdl.h"
#elif defined( GUI_GTK )
# include "configdialog_gtk.h"
#endif

#include "Controller_1.1.h"

#ifdef GUI_GTK
# include <gtk/gtk.h>
#endif
#include "SDL.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* defines for the force feedback rumble support */
#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

static unsigned short button_bits[] = {
    0x0001,  // R_DPAD
    0x0002,  // L_DPAD
    0x0004,  // D_DPAD
    0x0008,  // U_DPAD
    0x0010,  // START_BUTTON
    0x0020,  // Z_TRIG
    0x0040,  // B_BUTTON
    0x0080,  // A_BUTTON
    0x0100,  // R_CBUTTON
    0x0200,  // L_CBUTTON
    0x0400,  // D_CBUTTON
    0x0800,  // U_CBUTTON
    0x1000,  // R_TRIG
    0x2000,  // L_TRIG
    0x4000,  // Mempak switch
    0x8000   // Rumblepak switch
};

static SController controller[4];   // 4 controllers
static int romopen = 0;         // is a rom opened
static char configdir[PATH_MAX] = {0};  // holds config dir path

static const char *button_names[] = {
    "DPad R",       // R_DPAD
    "DPad L",       // L_DPAD
    "DPad D",       // D_DPAD
    "DPad U",       // U_DPAD
    "Start",        // START_BUTTON
    "Z Trig",       // Z_TRIG
    "B Button",     // B_BUTTON
    "A Button",     // A_BUTTON
    "C Button R",   // R_CBUTTON
    "C Button L",   // L_CBUTTON
    "C Button D",   // D_CBUTTON
    "C Button U",   // U_CBUTTON
    "R Trig",       // R_TRIG
    "L Trig",       // L_TRIG
    "Mempak switch",
    "Rumblepak switch",
    "Y Axis",       // Y_AXIS
    "X Axis"        // X_AXIS
};

static int
get_button_num_by_name( const char *name )
{
    int i;

    for( i = 0; i < NUM_BUTTONS; i++ )
        if( !strncasecmp( name, button_names[i], strlen( button_names[i] ) ) )
        {
#ifdef _DEBUG
            printf( "%s, %d: name = %s, button = %d\n", __FILE__, __LINE__, name, i );
#endif
            return i;
        }

#ifdef _DEBUG
    printf( "%s, %d: button '%s' unknown\n", __FILE__, __LINE__, name );
#endif
    return -1;
}
/*
static SDLKey
get_key_by_name( const char *name )
{
    int i;

    for( i = 0; i < SDLK_LAST; i++ )
        if( !strncasecmp( name, SDL_GetKeyName( i ), strlen( SDL_GetKeyName( i ) ) ) )
        {
#ifdef _DEBUG
            printf( "%s, %d: name = %s, key = %d\n", __FILE__, __LINE__, name, i );
#endif
            return i;
        }

#ifdef _DEBUG
    printf( "%s, %d: key '%s' unknown\n", __FILE__, __LINE__, name );
#endif
    return SDLK_UNKNOWN;
}
*/
static int
get_hat_pos_by_name( const char *name )
{
    if( !strcasecmp( name, "up" ) )
        return SDL_HAT_UP;
    if( !strcasecmp( name, "down" ) )
        return SDL_HAT_DOWN;
    if( !strcasecmp( name, "left" ) )
        return SDL_HAT_LEFT;
    if( !strcasecmp( name, "right" ) )
        return SDL_HAT_RIGHT;
    return -1;
}

void read_configuration( void )
{
    FILE *f;
    int cont, plugged, plugin, mouse, i, b, dev;
    char line[200], device[200], key_a[200], key_b[200], button_a[200], button_b[200],
             axis[200], button[200], hat[200], hat_pos_a[200], hat_pos_b[200], mbutton[200];
    const char *p;
    char path[PATH_MAX];

    for( i = 0; i < 4; i++ )
    {
        controller[i].device = DEVICE_NONE;
        controller[i].control.Present = FALSE;
        controller[i].control.RawData = FALSE;
        controller[i].control.Plugin = PLUGIN_NONE;
        for( b = 0; b < 16; b++ )
        {
            controller[i].button[b].button = -1;
            controller[i].button[b].key = SDLK_UNKNOWN;
            controller[i].button[b].axis = -1;
            controller[i].button[b].hat = -1;
            controller[i].button[b].hat_pos = -1;
            controller[i].button[b].mouse = -1;
        }
        for( b = 0; b < 2; b++ )
        {
            controller[i].axis[b].button_a = controller[i].axis[b].button_b = -1;
            controller[i].axis[b].key_a = controller[i].axis[b].key_a = SDLK_UNKNOWN;
            controller[i].axis[b].axis = -1;
            controller[i].axis[b].hat = -1;
            controller[i].axis[b].hat_pos_a = -1;
            controller[i].axis[b].hat_pos_b = -1;
        }
    }

    path[0] = '\0';
    if(strlen(configdir) > 0)
        strncpy(path, configdir, PATH_MAX);
    strncat(path, "blight_input.conf", PATH_MAX - strlen(path));
    f = fopen( path, "r" );
    if( f == NULL )
    {
        fprintf( stderr, "["PLUGIN_NAME"]: Couldn't open blight_input.conf for reading: %s\n", strerror( errno ) );
        return;
    }
    while( !feof( f ) )
    {
        if( fgets( line, 200, f ) == NULL )
            break;
        if( line[0] == '\n' || line[0] == '\0' )
            continue;
        if( sscanf( line, "[controller %d]", &cont ) == 1 )
            continue;
        if( sscanf( line, "plugged=%d", &plugged ) == 1 )
        {
            controller[cont].control.Present = plugged;
            continue;
        }
        if( sscanf( line, "plugin=%d", &plugin ) == 1 )
        {
            controller[cont].control.Plugin = plugin;
            continue;
        }
        if( sscanf( line, "mouse=%d", &mouse ) == 1 )
        {
            controller[cont].mouse = mouse;
            continue;
        }
        if( sscanf( line, "device=%200s", device ) == 1 )
        {
            dev = DEVICE_NONE;
            if( !strcasecmp( device, "keyboard" ) )
                dev = DEVICE_KEYBOARD;
            else if( sscanf( device, "%d", &i ) == 1 )
                dev = i;
            controller[cont].device = dev;
            continue;
        }
        p = strchr( line, '=' );
        if( p )
        {
            int len = p - line;
            int num;

            strncpy( button, line, len );
            button[len] = '\0';
            p++;

            b = get_button_num_by_name( button );
            if( (b == X_AXIS) || (b == Y_AXIS) )
            {
                num = sscanf( p, "key( %s , %s ); button( %s , %s ); axis( %s ); hat( %s , %s , %s )",
                    key_a, key_b, button_a, button_b, axis, hat, hat_pos_a, hat_pos_b );

#ifdef _DEBUG
                printf( "%s, %d: num = %d, key_a = %s, key_b = %s, button_a = %s, button_b = %s, axis = %s, hat = %s, hat_pos_a = %s, hat_pos_b = %s\n", __FILE__, __LINE__, num,
                        key_a, key_b, button_a, button_b, axis, hat, hat_pos_a, hat_pos_b );
#endif
                if( sscanf( key_a, "%d", (int *)&controller[cont].axis[b - Y_AXIS].key_a ) != 1 )
                    controller[cont].axis[b - Y_AXIS].key_a = -1;
                if( sscanf( key_b, "%d", (int *)&controller[cont].axis[b - Y_AXIS].key_b ) != 1 )
                    controller[cont].axis[b - Y_AXIS].key_b = -1;
                if( sscanf( button_a, "%d", &controller[cont].axis[b - Y_AXIS].button_a ) != 1 )
                    controller[cont].axis[b - Y_AXIS].button_a = -1;
                if( sscanf( button_b, "%d", &controller[cont].axis[b - Y_AXIS].button_b ) != 1 )
                    controller[cont].axis[b - Y_AXIS].button_b = -1;
                if( sscanf( axis, "%d", &controller[cont].axis[b - Y_AXIS].axis ) != 1 )
                    controller[cont].axis[b - Y_AXIS].axis = -1;
                if( sscanf( hat, "%d", &controller[cont].axis[b - Y_AXIS].hat ) != 1 )
                    controller[cont].axis[b - Y_AXIS].hat = -1;
                controller[cont].axis[b - Y_AXIS].hat_pos_a = get_hat_pos_by_name( hat_pos_a );
                controller[cont].axis[b - Y_AXIS].hat_pos_b = get_hat_pos_by_name( hat_pos_b );
            }
            else
            {
                num = sscanf( p, "key( %s ); button( %s ); axis( %s ); hat( %s , %s ); mouse( %s )",
                        key_a,
                        button_a,
                        axis,
                        hat,
                        hat_pos_a,
                        mbutton );
#ifdef _DEBUG
                printf( "%s, %d: num = %d, key = %s, button = %s, axis = %s, hat = %s, hat_pos = %s, mbutton = %s\n", __FILE__, __LINE__, num, key_a, button_a, axis, hat, hat_pos_a, mbutton );
#endif
                num = sscanf( axis, "%d%c", &controller[cont].button[b].axis, (char *)&controller[cont].button[b].axis_dir );
                if( num != 2 )
                {
                    controller[cont].button[b].axis = -1;
                    controller[cont].button[b].axis_dir = 0;
                }
                else
                {
                    if( controller[cont].button[b].axis_dir == '+' )
                        controller[cont].button[b].axis_dir = 1;
                    else if( controller[cont].button[b].axis_dir == '-' )
                        controller[cont].button[b].axis_dir = -1;
                }
                if( sscanf( key_a, "%d", (int *)&controller[cont].button[b].key ) != 1 )
                    controller[cont].button[b].key = -1;
                if( sscanf( button_a, "%d", &controller[cont].button[b].button ) != 1 )
                    controller[cont].button[b].button = -1;
                if( sscanf( hat, "%d", &controller[cont].button[b].hat ) != 1 )
                    controller[cont].button[b].hat = -1;
                controller[cont].button[b].hat_pos = get_hat_pos_by_name( hat_pos_a );
                if( sscanf( mbutton, "%d", &controller[cont].button[b].mouse ) != 1 )
                    controller[cont].button[b].mouse = -1;
            }
            continue;
        }
        fprintf( stderr, "["PLUGIN_NAME"]: Unknown config line: %s\n", line );
    }
    fclose( f );
}

#define HAT_POS_NAME( hat )         \
       ((hat == SDL_HAT_UP) ? "Up" :        \
       ((hat == SDL_HAT_DOWN) ? "Down" :    \
       ((hat == SDL_HAT_LEFT) ? "Left" :    \
       ((hat == SDL_HAT_RIGHT) ? "Right" :  \
         "None"))))

int
write_configuration( void )
{
    FILE *f;
    int i, b;
    char cKey_a[100], cKey_b[100];
    char cButton_a[100], cButton_b[100], cAxis[100];
    char cHat[100];
    char cMouse[100];
    char path[PATH_MAX];

    path[0] = '\0';
    if(strlen(configdir) > 0)
        strncpy(path, configdir, PATH_MAX);
    strncat(path, "blight_input.conf", PATH_MAX - strlen(path));
    f = fopen( path, "w" );
    if( f == NULL )
    {
        fprintf( stderr, "["PLUGIN_NAME"]: Couldn't open blight_input.conf for writing: %s\n", strerror( errno ) );
        return -1;
    }

    for( i = 0; i < 4; i++ )
    {
        fprintf( f, "[controller %d]\n", i );
        fprintf( f, "plugged=%d\n", controller[i].control.Present );
        fprintf( f, "plugin=%d\n", controller[i].control.Plugin );
        fprintf( f, "mouse=%d\n", controller[i].mouse );
        if( controller[i].device == DEVICE_KEYBOARD )
            fprintf( f, "device=Keyboard\n" );
        else if( controller[i].device >= 0 )
            fprintf( f, "device=%d\n", controller[i].device );
        else
            fprintf( f, "device=None\n" );

        for( b = 0; b < 16; b++ )
        {
//          cKey_a = (controller[i].button[b].key == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( controller[i].button[b].key );
            if( controller[i].button[b].key >= 0 )
                sprintf( cKey_a, "%d", controller[i].button[b].key );
            else
                strcpy( cButton_a, "None" );

            if( controller[i].button[b].button >= 0 )
                sprintf( cButton_a, "%d", controller[i].button[b].button );
            else
                strcpy( cButton_a, "None" );

            if( controller[i].button[b].axis >= 0 )
                sprintf( cAxis, "%d%c", controller[i].button[b].axis, (controller[i].button[b].axis_dir == -1) ? '-' : '+' );
            else
                strcpy( cAxis, "None" );

            if( controller[i].button[b].hat >= 0 )
                sprintf( cHat, "%d", controller[i].button[b].hat );
            else
                strcpy( cHat, "None" );

            if( controller[i].button[b].mouse >= 0 )
                sprintf( cMouse, "%d", controller[i].button[b].mouse );
            else
                strcpy( cMouse, "None" );

            fprintf( f, "%s=key( %s ); button( %s ); axis( %s ); hat( %s , %s ); mouse( %s )\n", button_names[b],
                    cKey_a, cButton_a, cAxis, cHat, HAT_POS_NAME(controller[i].button[b].hat_pos), cMouse );
        }
        for( b = 0; b < 2; b++ )
        {
//          cKey_a = (controller[i].axis[b].key_a == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( controller[i].axis[b].key_a );
//          cKey_b = (controller[i].axis[b].key_b == SDLK_UNKNOWN) ? "None" : SDL_GetKeyName( controller[i].axis[b].key_b );
            if( controller[i].axis[b].key_a >= 0 )
                sprintf( cKey_a, "%d", controller[i].axis[b].key_a );
            else
                strcpy( cKey_a, "None" );
            if( controller[i].axis[b].key_b >= 0 )
                sprintf( cKey_b, "%d", controller[i].axis[b].key_b );
            else
                strcpy( cKey_b, "None" );

            if( controller[i].axis[b].button_a >= 0 )
                sprintf( cButton_a, "%d", controller[i].axis[b].button_a );
            else
                strcpy( cButton_a, "None" );

            if( controller[i].axis[b].button_b >= 0 )
                sprintf( cButton_b, "%d", controller[i].axis[b].button_b );
            else
                strcpy( cButton_b, "None" );

            if( controller[i].axis[b].axis >= 0 )
                sprintf( cAxis, "%d", controller[i].axis[b].axis );
            else
                strcpy( cAxis, "None" );

            if( controller[i].axis[b].hat >= 0 )
                sprintf( cHat, "%d", controller[i].axis[b].hat );
            else
                strcpy( cHat, "None" );

            fprintf( f, "%s=key( %s , %s ); button( %s , %s ); axis( %s ); hat( %s , %s , %s )\n", button_names[b+16],
                        cKey_a, cKey_b, cButton_a, cButton_b, cAxis, cHat, HAT_POS_NAME(controller[i].axis[b].hat_pos_a), HAT_POS_NAME(controller[i].axis[b].hat_pos_b) );
        }
        fprintf( f, "\n" );
    }

    fclose( f );
    return 0;
}

BYTE lastCommand[6];

struct ff_effect ffeffect[3];
struct ff_effect ffstrong[3];
struct ff_effect ffweak[3];

BYTE DataCRC( BYTE *Data, int iLenght )
{
    register BYTE Remainder = Data[0];

    int iByte = 1;
    BYTE bBit = 0;

    while( iByte <= iLenght )
    {
        BOOL HighBit = ((Remainder & 0x80) != 0);
        Remainder = Remainder << 1;

        Remainder += ( iByte < iLenght && Data[iByte] & (0x80 >> bBit )) ? 1 : 0;

        Remainder ^= (HighBit) ? 0x85 : 0;

        bBit++;
        iByte += bBit/8;
        bBit %= 8;
    }

    return Remainder;
}

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/
void
CloseDLL( void )
{
    printf( "["PLUGIN_NAME"]: Closing...\n" );
    write_configuration();
}

/******************************************************************
  Function: ControllerCommand
  Purpose:  To process the raw data that has just been sent to a
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none

  note:     This function is only needed if the DLL is allowing raw
            data, or the plugin is set to raw

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
void
ControllerCommand(int Control, BYTE *Command)
{
    BYTE *Data = &Command[5];
    struct input_event play;

    if (Control == -1)
        return;

    switch (Command[2])
    {
        case RD_GETSTATUS:
            /*printf( "Get status\n" );*/
            break;
        case RD_READKEYS:
            /*printf( "Read keys\n" );*/
            break;
        case RD_READPAK:
            /*printf( "Read pak\n" );*/
            if (controller[Control].control.Plugin == PLUGIN_RAW)
            {
                DWORD dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);
    
                if(( dwAddress >= 0x8000 ) && ( dwAddress < 0x9000 ) )
                    memset( Data, 0x80, 32 );
                else
                    memset( Data, 0x00, 32 );
    
                Data[32] = DataCRC( Data, 32 );
                break;
                }
        case RD_WRITEPAK:
            /*printf( "Write pak\n" );*/
            if (controller[Control].control.Plugin == PLUGIN_RAW)
            {
                DWORD dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);
                if( dwAddress == PAK_IO_RUMBLE && controller[Control].event_joystick != 0)
                {
                    if( *Data )
                    {
                        play.type = EV_FF;
                        play.code = ffeffect[Control].id;
                        play.value = 1;
    
                        if (write(controller[Control].event_joystick, (const void*) &play, sizeof(play)) == -1)
                            perror("Error starting rumble effect");
                    }
                    else
                    {
                        play.type = EV_FF;
                        play.code = ffeffect[Control].id;
                        play.value = 0;

                        if (write(controller[Control].event_joystick, (const void*) &play, sizeof(play)) == -1)
                            perror("Error stopping rumble effect");
                    }
                }
                Data[32] = DataCRC( Data, 32 );
            }
            break;
        case RD_RESETCONTROLLER:
            /*printf( "Reset controller\n" );*/
            break;
        case RD_READEEPROM:
            /*printf( "Read eeprom\n" );*/
            break;
        case RD_WRITEEPROM:
            /*printf( "Write eeprom\n" );*/
            break;
        }
}

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
#ifdef GUI_SDL
# include "SDL_ttf.h"
# include "arial.ttf.h" // arial font
# include <stdarg.h>

# define FONT_SIZEPT 15
# define ABOUT_DIALOG_WIDTH 300
# define ABOUT_DIALOG_HEIGHT    145

// render text
static inline SDL_Surface *
render_text( TTF_Font *font, SDL_Color fg, SDL_Color bg, const char *fmt, ... )
{
    va_list ap;
    char buf[2049];

    va_start( ap, fmt );
    vsnprintf( buf, 2048, fmt, ap );
    va_end( ap );

    if( *buf == '\0' )
        return NULL;

    return( TTF_RenderText_Shaded( font, buf, fg, bg ) );
}

// write text
static inline void
write_text( SDL_Surface *dst, TTF_Font *font, int x, int y, SDL_Color fg, SDL_Color bg, const char *fmt, ... )
{
    SDL_Surface *text;
    SDL_Rect dstrect;
    va_list ap;
    char buf[2049];

    va_start( ap, fmt );
    vsnprintf( buf, 2048, fmt, ap );
    va_end( ap );

    if( *buf == '\0' )
        return;

    text = render_text( font, fg, bg, buf );
    if( text == NULL )
    {
        fprintf( stderr, "["PLUGIN_NAME"]: Couldn't render text: %s\n", SDL_GetError() );
        return;
    }

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = text->w;
    dstrect.h = text->h;
    SDL_BlitSurface( text, NULL, dst, &dstrect );
    SDL_FreeSurface( text );
}

void
DllAbout( HWND hParent )
{
    SDL_RWops *rw;
    TTF_Font *font;
    SDL_Surface *screen;
    SDL_Rect rect;
    // colors
    Uint32 u32black, u32gray, u32dark_gray;
    SDL_Color black      = { 0x00, 0x00, 0x00, 0 };
    SDL_Color gray       = { 0xAA, 0xAA, 0xAA, 0 };
    SDL_Color dark_gray  = { 0x66, 0x66, 0x66, 0 };

    // init sdl
    if( !SDL_WasInit( SDL_INIT_VIDEO ) )
        if( SDL_InitSubSystem( SDL_INIT_VIDEO ) < 0 )
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init SDL video subsystem: %s\n", SDL_GetError() );
            return;
        }

    // init sdl_ttf2
    if( !TTF_WasInit() )
        if( TTF_Init() < 0 )
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init TTF library: %s\n", SDL_GetError() );
            SDL_QuitSubSystem( SDL_INIT_VIDEO );
            return;
        }

    // open font
    rw = SDL_RWFromMem( (char *)arial.data, arial.size );
    font = TTF_OpenFontRW( rw, 0, FONT_SIZEPT );
    if( font == NULL )
    {
        fprintf( stderr, "["PLUGIN_NAME"]: Couldn't load %d pt font: %s\n", FONT_SIZEPT, SDL_GetError() );
        TTF_Quit();
        SDL_QuitSubSystem( SDL_INIT_VIDEO );
        return;
    }
    TTF_SetFontStyle( font, TTF_STYLE_NORMAL );

    // display dialog (set video mode)
    screen = SDL_SetVideoMode( ABOUT_DIALOG_WIDTH, ABOUT_DIALOG_HEIGHT, 0, SDL_SWSURFACE );
    if( !screen )
    {
        fprintf( stderr, "["PLUGIN_NAME"]: Couldn't set video mode %dx%d: %s\n", ABOUT_DIALOG_WIDTH, ABOUT_DIALOG_HEIGHT, SDL_GetError() );
        TTF_Quit();
        SDL_QuitSubSystem( SDL_INIT_VIDEO );
        return;
    }
    SDL_WM_SetCaption( PLUGIN_NAME" "PLUGIN_VERSION, NULL );

    // create colors
    u32black      = SDL_MapRGBA( screen->format, black.r, black.g, black.b, 0 );
    u32gray       = SDL_MapRGBA( screen->format, gray.r, gray.g, gray.b, 0 );
    u32dark_gray  = SDL_MapRGBA( screen->format, dark_gray.r, dark_gray.g, dark_gray.b, 0 );

    // draw dialog
    SDL_FillRect( screen, NULL, u32dark_gray );

    rect.x = rect.y = 5; rect.w = ABOUT_DIALOG_WIDTH - 10; rect.h = ABOUT_DIALOG_HEIGHT - 40;
    SDL_FillRect( screen, &rect, u32black );
    rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
    SDL_FillRect( screen, &rect, u32gray );

    write_text( screen, font, 15, 15, black, gray, PLUGIN_NAME" v"PLUGIN_VERSION":" );
    write_text( screen, font, 15, 35, black, gray, "coded by blight" );
    write_text( screen, font, 15, 55, black, gray, "This plugin uses the SDL library for input." );
    write_text( screen, font, 15, 75, black, gray, "Go to www.libsdl.org for more information." );

    rect.x = (ABOUT_DIALOG_WIDTH - 90) / 2; rect.y = ABOUT_DIALOG_HEIGHT - 30; rect.w = 90; rect.h = 25;
    SDL_FillRect( screen, &rect, u32black );
    rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
    SDL_FillRect( screen, &rect, u32gray );
    
    write_text( screen, font, rect.x + 33, rect.y + 2, black, gray, "Ok" );

    for(;;)
    {
        SDL_Event event;
        SDL_Flip( screen );
        if( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_KEYDOWN )
            {
                if( event.key.keysym.sym == SDLK_ESCAPE )
                    break;
            }
            else if( event.type == SDL_MOUSEBUTTONDOWN )
            {
                if( event.button.button == SDL_BUTTON_LEFT )
                {
                    if( event.button.x >= rect.x && event.button.x <= rect.x + rect.w &&
                        event.button.y >= rect.y && event.button.y <= rect.y + rect.h )
                        break;
                }
            }
        }
    }
    TTF_Quit();
    SDL_FreeSurface( screen );
    SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

#elif defined( GUI_GTK )
static int about_shown = 0;

static void
about_ok_clicked(   GtkWidget *widget,
            gpointer   data )
{
    gtk_widget_hide_all( GTK_WIDGET(data) );
    gtk_widget_destroy( GTK_WIDGET(data) );
    about_shown = 0;
}

void
DllAbout( HWND hParent )
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;

    if( about_shown )
        return;

    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( GTK_WINDOW(window), PLUGIN_NAME );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );
    gtk_window_set_policy( GTK_WINDOW(window), FALSE, FALSE, TRUE );

    vbox = gtk_vbox_new( FALSE, 10 );
    label = gtk_label_new( PLUGIN_NAME" version "PLUGIN_VERSION"\n\n"
                "This is a N64 input plugin using SDL.\n"
                "(c) 2002 by blight" );
    button = gtk_button_new_with_label( "Ok" );

    gtk_container_add( GTK_CONTAINER(window), GTK_WIDGET(vbox) );
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(label), TRUE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(button), TRUE, FALSE, 0 );

    gtk_signal_connect( GTK_OBJECT(button), "clicked",
            GTK_SIGNAL_FUNC(about_ok_clicked), (gpointer) window);

    // show the window
    about_shown = 1;
    gtk_widget_show_all( GTK_WIDGET(window) );
}
#endif

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
void
DllConfig( HWND hParent )
{
    if( !romopen )
    {
        read_configuration();
#ifdef GUI_SDL
        configure_sdl( controller );
#elif defined( GUI_GTK )
        configure_gtk( controller );
#endif
    }
}

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
void
DllTest( HWND hParent )
{
}

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/
void
GetDllInfo( PLUGIN_INFO *PluginInfo )
{
    strncpy( PluginInfo->Name, PLUGIN_NAME" "PLUGIN_VERSION, 100 );
    PluginInfo->Name[99] = '\0';
    PluginInfo->Version = 0x0101;
    PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
}

/******************************************************************
  Function: GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with
            the controller state.
  output:   none
*******************************************************************/
void
GetKeys( int Control, BUTTONS *Keys )
{
    struct input_event play;
    int b, axis_val, axis_max_val;
    SDL_Event event;
    Uint8 *keystate = SDL_GetKeyState( NULL );

    // read joystick state
    SDL_JoystickUpdate();

    controller[Control].buttons.button = 0;
    controller[Control].buttons.stick_x = 0;
    controller[Control].buttons.stick_y = 0;

    if( controller[Control].device >= 0 )
    {
        for( b = 0; b < 16; b++ )
        {
            if( controller[Control].button[b].button >= 0 )
                if( SDL_JoystickGetButton( controller[Control].joystick, controller[Control].button[b].button ) )
                    controller[Control].buttons.button |= button_bits[b];

            if( controller[Control].button[b].axis >= 0 )
            {
                axis_val = SDL_JoystickGetAxis( controller[Control].joystick, controller[Control].button[b].axis );
                if( (controller[Control].button[b].axis_dir < 0) && (axis_val <= -6000) )
                    controller[Control].buttons.button |= button_bits[b];
                else if( (controller[Control].button[b].axis_dir > 0) && (axis_val >= 6000) )
                    controller[Control].buttons.button |= button_bits[b];
            }

            if( controller[Control].button[b].hat >= 0 )
            {
                if( controller[Control].button[b].hat_pos > 0 )
                    if( SDL_JoystickGetHat( controller[Control].joystick, controller[Control].button[b].hat ) & controller[Control].button[b].hat_pos )
                        controller[Control].buttons.button |= button_bits[b];
            }
        }
        for( b = 0; b < 2; b++ )
        {
            // from the N64 func ref: The 3D Stick data is of type signed char and in
            // the range between 80 and -80. (32768 / 409 = ~80.1)
            axis_val = 0;

            if( controller[Control].axis[b].axis >= 0 )
                axis_val = SDL_JoystickGetAxis( controller[Control].joystick, controller[Control].axis[b].axis ) / 409;

            if( controller[Control].axis[b].hat >= 0 )
            {
                if( controller[Control].axis[b].hat_pos_a >= 0 )
                    if( SDL_JoystickGetHat( controller[Control].joystick, controller[Control].axis[b].hat ) & controller[Control].axis[b].hat_pos_a )
                        axis_val = 80;
                if( controller[Control].axis[b].hat_pos_b >= 0 )
                    if( SDL_JoystickGetHat( controller[Control].joystick, controller[Control].axis[b].hat ) & controller[Control].axis[b].hat_pos_b )
                        axis_val = -80;
            }

            if( controller[Control].axis[b].button_a >= 0 )
                if( SDL_JoystickGetButton( controller[Control].joystick, controller[Control].axis[b].button_a ) )
                    axis_val = 80;
            if( controller[Control].axis[b].button_b >= 0 )
                if( SDL_JoystickGetButton( controller[Control].joystick, controller[Control].axis[b].button_b ) )
                    axis_val = -80;

            if( b == 0 )
                controller[Control].buttons.stick_x = -axis_val;
            else
                controller[Control].buttons.stick_y = axis_val;
        }
    }

    // read keyboard state
//  else if( controller[Control].device == DEVICE_KEYBOARD )
    {
        axis_max_val = 80;
        if (keystate[SDLK_LCTRL])
            axis_max_val -= 40;
        if (keystate[SDLK_LSHIFT])
            axis_max_val -= 20;
        for( b = 0; b < 16; b++ )
        {
            if( controller[Control].button[b].key == SDLK_UNKNOWN || ((int) controller[Control].button[b].key) < 0)
                continue;
            if( keystate[controller[Control].button[b].key] )
                controller[Control].buttons.button |= button_bits[b];
        }
        for( b = 0; b < 2; b++ )
        {
            // from the N64 func ref: The 3D Stick data is of type signed char and in
            // the range between 80 and -80. (32768 / 409 = ~80.1)
            if( b == 0 )
                axis_val = controller[Control].buttons.stick_x;
            else
                axis_val = -controller[Control].buttons.stick_y;

            if( controller[Control].axis[b].key_a != SDLK_UNKNOWN && ((int) controller[Control].axis[b].key_a) > 0)
                if( keystate[controller[Control].axis[b].key_a] )
                    axis_val = axis_max_val;
            if( controller[Control].axis[b].key_b != SDLK_UNKNOWN && ((int) controller[Control].axis[b].key_b) > 0)
                if( keystate[controller[Control].axis[b].key_b] )
                    axis_val = -axis_max_val;

            if( b == 0 )
                controller[Control].buttons.stick_x = axis_val;
            else
                controller[Control].buttons.stick_y = -axis_val;
        }
    }

    // process mouse events
    {
        Uint8 mstate = SDL_GetMouseState( NULL, NULL );

        for( b = 0; b < 16; b++ )
        {
            if( controller[Control].button[b].mouse < 1 )
                continue;
            if( mstate & SDL_BUTTON(controller[Control].button[b].mouse) )
                controller[Control].buttons.button |= button_bits[b];
        }
    }

    if (controller[Control].mouse)
    {
        int grabmouse = -1;        
        while (SDL_PollEvent( &event ))
        {
            if (event.type == SDL_MOUSEMOTION && SDL_WM_GrabInput( SDL_GRAB_QUERY ) == SDL_GRAB_ON)
            {
                if (event.motion.xrel)
                {
                    axis_val = (event.motion.xrel * 10);
                    if (axis_val < -80)
                        axis_val = -80;
                    else if (axis_val > 80)
                        axis_val = 80;
                    controller[Control].buttons.stick_y = axis_val;
                }
                if (event.motion.yrel)
                {
                    axis_val = (event.motion.yrel * 10);
                    if (axis_val < -80)
                        axis_val = -80;
                    else if (axis_val > 80)
                        axis_val = 80;
                    controller[Control].buttons.stick_x = -axis_val;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    grabmouse = 1;
                }
            }
        }
        if (keystate[SDLK_LCTRL] && keystate[SDLK_LALT])
        {
            grabmouse = 0;
        }
        if (grabmouse >= 0)
        {
            // grab/ungrab mouse
            SDL_WM_GrabInput( grabmouse ? SDL_GRAB_ON : SDL_GRAB_OFF );
            SDL_ShowCursor( grabmouse ? 0 : 1 );
        }
    }

#ifdef _DEBUG
    printf( "Controller #%d value: 0x%8.8X\n", Control, *(int *)&controller[Control].buttons );
#endif
    *(int *)Keys = *(int *)&controller[Control].buttons;

    /* handle mempack / rumblepak switching */
    if (controller[Control].buttons.button & button_bits[14])
    {
        controller[Control].control.Plugin = PLUGIN_MEMPAK;
        if (controller[Control].event_joystick != 0)
        {
            play.type = EV_FF;
            play.code = ffweak[Control].id;
            play.value = 1;
            if (write(controller[Control].event_joystick, (const void*) &play, sizeof(play)) == -1)
                perror("Error starting rumble effect");
        }
    }
    if (controller[Control].buttons.button & button_bits[15])
    {
        controller[Control].control.Plugin = PLUGIN_RAW;
        if (controller[Control].event_joystick != 0)
        {
            play.type = EV_FF;
            play.code = ffstrong[Control].id;
            play.value = 1;
            if (write(controller[Control].event_joystick, (const void*) &play, sizeof(play)) == -1)
                perror("Error starting rumble effect");
        }
    }
}

int InitiateRumble(int cntrl)
{
    DIR *dp;
    struct dirent *ep;
    unsigned long features[4];
    char temp[128];
    char temp2[128];
    int iFound = 0;

    controller[cntrl].event_joystick = 0;

    sprintf(temp,"/sys/class/input/js%d/device", controller[cntrl].device);
    dp = opendir(temp);

    if (dp == NULL) return 0;

    while ((ep = readdir (dp)))
    {
        if (strncmp(ep->d_name,"event",5) == 0)
        {
            sprintf(temp,"/dev/input/%s",ep->d_name);
            iFound = 1;
            break;
        }
        else if (strncmp(ep->d_name,"input:event",11) == 0)
        {
            sscanf(ep->d_name,"input:%s",temp2);
            sprintf(temp,"/dev/input/%s",temp2);
            iFound = 1;
            break;
        }
        else if (strncmp(ep->d_name,"input:input",11) == 0)
        {
            strcat(temp, "/");
            strcat(temp, ep->d_name);
            closedir (dp);
            dp = opendir(temp);
            if (dp == NULL) return 0;
        }
    }
    closedir(dp);
    if (!iFound)
    {
        printf("["PLUGIN_NAME"]: Couldn't find input event for rumble support.\n", temp);
        return 0;
    }

    controller[cntrl].event_joystick = open(temp, O_RDWR);
    if (controller[cntrl].event_joystick == -1)
    {
        printf("["PLUGIN_NAME"]: Couldn't open device file '%s' for rumble support.\n", temp);
        controller[cntrl].event_joystick = 0;
        return 0;
    }

    if (ioctl(controller[cntrl].event_joystick, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1)
    {
        printf("["PLUGIN_NAME"]: Linux kernel communication failed for force feedback (rumble).\n", temp);
        controller[cntrl].event_joystick = 0;
        return 0;
    }

    if (!test_bit(FF_RUMBLE, features))
    {
        printf("["PLUGIN_NAME"]: No rumble supported on N64 joystick #%i\n", cntrl + 1);
        controller[cntrl].event_joystick = 0;
        return 0;
    }

    ffeffect[cntrl].type = FF_RUMBLE;
    ffeffect[cntrl].id = -1;
    ffeffect[cntrl].u.rumble.strong_magnitude = 0xFFFF;
    ffeffect[cntrl].u.rumble.weak_magnitude = 0xFFFF;

    ioctl(controller[cntrl].event_joystick, EVIOCSFF, &ffeffect[cntrl]);

    ffstrong[cntrl].type = FF_RUMBLE;
    ffstrong[cntrl].id = -1;
    ffstrong[cntrl].u.rumble.strong_magnitude = 0xFFFF;
    ffstrong[cntrl].u.rumble.weak_magnitude = 0x0000;
    ffstrong[cntrl].replay.length = 500;
    ffstrong[cntrl].replay.delay = 0;

    ioctl(controller[cntrl].event_joystick, EVIOCSFF, &ffstrong[cntrl]);

    ffweak[cntrl].type = FF_RUMBLE;
    ffweak[cntrl].id = -1;
    ffweak[cntrl].u.rumble.strong_magnitude = 0x0000;
    ffweak[cntrl].u.rumble.weak_magnitude = 0xFFFF;
    ffweak[cntrl].replay.length = 500;
    ffweak[cntrl].replay.delay = 0;

    ioctl(controller[cntrl].event_joystick, EVIOCSFF, &ffweak[cntrl]);

    printf("["PLUGIN_NAME"]: Rumble activated on N64 joystick #%i\n", cntrl + 1);
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
void
InitiateControllers( CONTROL_INFO ControlInfo )
{
    int i;

    // reset controllers
    memset( controller, 0, sizeof( SController ) * 4 );

    // read configuration
    read_configuration();

    for( i = 0; i < 4; i++ )
    {
        memcpy( ControlInfo.Controls + i, &controller[i].control, sizeof( CONTROL ) );
        InitiateRumble(i);
    }

    printf( "["PLUGIN_NAME"]: version "PLUGIN_VERSION" initialized.\n" );
}

/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
void
ReadController( int Control, BYTE *Command )
{
#if 0//def _DEBUG
    printf( "\nRaw Read (cont=%d):\n", Control );
    printf( "\t%02X %02X %02X %02X %02X %02X\n", Command[0], Command[1],
            Command[2], Command[3], Command[4], Command[5]);//, Command[6], Command[7] );
#endif
}

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
void
RomClosed( void )
{
    int i;

    // close joysticks
    for( i = 0; i < 4; i++ )
        if( controller[i].joystick )
        {
            SDL_JoystickClose( controller[i].joystick );
            controller[i].joystick = NULL;
        }

    // quit SDL joystick subsystem
    SDL_QuitSubSystem( SDL_INIT_JOYSTICK );

    // release/ungrab mouse
    SDL_WM_GrabInput( SDL_GRAB_OFF );
    SDL_ShowCursor( 1 );

    romopen = 0;
}

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the
            emulation thread)
  input:    none
  output:   none
*******************************************************************/
void
RomOpen( void )
{
    int i;

    // init SDL joystick subsystem
    if( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
        if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) == -1 )
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init SDL joystick subsystem: %s\n", SDL_GetError() );
            return;
        }

    // open joysticks
    for( i = 0; i < 4; i++ )
        if( controller[i].device >= 0 )
        {
            controller[i].joystick = SDL_JoystickOpen( controller[i].device );
            if( controller[i].joystick == NULL )
                fprintf( stderr, "["PLUGIN_NAME"]: Couldn't open joystick for controller #%d: %s\n", i, SDL_GetError() );
        }
        else
            controller[i].joystick = NULL;

    // grab mouse
    if (controller[0].mouse || controller[1].mouse || controller[2].mouse || controller[3].mouse)
    {
        SDL_ShowCursor( 0 );
        if (SDL_WM_GrabInput( SDL_GRAB_ON ) != SDL_GRAB_ON)
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't grab input! Mouse support won't work!\n" );
            fprintf( stderr, "["PLUGIN_NAME"]: Note: You have to set your graphics window fullscreen in order for this to work!\n" );
        }
    }

    romopen = 1;
}

/******************************************************************
  Function: WM_KeyDown
  Purpose:  To pass the WM_KeyDown message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
void
WM_KeyDown( WPARAM wParam, LPARAM lParam )
{
}

/******************************************************************
  Function: WM_KeyUp
  Purpose:  To pass the WM_KEYUP message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
void
WM_KeyUp( WPARAM wParam, LPARAM lParam )
{
}

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: SetConfigDir
  Purpose:  To pass the location where config files should be read/
            written to.
  input:    path to config directory
  output:   none
*******************************************************************/
void
SetConfigDir( char *configDir )
{
    strncpy(configdir, configDir, PATH_MAX);
}
