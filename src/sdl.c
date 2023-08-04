#include <SDL2/SDL.h>
#include "sdl.h"
#include "utils.h"

static const uint16_t WIN_W = 800, WIN_H = 600;

static SDL_Window* pWin = NULL;
static SDL_Renderer* pRen = NULL;
static SDL_Texture* pTex = NULL;
static const SDL_Rect TEX_RECT = {0,0,WIN_W, WIN_H};
static d_Canvas* pCan;

void
sdl_DeInitSDL()
{
  if (pTex != NULL) SDL_DestroyTexture(pTex);
  if (pRen != NULL) SDL_DestroyRenderer(pRen);
  if (pWin != NULL) SDL_DestroyWindow(pWin);
  SDL_Quit();
  d_freeCanvas(pCan);
}

void
sdl_InitSDL()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Cannot init SDL (line 10): %s\n", SDL_GetError());
    exit(1);
  }

  pWin = SDL_CreateWindow("VM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
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

  pTex = SDL_CreateTexture(pRen, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H);
  if (pTex == NULL)
  {
    printf("Cannot create texture (line 50): %s\n", SDL_GetError());
    ut_DeInitAll(1);
  }
  pCan = d_newCanvas(WIN_W, WIN_H);

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
  SDL_RenderCopy(pRen, pTex, &TEX_RECT, &TEX_RECT);
  SDL_RenderPresent(pRen);
}

void
sdl_Update()
{
  void *pixels;
  int pitch;
  SDL_LockTexture(pTex, &TEX_RECT, &pixels, &pitch);
  for (size_t y = 0; y < WIN_H; ++y) {
    memcpy((char*)pixels + y*pitch, pCan->pixels + y*WIN_W, WIN_W*sizeof(uint32_t));
  }
  SDL_UnlockTexture(pTex);

  SDL_RenderCopy(pRen, pTex, &TEX_RECT, &TEX_RECT);
  SDL_RenderPresent(pRen);
}

uint16_t
sdl_GetWidth()
{
  return WIN_W;
}

uint16_t
sdl_GetHeight()
{
  return WIN_H;
}

d_Canvas* sdl_GetCanvas()
{
  return pCan;
}

