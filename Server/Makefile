CC=gcc
LIBS=../lib/lib.a
CFLAGS=-g 
PROGS =	serv 

all:	${PROGS}

serv:	serv.o mash.o message.o tcp_listen.o ${LIBS}
	${CC} ${CFLAGS} -o serv serv.o mash.o message.o tcp_listen.o ${LIBS}

mash.o: mash.c
	${CC} ${CFLAGS} -c mash.c
message.o: message.c
	${CC} ${CFLAGS} -c message.c 

serv.o: serv.c
	${CC} ${CFLAGS} -c serv.c

tcp_lisen.o: tcp_listen.c
	${CC} ${CFLAGS} -c tcp_listen.c

log=log/*

CLEANFILES=*.o serv

clean:
	rm -f ${PROGS} ${CLEANFILES} ${log}
