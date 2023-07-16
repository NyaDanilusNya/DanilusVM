#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "dtexture.h"
#include "event_queue.h"
#include "lua.h"
#include "utils.h"
#include "config.h"

int
main()
{
  ut_InitAll();

  lua_Start();

  ut_DeInitAll(0);
  return 0;
}

