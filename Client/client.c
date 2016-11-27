#include	"unp.h"
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
	int	fd, fdm;
	pid_t	pid;
	int	nbytes;
	char 	slave_name[20];
	fd_set	rset;	

	char	request[MAXN];
	char	reply[MAXN];

	if (argc != 3)
		err_quit("usage: client <hostname or IPaddr> <port>");

	fd = Tcp_connect(argv[1], argv[2]);
	
	pid = pty_fork(&fdm, slave_name, 20, NULL, NULL);
	if (pid < 0)
		printf("fork error");

	else if (pid == 0) {		/* child with pty slave! */
		//set_noecho(STDIN_FILENO);
		if( execl("/usr/bin/bash", "bash", NULL)  == -1)
			printf("%s execve error!",strerror(errno));
	}

	int cflags = fcntl(fdm,F_GETFL,0);
	fcntl(fdm,F_SETFL, cflags|O_NONBLOCK);

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
		FD_SET(fdm, &rset);
		select (fdm + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(fd,&rset)){
			if ( (nbytes = Readline(fd, request, MAXN)) <= 0){
				printf("server returned %d bytes error %s", nbytes,strerror(errno));
				close(fd);
				exit(1);
			}
			printf("\n1,requested %d bytes: %s\n", nbytes, request);

			if (writen(fdm, request, nbytes) != nbytes)
				printf("writen error to master pty");
			memset(request,'\0', MAXN);
		}
		if(FD_ISSET(fdm,&rset)){
			if ( (nbytes = read(fdm, reply, BUFFSIZE)) <= 0)
				break;
			printf("3,read %d bytes from fdm : %s\n",nbytes, reply);

			Write(fd, reply, nbytes);
			memset(reply,'\0', MAXN);
			nbytes = 0;
		}
	}
	Close(fd);

	exit(0);
}
