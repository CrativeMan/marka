/* includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/* defines */
#define MARKA_VERSION "0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

/* data */
struct editorConfig {
  int cx, cy;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

struct editorConfig E;

/* terminal */
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr"); // flush original terminal sttings
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode); // run at exit

  struct termios raw = E.orig_termios;
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

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  if (c == '\x1b') { // if character is escape sequence
    char seq[3];

    // read 2 more bytes into buffer
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';

    if (seq[0] == '[') { // if starts with [
      switch (seq[1]) {
      case 'A': // up arrow
        return 'w';
      case 'B': // down arrow
        return 's';
      case 'C': // right arrow
        return 'd';
      case 'D': // left arrow
        return 'a';
      }
    }
    return '\x1b';
  } else {
    return c;
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

/* append buffer */

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

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

/* output */
void editorDrawRows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) { // for every row
    if (y == E.screenrows / 3) {       // if row is third down monitor
      char welcome[80];
      int welcomelen = snprintf(welcome, sizeof(welcome),
                                "Marka editor -- version %s", MARKA_VERSION);
      if (welcomelen > E.screencols)
        welcomelen = E.screencols;
      int padding = (E.screencols - welcomelen) / 2;
      if (padding) {
        abAppend(ab, "~", 1);
        padding--;
      }
      while (padding--)
        abAppend(ab, " ", 1);
      abAppend(ab, welcome, welcomelen);
    } else { // if not welcome message row
      abAppend(ab, "~", 1);
    }

    abAppend(ab, "\x1b[K", 3); // clear line currenlty operating on
    if (y < E.screenrows - 1) {
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  struct abuf ab = ABUF_INIT; // make new abuf

  abAppend(&ab, "\x1b[?25l", 6); // hide cursor
  abAppend(&ab, "\x1b[H", 3);    // cursor to 1,1

  editorDrawRows(&ab);

  char buf[32];
  // cursor to cx and cy
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6); // show cursor

  write(STDOUT_FILENO, ab.b, ab.len); // write to screen
  abFree(&ab);
}

/* input */

void editorMoveCursor(char key) {
  switch (key) {
  case 'a':
    E.cx--;
    break;
  case 'd':
    E.cx++;
    break;
  case 'w':
    E.cy--;
    break;
  case 's':
    E.cy++;
    break;
  }
}

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'): // case CTRL+Q
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;

  case 'w':
  case 's':
  case 'a':
  case 'd':
    editorMoveCursor(c);
    break;
  }
}

/* init */
void initEditor() {
  E.cx = 0;
  E.cy = 0;
  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSiza");
}

int main(int argc, char **argv) {
  enableRawMode();
  initEditor();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return 0;
}
