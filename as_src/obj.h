#ifndef AS_SRC_OBJ_H
#define AS_SRC_OBJ_H

/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */

#include <stdio.h>
#include "out.h"

#if defined(__i386__) || defined(i386)
#define BYTE_ORDER 0x0123
#endif

#if ! defined(BYTE_ORDER)
#define BYTE_ORDER 0x3210
#endif

#if (BYTE_ORDER == 0x3210 || BYTE_ORDER == 0x1032)
#define Xput2(i, c)	(((c)[0] = (i)), ((c)[1] = (i) >> 8))
#define put2(i, c)	{ int j = (i); Xput2(j, c); }
#else
#define Xput2(i, c)	(* ((short *) (c)) = (i))
#define put2(i, c)	Xput2(i, c)
#endif

#if BYTE_ORDER != 0x0123
#define put4(l, c)	{ long x=(l); \
			  Xput2((int)x,c); \
			  Xput2((int)(x>>16),(c)+2); \
			}
#else
#define put4(l, c)	(* ((long *) (c)) = (l))
#endif

#define SECTCNT	3	/* number of sections with own output buffer */
#define WBUFSIZ	BUFSIZ

struct fil {
	int	cnt;
	char	*pnow;
	char	*pbegin;
	long	currpos;
	int	fd;
	char	pbuf[WBUFSIZ];
};

extern struct fil __parts[];

#define	PARTEMIT	0
#define	PARTRELO	(PARTEMIT+SECTCNT)
#define	PARTNAME	(PARTRELO+1)
#define	PARTCHAR	(PARTNAME+1)
#ifdef SYMDBUG
#define PARTDBUG	(PARTCHAR+1)
#else
#define PARTDBUG	(PARTCHAR+0)
#endif
#define	NPARTS		(PARTDBUG + 1)

#define getsect(s)      (PARTEMIT+((s)>=(SECTCNT-1)?(SECTCNT-1):(s)))

#endif /* AS_SRC_OBJ_H */
