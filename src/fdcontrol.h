#ifndef DANILUS_FDCONTROL_H
#define DANILUS_FDCONTROL_H

#include <stdio.h>

typedef struct {
    char name[256];
} Entry_t;

int fdc_OpenFile(const char* path, const char* mode);
int fdc_CloseFile(int fd);
int fdc_WriteFile(int fd, const char* data);
int fdc_ReadFile(int fd, size_t bytes, char* buffer);

/*
* set = 0
* cur = 1
* end = 2
* */
int fdc_SeekFile(int fd, int type, size_t offset);
long int fdc_GetposFile(int fd);
void fdc_CloseAll();
Entry_t* fdc_ListDir(const char* path, int* num);
#endif
