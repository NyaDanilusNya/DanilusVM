#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
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
  FILE* f;
  if(access(path, F_OK) != 0)
  {
    f = fopen(path, "w");
    if (f == NULL)
    {
      fprintf(stderr, "Error while writing default config file.\n");
      return 1;
    }
    fprintf(f, "# in MB\nram_size=8\n# in MB\nvram_size=1\n# in seconds\nkill_timeout=10\n# 0 - no delay. 1-∞\ncpu_clock=0\n\nroot_path=./FS\n\n# resolution\nwindow_w = 800\nwindow_h = 600\n");
    fclose(f);
  }
  f = fopen(path, "r");

  if (f == NULL)
  {
    fprintf(stderr, "Error while opening config file.\n");
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
