#ifndef AS_SRC_COMM5_H
#define AS_SRC_COMM5_H

#include "comm0.h"

int hash(char *p);
int nextchar(void);
void putval(int c);
item_t* fb_alloc(int lab);
item_t * fb_shift(int lab);
item_t* item_alloc(int typ);
void item_insert(item_t *ip, int h);
item_t* item_search(char *p);

int yylex(void);

#endif /* AS_SRC_COMM5_H */
