#include "doscurs.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char nwindow[24][81];
char window[24][81];

static void clearscreen() { printf("\e[2J"); }

void wmv(int y, int x) { printf("\e[%d;%dH", y + 1, x + 1); }

void wnwrite(int y, int x, const char* s, size_t n) {
  // TODO(heder): Add clipping, i.e. don't write outside of the window.
  strncpy(&window[y][x], s, n);
}

void wwrite(int y, int x, const char* s) {
  wnwrite(y, x, s, strlen(s));
}

void wprintf(int y, int x, const char* format, ...) {
  va_list argp;
  va_start(argp, format);
  // TODO(heder): Add clipping.
  vsprintf(&window[y][x], format, argp);
  va_end(argp);
}

void wfill(int y, int x, int h, int w, char c) {
  for (int j = y; j < y + h; ++j) {
    for (int i = x; i < x + w; ++i) {
      window[j][i] = c;
    }
  }
}

static void wingo(void) {
  memset(window, ' ', sizeof(window));
  /*234567890123456789012345678901234567890123456789012345678901234567890*/
  wwrite(0, 0, "CS: 00  DS=SS=ES: 000 |=>---- | ");
  wwrite(1, 0, "AH:00 AL:00 AX:...... |  ---- | ");
  wwrite(2, 0, "BH:00 BL:00 BX:...... |  ---- | ");
  wwrite(3, 0, "CH:00 CL:00 CX:...... |  ---- | ");
  wwrite(4, 0, "DH:00 DL:00 DX:...... |  ---- | ");
  wwrite(5, 0, "SP: 0000 SF O D S Z C |  ---- | ");
  wwrite(6, 0, "BP: 0000 CC - - - - - |  ---- =>");
  wwrite(7, 0, "SI: 0000  IP:0000:PC  |  ---- | ");
  wwrite(8, 0, "DI: 0000  ........+0  |  ---- | ");
  wfill(9, 0, 1, 80, '-');
  wfill(16, 0, 1, 80, '-');
  wfill(10, 20, 6, 1, '|');
  memcpy(nwindow, window, sizeof(nwindow));
  for (int i = 0; i < 23; i++) {
    wmv(i, 0);
    printf("%-80.80s", window[i]);
  }
  wmv(23, 0);
  printf("%-79.79s", window[23]);
}

void winfirst(void) {
  clearscreen();
  wingo();
}

static void fixup_window_buffer() {
  // TODO(heder): Not quite sure, why we need this.
  // Turn '\0' into ' ' in the window buffer.
  for (char* p = window[0]; p < window[0] + sizeof(window); ++p) if (!(*p)) *p = ' ';
}

void immain(void) {
  int b = 1;
  char* p = window[0];
  char* q = nwindow[0];
  fixup_window_buffer();
  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 81; j++)
      if ((*p != *q) || (j != 80 && p < window[13] && p > window[10] + 22)) {
        *q = *p;
        if (b) wmv(i, j);
        putchar(*p);
        b = 0;
      } else {
        b = 1;
      }
    ++p;
    ++q;
  }
}

void refresh(void) {
  wingo();
  immain();
}
