// This file was written by Eldyj ^^
// https://github.com/Eldyj

#include <stddef.h>

/*
typedef enum {
  atom_number,
  atom_string,
  //...
} atom_kind_t;

typedef struct {
  atom_kind_t type;
  int number;
  char *string;
} atom_t;
*/

typedef struct {
  const char* arg[8];
  int len;
} EventArgs_t;

typedef struct {
  size_t length;
  EventArgs_t* values;
} Queue_t;

void eq_Push(Queue_t *q, EventArgs_t val);

EventArgs_t eq_Pop(Queue_t *q);

