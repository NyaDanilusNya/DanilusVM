#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
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

static void removeParentDirectory(char* path)
{
    char* parentDir = path;

    while ((parentDir = strstr(path, "/../")) != NULL) {
        char* previousSlash = parentDir;

        // Найдем предыдущий слеш перед "/../"
        while (previousSlash > path && *previousSlash != '/') {
            previousSlash--;
        }

        // Если найден слеш перед "/../", скопируем часть строки после "/../" после этого слеша
        if (*previousSlash == '/') {
            strcpy(previousSlash, parentDir + 3); // +3 для пропуска "/../"
        } else {
            // В противном случае, удаляем всю строку, так как "/../" в начале пути
            *parentDir = '\0';
        }
    }
}

char*
ut_PathAdd(const char* path,const char* add)
{
  size_t len1 = strlen(path);
  size_t len2 = strlen(add);

  char* result = malloc(len1 + len2 + 1);

  if (result == NULL)
  {
    puts("[C] Error while allocating memory");
    return NULL;
  }

  strcpy(result, path);
  strcpy(result + len1, add);

  return result;
}

char*
ut_Resolve(const char* path)
{
  char* npath = strdup(path);
  removeParentDirectory(npath);

  char* retpath = ut_PathAdd(cfg_GetValue("root_path"), npath);
  free(npath);
  return retpath;
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

