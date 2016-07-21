all: lib.a

lib.a:	error.o readline.o signal.o wrappthread.o wrapsock.o wrapunix.o writen.o
	ar crv lib.a *.o

error.o:	error.c
	gcc -c error.c

readline.o:	readline.c
	gcc -c readline.c

signal.o:	signal.c
	gcc -c signal.c

wrappthread.o:	wrappthread.c
	gcc -c wrappthread.c

wrapsock.o:	wrapsock.c
	gcc -c  wrapsock.c

wrapunix.o:	wrapunix.c
	gcc -c wrapunix.c

writen.o:	writen.c
	gcc -c writen.c

clean:
	rm  -rf *.o *.a
