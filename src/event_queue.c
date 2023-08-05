// This file was written by Eldyj ^^
// https://github.com/Eldyj

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "event_queue.h"

void
eq_Push(Queue_t *q, EventArgs_t val) {
  ++q->length;
  q->values = realloc(q->values, q->length * sizeof(EventArgs_t));
  q->values[q->length-1] = val;
}

EventArgs_t
eq_Pop(Queue_t *q) {
  EventArgs_t val = q->values[0];

  for (size_t i = 1; i < q->length; ++i) {
    q->values[i-1] = q->values[i];
  }

  --q->length;
  q->values = realloc(q->values, q->length * sizeof(EventArgs_t));
  return val;
}

