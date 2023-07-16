#ifndef DANILUS_CONFIG_H
#define DANILUS_CONFIG_H

#define keylen 50
#define valuelen 50
#define maxparams 10

typedef struct
{
  char key[keylen+1];
  char val[valuelen+1];
} cfg_KeyValue;

int cfg_ReadConfig(const char* path);
char* cfg_GetValue(const char* key);

#endif
