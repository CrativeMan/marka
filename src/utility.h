#ifndef UTILITY_H
#define UTILITY_H

#define ABUF_INIT {NULL, 0}
#define PEB_VERSION "2.0"
#define PEB_TAB_STOP 2
#define CTRL_KEY(k) ((k) & 0x1f)

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)


struct abuf {
  char *b;
  int len;
};

int is_seperator(int c);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif