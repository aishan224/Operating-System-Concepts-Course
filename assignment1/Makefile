

CC=gcc
CFLAGS=-Wall
PTHREADS=-lpthread
LRT = -lrt
MATH = -lm

all: 
	$(CC) -o dph dph.c $(LRT) $(PTHREADS)
	$(CC) -o mycall mycall.c
	$(CC) -o prod prod.c $(LRT) $(MATH) $(PTHREADS)
	$(CC) -o cons cons.c $(LRT) $(MATH) $(PTHREADS)

mycall: 
	$(CC) -o mycall mycall.c

dph: 
	$(CC) -o dph dph.c $(LRT) $(PTHREADS)

prod:
	$(CC) -o prod prod.c $(LRT) $(MATH) $(PTHREADS)

cons:
	$(CC) -o cons cons.c $(LRT) $(MATH) $(PTHREADS)

clean:
	rm -rf dph
	rm -rf mycall
	rm -rf prod
	rm -rf cons
