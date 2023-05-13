#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <setjmp.h>
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
#include <SDL2/SDL.h>
#include "../include/dtexture.h"

#define DISABLE_PRINT 0

SDL_Window* win = NULL;
SDL_Renderer* ren = NULL;
SDL_Texture* tex = NULL;
SDL_Event event;
bool lowMemory = false;
const int MAX_SIZE = 1; // in MiB
lua_State* L;
uint64_t* ud;
const char* FSName = "./FS";
const uint16_t WIN_W = 800, WIN_H = 600;
d_Canvas* can;
uint32_t currentColor;

jmp_buf kill;

void DeInitAll(int err)
{
  if (tex != NULL) SDL_DestroyTexture(tex);
  if (ren != NULL) SDL_DestroyRenderer(ren);
  if (win != NULL) SDL_DestroyWindow(win);
  SDL_Quit();
  exit(err);
}

void InitAll()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Cannot init SDL (line 10): %s\n", SDL_GetError());
    exit(1);
  }

  win = SDL_CreateWindow("VM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
  if (win == NULL)
  {
    printf("Cannot create window (line 16): %s\n", SDL_GetError());
    DeInitAll(1);
  }

  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  if (ren == NULL)
  {
    printf("Cannot create renderer (line 23): %s\n", SDL_GetError());
    DeInitAll(1);
  }

  tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H);
  if (tex == NULL)
  {
    printf("Cannot create texture (line 50): %s\n", SDL_GetError());
    DeInitAll(1);
  }
}

void RendClear()
{
  SDL_SetRenderDrawColor(ren, 0,0,0,0);
  SDL_RenderClear(ren);
  SDL_RenderPresent(ren);
}

static void* l_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
  uint64_t *used = (uint64_t*)ud;

  if(ptr == NULL)
  {
    osize = 0;
  }

  if (nsize == 0)
  {
    free(ptr);
    *used -= osize;
    if (*used < 1024*1024*MAX_SIZE-512)
      lowMemory = false;
    return NULL;
  }
  else
  {
    if (*used + (nsize - osize) > 1024*1024*MAX_SIZE)
    {
      puts("[critical] Memory out!");
      lua_yield(L, 0);
      //exit(1);
    }
    if (*used + (nsize - osize) > 1024*1024*MAX_SIZE-512)
    {
      if (lowMemory == false)
      {
        puts("[warn] low memory");
        lowMemory = true;
        luaL_error(L, "low mem");
      }
    }
    ptr = realloc(ptr, nsize);
    if (ptr)
      *used += (nsize - osize);

    if (*used < 1024*1024*MAX_SIZE-512)
      lowMemory = false;

    return ptr;
  }
}

// -------------- function -------------- //

int lf_gettotal(lua_State* L)
{
  lua_pushnumber(L, 1024*1024*MAX_SIZE);
  return 1;
}

int lf_getused(lua_State* L)
{
  lua_pushnumber(L, *ud);
  return 1;
}

int lf_pollevent(lua_State* L)
{
  while (1)
  {
    SDL_PollEvent(&event);
    switch (event.type)
    {
      case SDL_QUIT:
        longjmp(kill, 1);
        break;
      /*
      case SDL_WINDOWEVENT:
        RendClear();
        break;
      */
      case SDL_KEYDOWN:
        lua_pushstring(L, "keydown");
        lua_pushnumber(L, event.key.keysym.scancode);
        return 2;
      case SDL_KEYUP:
        lua_pushstring(L, "keyup");
        lua_pushnumber(L, event.key.keysym.scancode);
        return 2;
    }
  }
  longjmp(kill, 1);
  return 0;
}

// GUI //
int lf_gpusetcolor(lua_State* L)
{
  lua_Number col = lua_tonumber(L, -1);
  currentColor = col;

  return 0;
}

int lf_gpugetcolor(lua_State* L)
{
  lua_pushnumber(L, currentColor);
  return 1;
}

int lf_gpugetpixel(lua_State* L)
{
  lua_Number fx = lua_tonumber(L, -2);
  lua_Number fy = lua_tonumber(L, -1);

  lua_pushnumber(L, d_getPixel(can, fx, fy));

  return 1;
}

int lf_gpufill(lua_State* L)
{
  lua_Number fx = lua_tonumber(L, -4);
  lua_Number fy = lua_tonumber(L, -3);
  lua_Number fw = lua_tonumber(L, -2);
  lua_Number fh = lua_tonumber(L, -1);

  d_rect(can, fx, fy, fw, fh, currentColor);

  return 0;
}

int lf_gpucopy(lua_State* L)
{
  lua_Number fx =  lua_tonumber(L, -6);
  lua_Number fy =  lua_tonumber(L, -5);
  lua_Number fw =  lua_tonumber(L, -4);
  lua_Number fh =  lua_tonumber(L, -3);
  lua_Number fox = lua_tonumber(L, -2);
  lua_Number foy = lua_tonumber(L, -1);

  d_copy(can, fx, fy, fw, fh, fox, foy);

  return 0;
}

int lf_gpuclear()
{
  d_fill(can, currentColor);

  return 0;
}

int lf_gpuupdate()
{
  void *pixels;
  int pitch;
  SDL_Rect texrect = {0,0,WIN_W, WIN_H};
  SDL_LockTexture(tex, &texrect, &pixels, &pitch);
  for (size_t y = 0; y < WIN_H; ++y) {
    memcpy((char*)pixels + y*pitch, can->pixels + y*WIN_W, WIN_W*sizeof(uint32_t));
  }
  SDL_UnlockTexture(tex);

  SDL_RenderCopy(ren, tex, &texrect, &texrect);
  SDL_RenderPresent(ren);

  return 0;
}

// -------------------------------------- //

static const luaL_Reg computerlib[] =
{
  {"getTotal", lf_gettotal},
  {"getUsed", lf_getused},
  {"pollEvent", lf_pollevent}
};

int luaopen_computer (lua_State *L) {
  luaL_newlib(L, computerlib);  /* new module */
  return 1;
}

static const luaL_Reg gpulib[] =
{
  {"setColor", lf_gpusetcolor},
  {"getColor", lf_gpugetcolor},
  {"getPixel", lf_gpugetpixel},
  {"clear", lf_gpuclear},
  {"fill", lf_gpufill},
  {"copy", lf_gpucopy},
  {"update", lf_gpuupdate},
  {NULL, NULL}
};

int luaopen_gpu (lua_State *L) {
  luaL_newlib(L, gpulib);  /* new module */
  return 1;
}

static const luaL_Reg loadedlibs[] =
{
  {"_G", luaopen_base},
  //{LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  //{LUA_IOLIBNAME, luaopen_io},
  //{LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_BITLIBNAME, luaopen_bit32},
  {LUA_MATHLIBNAME, luaopen_math},
  //{LUA_DBLIBNAME, luaopen_debug},
  {"computer", luaopen_computer},
  {"gpu", luaopen_gpu},
  {NULL, NULL}
};


static const luaL_Reg preloadedlibs[] =
{
  {NULL, NULL}
};


void openlibs (lua_State *L)
{
  const luaL_Reg *lib;
  /* call open functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
  /* add open functions from 'preloadedlibs' into 'package.preload' table */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
  for (lib = preloadedlibs; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_setfield(L, -2, lib->name);
  }
  lua_pop(L, 1);  /* remove _PRELOAD table */
}

//int main(int argc, char** arcv)
int main()
{
  InitAll();

  ud = (uint64_t*)malloc(sizeof(uint64_t));
  *ud = 0;
  L = lua_newstate(l_alloc, ud);
  if (L == NULL)
  {
    printf("[E] Error in creating new lua state\n");
    return 1;
  }

  can = d_newCanvas(WIN_W, WIN_H);

  // load specific lua libs
  openlibs(L);

  //load special functions
  /*
  lua_pushcfunction(L, lf_getTotal);
  lua_setglobal(L, "getTotal");
  lua_pushcfunction(L, lf_getUsed);
  lua_setglobal(L, "getUsed");
  lua_pushcfunction(L, lf_pollevent);
  lua_setglobal(L, "pollEvent");
  lua_pushcfunction(L, lf_gpufill);
  lua_setglobal(L, "guiFill");
  lua_pushcfunction(L, lf_gpuupdate);
  lua_setglobal(L, "guiUpdate");
  */

  if (DISABLE_PRINT == 1)
  {
    lua_pushnil(L);
    lua_setglobal(L, "print");
  }

  SDL_SetWindowResizable(win, SDL_FALSE);
  SDL_ShowCursor(SDL_DISABLE);
  RendClear();


  if (setjmp(kill) == 0)
    luaL_dofile(L, "./FS/init.lua");


  lua_close(L);
  d_freeCanvas(can);
  DeInitAll(0);
  return 0;
}
