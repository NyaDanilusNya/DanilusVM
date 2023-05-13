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
} event_args_t;

typedef struct {
  size_t length;
  event_args_t* values;
} queue_t;

void queue_push(queue_t *q, event_args_t val);

event_args_t queue_pop(queue_t *q);

