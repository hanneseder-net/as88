#ifndef AS_SRC_COMM6_H
#define AS_SRC_COMM6_H

#include "comm0.h"

void newbase(valu_t base);
void newident(item_t *ip, short typ);
void newlabel(item_t *ip);
void newrelo(int s, int n);
void newsymb(char* name, unsigned short type, unsigned short desc, valu_t valu);
void switchsect(int newtyp);

#endif /* AS_SRC_COMM6_H */
