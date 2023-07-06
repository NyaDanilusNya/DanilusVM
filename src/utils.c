#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "lua.h"
#include "sdl.h"

char* int2str(uint64_t val)
{
  char* str = malloc((int)((ceil(log10(val+1))+1)*sizeof(char)));
  sprintf(str, "%ld", val);
  return str;
}

void ut_DeInitAll(int err)
{
  lua_DeInitLua();
  sdl_DeInitSDL();
  exit(err);
}

void ut_InitAll()
{
  sdl_InitSDL();
  lua_InitLua();
}
