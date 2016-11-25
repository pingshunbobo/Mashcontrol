CC=gcc
LIBS=../lib/lib.a
CFLAGS=-lpthread
PROGS =	client 

all:	${PROGS}

client:	client.o tcp_connect.o readline.o readn.o pty_fork.o
	${CC} ${CFLAGS} -o client client.o tcp_connect.o pty_fork.o readline.o readn.o  ${LIBS}

client.o: client.c
	${CC} -c client.c

pty_fork.o: pty_fork.c
	${CC} -c pty_fork.c

tcp_connect.o: tcp_connect.c
	${CC} -c tcp_connect.c

readline.o: readline.c
	${CC} -c readline.c

readn.o: readn.c
	${CC} -c readn.c

CLEANFILES=*.o 
clean:
	rm -f ${PROGS} ${CLEANFILES}
