/* Macros for memory references. */
#define M m

#if CHECK
#define MEM(x,b,t)\
	if ( (xx= ((long)b<<4) +(unsigned short)t) >= MEMBYTES)\
	    merr(b,t);\
	x = M + xx

#define CSMEM(x,t)\
	if ( (xx= cs16 +(unsigned short)t) >= MEMBYTES)\
	    merr(cs,t);\
	x = M + xx

#define STACKPTR(t)\
	xx = ( (long)ss << 4) + (unsigned short)sp;\
	t = (int16_t *) (M + xx);\
	stackck()

#define BSTORE(x)\
	progck1();\
	*eapc = x

#ifdef LITTLE_ENDIAN
#define MOV16\
	progck2();\
	*eapc++ = *pcx++;\
	*eapc++ = *pcx++
#else
/*
 * XXX watch for memory layout here
 */
#define MOV16\
	progck2();\
	if (eapc <= M + MEMBYTES) {\
	    *eapc++ = *pcx++;\
	    *eapc++ = *pcx++;\
	} else {\
	    *(eapc+1) = *pcx++;\
	    *eapc++ = *pcx++;\
	    eapc++;\
	}
#endif

#else	/* not check */

#define MEM(x,b,t)\
	x = M + (b<<4) + (unsigned short)t

#define CSMEM(x,t)\
	x = M + cs16 + (unsigned short)t

#define STACKPTR(t)\
	t = (int16_t *) (M + ((long)ss << 4) + (unsigned short)sp);

#define BSTORE(x)\
	*eapc = x

#ifdef LITTLE_ENDIAN
#define MOV16\
	*eapc++ = *pcx++;\
	*eapc++ = *pcx++
#else
/*
 * XXX watch for memory layout here
 */
#define MOV16\
	if (eapc <= M + MEMBYTES) {\
	    *eapc++ = *pcx++;\
	    *eapc++ = *pcx++;\
	} else {\
	    *(eapc+1) = *pcx++;\
	    *eapc++ = *pcx++;\
	    eapc++;\
	}
#endif

#endif	/* check */

#define PC (pcx - M) - cs16
#define CS(x) cs = x; cs16 = cs << 4;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
