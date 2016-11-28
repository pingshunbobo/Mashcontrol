CC=gcc
LIBS=../lib/lib.a
CFLAGS=
PROGS =	control 

all:	${PROGS}

control: control.o tcp_connect.o readn.o	
	 ${CC} ${CFLAGS} -o control control.o tcp_connect.o readn.o  ${LIBS}

control.o: control.c
	${CC} -c control.c

tcp_connect.o: tcp_connect.c
	${CC} -c tcp_connect.c

readn.o: readn.c
	${CC} -c readn.c

CLEANFILES=*.o 
clean:
	rm -f ${PROGS} ${CLEANFILES}
