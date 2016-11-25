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
		snprintf(request, sizeof(request), "ps\n");
		Writen(sockfd, request, 3);
		printf("request: %s",request);

		if ( (nread = read(sockfd, result, MAXN)) == 0)
			continue;		/* connection closed by other end */
		printf("result: %s\n",result);
		sleep(1);
	}
}
