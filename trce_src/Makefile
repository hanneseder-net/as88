CC=gcc
CP=cp
CFLAGS=-O -Wall -Wextra -Wpedantic -pedantic

all: s88 t88

88.o: 88.h macro.h 88.c
	$(CC) -c $(CFLAGS) 88.c 
 
table.o: 88.h macro.h  table.c
	$(CC) -c $(CFLAGS) table.c 
 
adr.o: 88.h macro.h  adr.c
	$(CC) -c $(CFLAGS) adr.c 
 
cond.o: 88.h macro.h cond.c
	$(CC) -c $(CFLAGS) cond.c 
 
store.o: 88.h macro.h  store.c
	$(CC) -c $(CFLAGS) store.c 
 
main.o: 88.h macro.h main.c
	$(CC) -c $(CFLAGS) main.c 
 
mains.o: 88.h macro.h mains.c
	$(CC) -c $(CFLAGS) mains.c 
 
doscurs.o: doscurs.c 
	$(CC) -c $(CFLAGS) doscurs.c 
 
bitmap.o: bitmap.c
	$(CC) -c $(CFLAGS) bitmap.c 

main_lib.o: main_lib.c
	$(CC) -c $(CFLAGS) main_lib.c

t88: 88.o table.o adr.o cond.o store.o main.o doscurs.o bitmap.o main_lib.o Makefile 
	$(CC) $(CFLAGS) -o t88 88.o table.o adr.o cond.o store.o main.o doscurs.o bitmap.o main_lib.o; \
	$(CP) t88 ../exercise/t88; $(CP) t88 ../examples/t88; $(CP) t88 ../bin/t88; $(CP) t88 ../regtest/t88;
 
s88: 88.o table.o adr.o cond.o store.o mains.o doscurs.o bitmap.o main_lib.o Makefile 
	$(CC) $(CFLAGS) -o s88 88.o table.o adr.o cond.o store.o mains.o doscurs.o bitmap.o main_lib.o; \
	$(CP) s88 ../exercise/s88; $(CP) s88 ../examples/s88; $(CP) s88 ../bin/s88; $(CP) s88 ../regtest/s88;
