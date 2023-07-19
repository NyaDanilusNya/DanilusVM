#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <setjmp.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
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
static queue_t event_queue;
static d_Canvas* pCan;

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
        //RendUpdate();
        break;
      case SDL_KEYDOWN:
        ea.arg[0] = strdup("keydown");
        ea.arg[1] = int2str(event.key.keysym.scancode);
        ea.len = 2;
        queue_push(&event_queue, ea);
        return true;
        /*
        lua_pushstring(L, "keydown");
        lua_pushnumber(L, event.key.keysym.scancode);
        */
      case SDL_KEYUP:
        ea.arg[0] = strdup("keydup");
        ea.arg[1] = int2str(event.key.keysym.scancode);
        ea.len = 2;
        queue_push(&event_queue, ea);
        return true;
        /*
        lua_pushstring(L, "keyup");
        lua_pushnumber(L, event.key.keysym.scancode);
        */
    }
  }
  return false;
}


static void
l_hook(lua_State* L, lua_Debug* d)
{
  if (L == NULL)
    return;

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

static int
lf_gettotal(lua_State* L)
{
  lua_pushnumber(L, 1024*1024*MAX_SIZE);
  return 1;
}

static int
lf_getused(lua_State* L)
{
  lua_pushnumber(L, *pUd);
  return 1;
}

static int
lf_pullevent(lua_State* L)
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
}

static int
lf_pushevent(lua_State* L)
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
static int
lf_gpusetcolor(lua_State* L)
{
  lua_Number col = luaL_checknumber(L, -1);
  currentColor = col;

  return 0;
}

static int
lf_gpugetcolor(lua_State* L)
{
  lua_pushnumber(L, currentColor);
  return 1;
}

static int
lf_gpugetpixel(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, -2);
  lua_Number fy = luaL_checknumber(L, -1);

  lua_pushnumber(L, d_getPixel(pCan, fx-1, fy-1));

  return 1;
}


static int
lf_gpufill(lua_State* L)
{
  lua_Number fx = luaL_checknumber(L, -4);
  lua_Number fy = luaL_checknumber(L, -3);
  lua_Number fw = luaL_checknumber(L, -2);
  lua_Number fh = luaL_checknumber(L, -1);

  d_rect(pCan, fx-1, fy-1, fw, fh, currentColor);
  return 0;
}

static int
lf_gpucopy(lua_State* L)
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

static int
lf_gpuclear()
{
  d_fill(pCan, currentColor);

  return 0;
}

static int
lf_gpuupdate()
{
  sdl_Update();

  return 0;
}

// FileSystem //

static int
lf_fsexists(lua_State* L)
{
  const char * path = lua_tostring(L, -1);
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
lf_fsisdir(lua_State* L)
{
  const char * path = lua_tostring(L, -1);
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
lf_fsmkdir(lua_State* L)
{
  const char * path = lua_tostring(L, -1);
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
lf_fsrmfile(lua_State* L)
{
  const char * path = lua_tostring(L, -1);
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
lf_fsrmdir(lua_State* L)
{
  const char * path = lua_tostring(L, -1);
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
lf_fsopen(lua_State* L)
{
  const char* path = lua_tostring(L, -2);
  const char* mode = lua_tostring(L, -1);
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
lf_fsclose(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, -1);

  if (fdc_CloseFile(fd) == 0)
  {
    lua_pushboolean(L, true);
    return 1;
  }
  lua_pushboolean(L, false);
  return 1;
}

static int
lf_fswrite(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, -2);
  const char* data = lua_tostring(L, -1);

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
lf_fsread(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, -2);
  lua_Number bytes = luaL_checknumber(L, -1);

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
lf_fsseek(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, -3);
  lua_Number type = luaL_checknumber(L, -2);
  lua_Number offset = luaL_checknumber(L, -1);

  if (fdc_SeekFile(fd, type, offset) == 1)
  {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, true);
  return 1;
}

static int
lf_fsgetpos(lua_State* L)
{
  lua_Number fd = luaL_checknumber(L, -1);
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
lf_fslistdir(lua_State* L)
{
  const char* path = lua_tostring(L, -1);
  if (path == NULL || path[0] != '/')
  {
    lua_pushboolean(L, false);
    return 1;
  }

  int num;
  char* npath = ut_Resolve(path);
  ENTRY* entries = fdc_ListDir(npath, &num);
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
lf_fsrename(lua_State* L)
{
  const char* path = lua_tostring(L, -1);
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
lf_fssize(lua_State* L)
{
  const char* path = lua_tostring(L, -1);
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
lf_corocreate(lua_State* L)
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
  {"gettotal", lf_gettotal},
  {"getused", lf_getused},
  {"pullevent", lf_pullevent},
  {"pushevent", lf_pushevent},
  {NULL, NULL}
};

static int
luaopen_computer (lua_State* L)
{
  luaL_newlib(L, computerlib);  /* new module */
  return 1;
}

static const luaL_Reg gpulib[] =
{
  {"setcolor", lf_gpusetcolor},
  {"getcolor", lf_gpugetcolor},
  {"getpixel", lf_gpugetpixel},
  {"clear", lf_gpuclear},
  {"fill", lf_gpufill},
  {"copy", lf_gpucopy},
  {"update", lf_gpuupdate},
  {NULL, NULL}
};

static int
luaopen_gpu (lua_State* L)
{
  luaL_newlib(L, gpulib);  /* new module */
  return 1;
}

static const luaL_Reg fslib[] =
{
  {"exists", lf_fsexists},
  {"isdir", lf_fsisdir},
  {"mkdir", lf_fsmkdir},
  {"rmdir", lf_fsrmdir},
  {"rmfile", lf_fsrmfile},
  {"open", lf_fsopen},
  {"close", lf_fsclose},
  {"read", lf_fsread},
  {"write", lf_fswrite},
  {"seek", lf_fsseek},
  {"getpos", lf_fsgetpos},
  {"listdir", lf_fslistdir},
  {"rename", lf_fsrename},
  {"size", lf_fssize},
  {NULL, NULL}
};

static int
luaopen_fs (lua_State* L)
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
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_BITLIBNAME, luaopen_bit32},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_DBLIBNAME, luaopen_debug},
  {"computer", luaopen_computer},
  {"gpu", luaopen_gpu},
  {"filesystem", luaopen_fs},
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
  lua_getglobal(L, "coroutine");
  lua_pushcfunction(L, lf_corocreate);
  lua_setfield(L, -2, "create");
  lua_setglobal(L, "coroutine");
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
    else
    {
      CPU_HZ = 10;
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
  lua_sethook(L, l_hook, LUA_MASKCOUNT | LUA_MASKCALL, 1000);

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

