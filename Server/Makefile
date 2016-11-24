CC=gcc
LIBS=../lib/lib.a
CFLAGS=-lpthread -g
PROGS =	serv 

all:	${PROGS}

serv:	serv.o web_child.o pr_cpu_time.o tcp_listen.o  pthread.o
	${CC} ${CFLAGS}  -o serv serv.o web_child.o pr_cpu_time.o tcp_listen.o pthread.o ${LIBS}

serv.o: serv.c
	${CC} -c serv.c

tcp_lisen.o: tcp_listen.c
	${CC} -c tcp_listen.c

pthread.o: pthread.c
	${CC} -c pthread.c

CLEANFILES=*.o serv
clean:
	rm -f ${PROGS} ${CLEANFILES}
