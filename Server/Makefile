CC=gcc
LIBS=../lib/lib.a
CFLAGS=-g -lpthread 
PROGS =	serv 

all:	${PROGS}

serv:	serv.o mash.o message.o log.o threadpool.o locker.o tcp_listen.o ${LIBS}
	${CC} ${CFLAGS} -o serv serv.o mash.o message.o log.o threadpool.o locker.o tcp_listen.o ${LIBS}
serv.o: serv.c
	${CC} ${CFLAGS} -c serv.c
mash.o: mash.c
	${CC} ${CFLAGS} -c mash.c
message.o: message.c
	${CC} ${CFLAGS} -c message.c 
log.o: log.c
	${CC} ${CFLAGS} -c log.c
threadpool.o: threadpool.c
	${CC} ${CFLAGS} -c threadpool.c
locker.o: locker.c
	${CC} ${CFLAGS} -c locker.c
tcp_lisen.o: tcp_listen.c
	${CC} ${CFLAGS} -c tcp_listen.c

logs:=logs/read.log logs/write.log logs/message.log

CLEANFILES=*.o serv

clean:
	@rm -f ${PROGS} ${CLEANFILES}; rm -rf $(logs)
