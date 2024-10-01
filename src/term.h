#ifndef TERM_H
#define TERM_H

#include <termios.h>
enum editorModes { NORMAL = 0, INSERT };

enum editorHighlight {
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_STRING,
  HL_NUMBER,
  HL_MATCH
};

enum editorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
int editorReadKey();

#endif // TERM_
