CC=gcc
LIBS=../lib/lib.a
CFLAGS= -g 
PROGS =	serv 

all:	${PROGS}

serv:	serv.o mash.o tcp_listen.o ${LIBS}
	${CC} ${CFLAGS} -o serv serv.o mash.o tcp_listen.o ${LIBS}

mash.o: mash.c
	${CC} ${CFLAGS} -c mash.c

serv.o: serv.c
	${CC} ${CFLAGS} -c serv.c

tcp_lisen.o:	tcp_listen.c
	${CC} ${CFLAGS} -c tcp_listen.c

CLEANFILES=*.o serv

clean:
	rm -f ${PROGS} ${CLEANFILES}
