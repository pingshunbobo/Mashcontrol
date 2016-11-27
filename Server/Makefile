CC=gcc
LIBS=../lib/lib.a
CFLAGS=-lpthread -g
PROGS =	serv 

all:	${PROGS}

serv:	serv.o mash_child.o pr_cpu_time.o tcp_listen.o
	${CC} ${CFLAGS}  -o serv serv.o mash_child.o pr_cpu_time.o tcp_listen.o ${LIBS}

mash_child.o: mash_child.c
	${CC} -c mash_child.c

serv.o: serv.c
	${CC} -c serv.c

tcp_lisen.o: tcp_listen.c
	${CC} -c tcp_listen.c

CLEANFILES=*.o serv
clean:
	rm -f ${PROGS} ${CLEANFILES}
