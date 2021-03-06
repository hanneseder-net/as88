#ifndef AS_SRC_COMM7_H
#define AS_SRC_COMM7_H

#include <stdio.h>

#include "comm0.h"

valu_t load(item_t *ip);
int store(item_t *ip, valu_t val);
int combine(int typ1, int typ2, int op);
int small(int fitsmall, int gain);
int printx(int ndig, valu_t val);
char* remember(char *s);

void emit1(int arg);
void emit2(int arg);
void emit4(long arg);
void emitx(valu_t val, int n);
void emitstr(int zero);

void listline(int textline);

void assert2(char *file, int line);
void nosect(void);
void serror(char* s, ...);
void warning(char *s, ...);
void fatal(char *s, ...);
void nofit(void);
void wr_fatal(void);

void ffreopen(char *s, FILE *f);
FILE * ffcreat(char *s);
FILE* fftemp(char *path, char *tail);

void yyerror(char const *s);

#endif /* AS_SRC_COMM7_H */
