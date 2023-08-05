#include <SDL2/SDL.h>
#include "sdl.h"
#include "utils.h"
#include "config.h"

static uint16_t winW = 800, winH = 600;

static SDL_Window* pWin = NULL;
static SDL_Renderer* pRen = NULL;
static SDL_Texture* pTex = NULL;
static SDL_Rect texRect = {0,0,800,600};
static Canvas_t* pCan;

void
sdl_DeInitSDL()
{
  if (pTex != NULL) SDL_DestroyTexture(pTex);
  if (pRen != NULL) SDL_DestroyRenderer(pRen);
  if (pWin != NULL) SDL_DestroyWindow(pWin);
  SDL_Quit();
  dt_FreeCanvas(pCan);
}

void
sdl_InitSDL()
{
  char* val = cfg_GetValue("window_w");
  if (val != NULL)
  {
    winW = atoi(val);
  }
  val = cfg_GetValue("window_h");
  if (val != NULL)
  {
    winH = atoi(val);
  }
  texRect.w = winW;
  texRect.h = winH;
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Cannot init SDL (line 10): %s\n", SDL_GetError());
    exit(1);
  }

  pWin = SDL_CreateWindow("VM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_SHOWN);
  if (pWin == NULL)
  {
    printf("Cannot create window (line 16): %s\n", SDL_GetError());
    ut_DeInitAll(1);
  }

  pRen = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED);
  if (pRen == NULL)
  {
    printf("Cannot create renderer (line 23): %s\n", SDL_GetError());
    ut_DeInitAll(1);
  }

  pTex = SDL_CreateTexture(pRen, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, winW, winH);
  if (pTex == NULL)
  {
    printf("Cannot create texture (line 50): %s\n", SDL_GetError());
    ut_DeInitAll(1);
  }
  pCan = dt_NewCanvas(winW, winH);

  SDL_SetWindowResizable(pWin, SDL_FALSE);
  SDL_ShowCursor(SDL_DISABLE);
  SDL_SetWindowGrab(pWin, SDL_TRUE);
}

void
sdl_RendClear()
{
  SDL_SetRenderDrawColor(pRen, 0,0,0,0);
  SDL_RenderClear(pRen);
  SDL_RenderPresent(pRen);
}

void
sdl_RendUpdate()
{
  SDL_RenderCopy(pRen, pTex, &texRect, &texRect);
  SDL_RenderPresent(pRen);
}

void
sdl_Update()
{
  void *pixels;
  int pitch;
  SDL_LockTexture(pTex, &texRect, &pixels, &pitch);
  for (size_t y = 0; y < winH; ++y) {
    memcpy((char*)pixels + y*pitch, pCan->pixels + y*winW, winW*sizeof(uint32_t));
  }
  SDL_UnlockTexture(pTex);

  SDL_RenderCopy(pRen, pTex, &texRect, &texRect);
  SDL_RenderPresent(pRen);
}

uint16_t
sdl_GetWidth()
{
  return winW;
}

uint16_t
sdl_GetHeight()
{
  return winH;
}

Canvas_t*
sdl_GetCanvas()
{
  return pCan;
}

