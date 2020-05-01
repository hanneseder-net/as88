#include "util.h"

int spiegel(int m, int n) {
  int i,j,k,l;
  if(m == 0) {
    i = 1; j = 128; l = 0;
    for(k=0;k<8;k++) {if((i&n)>0) l |= j; i <<= 1; j >>= 1;}
  } else if (m == 1) {
    i = 16; j = 128; l = 0; for(k=0;k<4;k++) {
	 if((i&n)>0) l |= j;j >>= 1; if((i&n)>0) l |= j;j >>= 1; i <<= 1;}
  } else {
    i = 1; j = 128; l = 0; for(k=0;k<4;k++) {
	 if((i&n)>0) l |= j;j >>= 1; if((i&n)>0) l |= j;j >>= 1; i <<= 1;}
  }
  /* if(l>0) fprintf(stderr,"l %x n %x l %d n %d m %d\n",l,n,l,n,m);*/
  return(l);
} 
