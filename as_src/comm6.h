#ifndef AS_SRC_COMM6_H
#define AS_SRC_COMM6_H

#include "comm0.h"

void align(valu_t bytes);
void newequate(item_t *ip, int typ);
void newcomm(item_t *ip, valu_t val);
void newbase(valu_t base);
void newident(item_t *ip, short typ);
void newlabel(item_t *ip);
void newrelo(int s, int n);
void newsect(item_t *ip);
long new_string(char *s);
void newsymb(char* name, unsigned short type, unsigned short desc, valu_t valu);
void switchsect(int newtyp);

#endif /* AS_SRC_COMM6_H */
