#ifndef TRCE_SRC_UTIL_H
#define TRCE_SRC_UTIL_H

int main_lib(int argc, char **argv);

void dump(void);
void procdepth(int s);
void breakpt(void);
void meldroutine(void);
void spare(int t);
void notim(int t);
void interrupt(int t);
void panic(char *s);
void syscal(void);

#endif /* TRCE_SRC_UTIL_H */
