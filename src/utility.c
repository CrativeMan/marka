#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "utility.h"

int is_seperator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void abAppend(struct abuf *ab, const char *s, int len) {
  // allocate new mem for expanded string
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) // if realloc failed, return
    return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab) { free(ab->b); }