#include	"unp.h"
#include <termios.h>
#define	MAXN	16384		/* max #bytes that a client can request */

/* turn off echo (for slave pty) */
static void set_noecho(int fd);

void web_child(int sockfd)
{
	int	ntowrite;
	ssize_t	nread;
	char	result[MAXN],request[MAXLINE];
	fd_set	rset,wset;

	int cflags = fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL, cflags|O_NONBLOCK);

	//set_noecho(STDOUT_FILENO);
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	for ( ; ; ) {
		FD_SET(STDIN_FILENO, &rset);
		FD_SET(sockfd, &rset);
		select (sockfd + 1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO,&rset)){
			nread = read(STDIN_FILENO,request,MAXN);
			//debug_printf("1,%d  Request: %s\n",nread, request);
			Writen(sockfd, request, nread);
			//debug_printf("1, Request: %s\n",request);
			memset(request,'\0',MAXN);
		}
		if(FD_ISSET(sockfd,&rset)){
			if ( (nread = read(sockfd, result, MAXN)) == 0)
				exit(1);
			//debug_printf("2, Return %d bytes: %s\n", nread, result);
			printf("%s", result);
			fflush(stdout);
			memset(result,'\0',MAXN);
		}
		nread = 0;
	}
}

static void set_noecho(int fd)          /* turn off echo (for slave pty) */
{
        struct termios  stermios;

        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");

        stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
        stermios.c_oflag &= ~(ONLCR);
                        /* also turn off NL to CR/NL mapping on output */

        if (tcsetattr(fd, TCSANOW, &stermios) < 0)
                printf("tcsetattr error");
}
