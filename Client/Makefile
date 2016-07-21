CC=gcc
LIBS=../lib/lib.a
CFLAGS=-lpthread
PROGS =	serv 

all:	${PROGS}

serv:	client.o tcp_connect.o readline.o readn.o	
	${CC} ${CFLAGS}  client.o tcp_connect.o readline.o readn.o  ${LIBS}

client.o: client.c
	${CC} -c client.c

tcp_connect.o: tcp_connect.c
	${CC} -c tcp_connect.c

readline.o: readline.c
	${CC} -c readline.c

readn.o: readn.c
	${CC} -c readn.c

CLEANFILES=*.o 
clean:
	rm -f ${PROGS} ${CLEANFILES}
