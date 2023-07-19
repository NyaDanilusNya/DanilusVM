#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "utils.h"

cfg_KeyValue params[maxparams];

char*
cfg_GetValue(const char* key)
{
  for (int i = 0; i < 10; i++)
  {
    if (strcmp(params[i].key, key) == 0)
    {
      return params[i].val;
    }
  }
  return NULL;
}

int
cfg_ReadConfig(const char* path)
{
  FILE* f = fopen(path, "r");

  if (f == NULL)
  {
    puts("Error while opening config file.");
    return 1;
  }

  int i = 0;
  char line[keylen+1+valuelen+1]; // key len + equals sign + value len + terminator
  while (fgets(line, sizeof(line), f))
  {
    char * test = ut_StrTrim(line);

    if (test[0] == '#' || test[0] == '\0')
      continue;

    char* key = strtok(line, "=");
    char* val = strtok(NULL, "\n");
    if (key == NULL || val == NULL)
    {
      puts("Error while parsing key=value. Skiping.");
      continue;
    }
    key = ut_StrTrim(key);
    val = ut_StrTrim(val);

    strncpy(params[i].key, key, sizeof(params[i].key));
    strncpy(params[i].val, val, sizeof(params[i].val));
    i++;
    if (i > maxparams)
    {
      puts("Max params reached.");
      return 1;
    }
  }

  fclose(f);
  return 0;
}
