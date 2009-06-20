/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - getinput.cpp                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 olejl77@gmail.com                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "getinput.h"
#include "SDL.h"

GetInput::GetInput(int *key)
{
    SDL_Event event;
    *key = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Could not initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
    if(SDL_SetVideoMode(300, 1, 0, SDL_OPENGL) == NULL) {
        printf("Could not set SDL Video mode: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
    SDL_WM_SetCaption("Waiting for input...", NULL);

    for (int i = 0; i < 10000; i++) {   // Wait maximum 10 sec before exiting
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                i = 10000;
                break;
            case SDL_KEYDOWN:
                *key = event.key.keysym.sym;
                i = 10000;
                break;
            }
        }
        SDL_Delay(1);
    }
    SDL_Quit();
}

