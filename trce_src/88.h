#include <stdint.h>

/*
 * LOOK IN table.c for declarations of all of this stuff
 */

#ifndef EXTERN
#define EXTERN
#endif
#define CHECK 0 /* To turn on runtime checking, set CHECK 1 */

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN /* vax and the like */
#endif                /* LITTLE_ENDIAN */
#undef BIG_ENDIAN     /* sun and the like */

#ifdef LITTLE_ENDIAN
typedef struct {
  uint8_t lo;
  uint8_t hi;
} pair;

#define AL 0
#define AH 1
#define BL 2
#define BH 3
#define CL 4
#define CH 5
#define DL 6
#define DH 7
#endif

#ifdef BIG_ENDIAN
typedef struct {
  uint8_t hi;
  uint8_t lo;
} pair;

#define AL 1
#define AH 0
#define BL 3
#define BH 2
#define CL 5
#define CH 4
#define DL 7
#define DH 6
#endif

typedef union {
  pair b;
  int16_t w;
} reg;

#define AX 0
#define BX 1
#define CX 2
#define DX 3
#define SI 4
#define DI 5
#define BP 6
#define SP 7

#define ax r.rw[AX]
#define bx r.rw[BX]
#define cx r.rw[CX]
#define dx r.rw[DX]
#define si r.rw[SI]
#define di r.rw[DI]
#define bp r.rw[BP]
#define sp r.rw[SP]

#define al r.rc[AL]
#define ah r.rc[AH]
#define bl r.rc[BL]
#define bh r.rc[BH]
#define cl r.rc[CL]
#define ch r.rc[CH]
#define dl r.rc[DL]
#define dh r.rc[DH]

#define W00 AX /* Wxx used for submultiplexing, e.g. op 81 */
#define W01 CX
#define W02 DX
#define W03 BX
#define W04 SP
#define W05 BP
#define W06 SI
#define W07 DI

#define B00 AL
#define B01 CL
#define B02 DL
#define B03 BL
#define B04 AH
#define B05 CH
#define B06 DH
#define B07 BH

#define ea EA.w
#define ealo EA.b.lo
#define eahi EA.b.hi

#define ra RA.w
#define ralo RA.b.lo
#define rahi RA.b.hi

#define eop EOP.w
#define eoplo EOP.b.lo
#define eophi EOP.b.hi

#define rop ROP.w
#define roplo ROP.b.lo
#define rophi ROP.b.hi

#define ADDW 0 /* codes used for lazy condition code eval */
#define ADDB 1
#define ADCW 2
#define ADCB 3
#define INCW 4
#define INCB 5
#define SUBW 6
#define SUBB 7
#define SBBW 8
#define SBBB 9
#define DECW 10
#define DECB 11
#define BOOLW 12
#define BOOLC 13

/* Here are the values used in 'nextint' to tell which kind of interrupt next.*/
#define SEGOVER 1
#define CLOCK 2
#define TTYIN 3
#define TTYOUT 4
#define DISK 5
#define KBD 6
#define NDEV 8 /* number of I/O devices */

#define MEMBYTES 1048576L /* how many bytes does 8088 have? */
// #define MEMBYTES 32768 		/* how many bytes ew small simulator*/

EXTERN reg EA, RA, EOP, ROP;
EXTERN int ovf, dirf, intf, signf, zerof, cf; /* flag bits */
EXTERN char* pcx;                             /* pcx = &m[ (cs<<4) + pc] */
EXTERN char* xapc;
EXTERN char *eapc, *rapc;
EXTERN int16_t* rapw; /* eapw is unusable since it might be odd */
EXTERN int16_t* stkp; /* scratch variable used by PUSH and POP */

EXTERN uint16_t cs, ds, ss, es; /* contents of segment registers */
EXTERN long cs16;               /* cs16 = 16*cs  (= cs<<4) */

EXTERN unsigned timer, ticks, nextint;
EXTERN struct intstruct {
  long int_time;  /* time of next interrupt (in mach instrs) */
  int int_status; /* status information */
  int int_vector;
} intstruct[NDEV];

EXTERN long l, l1, l2;     /* scratch variables used for setting carry */
EXTERN short x, y, z;      /* used in lazy condition code evaluation */
EXTERN uint8_t xc, yc, zc; /* ditto */
EXTERN int ccop, ccvalid;  /* ditto */
#if CHECK
EXTERN long xx; /* scratch variable used for mem checking */
#endif
EXTERN uint8_t stopvlag, dumpt; /* ew dumping vlag and saved t */

/* The 8088 memory array is declared below. */
extern char m[MEMBYTES];
typedef union {
  uint8_t rc[16];
  int16_t rw[8];
} REG;
extern REG r;
extern int traceflag, instrcount, codelength;

void interp(void);
