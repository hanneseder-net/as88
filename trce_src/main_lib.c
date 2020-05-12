#include "main_lib.h"

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXTERN extern
#include "88.h"
#include "bitmap.h"
#include "doscurs.h"
#include "macro.h"

int traceflag; /* set to 1 if tracing is enabled */
int codelength, instrcount;

static char errbuf[292];

static int lfptr; /* load file pointer */

static int errflag;

#define CBUF 180
#define MAGIC 0X0201
#define MAXSHORT 0XFFFF
#define MAXSYMTAB 0XFFF

static char nulsymbol[]="NULLSYMBOL";
static time_t t1,t2;
static struct stat astat[2];

#define OUTBUFFER_SIZE 4
#define OUTBUFFER_LEN 58
static char outbuffer[OUTBUFFER_SIZE][OUTBUFFER_LEN];

static char cmdchar, cmdline[30];
static uint8_t prdepth, bprdepth; /*prdepth altijd bijhouden; bprdepth zetten bij +-= */
static int stckprdepth[20], prstckpos[20];
static int nsymtab, maxsp;

typedef struct { short pcp; uint8_t bprt; } bprveld;
static bprveld bparr[32];  /* break point fields */
typedef struct {char typ; char sct; short smb; int adrs;} relocveld;
static relocveld relocarr[1024]; /* relocation variables field */

static short lndotarr[0X2000],lnsymarr[0X2000],dotlnarr[0X6000];
static int lnfilarr[0X6000], maxln, symp;/*fileptr in source, max source, last symbol*/

static int cmdfl,inpfl; /*command or input from file*/
static char *syssp;  /*pointer in stack for conversion in system calls*/

typedef struct {
  int symvalue;
  char* symbol;
  int nextsym;
  int lnr;
  char symsect;
} tsymtab; /*symbol table stucture */
static tsymtab symtab[MAXSYMTAB];

static char tringfield[1600], stringfield[8000];
static char inbuf[2024],*inbpl,*inbpu;
static char  *datadm[7], datadarr[7][81], basename[CBUF];
static int datadp, /*pointer in datadarr */ symhash[32];  /* pointer in symbol table*/

typedef struct {
  int startad;
  int lengte;
  int fstart;
  int flen;
  int align;
} segmenthead;
static segmenthead segmhead[8];
typedef union {int ii; char *cp;} paramfield;
typedef union {int ii; char cp[140];} sscanfield;

static FILE *prog, *L, *CMD, *INP;
#ifdef DEBUG
static FILE *LOG;
#endif

/* forward decls */
static int lcs(char *p, int s);
static void symlcorr(int i);
static void erroutine(void);
static int getchcmd(void);
static void gtabstr(int i, char *a, FILE *f);
static void nextput(int c);
static void rdcmd(void);
static void relocate(int n);
static void winupdate(void);
static char* remove_ext(char* dest, char* path);
static void copy_args_onto_stack(int argc, char** argv);

static int getint(FILE *f) {
  int i,j,k;
  lfptr += 4;
  i = getc(f); j = getc(f);
  k = 0; k |= (i&255); k |= ((j&255)<<8);
  i = getc(f); j = getc(f);
  k |= ((i&255)<<16); k |= ((j&255)<<24);
  return(k);
}

static int getsh(FILE* f) {
  int i = getc(f);
  int j = getc(f);
  lfptr += 2;
  return ((i & 255) | ((j & 255) << 8));
}

// TODO(heder): This is an rather poor hash fuction for strings. Fix it.
static int hashstring(char *p) {
  int h;
  h = *p - 'A'; h &= 31;
  return(h);
}

// TODO(heder): This function needs a better name.
static int load_hash_file(const char* filename) {
  FILE* FH = NULL;
  if ((FH = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Cannot open %s\n", filename);
    return 1;
  }
  int i = 0;
  int j = 0;
  int a, b;
  while (fscanf(FH, "%d %d", &a, &b) > 0) {
    while (i <= a) lndotarr[i++] = b;
    while (j <= b) dotlnarr[j++] = a;
  }
  fclose(FH);
  return 0;
}

static int load(int argc, char **argv) {
  int i,ii,j,k,sections, outrelo, *pi;
  char *p;

  char fnameS[CBUF];
  char fname88[CBUF];

  memset(inbuf, '\0', sizeof(inbuf));
  memset(m, '\0', sizeof(m));

  remove_ext(basename, argv[0]);
  sprintf(fnameS, "%s.s", basename);
  sprintf(fname88, "%s.88", basename);

  if ((prog = fopen(fname88, "rb")) == NULL) {
    fprintf(stderr, "Warning: Interpreter 8088 could not find file %s\n",
            fname88);
    system("sleep 5");
  }
  if (stat(fnameS, astat)) {
    fprintf(stderr, "Warning: Does %s exist?\n", fnameS);
    system("sleep 2");
  }
  t1 = astat[0].st_mtime;
  if (stat(fname88, astat)) {
    fprintf(stderr, "Warning Does %s exist?\n", fname88);
    system("sleep 5");
  } else {
    t2 = astat[0].st_mtime;
  }
  if (t2 < t1) {
    fprintf(stderr, "Warning %s is older than %s.\n", fname88, fnameS);
    system("sleep 5");
  }
  if (traceflag) {
    char fnameL[CBUF];
    char fnamei[CBUF];
    char fnamet[CBUF];
    sprintf(fnameS, "%s.$", basename);
    sprintf(fnameL, "%s.#", basename);
    sprintf(fnamei, "%s.i", basename);
    sprintf(fnamet, "%s.t", basename);

#ifdef DEBUG
    fprintf(LOG, "Before open #-file |%s|\n", fnameL);
    fflush(LOG);
#endif
    if (load_hash_file(fnameL)) {
      return 1;
    }
    if ((INP = fopen(fnamei, "r")) != NULL)
      inpfl = 1;
    else {
      inpfl = 0;
      INP = stdin;
    }
    if ((CMD = fopen(fnamet, "r")) != NULL)
      cmdfl = 1;
    else {
      cmdfl = 0;
      CMD = stdin;
    }
    if ((L = fopen(fnameS, "rb")) == NULL) {
      fprintf(stderr, "Cannot open %s\n", fnameS);
      return 1;
    }
    int tstrl = 0;
    lnfilarr[1] = lnfilarr[0] = 0;
    stckprdepth[0] = 1;
    for (i = 2; i < 0Xff8; i++) {
      while ((j = getc(L)) != EOF) {
        tstrl++;
        if (j == '\n') break;
      }
      if (j == EOF) break;
      lnfilarr[i] = tstrl;
    }
    maxln = i;
    rewind(L);
  }
  for (unsigned int i = 0; i < ARRAYSIZE(datadm); i++) datadm[i] = NULL;
  for (unsigned int i = 0; i < ARRAYSIZE(datadarr); i++) {
    memset(&datadarr[i][0], ' ', sizeof(datadarr[i]));
    datadarr[i][sizeof(datadarr[i]) - 1] = '\0';
  }
  pcx = p = m;
  ss = ds = es = 0;
  CS(0);
  datadp = 0; lfptr = 0;
  if ((i = getsh(prog)) != MAGIC) {
    fprintf(stderr, "wrong magic load file, expected %d found %d\n", MAGIC, i);
    return (1);
  }
  i = getsh(prog);                /*stamps unimportant */
  i = getsh(prog);                /*flags unimportant */
  sections = getsh(prog);         /*number of load sections*/
  outrelo = getsh(prog);          /*number of reloactable parts*/
  nsymtab = getsh(prog);          /*number of entries in symbol table*/
  const int loadl = getint(prog); /*length of core image in load file*/
  const int strl = getint(prog); /*length of string section in load file*/
#ifdef DEBUG
  fprintf(LOG, "sections %d outrelo %d nsymtab %d loadl %d strl %d\n", sections,
          outrelo, nsymtab, loadl, strl);
  fflush(LOG);
#else
  /* Have to compiler not complain about unused var. */
  (void)loadl;
#endif
  j = 0; for(i=0;i<sections;i++) {
	segmhead[i].startad = getint(prog);
	 j += ( segmhead[i].lengte = getint(prog));
	if(i==0) codelength = segmhead[i].lengte;
	segmhead[i].fstart = getint(prog);
	segmhead[i].flen = getint(prog);
	segmhead[i].align = getint(prog);
#ifdef DEBUG
  fprintf(LOG,"loadlengte %o %d %x na segment %d\n",j,j,j,i+2); fprintf(LOG,
"%6d%5x startad | %6d%5x lengte | %6d%5x fstart | %6d%5x flen | %6d%5x align\n",
  segmhead[i].startad,segmhead[i].startad,segmhead[i].lengte,segmhead[i].lengte,
   segmhead[i].fstart,segmhead[i].fstart,segmhead[i].flen,segmhead[i].flen,
   segmhead[i].align,segmhead[i].align); fflush (LOG);
#endif
  if(j>99000) {fprintf(stderr,"Insufficient amount of memory %x\n",j); exit(1);}
  } ss = ((j+31)>>4); /* stack segment behind loaded text, data, bss segments */
  for(i=0;i<sections;i++) {
    if(lfptr > segmhead[i].fstart) {
	fprintf(stderr,"misalignment in load file\n"); return(1); }
    if(i<2) p = m + segmhead[i].startad+(ds<<4);
    while((p-m)%segmhead[i].align) p++;
    if(i>1) segmhead[i].startad = (p - (ds<<4)) - m;
    for(j=0;j<segmhead[i].flen;j++) {*p++ = getc(prog); lfptr++;}
    if(!i) {es = ds = (segmhead[i].lengte + 15)>>4; bp = sp = maxsp = 0x7ff8;}
#ifdef DEBUG
    fprintf(LOG,"i %d startad %d\n",i,segmhead[i].startad);
#endif
  }
    for(i=0;i<outrelo;i++) { /* reads relocation information */
      relocarr[i].typ = getc(prog); relocarr[i].sct = getc(prog);
      relocarr[i].smb = getsh(prog); relocarr[i].adrs = getint(prog);
#ifdef DEBUG
    fprintf(LOG,"i %d typ %d sect %d symbol %d adres %d %x\n",i,
	(int) relocarr[i].typ, (int) relocarr[i].sct, relocarr[i].smb,
	relocarr[i].adrs,relocarr[i].adrs); fflush(LOG);
#endif
    }
  if(traceflag) { for(i=0;i<32;i++) { bparr[i].pcp = 0; bparr[i].bprt = *pcx;}}
/*Break point fields initialised on zero field. Next initialise symbol table*/
    for(i=0;i<MAXSYMTAB;i++){symtab[i].symvalue=0;symtab[i].symbol=nulsymbol;
    symtab[i].nextsym = -1; symtab[i].lnr = 0; symtab[i].symsect = 0;}
    for(i=0;i<32;i++) symhash[i] = -1; for(i=0;i<nsymtab;i++) {
	symtab[i].nextsym = getint(prog); j = getint(prog); j &= 255;
	symtab[i].symsect = (char)(j); symtab[i].symvalue = getint(prog);
	if(j==2) symtab[i].lnr = dotlnarr[symtab[i].symvalue];
	else if(j > 3) symtab[i].symvalue += segmhead[(j&255)-2].startad;
#ifdef DEBUG
fprintf(LOG,"i %d j %d nextsym %d symval %d symsect %o\n",i,j,
 symtab[i].nextsym,symtab[i].symvalue,(int)(symtab[i].symsect)); fflush(LOG);
#endif
    }
    j = ftell(prog);
    for(i=0;i<strl;i++){k=getc(prog);stringfield[i]=(char)(k&255);} /*augustus*/
  for(i=0;i<nsymtab;i++) {
	symtab[i].nextsym -= j;
	symtab[i].symbol = stringfield + symtab[i].nextsym;
	symtab[i].nextsym = -1;
    }
    p = stringfield; i = 0;
    while((i++<8190) && ((j = getc(prog)) != EOF)) *p++ = j;
    for(i=0;i<nsymtab;i++) {j = hashstring(symtab[i].symbol);
	pi = &symhash[j]; while (*pi>=0) pi = &(symtab[*pi].nextsym); *pi = i;
    } prdepth = 0;
    for(i=0;i<0X800;i++) lnsymarr[i] = nsymtab-2;
    for(i=0;i<nsymtab-2;i++){
	if(symtab[i].symsect == 2) symlcorr(i);
    }
    j = 0; for(i=0;i<=maxln;i++) {
      while((j<nsymtab)) {
        if((symtab[j].symsect!=2) ||
		(symtab[j].lnr<i) || (symtab[j].symbol[0]=='.')){j++; continue;}
	if(symtab[j].lnr == i) k = j; break;} lnsymarr[i] = k;
	if(k>=0) for(ii=k+1;ii<=nsymtab;ii++) {
	  if((symtab[ii].symsect==2) && (symtab[ii].lnr == i))lnsymarr[i] = ii;}
    }
  for(i=0;i<outrelo;i++) relocate(i);
  if (traceflag) {
    memset(inbuf, '\0', sizeof(inbuf));
    for (i = 1; i < 27; i++) fprintf(stderr, "\n");
    winfirst();
    inbpl = inbpu = inbuf;
    nextput('>');
    nextput(' ');
  } else {
    INP = stdin;
  }
  copy_args_onto_stack(argc, argv);
  return(0);
}

#ifdef DEBUG
static void logprint(void) {
  for (int i = 0; i < 32; i++) {
    int j = symhash[i];
    fprintf(LOG, "%2d %3d\n", i, j);
    while (j >= 0) {
      fprintf(LOG, "%4d val %8s sym %3d nxt %4d lnr %1d sect\n",
              symtab[j].symvalue, symtab[j].symbol, symtab[j].nextsym,
              symtab[j].lnr, symtab[j].symsect);
      j = symtab[j].nextsym;
    }
  }
  fprintf(LOG, "EIND SYMBOL TABLE\n\n");
}
#endif

static void copy_args_onto_stack(int argc, char** argv) {
#ifdef DEBUG
  logprint();
  fprintf(LOG, "argc %d  ", argc);
  for (i = 0; i < argc; i++) fprintf(LOG, "%s ", argv[i]);
  putc('\n', LOG);
  fprintf(LOG, "maxsp %d sp %d bp %d sp %x bp %x\n", maxsp, sp, bp, sp, bp);
  fflush(LOG);
  fflush(LOG);
#endif
  argv++;
  argc--;
  if (argc) {
    int ii = argc + 4;
    for (int i = 0; i < argc; i++) ii += strlen(argv[i]);
    ii &= 0Xffe;
    maxsp -= ii;
    int k = maxsp + 2;
    char* p2 = m + k + (ss << 4);
    char* p1 = p2 - 4 - (argc << 1);
    bp = sp = maxsp - 2 - (argc << 1);
    *p1++ = argc & 255;
    *p1++ = (argc >> 8) & 255;
    for (int i = 0; i < argc; i++) {
      *p1++ = k & 255;
      *p1++ = (k >> 8) & 255;
      int j = strlen(argv[i]) + 1;
      strcpy(p2, argv[i]);
      p2 += j;
      k += j;
    }
  }
#ifdef DEBUG
  fprintf(LOG,"maxsp %d sp %d bp %d sp %x bp %x\n",maxsp,sp,bp,sp,bp); fflush (LOG);
#endif
}

static void relocate(int n) {
  int tp, sc, st, sa, ss, i, j;
  char *p, octs[4];
#ifdef DEBUG
  for (i = 0; i < 3; i++)
    fprintf(LOG,
            "%6d %4x startad | %6d %4x lengte | %6d %4x fstart | %6d %4x flen "
            "| %6d %4x align\n",
            segmhead[i].startad, segmhead[i].startad, segmhead[i].lengte,
            segmhead[i].lengte, segmhead[i].fstart, segmhead[i].fstart,
            segmhead[i].flen, segmhead[i].flen, segmhead[i].align,
            segmhead[i].align);
  fprintf(LOG, "Erin\n");
  fflush(LOG);
#endif
  tp = relocarr[n].typ;
  sc = relocarr[n].sct;
  tp &= 0X3f;
  sc &= 0Xffff;
  st = relocarr[n].smb;
  st &= 0Xffff;
  sa = relocarr[n].adrs;
  sa &= 0Xffff;
#ifdef DEBUG
  fprintf(LOG, "n %d typ %d sect %d symbol %d %s adres %d <--> ", n, tp, sc, st,
          symtab[st].symbol, sa);
  fflush(LOG);
#endif
  sa += segmhead[sc - 2].startad;
  if (sc > 2) sa += ds << 4; /*bodem data segment */
#ifdef DEBUG
  fprintf(LOG, "sa %d %x symval %d \n", sa, sa, symtab[st].symvalue);
  fflush(LOG);
#endif
  p = m + sa;
  ss = i = 0; /* zoek de unsigned waarde op de positie mem+sa gaat in ss*/
  for (int k = 0; k < (tp & 3); k++) {
    j = *p++;
    j &= 255;
    octs[k] = j;
    j <<= i;
    i += 8;
    ss += j;
  }
#ifdef DEBUG
  fprintf(LOG, "ss %d %x symval %d ", ss, ss, symtab[st].symvalue);
  fflush(LOG);
#endif
  ss += symtab[st].symvalue;
#ifdef DEBUG
  fprintf(LOG, "ss %d %x symval %d \n", ss, ss, symtab[st].symvalue);
  fflush(LOG);
#endif
  p = m + sa; /* De nieuwe waarde terugzetten */
  for (int k = 0; k < (tp & 3); k++) {
    *p++ = octs[k + 2] = ss & 255;
    ss >>= 8;
  }
#ifdef DEBUG
  fprintf(LOG, "octs %d %d %d %d \n", (int)octs[0], (int)octs[1], (int)octs[2],
          (int)octs[3]);
  fflush(LOG);
#endif
}

static void symlcorr(int i) {
  /* corrigeert line number bug voor symbolen uit de text. Zonder correctie
   wordt niet het line number, maar de eerste code doorgegeven */
  int ln,cd,j;
  ln = symtab[i].lnr;
  cd = symtab[i].symvalue;
  while(ln>0 && lndotarr[ln] == cd) {
     fseek(L,(int)lnfilarr[ln],0);
    j = fread(inbuf,1,lnfilarr[ln+1]-lnfilarr[ln],L);
    inbuf[j] = '\0';
    if(!lcs(inbuf,i)) {symtab[i].lnr = ln; return;}
    ln--;
  }
}

static int lcs(char *p, int s) {
  char c, *q;
  size_t j;
  int k,add;
#ifdef DEBUG
fprintf(LOG, "lcs symbool %d buffer %.15s\n",s,p); fflush(LOG);
#endif
  while ((*p <= ' ') || (*p == ':')) if(*p==0) return(-1); else p++;
  j = 0; q = p; while((c = *p++)) {j++;
	if(c=='\n' || c == '!'|| c < '\t' || c > 126) return(-1);
	if(c<= ' ') return(lcs(q+j,s));
	if(c==':') {k=j; p -= 2;
		add = hashstring(q); 
		k = symhash[add]; 
		if(k<0) return(-1);
		while (k>=0) { if(k==s) break;
			if(symtab[k].nextsym<0) break;
					k = symtab[k].nextsym;}
#ifdef DEBUG
  	fprintf(LOG,"lcs symbool %d kop %d zoek %d %s buffer %.15s  j %d %d\n",
		s,add,k,symtab[k].symbol,q,j,strlen(symtab[k].symbol)); fflush(LOG);
#endif
		if((k==s)&&(!strncmp(q,symtab[k].symbol,j-1))
			 && (strlen(symtab[k].symbol)==j-1)) return(0);
		if (*q<='\n') return(-1); return(lcs(q+j,s));
	}
  } return(-1);
}

static int spint(void) {
  int i = 0;
  i |= (*syssp++) & 0xff;
  i |= ((*syssp++) << 8) & 0xffff;
  return (i);
}

static char* spadr(void) {
  int i = spint();
  return (m + i + (ds << 4));
}

static void returnax(int retval) {
  al=(char)(retval&0xff);
  ah=(char)((retval>>8)&0xff);
}

static int getchbp(void) { 
  int i;
  char *p;
  if(inbpl==inbpu) {wmv(11,24);inbpl = inbpu = inbuf;
    if(traceflag && !(inpfl)) {
      errprintf_report("Input expected");
	    p = inbuf; *p++ = '\n'; *p++ = '\0';
	    winupdate();wmv(11,24);
    }
    while((i= getc(INP)) !=EOF) { if(i=='\r') continue;
	*inbpl++ =i; if(i=='\n') break;}
    if(i==EOF) {
	if(inpfl>0) {fclose(INP); inpfl = 0; INP=stdin; return(getchbp());}
	else if(inpfl) {
    errprintf_report("Second time end of input so exit");
		exit(0);}
	else {fclose(stdin); fopen("/dev/tty","rb"); inpfl--; return(i);}
    }
  }
  return((int)*inbpu++);
}

void syscal(void) {
  char calnr, *q, *p, c;
  int retval,i,j,ar[9],k,l,fwidth[9];
  paramfield pram[8];
  sscanfield s[9];
  syssp = (sp&0xffff) + (ss<<4) + m;
  calnr = *syssp;
  syssp += 2;
  switch (calnr) {
   	case 0x01: /*exit*/
		if(traceflag) {winupdate(); wmv(24,0); winupdate(); wmv(24,0);
		fprintf(stderr,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");}
		exit(spint());
		break;
   	case 0x02: /*fork*/
		retval = fork();
		returnax(retval); break;
   	case 0x03: /*read*/
		pram[0].ii = spint();
		pram[1].cp = spadr(); 
		pram[2].ii = spint();
		if (traceflag && (pram[0].ii == 0)) {retval = 0; p = pram[1].cp;
			while((j=getchbp())>0) {*p++ = j; retval++;
		          if(! --(pram[2].ii)) break;}
			if(retval==0) retval = j;}
		else retval = read(pram[0].ii,pram[1].cp,pram[2].ii);
		returnax(retval); break;
   	case 0x04: /*write*/
		pram[0].ii = spint(); 
		pram[1].cp = spadr();
		pram[2].ii = spint();
		if(traceflag && (pram[0].ii==1)){ retval = pram[2].ii;
		  p = pram[1].cp; while(pram[2].ii){nextput(*p++);(pram[2].ii)--;}
		  /*winupdate();*/
		}
		else {if(traceflag && (pram[0].ii==2)){ retval = pram[2].ii;
		  p = pram[1].cp; errprintf_report("%.55s",p);
		  /*winupdate();*/
		}
		else retval = write(pram[0].ii,pram[1].cp,pram[2].ii);}
		returnax(retval); break;
   	case 0x05: /*open*/
		pram[0].cp = spadr();
		pram[1].ii = spint();
		retval = open(pram[0].cp,pram[1].ii);
		returnax(retval); break;
   	case 0x06: /*close*/
		pram[0].ii = spint();
		if(traceflag && (pram[0].ii)==1) {
		  errprintf_report("close call standard output cannot be traced");
		  retval = 1;
		} else retval = close(pram[0].ii);
		 returnax(retval); break;
   	case 0x08: /*creat*/
		pram[0].cp = spadr();
		pram[1].ii = spint();
		retval = creat(pram[0].cp,pram[1].ii);
		returnax(retval); break;
   	case 0x09: /*link*/
		pram[0].cp = spadr();
		pram[1].cp = spadr();
		retval = link(pram[0].cp,pram[1].cp);
		returnax(retval); break;
   	case 0x0b: /*exec*/
		pram[0].cp = spadr();
  		if((prog = fopen(pram[0].cp,"rb")) == NULL) {
		  errprintf_report("Interpreter 8088 cannot open %s", pram[0].cp);
		  returnax(-1);
		} else {
		  if(load(0, NULL)) returnax(-1);
		  fclose(prog);
		} break;
   	case 0x13: /*lseek*/
		pram[0].ii = spint(); 
		pram[1].ii = 0;
		pram[1].ii |= spint();
		pram[1].ii |= (spint()<<16);
		pram[2].ii = spint();
		retval = lseek(pram[0].ii,pram[1].ii,pram[2].ii);
  		dl=(char)((retval>>16)&0xff);
  		dh=(char)((retval>>24)&0xff);
		returnax(retval); break;
   	case 0x40: pram[0].ii = spint(); pram[1].ii = spint(); 
		pram[2].ii = spint();
		bitmapopen(pram[0].ii,pram[1].ii,pram[2].ii);
		break;
   	case 0x41: pram[0].ii = spint(); pram[1].ii = spint();
		pram[2].cp = spadr();
		bitmapdump(pram[0].ii,pram[1].ii,pram[2].cp);
		break;
   	case 0x75: /*getchar*/
		  if(traceflag) { retval = getchbp();
		} else retval = getchar();
		returnax(retval); break;

   	case 0x79: /*sprintf* / sprintf(buf,"%s\n",spadr());break; */
		pram[0].cp = spadr();
		p = pram[1].cp = spadr(); j = 2;
		while((c = *p++)) {
		    if(c == '%') {
		    i = 1;
		    while (i && (c = *p++)) {
		       switch(c) {
			case ' ': case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7': case '8':
			case '9': case '.': case '-': case '+': case '#': break;
			case '*': pram[j++].ii = spint(); break;
			case 'l': case 'L': i = 2; break;
			case 'D': case 'O': case 'X': case 'U': i=2;
			case 'd': case 'o': case 'x': case 'u': case 'c':
			case 'i':
				pram[j].ii = spint();
				if (i==2) pram[j].ii |= (spint()<<16);
				else if(((c=='d') || (c == 'i'))
					&& (pram[j].ii & 0x8000))
					pram[j].ii |= 0xffff0000;
				j++; i=0; break;
			case 's': pram[j].cp = spadr(); i=0;j++;break;
			case 'e': case 'E': case 'f': case 'F': case 'g':
			  errprintf_report("Floats not implemented");
			default: i = 0; break;
			}
		      }
		    }

		}
 if(j>8) { errprintf_report("not more than 6 conversions in printf"); }
	retval = (int) (sprintf(pram[0].cp,pram[1].cp,pram[2].ii,
	    pram[3].ii,pram[4].ii,pram[5].ii,pram[6].ii,pram[7].ii));
		returnax(retval); break;

   	case 0x7a: /*putchar*/
		pram[0].ii = spint();
		if (traceflag) {nextput(pram[0].ii); retval = pram[0].ii;
  				/*winupdate();*/}
		else {retval = putchar(pram[0].ii); fflush(stdout);}
		returnax(retval); break;

   	case 0x7d: /*sscanf*/
		for(i=0;i<9;i++) {ar[i] = 0; fwidth[i] = 1;}
		for(i=0;i<9;i++) for(j=0;j<140;j++) s[i].cp[j] = '\0';
		pram[0].cp = spadr();
		p = pram[1].cp = spadr(); j = 2;
		while((c = *p++)) {
		    if(c == '%') {
		    i = 2;
		    while (i && (c = *p++)) {
		       switch(c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			p--; sscanf(p,"%d",&fwidth[j]);
			  while ((c>= '0') && (c<='9')) c=*p++ ; p--;continue;
			case '.': case '-': case '+': case '*': break;
			case 'l': case 'L': i=4; break;
			case 'h': case 'H': i=1; break;
			case 'D': case 'O': case 'X': case 'U': case 'd':
			case 'o': case 'x': case 'u': ar[j] = i; i = 0;
			pram[j].cp = spadr(); j++; break;
			case 'c': ar[j] = 3;
				pram[j].cp = spadr(); j++; i=0; break;
			case 's':
				pram[j].cp = spadr(); j++; i=0; break;
			case '[': pram[j].cp = spadr(); j++; i=0;
			  while((c=*p++) != ']') if(c < ' ') break; break;
			case 'e': case 'E': case 'f': case 'F': case 'g':
			  errprintf_report("Floats not implemented");
			default: i = 0; break;
			}
#ifdef DEBUG
fprintf(LOG,"%c c  %d j  %d i  %d ar[j]  %s pram %d string: %s\n",
		c,j,i,ar[j-1],pram[j-1].cp,pram[j-1].ii,p-1);
#endif
		      }
		    } 
		}
 if(j>8) { errprintf_report("not more than 6 conversions in sscanf"); }
#ifdef DEBUG
fprintf(LOG,"voor sscanf\n");
#endif
    retval = sscanf(pram[0].cp,pram[1].cp,&s[2],&s[3],&s[4],&s[5],&s[6],&s[7]);
#ifdef DEBUG
fprintf(LOG,"\n\nna sscanf |%s|%s| %d %d %d %d %d %d fwidth[4] %d\n", pram[0].cp,
	pram[1].cp,s[2].ii,s[3].cp,s[4].ii,s[5].ii,s[6].ii,s[7].ii,fwidth[4]);
fprintf(LOG,"\nna sscanf |%s|%s| %d %d %d %d %d %d pram\n", pram[0].cp,pram[1].cp,
	pram[2].ii,pram[3].cp,pram[4].ii,pram[5].ii,pram[6].ii,pram[7].ii);
	for(i=2;i<j;i++) fprintf(LOG," | %d %d  %d %d %d",i,ar[i],
		s[i].cp[0],s[i].cp[1],s[i].cp[2]); putc('\n',LOG);
#endif
   for(i=2;i<j;i++) {
     if(ar[i] == 0) {strcpy(pram[i].cp,s[i].cp); continue;}
     if(ar[i] == 1) {*pram[i].cp = *(s[i].cp); continue;}
     if(ar[i] == 2) {k = (s[i].ii & 255); *(pram[i].cp) = (char)k;
	l = (s[i].ii >> 8) & 255; *(pram[i].cp+1) = (char)(l); continue;}
     if(ar[i] == 3) { q = pram[i].cp; for(k=0;k<fwidth[i];k++) *q++ = s[i].cp[k];
			 continue;}
     if(ar[i] == 4) {l = (s[i].ii >> 24) & 255; *(pram[i].cp+3) = (char)l;
	 l = (s[i].ii >> 16) & 255; *(pram[i].cp+2) = (char)l;
	 l = (s[i].ii >> 8) & 255; *(pram[i].cp+1) = (char)l;
	 l = s[i].ii  & 255; *(pram[i].cp) = (char)l; continue;}
  }
		returnax(retval); break;

   	case 0x7f: /*printf*/
		p = pram[0].cp = spadr(); j = 1;
		while((c = *p++)) {
		    /* fprintf(stderr,"`%c ",c);*/
		    if(c == '%') {
		    i = 1;
		    while (i && (c = *p++)) {
		       switch(c) {
			case ' ': case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7': case '8':
			case '9': case '.': case '-': case '+': case '#': break;
			case '*': pram[j++].ii = spint(); break;
			case 'l': case 'L': i = 2; break;
			case 'D': case 'O': case 'X': case 'U': i=2;
			case 'd': case 'o': case 'x': case 'u': case 'c':
			case 'i':
				pram[j].ii = spint();
				if (i==2) pram[j].ii |= (spint()<<16);
				else if(((c=='d') || (c == 'i'))
					&& (pram[j].ii & 0x8000))
					pram[j].ii |= 0xffff0000;
				j++; i=0; break;
			case 's': pram[j].cp = spadr(); i=0;j++;break;
			case 'e': case 'E': case 'f': case 'F': case 'g':
			  errprintf_report("Floats not implemented");
			default: i = 0; break;
			}
		      }
		    }
		}
 if(j>8) { errprintf_report("not more than 7 conversions in printf"); }
	if(traceflag) {retval = j; sprintf(tringfield+200,pram[0].cp,pram[1].ii,
	    pram[2].ii,pram[3].ii,pram[4].ii,pram[5].ii,pram[6].ii,pram[7].ii);
	  p = tringfield+200; while((j = *p++)) nextput(j); /*winupdate();*/}
	else {retval = printf(pram[0].cp,pram[1].ii,pram[2].ii,
	    pram[3].ii,pram[4].ii,pram[5].ii,pram[6].ii,pram[7].ii);
	  fflush(stdout);}
		returnax(retval); break;
	default: panicf("Unimplemented sys call %d", calnr); break;
  // case 0x07: /* wait */ break;
  // case 0x0c: /* cdir */ break;
  // case 0x0d: /* time */ break;
  // case 0x0e: /* mknod */ break;
  // case 0x0f: /* chmod */ break;
  // case 0x10: /* chown */ break;
  // case 0x11: /* brk */ break;
  // case 0x12: /* stat */ break;
  // case 0x14: /* getpid */ break;
  // case 0x15: /* mount */ break;
  // case 0x16: /* umount */ break;
  // case 0x17: /* setuid */ break;
  // case 0x18: /* getuid */ break;
  // case 0x19: /* stime */ break;
  // case 0x1a: /* ptrace */ break;
  // case 0x1b: /* alarm */ break;
  // case 0x1c: /* fstat */ break;
  // case 0x1d: /* pause */ break;
  // case 0x1e: /* utime */ break;
  // case 0x1f: /* stty */ break;
  // case 0x20: /* gtty */ break;
  // case 0x21: /* access */ break;
  // case 0x22: /* nice */ break;
  // case 0x23: /* ftime */ break;
  // case 0x24: /* sync */ break;
  // case 0x25: /* kill */ break;
  // case 0x29: /* dup */ break;
  // case 0x2a: /* pipe */ break;
  // case 0x2b: /* times */ break;
  // case 0x2c: /* profil */ break;
  // case 0x2e: /* setgid */ break;
  // case 0x2f: /* getgid */ break;
  // case 0x30: /* signal */ break;
  // case 0x33: /* acct */ break;
  // case 0x35: /* lock */ break;
  // case 0x36: /* ioctl */ break;
  // case 0x3b: /* exece */ break;
  // case 0x3c: /* umask */ break;
  // case 0x3d: /* chroot */ break;
  // case 0x76: /* getc */ break;
  // case 0x77: /* gets */ break;
  // case 0x78: /* ungetc */ break;
  // case 0x7e: /* scanf */ break;
  // case 0x7b: /* putc */ break;
  // case 0x7c: /* puts */ break;
    }
}

static void pdmpadr(void) {
  char *o, *p, *q, *r, c;
  unsigned long ii, i, j;
  for (i = 0; i < 7; i++) {
    o = q = p = datadm[i];
    if (p != NULL) {
      for (j = 0; j < 8; j++) {
        r = datadarr[i];
        r += 19 + 3 * j;
        sprintf(r, "%3x", *p++ & 0xff);
      }
      datadarr[i][43] = ' ';
      for (j = 0; j < 12; j++) {
        c = *q++;
        if (c < 32 || c > 126) c = '.';
        datadarr[i][44 + j] = c;
      }
      for (j = 0; j < 4; j++) {
        r = datadarr[i];
        r += 55 + 6 * j;
        ii = *o++ & 255;
        if (*o & 128) ii |= 0Xffff0000;
        ii |= (*o++ & 255) << 8;
        sprintf(r, "%6lu", ii);
      }
    }
  }
}

static void cnulbp(void) {
  char* p;
  if (bparr[0].pcp != -1) {
    p = m + cs16 + bparr[0].pcp;
    if (*p == '\360') *p = bparr[0].bprt;
    bparr[0].pcp = 0Xffff;
  }
}

void dump(void) {
  cnulbp();
  pdmpadr();

  if (errflag) system("sleep 1");
  rdcmd();
  if (errflag) {
    wprintf(10, 22, "E Last message: %-37.37s", errbuf);
    errflag = 0;
  } else if (window[10][22] == 'M') {
    wprintf(10, 22, "  %-55.55s", window[10] + 37);
  }
  memset(errbuf, ' ', sizeof(errbuf) -1);
}

static void checkret(void) {
  if (sp != prstckpos[prdepth]) {
    errprintf_report("Return on suspicious stack pointer prdepth %d", prdepth);
    if (traceflag) dump();
    exit(1);
  }
}

void procdepth(int s) {
  if (s > 0) {
    prdepth++;
    stckprdepth[prdepth] = dotlnarr[(((int)(PC)) & 0Xffff) - s];
    prstckpos[prdepth] = sp - 2;
  }
  if (s == -1) {
    checkret();
    prdepth--;
  }
  if (prdepth == bprdepth) {
    stopvlag = 1;
  } else {
    stopvlag = 0;
  }
}

static void zetbp(short textdot) {
  int i;
  for (i = 1; i < 32; i++)
    if (!bparr[i].pcp) break;
  if (i == 32) {
    errprintf_report("break point table full");
    return;
  }
  bparr[i].pcp = textdot;
  bparr[i].bprt = m[cs16 + textdot];
  m[cs16 + textdot] = 0xF0;
  return;
}

static void clearbp(short textdot) {
  int i;
  for (i = 1; i < 32; i++)
    if (bparr[i].pcp == textdot) break;
  if (i == 32) {
    errprintf_report("break point not found");
    return;
  }
  bparr[i].pcp = 0;
  m[cs16 + textdot] = bparr[i].bprt;
  bparr[i].bprt = m[0];
  return;
}

static void nulbp(int ln) {
  char*p = m + cs16 + bparr[0].pcp;
  if (*p == '\360') *p = bparr[0].bprt;
  int dott = lndotarr[ln];
  p = m + cs16 + dott;
  bparr[0].pcp = dott;
  bparr[0].bprt = *p;
  *p = 0XF0;
}

void breakpt(void) {
  int j;
  int i = ((int)(PC)) & 0xffff;
  i--;
  for (j = 0; j < 32; j++)
    if (bparr[j].pcp == i) break;
  if (j == 32) {
    errprintf_report("Wrong breakpoint");
    exit(1);
  }
  dumpt = bparr[j].bprt;
  dump();
}

static void dmpadr(int adre) {
  datadm[datadp] = m + (ds<<4) + adre; 
  sprintf(datadarr[datadp],"%.19s",tringfield+90);
  datadarr[datadp][19] = ' ';
  datadarr[datadp][18] = ':';
  datadp++; datadp %= 7;
}

static int rdstrg(void) {
  size_t i;
  int j,stradr,k,syk;
  char *p,*q,c,cc;
  i=0; p = cmdline; q = tringfield+90;
  while((c = *p++)) if (c=='!' || c == '/' || c == '+') break;
		  else {i++; *q++ = c;}
  if((c=='/') && (*p != '+')) *(--p) = '0';
  *q = '\0';
#ifdef DEBUG
  logprint();
  fprintf(LOG, "gelezen: string |%s|\n", tringfield + 90);
#endif
  stradr = hashstring(tringfield+90); symp = symhash[stradr];
  if(i>8) i=8; syk = -1;
  while(symp != -1) {
#ifdef DEBUG
    fprintf(LOG, "Vergeleken: string |%s| symtab |%s| symp %2d i %2d\n",
            tringfield + 900, symtab[symp].symbol, symp, i);
#endif
    /*if(!strncmp(tringfield+90, symtab[symp].symbol,i)) { syk = symp; break;}
    else {symp = symtab[symp].nextsym;}*/
    if(!strncmp(tringfield+90, symtab[symp].symbol,i)) { syk = symp;
      if(i==strlen(symtab[symp].symbol)) break; }
    symp = symtab[symp].nextsym;
  }
  symp = syk;  /*april*/
  if(symp < 0) { errprintf_report("No Match"); return(-1); }
  for(k=0;k<18;k++) tringfield[k+90] = ' '; tringfield[98] = '+';
  for(k=0;k<8;k++) if((cc = symtab[symp].symbol[k])>32) tringfield[k+90] = cc;
	else break;
#ifdef DEBUG
  fprintf(LOG, "tringfield: |%s|\n", tringfield + 90);
  logprint();
#endif
  if(symtab[symp].symsect > 2) stradr = symtab[symp].symvalue;
  else if(symtab[symp].symsect == 2) stradr = symtab[symp].lnr;
  j = -1; if(c == '\0' || c=='!') {cmdchar = '!'; j=0; }
  if(j) {
    while(*p =='+' || *p == '\t' || *p == ' ') p++;
    if(*p == '\0' || *p=='!') {cmdchar = '!'; j=0;}
  }
  if(j){
    if(*p<'0'||*p>'9') {
      errprintf_report("cmd constant expected.");
	    system("sleep 1");
      return(-1);
    }
    sscanf(p,"%d%1s",&j,&cmdchar);
  }
  if(symtab[symp].symsect == 2){if(cmdchar == '!') cmdchar = 'g'; sprintf(tringfield+99,"%2d",j);}
  else  {if(cmdchar == '!')cmdchar = 'd'; sprintf(tringfield+99,"%-4d=%04x:",j,(stradr+j));}
  return(stradr+j);
}

static int rdadr(void) {
  int i;
  sscanf(cmdline,"%d%1s",&i,&cmdchar);
  if((cmdchar == '!') && (symtab[symp].symsect == 2)) cmdchar = 'g';
  return(i);
}

static void rdcline(int c) {
  char *p = cmdline;
  while(c != '\n') {if (c==EOF) break; *p++ = c; c = getchcmd();}
  *p++ = '\0'; *p = '\0';
  wprintf(14, 0, "%-18.18s", cmdline);
  p--; *p = '!';
}

static void rdcmd(void) {
  int c,d,adre;
  wmv(15,0);
  for(c=0;c<20;c++) putchar(' ');
  wmv(15,0);
  wprintf(15, 0, "                 ");
  winupdate();
  symp = -1; c = getchcmd(); if(c=='/') { c=getchcmd(); d=1;} else d = 0;
  stopvlag = 0;
  rdcline(c);
  if(d){adre=rdstrg(); if(adre==-1) {
    rdcmd(); return;
  }
	if (symp == -1){
    rdcmd(); return;
  }
  } else if((c>='0')&&(c<='9')){ adre=rdadr();
            if(cmdchar == '!') cmdchar = (symtab[symp].symsect <3) ? 'x' : 'd';}
	 else { adre=dotlnarr[(PC)-1]; cmdchar = c; if(c=='\n') c = '?'; }
/* Note: (PC)-1. The next instruction initial byte is fetched before the dump*/
  if(cmdchar=='\n') cmdchar = 'S';
   instrcount = 0X7fffffff; bprdepth = -1;
   if(cmdchar == '!') cmdchar = (symtab[symp].symsect <3) ? 'x' : 'd';
   if((cmdchar < 5) || (cmdchar > 126)) cmdchar =  'q' ;
    switch(cmdchar) {
	case 'q' : fprintf(stderr,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"); exit(0); break;
	case 'r' : instrcount = 0X7fffffff; break;
	case '-' : bprdepth = prdepth-1; break;
	case '+' : bprdepth = prdepth+1; break;
	case '=' : bprdepth = prdepth; stopvlag = 1; break;
	case '!' : nulbp(adre); break;
	case 'g' : /* errprintf_report("g %d",adre); */ nulbp(adre); break;
	case 0xc : 
	case 'R' : refresh(); rdcmd();break;
  case 'd':
    if (symtab[symp].symsect > 2) {
      dmpadr(adre);
     } else {
      errprintf_report("Cannot dump this label");
     }
     dump();
     break;
  case 'b' : zetbp(lndotarr[adre]); rdcmd() ; break;
	case 'c' : clearbp(lndotarr[adre]); rdcmd(); break;
	case 'n' : nulbp(dotlnarr[lndotarr[dotlnarr[(PC-1)]+1]]); break;
	case 'x' : case 'X': case '*': instrcount = adre; break;
	default : stopvlag = 1;
    } 
}

static void winupdate(void) {
  int i,j,k,l; char *p,*q;
  wprintf(0, 4, "%02x", cs & 0Xffff);
  wprintf(0, 18, "%03x", ds & 0Xffff);
  wprintf(1, 3, "%02x", ah);
  wprintf(1, 9, "%02x", al);
  wprintf(1, 15, "%6d", ax);
  wprintf(2, 3, "%02x", bh);
  wprintf(2, 9, "%02x", bl);
  wprintf(2, 15, "%6d", bx);
  wprintf(3, 3, "%02x", ch);
  wprintf(3, 9, "%02x", cl);
  wprintf(3, 15, "%6d", cx);
  wprintf(4, 3, "%02x", dh);
  wprintf(4, 9, "%02x", dl);
  wprintf(4, 15, "%6d", dx);
  wprintf(5, 4, "%04x", sp & 0Xffff);
  wprintf(6, 4, "%04x", bp & 0Xffff);
  wprintf(7, 4, "%04x", si & 0Xffff);
  wprintf(8, 4, "%04x", di & 0Xffff);
  wprintf(7, 13, "%04lx:PC", (PC)-1);
  /* Note: (PC)-1. The next instruction initial byte is fetched before the
   * dump*/
  wprintf(8, 10,"           ");
  i = dotlnarr[(PC)-1];
  j = lnsymarr[i];
  if (j > nsymtab - 3) {
    fprintf(stderr, "\nNo BSS or no head label?\n");
    system("sleep 2");
    refresh(); /*exit(1);*/
  }
  wprintf(8, 10, "%s+%1d", symtab[j].symbol, i - symtab[j].lnr);
  wprintf(6, 12, "%c", (ovf) ? 'v' : '-');
  wprintf(6, 14, "%c", (dirf) ? '<' : '>');
  wprintf(6, 16, "%c", (signf) ? 'n' : 'p');
  wprintf(6, 18, "%c", (zerof) ? 'z' : '-');
  wprintf(6, 20, "%c", (cf) ? 'c' : '-');
  for (j = 0; j < 9; j++) {
    fseek(L, (int)lnfilarr[i + j - 6], 0);
    p = window[j] + 32;
    gtabstr(48, p, L);
  }
  for (i = 0; i < OUTBUFFER_SIZE; i++) {
    wnwrite(15 - i, 22, outbuffer[i], sizeof(outbuffer[i]));
  }
  for (i = 0; i < 7; i++) {
    wprintf(17 + i, 0, "%s", datadarr[i]);
  }

  // Print stack.
  l = prdepth; j= (maxsp-sp > 18) ? sp : maxsp-18; j &= 0xffff; p = m+j+(ss<<4);
#ifdef DEBUG
  fprintf(LOG,"maxsp %d sp %d bp %d sp %x bp %x j %d\n",maxsp,sp,bp,sp,bp,j);fflush (LOG);
#endif
  for (i = 0; i < 9; i++) {
    if (j < sp) {
      p += 2;
      wprintf(i, 23, "      ");
    } else {
      // TODO(heder): Use a function to read a short from the VM.
      k = *p++ & 255;
      k |= ((*p++ & 255) << 8);
      if (j == sp) {
        wprintf(i, 23, "=>%04x", k);
      } else {
        wprintf(i, 23, "  %04x", k);
      }
      if (j == prstckpos[l]) {
        wprintf(i, 23, "%c", (char)(l + 48));
        l--;
      }
    }
    j += 2;
  }

  l = (prdepth > 2) ? prdepth - 2 : 1;
  for (i = 12; i > 9; i--) {
    if (l > (int)prdepth) {
      wprintf(i, 0, "                   ");
    } else {
      j = stckprdepth[l];
      k = lnsymarr[j];
      wprintf(i, 0, "%1d <= %-8s + %3d", l, symtab[k].symbol,
              j - symtab[k].lnr);
    }
    l++;
  }

  wprintf(11, 22, "I ");
  p = inbuf;
  q = window[11] + 24;
  while (p < inbpu - 85) p += 55;
  while ((j = (int)(*p)) != '\n') {
    if(*p =='\0') break;
    if ((q >= window[11] + 75) && (q < window[12])) {
      *q++ = ' '; *q++ = ' ';
		  *q++ = ' '; q = window[12]+24; window[12][22] = 'I';
    }
    if (p == inbpu) {
      *q++ = '-'; *q++ = '>';
    }
    if (j == 0 || q > window[12] + 75) break;
    if (j < ' ') {
      *q++ = '^'; j += 64;
    }
    *q++ = j;
    p++;
  }
  if (p == inbpu && *p == '\n') {
    *q++ = '-'; *q++ = '>';
  }
  if ((inbpu != inbpl) && (*p == '\n')) {
    *q++ = '\\'; *q++ = 'n';
  }
  if (*(inbpu - 1) == '\n') {
    *q++ = '\\'; *q++ = 'n'; *q++ = '-'; *q++ = '>';
  }
  while (q < window[12] - 1) *q++ = ' ';
  if (q > window[12]) {
    while (q < window[13] - 1) *q++ = ' ';
  }
  immain(); wmv(15,0);
}

static void gtabstr(int i, char* a, FILE* f) {
  int j = 0;
  while (j < i) {
    int c = getc(f);
    if (c == EOF || c == '\r' || c == '\n') break;
    if (c == '\t') {
      if (j % 8 != 7) ungetc(c, f);
      c = ' ';
    }
    j++;
    *a++ = c;
  }
  while (j++ < i) *a++ = ' ';
}

static void outbuffer_scroll(void) {
  for (int i = OUTBUFFER_SIZE - 1; i > 0; i--) {
    memcpy(outbuffer[i], outbuffer[i - 1], sizeof(outbuffer[i]));
  }
  memset(outbuffer[0], ' ', sizeof(outbuffer[0]));
}

static void nextput(int c) {
  static int puthp = 0; /*horizontale en verticale putpositie*/

  if (c == '\n') {
    nextput('\\');
    nextput('n');
    puthp = -1;
    return;
  }
  if (puthp > (int)sizeof(outbuffer[0]) - 1) puthp = -1;
  if (puthp < 0) {
    outbuffer_scroll();
    puthp = 0;
    nextput('>');
    nextput(' ');
  }
  if (c < ' ') {
    outbuffer[0][puthp++] = '^';
    nextput(c + 64);
    return;
  }
  outbuffer[0][puthp++] = c;
}

static int getchcmd(void) {
  int i;
  if ((i = getc(CMD)) != EOF) {
    if (i != '\r')
      return (i);
    else
      return (getchcmd());
  }
  if (cmdfl) {
    fclose(CMD);
    CMD = stdin;
    cmdfl = 0;
    i = getchcmd();
  }
  if (i != '\r')
    return (i);
  else
    return (getchcmd());
}

void meldroutine(void) {
  if (traceflag) {
    wmv(10, 24);
    printf("%s", errbuf);
    system("sleep 1");
    wmv(10, 78);
    printf("\n");
    fprintf(stderr, "\n");
    wprintf(10, 22, "M %-55.55s", errbuf);
    winupdate();
  } else {
    fprintf(stderr, "%s\n", errbuf);
  }
}

static void sanitize_errbuf() {
  int aa = 0;
  char *p = errbuf;
  for (int i = 0; i < (int)sizeof(errbuf) - 1; i++) {
    if (*p <= '\n') {
      *p = ' ';
    } else if (*p == '\0' || *p == '\n') {
      aa = 1;
    } else if (aa) {
      *p = ' ';
    }
    ++p;
  }
  *p = '\0';
}

static void erroutine(void) {
  if (traceflag) {
    sanitize_errbuf();
    if (errflag) system("sleep 1");
    wmv(10, 24);
    printf("%s", errbuf);
    wmv(10, 78);
    printf("\n");
    fprintf(stderr, "");
    wprintf(10, 22, "M %-55.55s", errbuf);
    winupdate();
    errflag = 1;
  } else {
    fprintf(stderr, "%s\n", errbuf);
  }
}

void spare(int t) {
  errprintf_report("8086 undefined instruction %0X", t & 0xff);
}

void notim(int t) {
  errprintf_report("Instruction %0X not implemented", t & 0xff);
}

void interrupt(int t) {
  errprintf_report("Interrupt %0X. Bad division?", t & 0xff);
}

void panicf(const char* format, ...) {
  va_list argp;
  va_start(argp, format);
  vsnprintf(errbuf, sizeof(errbuf), format, argp);
  va_end(argp);
  erroutine();
  system("sleep 5");
  exit(1);
}

void errprintf(const char* format, ...) {
  va_list argp;
  va_start(argp, format);
  vsnprintf(errbuf, sizeof(errbuf), format, argp);
  va_end(argp);
}

void errprintf_report(const char* format, ...) {
  va_list argp;
  va_start(argp, format);
  vsnprintf(errbuf, sizeof(errbuf), format, argp);
  va_end(argp);

  erroutine();
}

static char* remove_ext(char* dest, char* path) {
  strcpy(dest, path);
  char* last_ext = strrchr(dest, '.');
  char* last_path = strrchr(dest, '/');
  if (last_ext && (!last_path || last_ext > last_path)) {
    *last_ext = '\0';
  }
  return dest;
}

// TOOD(heder): Use a proper testing framework.
static void TEST_remove_ext() {
  char basename[100];
  assert(strcmp(remove_ext(basename, "hello.txt"), "hello") == 0);
  assert(strcmp(remove_ext(basename, "hello"), "hello") == 0);
  assert(strcmp(remove_ext(basename, "/home/hello.txt"), "/home/hello") == 0);
  assert(strcmp(remove_ext(basename, "/usr/bin"), "/usr/bin") == 0);
  assert(strcmp(remove_ext(basename, "../hello"), "../hello") == 0);
}

static void TEST_ALL(void) {
  TEST_remove_ext();
}

int main_lib(int argc, char **argv) {
  TEST_ALL();

  if (argc < 2) {
    fprintf(stderr, "No .88 load file? Usage t88 loadfile\n");
    return 1;
  }
  sp = 0; ss = 0; pcx = m; cs = 0;
  argc--; argv++;
  if (!argc) {
    fprintf(stderr, "No load file? argc %d %s\n", argc, argv[0]);
    return 1;
  }
  int load_res = load(argc, argv);
  if (load_res) return load_res;
  fclose(prog);
  interp();
  return 0;
}
