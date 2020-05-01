#ifndef AS_SRC_COMM7_H
#define AS_SRC_COMM7_H

#include "comm0.h"

int combine(int typ1, int typ2, int op);
int store(item_t *ip, valu_t val);

void assert2(char *file, int line);
void nosect(void);
void serror(char* s, ...);
void warning(char *s, ...);
void fatal(char *s, ...);
void nofit(void);
void wr_fatal(void);
void wr_close(void);

#endif /* AS_SRC_COMM7_H */
