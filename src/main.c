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

static SDL_Window* win = NULL;
static SDL_Renderer* ren = NULL;
static SDL_Texture* tex = NULL;
static SDL_Event event;
static lua_State* L;
static d_Canvas* can;
static uint32_t currentColor;
static bool lowMemory = false;
static uint64_t* ud;
static uint64_t currentTimeout;
static const int MAX_SIZE = 1; // RAM in MiB
//static const char* FSName = "./FS";
static const uint16_t WIN_W = 800, WIN_H = 600;
static const SDL_Rect TEX_RECT = {0,0,WIN_W, WIN_H};
static const uint32_t CPU_HZ = 10;
static const uint64_t KILL_TIMEOUT = 10*1000; // 10 seconds

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

void RendUpdate()
{
  SDL_RenderCopy(ren, tex, &TEX_RECT, &TEX_RECT);
  SDL_RenderPresent(ren);
}

static void* l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
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

static void l_callhook(/*lua_State* L, lua_Debug* ar*/)
{
  currentTimeout = SDL_GetTicks64() + KILL_TIMEOUT;
  SDL_Delay(1000/CPU_HZ);
}

static void l_counthook()
{
  if (SDL_GetTicks64() > currentTimeout)
  {
    puts("[C] Too long without yielding (10s)");
    longjmp(kill, 1);
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

int lf_pullevent(lua_State* L)
{
  while (1)
  {
    SDL_PollEvent(&event);
    switch (event.type)
    {
      case SDL_QUIT:
        longjmp(kill, 1);
        break;
      case SDL_WINDOWEVENT:
        RendUpdate();
        break;
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

// GPU //
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

  lua_pushnumber(L, d_getPixel(can, fx-1, fy-1));

  return 1;
}

int lf_gpufill(lua_State* L)
{
  lua_Number fx = lua_tonumber(L, -4);
  lua_Number fy = lua_tonumber(L, -3);
  lua_Number fw = lua_tonumber(L, -2);
  lua_Number fh = lua_tonumber(L, -1);

  d_rect(can, fx-1, fy-1, fw, fh, currentColor);

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

  d_copy(can, fx-1, fy-1, fw, fh, fox, foy);

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
  SDL_LockTexture(tex, &TEX_RECT, &pixels, &pitch);
  for (size_t y = 0; y < WIN_H; ++y) {
    memcpy((char*)pixels + y*pitch, can->pixels + y*WIN_W, WIN_W*sizeof(uint32_t));
  }
  SDL_UnlockTexture(tex);

  SDL_RenderCopy(ren, tex, &TEX_RECT, &TEX_RECT);
  SDL_RenderPresent(ren);

  return 0;
}

// -------------------------------------- //

static const luaL_Reg computerlib[] =
{
  {"getTotal", lf_gettotal},
  {"getUsed", lf_getused},
  {"pullEvent", lf_pullevent},
  {NULL, NULL}
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

  if (DISABLE_PRINT == 1)
  {
    lua_pushnil(L);
    lua_setglobal(L, "print");
  }

  SDL_SetWindowResizable(win, SDL_FALSE);
  SDL_ShowCursor(SDL_DISABLE);
  lua_sethook(L, l_callhook, LUA_MASKCALL, 0);
  lua_sethook(L, l_counthook, LUA_MASKCOUNT, 1000);
  RendClear();


  currentTimeout = SDL_GetTicks64() + KILL_TIMEOUT;
  if (setjmp(kill) == 0)
    luaL_dofile(L, "./FS/init.lua");


  lua_close(L);
  d_freeCanvas(can);
  DeInitAll(0);
  return 0;
}
