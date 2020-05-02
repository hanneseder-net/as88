#include "doscurs.h"

#include <stdio.h>
#include <string.h>

char nwindow[24][81];
char window[24][81];

static void clearscreen() { printf("\e[2J"); }

void wmv(int a, int b) { printf("\e[%d;%dH", a + 1, b + 1); }

static void wingo(void) {
  int i;
  memset(window, ' ', sizeof(window));
  /*234567890123456789012345678901234567890123456789012345678901234567890*/
  strncpy(window[0], "CS: 00  DS=SS=ES: 000 |=>---- | ", 32);
  strncpy(window[1], "AH:00 AL:00 AX:...... |  ---- | ", 32);
  strncpy(window[2], "BH:00 BL:00 BX:...... |  ---- | ", 32);
  strncpy(window[3], "CH:00 CL:00 CX:...... |  ---- | ", 32);
  strncpy(window[4], "DH:00 DL:00 DX:...... |  ---- | ", 32);
  strncpy(window[5], "SP: 0000 SF O D S Z C |  ---- | ", 32);
  strncpy(window[6], "BP: 0000 CC - - - - - |  ---- =>", 32);
  strncpy(window[7], "SI: 0000  IP:0000:PC  |  ---- | ", 32);
  strncpy(window[8], "DI: 0000  ........+0  |  ---- | ", 32);
  for (i = 0; i < 80; i++) window[16][i] = window[9][i] = '-';
  for (i = 0; i < 20; i++) window[13][i] = '-';
  for (i = 0; i < 6; i++) window[i + 10][20] = '|';
  memcpy(nwindow, window, sizeof(nwindow));
  for (i = 0; i < 23; i++) {
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

void immain(void) {
  int i, j, b;
  char *p, *q;
  b = 1;
  p = window[0] - 1;
  q = nwindow[0] - 1;
  for (i = 0; i < 23; i++)
    for (j = 0; j < 81; j++)
      if ((*(++p) != *(++q)) ||
          (j != 80 && p < window[13] && p > window[10] + 22)) {
        *q = *p;
        if (b) wmv(i, j);
        putchar(*p);
        b = 0;
      } else {
        b = 1;
      }
  for (j = 0; j < 81; j++)
    if ((*(++p) != *(++q)) ||
        (j != 80 && p < window[13] && p > window[10] + 22)) {
      *q = *p;
      if (b) wmv(i, j);
      putchar(*p);
      b = 0;
    } else {
      b = 1;
    }
}

void refresh(void) {
  wingo();
  immain();
}
