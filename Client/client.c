#include	"unp.h"
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#define	MAXN	16384		/* max #bytes to request from server */


void sig_child(int signo);
void server(char *host, char *port);

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
	signal(SIGCHLD, sig_child);
	server("127.0.0.1", "9367");
}

void server(char *host, char *port)
{
	int	fd, fdm;
	int	nbytes;
	int	logined;
	char 	slave_name[20];
	pid_t	pid;
	fd_set	rset;

	char	request[MAXN];
	char	reply[MAXN];

start:
	fd = Tcp_connect(host, port);
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
			memset(request, '\0', MAXN);
			if ( (nbytes = Readline(fd, request, MAXN)) <= 0){
				printf("server returned %d bytes error %s", nbytes, strerror(errno));
				goto restart;
			}
			printf("requested %d bytes: %s\n", nbytes, request);

			if (writen(fdm, request, nbytes) != nbytes)
				printf("writen error to master pty");
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

restart:
	Close(fd);
	sleep(2);
	printf("restart\n");
	goto start;
}

void sig_child(int signo)
{
	pid_t pid;
	int stat;
	pid = wait(&stat);
	printf("child %d terminated\n", pid);
	sleep(5);
	server("127.0.0.1", "9367");
}
