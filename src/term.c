#include "term.h"
#include "error.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int editorReadKey() {
  int nread; // num of bytes read
  char c;    // char to be read in
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  if (c == '\x1b') { // if character is escape char
    char seq[3];

    // read 2 more bytes into buffer
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';

    if (seq[0] == '[') {                    // if starts with [
      if (seq[1] >= '0' && seq[1] <= '9') { // if char is num between 0 and 9
        if (read(STDIN_FILENO, &seq[2], 1) != 1) // read 3rd byte
          return '\x1b';
        if (seq[2] == '~') { // if is tilde
          switch (seq[1]) {  // switch the nums between 0 and 9
          case '1':
            return HOME_KEY;
          case '3':
            return DEL_KEY;
          case '4':
            return END_KEY;
          case '5':
            return PAGE_UP;
          case '6':
            return PAGE_DOWN;
          case '7':
            return HOME_KEY;
          case '8':
            return END_KEY;
          }
        }
      } else { // if not is char betwwen 0 and 9
        switch (seq[1]) {
        case 'A':
          return ARROW_UP;
        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
    } else if (seq[0] == 'O') { // if first byte aftr escp seq is 0
      switch (seq[1]) {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }
    return '\x1b'; // if not recogniesd escp seq return \x1b
  } else {
    return c; // return norml char
  }
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  // return pos of cursor
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  // expected message is smth like \x1b[5;10R
  while (i < sizeof(buf) - 1) { // read message into buffer
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R') // if get R then break cause its eof
      break;
    i++;
  }
  buf[i] = '\0'; // terminate buffer

  if (buf[0] != '\x1b' || buf[1] != '[') // if format is somehow wrong
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) // get ints out of buffer
    return -1;

  return 0;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  // get the window szie and if it fails or the window width is 0 return -1
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // move cursor to the bottom left of the screen by
    // sending a long move to right and long move down signal
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return getCursorPosition(rows, cols);
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}