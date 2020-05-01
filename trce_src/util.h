#ifndef TRCE_SRC_UTIL_H
#define TRCE_SRC_UTIL_H

#include <stdio.h>

extern FILE *bituit;

extern int lfptr; /* load file pointer */

extern int termbitsize;
extern char bmbuf[9000];

extern int errflag;

void meldroutine(void);
void erroutine(void);
void spare(int t);
void notim(int t);
void interrupt(int t);
void panic(char *s);
int getint(FILE *f);
int getsh(FILE *f);
void bitmapdump(int b, int h, char *buff);
void bitmapopen(int b, int h, int s);
void schrijf(void);
int spiegel(int m, int n);
void schrijfmap(int b, int h, int s, char *buf, FILE *uitf);

#endif /* TRCE_SRC_UTIL_H */
