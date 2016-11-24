#include	"unp.h"

#define	MAXN	16384		/* max #bytes to request from server */

int main(int argc, char **argv)
{
	int	fd, fdm;
	pid_t	pid;
	int	nbytes;
	char 	slave_name[20];
	fd_set	rset;	

	char	request[MAXLINE];
	char	reply[MAXN] = "ok!";

	if (argc != 3)
		err_quit("usage: client <hostname or IPaddr> <port>");

	fd = Tcp_connect(argv[1], argv[2]);
	
	pid = pty_fork(&fdm, slave_name, 20, NULL, NULL);
	if (pid < 0)
		printf("fork error");

	else if (pid == 0) {		/* child with pty slave! */
		if( execl("/usr/bin/bash", "bash", NULL)  == -1)
			printf("%s execve error!",strerror(errno));
	}

	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		/*
			1,read from socket.
			2,write to pty.
			3,read from pty.
			4,write to socket.
		*/
		FD_SET(fd, &rset);
		select (fd+1, &rset, NULL, NULL, NULL);

		if ( (nbytes = read(fd, request, MAXN)) < 0)
			printf("server returned %d bytes", nbytes);
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
