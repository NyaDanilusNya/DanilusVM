#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <setjmp.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
#include <SDL2/SDL.h>
#include "event_queue.h"
#include "lua.h"
#include "utils.h"
#include "dtexture.h"
#include "sdl.h"
#include "config.h"
#include "fdcontrol.h"

#define DISABLE_PRINT 0

static uint64_t* pUd;
static lua_State* L = NULL;
static uint32_t currentColor;
static uint64_t currentTimeout;
static uint64_t KILL_TIMEOUT = 10*1000; // 10 seconds
static uint32_t CPU_HZ = 10;
static int MAX_SIZE = 1; // RAM in MiB
static bool lowMemory = false;
static SDL_Event event;
static Queue_t event_queue;
static Canvas_t* pCan;

jmp_buf kill;

static void*
l_alloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
  uint64_t* used = (uint64_t*)ud;

  if(ptr == NULL)
  {
    osize = 0;
  }

  if (nsize == 0)
  {
    free(ptr);
    *used -= osize;
    if (*used < (uint64_t)1024*1024*MAX_SIZE-512)
      lowMemory = false;
    return NULL;
  }
  else
  {
    if (*used + (nsize - osize) > (uint64_t)1024*1024*MAX_SIZE)
    {
      puts("[critical] Memory out!");
      lua_yield(L, 0);
      //exit(1);
    }
    if (*used + (nsize - osize) > (uint64_t)1024*1024*MAX_SIZE-512)
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

    if (*used < (uint64_t)1024*1024*MAX_SIZE-512)
      lowMemory = false;

    return ptr;
  }
}


static bool
GetEvent(int timeout)
{
  EventArgs_t ea;
  uint64_t end = SDL_GetTicks64() + timeout;
  do
  {
    SDL_PollEvent(&event);
    switch (event.type)
    {
      case SDL_QUIT:
        longjmp(kill, 1);
        break;
      case SDL_WINDOWEVENT:
        break;
      case SDL_KEYDOWN:
        ea.arg[0] = strdup("keydown");
        ea.arg[1] = ut_Int2Str(event.key.keysym.scancode);
        ea.len = 2;
        eq_Push(&event_queue, ea);
        return true;
      case SDL_KEYUP:
        ea.arg[0] = strdup("keyup");
        ea.arg[1] = ut_Int2Str(event.key.keysym.scancode);
        ea.len = 2;
        eq_Push(&event_queue, ea);
        return true;
      case SDL_MOUSEBUTTONDOWN:
        ea.arg[0] = strdup("mousedown");
        ea.arg[1] = ut_Int2Str(event.button.button);
        ea.len = 2;
        eq_Push(&event_queue, ea);
        return true;
      case SDL_MOUSEBUTTONUP:
        ea.arg[0] = strdup("mouseup");
        ea.arg[1] = ut_Int2Str(event.button.button);
        ea.len = 2;
        eq_Push(&event_queue, ea);
        return true;
      case SDL_MOUSEMOTION:
        ea.arg[0] = strdup("mousemotion");
        ea.arg[1] = ut_Int2Str(event.motion.x);
        ea.arg[2] = ut_Int2Str(event.motion.y);
        ea.len = 3;
        eq_Push(&event_queue, ea);
        return true;
    }
  } while (end > SDL_GetTicks64());
  return false;
}


static void
l_Hook(lua_State* L, lua_Debug* d)
{
  if (L == NULL)
    return;

  if (d->event == LUA_HOOKCALL)
  {
    GetEvent(0);
    currentTimeout = SDL_GetTicks64() + KILL_TIMEOUT;
    if (CPU_HZ != 0)
    {
      SDL_Delay(1000/CPU_HZ);
    }
  }
  else if (d->event == LUA_HOOKCOUNT)
  {
    if (SDL_GetTicks64() > currentTimeout)
    {
      puts("[C] Too long without yielding");
      longjmp(kill, 1);
    }
  }
}

// -------------- function -------------- //

// Computer //
static int
lf_CompGetTotal(lua_State* L)
{
  lua_pushnumber(L, 1024*1024*MAX_SIZE);
  return 1;
}

static int
lf_CompGetUsed(lua_State* L)
{
  lua_pushnumber(L, *pUd);
  return 1;
}

static int
lf_CompPullEvent(lua_State* L)
{
  lua_Number arg = luaL_checknumber(L, 1);
  if (event_queue.length == 0)
  {
    if(!GetEvent(arg))
      return 0;
  }

  EventArgs_t e = eq_Pop(&event_queue);
  for (int i = 0; i < e.len; i++)
  {
    lua_pushstring(L, e.arg[i]);
    free((void*)e.arg[i]);
  }
  return e.len;
}

static int
lf_CompPushEvent(lua_State* L)
{
  EventArgs_t ea;
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
    eq_Push(&event_queue, ea);
  return 0;
}

// GPU //
static int
lf_GpuGetResolution(lua_State* L)
{
  lua_pushnumber(L, sdl_GetWidth());
  lua_pushnumber(L, sdl_GetHeight());
  return 2;
}

static int
lf_GpuSetColor(lua_State* L)
{
  lua_Number col = luaL_checknumber(L, 1);
  currentColor = col;

  return 0;
}

static int
lf_GpuGetColor(lua_State* L)
{
  lua_pushnumber(L, currentColor);
  return 1;
}

static int
lf_GpuGetPixel(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, 1);
  lua_Number fy = luaL_checknumber(L, 2);

  lua_pushnumber(L, dt_GetPixel(pCan, fx-1, fy-1));

  return 1;
}


static int
lf_GpuFill(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, 1);
  lua_Number fy = luaL_checknumber(L, 2);
  lua_Number fw = luaL_checknumber(L, 3);
  lua_Number fh = luaL_checknumber(L, 4);

  dt_Rect(pCan, fx-1, fy-1, fw, fh, currentColor);
  return 0;
}

static int
lf_GpuCopy(lua_State* L)
{
  lua_Number fx =  luaL_checknumber(L, 1);
  lua_Number fy =  luaL_checknumber(L, 2);
  lua_Number fw =  luaL_checknumber(L, 3);
  lua_Number fh =  luaL_checknumber(L, 4);
  lua_Number fox = luaL_checknumber(L, 5);
  lua_Number foy = luaL_checknumber(L, 6);

  dt_Copy(pCan, fx-1, fy-1, fw, fh, fox, foy);
  return 0;
}

static int
lf_GpuClear()
{
  dt_Fill(pCan, currentColor);

  return 0;
}

static int
lf_GpuUpdate()
{
  sdl_Update();

  return 0;
}

// FileSystem //

static int
lf_FsExists(lua_State* L)
{
  const char * path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  char* npath = ut_Resolve(path);
  if(access(npath, F_OK) == 0)
  {
    free(npath);
    lua_pushboolean(L, true);
    return 1;
  }
  free(npath);
  lua_pushboolean(L, false);
  return 1;
}

static int
lf_FsIsDir(lua_State* L)
{
  const char * path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  char* npath = ut_Resolve(path);
  DIR* dir = opendir(npath);
  free(npath);
  if (dir)
  {
    closedir(dir);
    lua_pushboolean(L, true);
    return 1;
  }
  else
  {
    lua_pushboolean(L, false);
    return 1;
  }
}

static int
lf_FsMkDir(lua_State* L)
{
  const char * path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  char* npath = ut_Resolve(path);
  int ret = mkdir(npath, S_IRWXU);
  free(npath);
  if (ret == -1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  else
  {
    lua_pushboolean(L, true);
    return 1;
  }
}

static int
lf_FsRmFile(lua_State* L)
{
  const char * path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  char* npath = ut_Resolve(path);
  if (remove(npath) == 0)
  {
    free(npath);
    lua_pushboolean(L, true);
    return 1;
  }
  else
  {
    free(npath);
    lua_pushboolean(L, false);
    return 1;
  }

}

static int
lf_FsRmDir(lua_State* L)
{
  const char * path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  char* npath = ut_Resolve(path);
  if (rmdir(npath) == 0)
  {
    free(npath);
    lua_pushboolean(L, true);
    return 1;
  }
  else
  {
    free(npath);
    lua_pushboolean(L, false);
    return 1;
  }

}

static int
lf_FsOpen(lua_State* L)
{
  const char* path = lua_tostring(L, 1);
  const char* mode = lua_tostring(L, 2);
  if (path == NULL || mode == NULL)
  {
    lua_pushboolean(L, false);
    return 1;
  }

  char* npath = ut_Resolve(path);
  int fd = fdc_OpenFile(npath, mode);
  free(npath);
  if (fd == -1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  lua_pushnumber(L, fd);
  return 2;
}

static int
lf_FsClose(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, 1);

  if (fdc_CloseFile(fd) == 0)
  {
    lua_pushboolean(L, true);
    return 1;
  }
  lua_pushboolean(L, false);
  return 1;
}

static int
lf_FsWrite(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, 1);
  const char* data = lua_tostring(L, 2);

  if (data == NULL)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  if (fdc_WriteFile(fd, data) == 1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  return 1;
}

static int
lf_FsRead(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, 1);
  lua_Number bytes = luaL_checknumber(L, 2);

  char* data = malloc(sizeof(char)*(bytes+1));
  int res = fdc_ReadFile(fd, bytes, data);
  if (res == 1)
  {
    free(data);
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  lua_pushstring(L, data);
  free(data);
  return 2;
}

static int
lf_FsSeek(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, 1);
  lua_Number type = luaL_checknumber(L, 2);
  lua_Number offset = luaL_checknumber(L, 3);

  if (fdc_SeekFile(fd, type, offset) == 1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  return 1;
}

static int
lf_FsGetPos(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, 1);
  int res = fdc_GetposFile(fd);
  if (res == -1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  lua_pushnumber(L, res);
  return 2;
}

static int
lf_FsListDir(lua_State* L)
{
  const char* path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }

  int num;
  char* npath = ut_Resolve(path);
  Entry_t* entries = fdc_ListDir(npath, &num);
  free(npath);
  if (entries == NULL)
  {
    lua_pushboolean(L, false);
    return 1;
  }


  lua_pushboolean(L, true);
  lua_newtable(L);

  for (int i = 0; i < num; i++)
  {
    lua_pushinteger(L, i+1);
    lua_pushstring(L, entries[i].name);
    lua_settable(L, -3);
  }
  free(entries);

  return 2;
}

static int
lf_FsRename(lua_State* L)
{
  const char* path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }

  char* new;
  char* npath = ut_Resolve(path);
  if (rename(npath, new) == 0)
  {
    free(npath);
    lua_pushboolean(L, true);
    return 1;
  }
  else
  {
    free(npath);
    lua_pushboolean(L, false);
    return 1;
  }
}

static int
lf_FsSize(lua_State* L)
{
  const char* path = lua_tostring(L, 1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }
  struct stat sb;
  char* npath = ut_Resolve(path);
  if (stat(npath, &sb) == 0)
  {
    free(npath);
    lua_pushboolean(L, true);
    lua_pushnumber(L, sb.st_size);
    return 2;
  }
  else
  {
    free(npath);
    lua_pushboolean(L, false);
    return 1;
  }

}

// Modifications //

static int
lf_CoroCreate(lua_State* L)
{
  puts("[C] Coro create e");
  lua_State* NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);
  lua_xmove(L, NL, 1);
  lua_sethook(NL, l_Hook, LUA_MASKCOUNT | LUA_MASKCALL, 1000);
  return 1;
}

// -------------------------------------- //

static const luaL_Reg computerlib[] =
{
  {"gettotal", lf_CompGetTotal},
  {"getused", lf_CompGetUsed},
  {"pullevent", lf_CompPullEvent},
  {"pushevent", lf_CompPushEvent},
  {NULL, NULL}
};

static int
luaopen_Computer (lua_State* L)
{
  luaL_newlib(L, computerlib);  /* new module */
  return 1;
}

static const luaL_Reg gpulib[] =
{
  {"setcolor", lf_GpuSetColor},
  {"getcolor", lf_GpuGetColor},
  {"getpixel", lf_GpuGetPixel},
  {"clear", lf_GpuClear},
  {"fill", lf_GpuFill},
  {"copy", lf_GpuCopy},
  {"update", lf_GpuUpdate},
  {"getresolution", lf_GpuGetResolution},
  {NULL, NULL}
};

static int
luaopen_Gpu (lua_State* L)
{
  luaL_newlib(L, gpulib);  /* new module */
  return 1;
}

static const luaL_Reg fslib[] =
{
  {"exists", lf_FsExists},
  {"isdir", lf_FsIsDir},
  {"mkdir", lf_FsMkDir},
  {"rmdir", lf_FsRmDir},
  {"rmfile", lf_FsRmFile},
  {"open", lf_FsOpen},
  {"close", lf_FsClose},
  {"read", lf_FsRead},
  {"write", lf_FsWrite},
  {"seek", lf_FsSeek},
  {"getpos", lf_FsGetPos},
  {"listdir", lf_FsListDir},
  {"rename", lf_FsRename},
  {"size", lf_FsSize},
  {NULL, NULL}
};

static int
luaopen_Fs (lua_State* L)
{
  luaL_newlib(L, fslib);
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
  {LUA_DBLIBNAME, luaopen_debug},
  {"computer", luaopen_Computer},
  {"gpu", luaopen_Gpu},
  {"filesystem", luaopen_Fs},
  {NULL, NULL}
};


static const luaL_Reg preloadedlibs[] =
{
  {NULL, NULL}
};


static void
OpenLibs (lua_State* L)
{
  const luaL_Reg* lib;
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

static void
ModifyLibs(lua_State* L)
{
  if (DISABLE_PRINT == 1)
  {
    lua_pushnil(L);
    lua_setglobal(L, "print");
  }
  lua_pushnil(L);
  lua_setglobal(L, "loadfile");
  lua_pushnil(L);
  lua_setglobal(L, "dofile");
  lua_pushnil(L);
  lua_setglobal(L, "module");
  lua_pushnil(L);
  lua_setglobal(L, "require");

  lua_getglobal(L, "coroutine");
  lua_pushcfunction(L, lf_CoroCreate);
  lua_setfield(L, -2, "create");
  lua_pop(L, 1);

  lua_getglobal(L, "debug");
  lua_pushnil(L);
  lua_setfield(L, -2, "setuservalue");
  lua_pushnil(L);
  lua_setfield(L, -2, "getmetatable");
  lua_pushnil(L);
  lua_setfield(L, -2, "gethook");
  lua_pushnil(L);
  lua_setfield(L, -2, "getregistry");
  lua_pushnil(L);
  lua_setfield(L, -2, "getuservalue");
  lua_pushnil(L);
  lua_setfield(L, -2, "setmetatable");
  lua_pushnil(L);
  lua_setfield(L, -2, "setupvalue");
  lua_pushnil(L);
  lua_setfield(L, -2, "upvaluejoin");
  lua_pushnil(L);
  lua_setfield(L, -2, "getupvalue");
  lua_pushnil(L);
  lua_setfield(L, -2, "sethook");
  lua_pushnil(L);
  lua_setfield(L, -2, "upvalueid");
  lua_pushnil(L);
  lua_setfield(L, -2, "debug");
  lua_pop(L, 1);
}

void
lua_DeInitLua()
{
  if (L != NULL)
    lua_close(L);
  fdc_CloseAll();
  free(pUd);
}

int
lua_InitLua(/*lua_State* NL*/)
{
  char* val = cfg_GetValue("ram_size");
  if (val != NULL)
  {
    int ival = atoi(val);
    if (ival >= 0)
    {
      MAX_SIZE = ival;
    }
    else
    {
      MAX_SIZE = 1;
    }
  }
  val = cfg_GetValue("kill_timeout");
  if (val != NULL)
  {
    int ival = atoi(val);
    if (ival >= 0)
    {
      KILL_TIMEOUT = ival*1000;
    }
    else
    {
      KILL_TIMEOUT = 10*1000;
    }
  }
  val = cfg_GetValue("cpu_clock");
  if (val != NULL)
  {
    int ival = atoi(val);
    if (ival >= 0)
    {
      CPU_HZ = ival;
    }
    else if (ival == -1)
    {
      CPU_HZ = 0;
    }
    else
    {
      CPU_HZ = 100;
    }
  }

  pUd = (uint64_t*)malloc(sizeof(uint64_t));
  *pUd = 0;
  L = lua_newstate(l_alloc, pUd);
  if (L == NULL)
  {
    printf("[E] Error in creating new lua state\n");
    return 1;
  }
  OpenLibs(L);
  ModifyLibs(L);
  return 0;
}

void
lua_Start()
{
  pCan = sdl_GetCanvas();
  OpenLibs(L);
  ModifyLibs(L);

  sdl_RendClear();
  lua_sethook(L, l_Hook, LUA_MASKCOUNT | LUA_MASKCALL, 1000);

  currentTimeout = SDL_GetTicks64() + KILL_TIMEOUT;
  char* rootpath = cfg_GetValue("root_path");
  char* dopath;
  if (rootpath != NULL)
  {
    dopath = ut_PathAdd(rootpath, "/init.lua");
  }
  else
    dopath = strdup("./FS/init.lua");

  if (setjmp(kill) == 0)
    luaL_dofile(L, dopath);
  free(dopath);
}

