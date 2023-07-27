#ifndef DANILUS_SDL_H
#define DANILUS_SDL_H

#include "dtexture.h"


void sdl_DeInitSDL();
void sdl_InitSDL();

void sdl_RendClear();
void sdl_RendUpdate();
void sdl_Update();
uint16_t sdl_GetWidth();
uint16_t sdl_GetHeight();
d_Canvas* sdl_GetCanvas();

#endif
