#ifndef TRCE_SRC_DOSCURS_H
#define TRCE_SRC_DOSCURS_H

// TODO(heder): Can we make this private?
extern char window[24][81];

void winfirst(void);
void immain(void);
void wmv(int y, int x);
void wwrite(int y, int x, char* s);
void refresh(void);

#endif /* TRCE_SRC_DOSCURS_H */
