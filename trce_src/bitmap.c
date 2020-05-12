#include "bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int termbitsize;
static char bmbuf[9000];
static FILE *bituit;
static int pfildes[2];
static int pnr;

static int spiegel(int m, int n) {
  int l = 0;
  if (m == 0) {
    int i = 1;
    int j = 128;
    for (int k = 0; k < 8; k++) {
      if ((i & n) > 0) l |= j;
      i <<= 1;
      j >>= 1;
    }
  } else if (m == 1) {
    int i = 16;
    int j = 128;
    for (int k = 0; k < 4; k++) {
      if ((i & n) > 0) l |= j;
      j >>= 1;
      if ((i & n) > 0) l |= j;
      j >>= 1;
      i <<= 1;
    }
  } else {
    int i = 1;
    int j = 128;
    for (int k = 0; k < 4; k++) {
      if ((i & n) > 0) l |= j;
      j >>= 1;
      if ((i & n) > 0) l |= j;
      j >>= 1;
      i <<= 1;
    }
  }
  return (l);
}

static void schrijf(void) {
  write(pfildes[1], bmbuf, strlen(bmbuf));
}

static void schrijfmap(int b, int h, int s, char *buf, FILE *uitf) {
  int i,j,k,l,m,c;
  char *p,*q;
  if(b%8!=0){
	fprintf(stderr,"Bitmap breedte hoort een heel aantal bytes te zijn\n");
	exit(1);}
  fprintf(uitf,"#define noname_width %d\n",b*s);
  fprintf(uitf,"#define noname_height %d\n",h*s);
  fprintf(uitf,"static char noname_bits[] = {\n ");
  p = buf; b >>= 3; k = b*h*s*s-1; m = 1;
  for(i=0;i<h;i++) { q = bmbuf; for(j=0;j<b;j++)
    if(termbitsize == 2){ c = *p++; *q++ = spiegel(1,c); *q++ = spiegel(2,c);
    } else { c = *p++; *q++ = (255 & spiegel(0,c));
    }
    for(l=0;l<s;l++) {
      q = bmbuf; for(j=0;j<b*s;j++) {
	fprintf(uitf,"0x%02x",(255 & *q++)); if(k==0) fprintf(uitf,"};\n");
	else {putc(',',uitf); if((m % 15) == 0) fprintf(uitf,"\n "); k--; m++;}
      }
    }
  }
}

void bitmapdump(int b, int h, char *buff) {
  if(termbitsize == 2) { if ((bituit=fopen("tERMbITMAP","wb")) == NULL) {
	fprintf(stderr,"Kan tERMbITMAP niet openen\n"); exit(1);}
  } else { if ((bituit=fopen("tERMbITmAP","wb")) == NULL) {
	fprintf(stderr,"Kan tERMbITmAP niet openen\n"); exit(1);}
  }
  schrijfmap(b,h,termbitsize,buff,bituit);
  fclose(bituit);
  if(termbitsize == 2) sprintf(bmbuf,".c configure -bitmap @lEEGbITMAP\n");
  else sprintf(bmbuf,".c configure -bitmap @lEEGbITmAP\n");
  schrijf(); system("sleep 1");
  if(termbitsize == 2) sprintf(bmbuf,".c configure -bitmap @tERMbITMAP\n");
  else sprintf(bmbuf,".c configure -bitmap @tERMbITmAP\n");
  schrijf();  system("sleep 1");
}
 
void bitmapopen(int b, int h, int s) {
  /*FAKE SYSTEM CALL TO OPEN A BITMAP FOR OPGAVE 1 */
  if(pipe(pfildes)< 0) {fprintf(stderr,"Kan geen pipe creeren\n"); exit(1);}
  if((pnr = fork()) < 0) {fprintf(stderr,"Kan niet vorken\n"); exit(1);}
  if(pnr == 0){dup2(pfildes[0],0); system("exec /usr/local/bin/wish"); exit(0);}
  termbitsize = 1; if(s==2) termbitsize = 2;

  if(termbitsize == 2) {
    if ((bituit=fopen("lEEGbITMAP","wb")) == NULL) {
	fprintf(stderr,"Kan lEEGbITMAP niet openen\n"); exit(1);}
  } else {
    if ((bituit=fopen("lEEGbITmAP","wb")) == NULL) {
	fprintf(stderr,"Kan lEEGbITmAP niet openen\n"); exit(1);}
  }
  schrijfmap(b,h,termbitsize,bmbuf+800,bituit);
  fclose(bituit);

  sprintf(bmbuf,"#!/usr/local/bin/wish -f\n. configure -background gray\n");
  schrijf();
  if (termbitsize == 2) {
sprintf(bmbuf,". configure -width 591\n. configure -height 509\n"); schrijf();
sprintf(bmbuf,"button .b -text %cexit window %c",'"','"'); schrijf();
sprintf(bmbuf,"  -font *-helvetica-bold-r-normal--*-180-* "); schrijf();
sprintf(bmbuf," -command %cdestroy .%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .b -x 4 -y 4 -relwidth 0.32 -height 1.1c\n"); schrijf();
sprintf(bmbuf,"label .c -bitmap @lEEGbITMAP\nplace .c -x 4 -y 1.4c\n");schrijf();
sprintf(bmbuf,"button .d -text %cdisplay new%c ",'"','"'); schrijf();
sprintf(bmbuf,"-font *-helvetica-bold-r-normal--*-180-* -command"); schrijf();
sprintf(bmbuf," %c.c configure -bitmap @tERMbITMAP%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .d -x 200 -y 4 -relwidth 0.32 -height 1.1c\n"); schrijf();
sprintf(bmbuf,"button .e -text %cclear window %c ",'"','"'); schrijf();
sprintf(bmbuf,"-font *-helvetica-bold-r-normal--*-180-* -command"); schrijf();
sprintf(bmbuf," %c.c configure -bitmap @lEEGbITMAP%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .e -x 396 -y 4 -relwidth 0.32 -height 1.1c\n"); schrijf();
  } else {
sprintf(bmbuf,". configure -width 302\n. configure -height 266\n"); schrijf();
sprintf(bmbuf,"button .b -text %cexit window %c",'"','"'); schrijf();
sprintf(bmbuf,"  -font *-helvetica-bold-r-normal--*-120-* "); schrijf();
sprintf(bmbuf," -command %cdestroy .%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .b -x 2 -y 3 -relwidth 0.32 -height 0.7c\n"); schrijf();
sprintf(bmbuf,"label .c -bitmap @lEEGbITmAP\nplace .c -x 3 -y 0.9c\n");schrijf();
sprintf(bmbuf,"button .d -text %cdisplay new%c ",'"','"'); schrijf();
sprintf(bmbuf,"-font *-helvetica-bold-r-normal--*-120-* -command"); schrijf();
sprintf(bmbuf," %c.c configure -bitmap @tERMbITmAP%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .d -x 101 -y 3 -relwidth 0.32 -height 0.7c\n"); schrijf();
sprintf(bmbuf,"button .e -text %cclear window %c ",'"','"'); schrijf();
sprintf(bmbuf,"-font *-helvetica-bold-r-normal--*-120-* -command"); schrijf();
sprintf(bmbuf," %c.c configure -bitmap @lEEGbITmAP%c\n",'"','"'); schrijf();
sprintf(bmbuf,"place .e -x 200 -y 3 -relwidth 0.32 -height 0.7c\n"); schrijf();
  }
sprintf(bmbuf,".c configure -background black -foreground white\n"); schrijf();
sprintf(bmbuf,".b configure -background black -foreground white\n"); schrijf();
sprintf(bmbuf,".d configure -background black -foreground white\n"); schrijf();
sprintf(bmbuf,".e configure -background black -foreground white\n"); schrijf();
  /* system("sleep 1");*/
}
