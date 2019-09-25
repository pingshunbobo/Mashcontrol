CC=gcc
LIBS=../lib/lib.a
CFLAGS= -g
PROGS =	control 

all:	${PROGS}

control: control.o mash.o tcp_connect.o readn.o	
	 ${CC} ${CFLAGS} -o control mash.o control.o tcp_connect.o readn.o  ${LIBS}
mash.o: mash.c
	${CC} ${CFLAGS} -c mash.c

control.o: control.c
	${CC} ${CFLAGS} -c control.c

tcp_connect.o: tcp_connect.c
	${CC} ${CFLAGS} -c tcp_connect.c

readn.o: readn.c
	${CC} ${CFLAGS} -c readn.c

CLEANFILES=*.o 
clean:
	rm -f ${PROGS} ${CLEANFILES}
