#ifndef AS_SRC_COMM7_H
#define AS_SRC_COMM7_H

void assert2(char *file, int line);
void nosect(void);
void serror(char* s, ...);
void warning(char *s, ...);
void fatal(char *s, ...);
void nofit(void);
void wr_fatal(void);
void wr_close(void);

#endif /* AS_SRC_COMM7_H */