/* Macros for memory references. */
#define M m

#if CHECK
#define MEM(x, b, t)                                                     \
  if ((xx = ((long)b << 4) + (unsigned short)t) >= MEMBYTES) merr(b, t); \
  x = M + xx

#else /* !CHECK */

#define MEM(x, b, t) x = M + (b << 4) + (unsigned short)t

#endif /* CHECK */

#define PC (pcx - M) - cs16
#define CS(x) cs = x; cs16 = cs << 4;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
