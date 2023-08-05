#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include "fdcontrol.h"

#define MAX_ENTRIES 128

typedef struct
{
  FILE* file;
  bool isalive;
  char* path;
  char* mode;
} FileInfo_t;


static FileInfo_t files[10];

int
fdc_OpenFile(const char* path, const char* mode)
{
  for (int i = 0; i < 10; i++)
  {
    if (files[i].isalive == false)
    {
      FILE * file = fopen(path, mode);
      if (file == NULL)
        return -1;
      files[i].file = file;
      files[i].isalive = true;
      files[i].path = strdup(path);
      files[i].mode = strdup(mode);
      return i;
    }
  }
  return -1;
}

int
fdc_CloseFile(int fd)
{
  if (files[fd].isalive == true)
  {
    fclose(files[fd].file);
    files[fd].isalive = false;
    free(files[fd].path);
    free(files[fd].mode);
    return 0;
  }
  return 1;
}

int
fdc_WriteFile(int fd, const char* data)
{
  if (files[fd].isalive == false)
    return 1;
  if (files[fd].mode[0] == 'w' || files[fd].mode[0] == 'a' || strcmp(files[fd].mode, "rw") == 0)
  {
    fputs(data, files[fd].file);
    return 0;
  }
  return 1;
}

int
fdc_ReadFile(int fd, size_t bytes, char* buffer)
{
  if (files[fd].isalive == false )
    return 1;
  if (files[fd].mode[0] == 'r' || strcmp(files[fd].mode, "rw") == 0)
  {
    unsigned long size = fread(buffer, 1, bytes, files[fd].file);
    buffer[size] = '\0';
    if (size == 0)
      return 1;
    return 0;
  }
  return 1;
}

int
fdc_SeekFile(int fd, int type, size_t offset)
{
  /*
   * set = 0
   * cur = 1
   * end = 2
   * */
  if (files[fd].isalive == false)
    return 1;
  fseek(files[fd].file, offset, type);
  return 0;
}

long int
fdc_GetposFile(int fd)
{
  if (files[fd].isalive == false)
    return -1;

  return ftell(files[fd].file);
}


void
fdc_CloseAll()
{
  for (int i = 0; i < 10; i++)
  {
    fdc_CloseFile(i);
  }
}

Entry_t*
fdc_ListDir(const char* path, int* pNum)
{
  int num = 0;
  Entry_t* entries = malloc(sizeof(Entry_t)*MAX_ENTRIES);
  if (entries == NULL)
  {
    puts("[C] Memory alloc error");
    return NULL;
  }

  DIR* dir = opendir(path);
  if (dir == NULL)
  {
    puts("[C] Error while opening dir");
    return NULL;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      strncpy(entries[num].name, entry->d_name, sizeof(entries[num].name));
      num ++;
      if (num >= MAX_ENTRIES) {
        puts("[C] List of files overloaded");
        break;
      }
    }
  }

  closedir(dir);
  *pNum = num;

  return entries;
}

