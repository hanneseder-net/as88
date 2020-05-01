#ifndef TRCE_SRC_UTIL_H
#define TRCE_SRC_UTIL_H

void dump(void);
void procdepth(int s);
void breakpt(void);
void meldroutine(void);
void spare(int t);
void notim(int t);
void interrupt(int t);
void panic(char *s);
int util_main(int argc, char **argv);

#endif /* TRCE_SRC_UTIL_H */
