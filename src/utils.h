#ifndef DANILUS_UTILS_H
#define DANILUS_UTILS_H

#include <stdint.h>

char* int2str(uint64_t val);
void ut_DeInitAll(int err);
void ut_InitAll();
char* ut_PathAdd(const char* path, const char* add);
char* ut_Resolve(const char* path);
char* ut_StrTrim(char *str);

#endif
