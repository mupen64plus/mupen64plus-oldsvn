//
// SDL Input for TASinput
// written by nmn
//

#include <SDL.h>
#include "Controller.h"
#include "SDLinput.h"
#include "DefSDL.h"

bool InitSDLInput() 
{
    // init SDL joystick subsystem
    if( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
    {
        if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) == -1 )
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't init SDL joystick subsystem: %s\n", SDL_GetError() );
            return false;
        }
    }
    return true;
}

void FreeSDLInput()
{
    if( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
    {
        if( SDL_QuitSubSystem ( SDL_INIT_JOYSTICK ) == -1 )
        {
            fprintf( stderr, "["PLUGIN_NAME"]: Couldn't quit SDL joystick subsystem: %s\n", SDL_GetError() );
            return;
        }
    }
}