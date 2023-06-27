#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "dtexture.h"
#include "event_queue.h"
#include "lua.h"
#include "utils.h"

#define DISABLE_PRINT 0



int
main()
{
  ut_InitAll();

  lua_Start();

  ut_DeInitAll(0);
  return 0;
}


/*
static bool GetEvent(int timeout)
{
  event_args_t ea;
  uint64_t end = SDL_GetTicks64() + timeout;
  while (end > SDL_GetTicks64())
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
        ea.arg[0] = "keydown";
        ea.arg[1] = int2str(event.key.keysym.scancode);
        ea.len = 2;
        queue_push(&event_queue, ea);
        return true;
        /
        lua_pushstring(L, "keydown");
        lua_pushnumber(L, event.key.keysym.scancode);
        /
      case SDL_KEYUP:
        ea.arg[0] = "keydup";
        ea.arg[1] = int2str(event.key.keysym.scancode);
        ea.len = 2;
        queue_push(&event_queue, ea);
        return true;
        /
        lua_pushstring(L, "keyup");
        lua_pushnumber(L, event.key.keysym.scancode);
        /
    }
  }
  return false;
}
*/

/*
static void l_hook(lua_State* L, lua_Debug* d)
{
  if (d->event == 0)
  {
    GetEvent(1);
    currentTimeout = SDL_GetTicks64() + KILL_TIMEOUT;
    SDL_Delay(1000/CPU_HZ);
  }
  else if (d->event == 3)
  {
    if (SDL_GetTicks64() > currentTimeout)
    {
      puts("[C] Too long without yielding");
      longjmp(kill, 1);
    }
  }
}

// -------------- function -------------- //

static int lf_gettotal(lua_State* L)
{
  lua_pushnumber(L, 1024*1024*MAX_SIZE);
  return 1;
}

static int lf_getused(lua_State* L)
{
  lua_pushnumber(L, *pUd);
  return 1;
}

static int lf_pullevent(lua_State* L)
{
  lua_Number arg = luaL_checknumber(L, 1);
  if (event_queue.length == 0)
  {
    if(!GetEvent(arg))
      return 0;
  }

  event_args_t e = queue_pop(&event_queue);
  for (int i = 0; i < e.len; i++)
  {
    lua_pushstring(L, e.arg[i]);
  }
  return e.len;
  return 0;
}

static int lf_pushevent(lua_State* L)
{
  event_args_t ea;
  for (int i = 0; i < lua_gettop(L); i++)
  {
    if (lua_isstring(L, i+1))
    {
      ea.arg[i] = lua_tostring(L, i+1);
    }
    else
    {
      ea.len = i;
      break;
    }
  }
  if (ea.len > 0)
    queue_push(&event_queue, ea);
  return 0;
}

// GPU //
static int lf_gpusetcolor(lua_State* L)
{
  lua_Number col = luaL_checknumber(L, -1);
  currentColor = col;

  return 0;
}

static int lf_gpugetcolor(lua_State* L)
{
  lua_pushnumber(L, currentColor);
  return 1;
}

static int lf_gpugetpixel(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, -2);
  lua_Number fy = luaL_checknumber(L, -1);

  lua_pushnumber(L, d_getPixel(pCan, fx-1, fy-1));

  return 1;
}


static int lf_gpufill(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, -4);
  lua_Number fy = luaL_checknumber(L, -3);
  lua_Number fw = luaL_checknumber(L, -2);
  lua_Number fh = luaL_checknumber(L, -1);

  d_rect(pCan, fx-1, fy-1, fw, fh, currentColor);
  return 0;
}

static int lf_gpucopy(lua_State* L)
{
  lua_Number fx =  luaL_checknumber(L, -6);
  lua_Number fy =  luaL_checknumber(L, -5);
  lua_Number fw =  luaL_checknumber(L, -4);
  lua_Number fh =  luaL_checknumber(L, -3);
  lua_Number fox = luaL_checknumber(L, -2);
  lua_Number foy = luaL_checknumber(L, -1);

  d_copy(pCan, fx-1, fy-1, fw, fh, fox, foy);
  return 0;
}

static int lf_gpuclear()
{
  d_fill(pCan, currentColor);

  return 0;
}

static int lf_gpuupdate()
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

  return 0;
}

// Modifications //

static int lf_corocreate(lua_State* L)
{
  puts("[C] Coro create e");
  lua_State* NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);
  lua_xmove(L, NL, 1);
  lua_sethook(NL, l_hook, LUA_MASKCOUNT | LUA_MASKCALL, 1000);
  return 1;
}

// -------------------------------------- //

static const luaL_Reg computerlib[] =
{
  {"getTotal", lf_gettotal},
  {"getUsed", lf_getused},
  {"pullEvent", lf_pullevent},
  {"pushEvent", lf_pushevent},
  {NULL, NULL}
};

static int luaopen_computer (lua_State *L) {
  luaL_newlib(L, computerlib);  / new module /
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

static int luaopen_gpu (lua_State *L) {
  luaL_newlib(L, gpulib);  / new module /
  return 1;
}

static const luaL_Reg loadedlibs[] =
{
  {"_G", luaopen_base},
  //{LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  //{LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_BITLIBNAME, luaopen_bit32},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_DBLIBNAME, luaopen_debug},
  {"computer", luaopen_computer},
  {"gpu", luaopen_gpu},
  {NULL, NULL}
};


static const luaL_Reg preloadedlibs[] =
{
  {NULL, NULL}
};


static void OpenLibs (lua_State *L)
{
  const luaL_Reg *lib;
  / call open functions from 'loadedlibs' and set results to global table /
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  / remove lib /
  }
  / add open functions from 'preloadedlibs' into 'package.preload' table /
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
  for (lib = preloadedlibs; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_setfield(L, -2, lib->name);
  }
  lua_pop(L, 1);  / remove _PRELOAD table /
}

static void ModifyLibs(lua_State* L)
{
  if (DISABLE_PRINT == 1)
  {
    lua_pushnil(L);
    lua_setglobal(L, "print");
  }
  lua_getglobal(L, "coroutine");
  lua_pushcfunction(L, lf_corocreate);
  lua_setfield(L, -2, "create");
  lua_setglobal(L, "coroutine");
}
*/
