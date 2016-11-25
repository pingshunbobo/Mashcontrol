#include	"unp.h"

#define	MAXN	16384		/* max #bytes to request from server */

int main(int argc, char **argv)
{
	int	fd, fdm;
	pid_t	pid;
	int	nbytes;
	char 	slave_name[20];	

	char	request[MAXLINE];
	char	reply[MAXN] = "ok!";

	if (argc != 3)
		err_quit("usage: client <hostname or IPaddr> <port> <#bytes/request>");

	fd = Tcp_connect(argv[1], argv[2]);
	
	pid = pty_fork(&fdm, slave_name, 20, NULL, NULL);
	if (pid < 0)
		printf("fork error");

	else if (pid == 0) {		/* child */
		//if (execvp(argv[optind], &argv[optind]) < 0)
		//	printf("can't execute: %s", argv[optind]);
		if( execl("/usr/bin/bash", "bash", NULL)  == -1)
			printf("%s execve error!",strerror(errno));
	}
	/*  parent process  */
	while(1){
		/*
			1,read from socket.
			2,write to pty.
			3,read from pty.
			4,write to socket.
		*/
		if ( (nbytes = Readn(fd, request, 3)) < 0)
			err_quit("server returned %d bytes", nbytes);
		printf("\n1,requested %d: %s\n", nbytes, request);

		if (writen(fdm, request, nbytes) != nbytes)
			printf("writen error to master pty");

		if ( (nbytes = read(fdm, reply, BUFFSIZE)) <= 0)
			break;
		printf("3,read %d bytes from fdm \n",nbytes);

		Write(fd, reply, nbytes);
	}
	Close(fd);

	exit(0);
}
