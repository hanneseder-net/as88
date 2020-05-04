#ifndef TRCE_SRC_STORE_H
#define TRCE_SRC_STORE_H

// TODO(heder): This typedef shouldn't be here.
typedef short word;

void wstore(word x);
void xstore(char* x);
void rapwstore(word w);

#endif /* TRCE_SRC_STORE_H */
