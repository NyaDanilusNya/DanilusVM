#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "lua.h"
#include "sdl.h"
#include "config.h"

char*
int2str(uint64_t val)
{
  char* str = malloc((int)((ceil(log10(val+1))+1)*sizeof(char)));
  sprintf(str, "%ld", val);
  return str;
}

void
ut_DeInitAll(int err)
{
  lua_DeInitLua();
  sdl_DeInitSDL();
  exit(err);
}

void
ut_InitAll()
{
  if (cfg_ReadConfig("./vm.cfg") == 1)
    exit(1);
  sdl_InitSDL();
  lua_InitLua();
}

char*
ut_PathAdd(char* path, char* add)
{
  size_t len1 = strlen(path);
  size_t len2 = strlen(add);

  char* result = malloc(len1 + len2 + 1);

  if (result == NULL)
  {
    printf("Error while allocating memory.\n");
    return NULL;
  }

  strcpy(result, path);
  strcpy(result + len1, add);

  return result;
}

char*
ut_StrTrim(char *str)
{
  char *end;

  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
      return str;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';

  return str;
}

