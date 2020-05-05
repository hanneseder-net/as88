#ifndef TRCE_SRC_DOSCURS_H
#define TRCE_SRC_DOSCURS_H

#include <stddef.h>

// TODO(heder): Can we make this private?
extern char window[24][81];

void winfirst(void);
void immain(void);
void wmv(int y, int x);
void wprintf(int y, int x, const char* format, ...);
void wwrite(int y, int x, const char* s);
void wnwrite(int y, int x, const char* s, size_t n);
void refresh(void);

#endif /* TRCE_SRC_DOSCURS_H */
