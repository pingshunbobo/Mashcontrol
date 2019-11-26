CC=gcc
LIBS=../lib/lib.a
CFLAGS= -g
PROGS =	control 

all:	${PROGS}

control: control.o mash.o message.o log.o tcp_connect.o readn.o	
	 ${CC} ${CFLAGS} -o control mash.o message.o log.o control.o tcp_connect.o readn.o  ${LIBS}

mash.o: mash.c 
	${CC} ${CFLAGS} -c mash.c

message.o: message.c
	${CC} ${CFLAGS} -c message.c

log.o: log.c
	${CC} ${CFLAGS} -c log.c

control.o: control.c
	${CC} ${CFLAGS} -c control.c

tcp_connect.o: tcp_connect.c
	${CC} ${CFLAGS} -c tcp_connect.c

readn.o: readn.c
	${CC} ${CFLAGS} -c readn.c

logs=logs/read.log logs/write.log logs/message.log
CLEANFILES=*.o 

clean:
	@rm -f ${PROGS} ${CLEANFILES}; rm -rf $(logs)
