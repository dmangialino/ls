CC=gcc
CLFLAGS=-Wall

myls: myls.o
myls.o: myls.c

clean:
	rm -f myls myls.o
