#include	"unp.h"

#define	MAXN	16384		/* max #bytes that a client can request */

void
web_child(int sockfd)
{
	int	ntowrite;
	ssize_t	nread;
	char	result[MAXN],request[MAXLINE];
	int cflags = fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL, cflags|O_NONBLOCK);

	for ( ; ; ) {
		nread = read(STDIN_FILENO,request,MAXN);
		printf("1,%d  Request: %s\n",nread, request);
		//Writen(sockfd, request, nread);
		write(sockfd, request, nread);
		printf("1, Request: %s\n",request);
		sleep(1);
		if ( (nread = read(sockfd, result, MAXN)) == 0)
			continue;		/* connection closed by other end */
		printf("2, Return %d bytes: %s\n", nread, result);
		sleep(1);
	}
}
