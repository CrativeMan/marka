#include <termios.h>
#include <stdlib.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // flush original terminal sttings
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode); // run at exit
  
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO); // disable output
  
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(int argc, char **argv) {
  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    ;
  return 0;
}
