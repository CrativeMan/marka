/* includes */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* data */
struct termios orig_termios;

/* terminal */
void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH,
            &orig_termios) == -1) die("tcsetattr"); // flush original terminal sttings
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode); // run at exit

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // turn off output processing
  raw.c_oflag &= ~(OPOST);
  // bitmask
  raw.c_cflag |= (CS8);
  // disable output, disable canonical mode, disable CRTL-V -O,
  // disable canonical mode, disable CTRL-C -Z,
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // min num of bytes before read timesout
  raw.c_cc[VMIN] = 0;
  // max num of time to wait before read times out
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/* init */

int main(int argc, char **argv) {
  enableRawMode();

  while (1) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }
  return 0;
}
