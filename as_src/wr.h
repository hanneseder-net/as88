#ifndef AS_SRC_WR_H
#define AS_SRC_WR_H

#include "obj.h"
#include "out.h"

void wr_name(struct outname	*name, unsigned int cnt);
void wr_relo(struct outrelo	*relo, unsigned int cnt);
void __wr_flush(struct fil *ptr);

#endif /* AS_SRC_WR_H */
