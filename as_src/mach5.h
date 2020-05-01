#ifndef AS_SRC_MACC5_H
#define AS_SRC_MACH5_H

#include "comm0.h"

void ea_1(int param);
void ea_2(int param);

void branch(int opc, expr_t exp);
void pushop(int opc);
void addop(int opc);
void rolop(int opc);
void incop(int opc);
void callop(int opc);
void xchg(int opc);
void test(int opc);
void mov(int opc);
void imul(int opc);

void indexed(void);
void regsize(int sz);

#endif /* AS_SRC_MACH5_H */
