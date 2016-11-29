#include "unp.h"
#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#define	MAXN	16384		/* max #bytes to request from server */

static void set_noecho(int fd)		/* turn off echo (for slave pty) */
{
	struct termios	stermios;

	if (tcgetattr(fd, &stermios) < 0)
		printf("tcgetattr error");

	stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	stermios.c_oflag &= ~(ONLCR);
			/* also turn off NL to CR/NL mapping on output */

	if (tcsetattr(fd, TCSANOW, &stermios) < 0)
		printf("tcsetattr error");
}

int main(int argc, char **argv)
{
	int	sockfd;
	int	nbytes;
	fd_set	rset;	

	char	request[MAXN];
	char	reply[MAXN];

	if (argc != 3)
		err_quit("usage: client <hostname or IPaddr> <port>");
	sockfd = Tcp_connect(argv[1], argv[2]);
	//set_noecho(sockfd);	

	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		//printf("Mashcmd#");
		fflush(stdout);
		FD_SET(sockfd, &rset);
		FD_SET(STDIN_FILENO, &rset);
		select (sockfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(sockfd,&rset)){
			//if ( (nbytes = Readline(sockfd, reply, MAXN)) <= 0){
			if ( (nbytes = read(sockfd, reply, MAXN)) <= 0){
				printf("server returned %d bytes error %s", nbytes,strerror(errno));
				close(sockfd);
				exit(1);
			}
			//printf("requested %d bytes: %s\n", nbytes, request);

			if (writen(STDOUT_FILENO, reply, nbytes) != nbytes)
				printf("writen error to master pty");
			memset(request,'\0', MAXN);
		}
		if(FD_ISSET(STDIN_FILENO,&rset)){
			memcpy(request, "Mashcmd:", 8);
			if ( (nbytes = read(STDIN_FILENO, request + 8, BUFFSIZE)) <= 0)
				break;
			//printf("read %d bytes from stdin : %s\n", nbytes, reply);

			Write(sockfd, request, nbytes + 8);
			memset(reply,'\0', MAXN);
			nbytes = 0;
		}
	}
	Close(sockfd);

	exit(0);
}
