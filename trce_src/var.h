#ifndef TRCE_SRC_VAR_H
#define TRCE_SRC_VAR_H

// TODO(heder): These could move to .c files.
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

char *traceptr;			/* ptr to word to be traced */
char *prevpcx;			/* previous pcx (used for dumping) */

int a_used;			/* set if -a flag present */
char *alow, *ahigh;		/* turn dumping on and off by address */
char *breakadr;			/* breakpoint address */
int breakcount;			/* cause break when this reaches 0 */
int c_used;			/* set if -c flag present */
char *c_adr;			/* address to count */
long c_count;			/* counter used by -c flag */
int e_used;			/* set if -e flag present */
int f_used;			/* set by -f flag (explicit input file) */
int h_used;			/* set to enable histogram output */
int i_used;			/* set to enable counting of instructions */
long m_used;			/* how often to sample */
long n_used;			/* when to begin dumping */
int p_used;			/* counts number of -p flags read */
int s_used;			/* counts number of -s flags read */
char *startadr; 		/* starting address (hex) */
int u_used;			/* set if -u flag present */
int wval;			/* value of 'w' arg */
int y_used;			/* counts number of -y flags read */
long z_used;			/* # instructions to execute */

char hexfill;
int traceflag;			/* set to 1 if tracing of 'traceptr' enabled */
long instcount;			/* count instructions executed */
word initseg;			/* initial value of all the segment registers */

#define CLICKSIZE 16
#define CLICKSHIFT 4
#define NCLICKS (MEMBYTES/16)
long histo[NCLICKS];

int dumphdr;

#endif /* TRCE_SRC_VAR_H */
