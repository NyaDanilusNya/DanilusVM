#include <SDL2/SDL.h>
#include "inithelper.h"


static void
DeInitAll(int err, SDL_Texture* pTex, SDL_Renderer* pRen, SDL_Window* pWin)
{
  DeInitLua();
  DeInitSDL();
  exit(err);
}

static void InitAll()
{
  InitSDL();
  InitLua();
}
