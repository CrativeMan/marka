#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[H", 3); // place cursor top left corner
  perror(s);
  exit(1);
}