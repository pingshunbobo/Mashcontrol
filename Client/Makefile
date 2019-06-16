CC=gcc
LIBS=../lib/lib.a
CFLAGS = -g
PROGS =	client 

all:	${PROGS}

client:	client.o tcp_connect.o readn.o pty_fork.o daemon.o child_work.o
	${CC} ${CFLAGS} -o client client.o tcp_connect.o pty_fork.o daemon.o readn.o child_work.o  ${LIBS}

client.o: client.c
	${CC} ${CFLAGS} -c client.c

child_work.o: child_work.c
	${CC} ${CFLAGS} -c child_work.c

daemon.o: daemon.c
	${CC} ${CFLAGS} -c daemon.c

pty_fork.o: pty_fork.c
	${CC} ${CFLAGS} -c pty_fork.c

tcp_connect.o: tcp_connect.c
	${CC} ${CFLAGS} -c tcp_connect.c

readn.o: readn.c
	${CC} ${CFLAGS} -c readn.c

CLEANFILES=*.o 
clean:
	rm -f ${PROGS} ${CLEANFILES}
