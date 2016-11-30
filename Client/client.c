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
	int	nbytes;
	int	logined;
	char 	slave_name[20];
	pid_t	pid;
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
		set_noecho(STDIN_FILENO);
		if( execl("/usr/bin/bash", "bash", NULL)  == -1)
			printf("%s execve error!",strerror(errno));
	}
	int cflags = fcntl(fdm,F_GETFL,0);
	fcntl(fdm,F_SETFL, cflags|O_NONBLOCK);

	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		FD_SET(fd, &rset);
		FD_SET(fdm, &rset);
		select (fdm + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(fd,&rset)){
			if ( (nbytes = Readline(fd, request, MAXN)) <= 0){
				printf("server returned %d bytes error %s", nbytes, strerror(errno));
				close(fd);
				exit(1);
			}
			printf("requested %d bytes: %s\n", nbytes, request);

			if (writen(fdm, request, nbytes) != nbytes)
				printf("writen error to master pty");
			memset(request, '\0', MAXN);
		}
		if(FD_ISSET(fdm,&rset)){
			memset(reply, '\0', MAXN);
			if ( (nbytes = read(fdm, reply, BUFFSIZE)) <= 0)
				break;
			printf("pid: %d read %d bytes from fdm : %s\n",getpid(), nbytes, reply);
			fflush(stdout);
			Write(fd, reply, nbytes);
		}
	}
	Close(fd);

	exit(0);
}
