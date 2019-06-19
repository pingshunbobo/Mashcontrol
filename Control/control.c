#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include "unp.h"

#define	BUF_SIZE	16384		/* max #bytes to request from server */
enum CONTROL_STATUS {MASHCMD, INTERFACE};

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

	char	request[BUF_SIZE];
	char	reply[BUF_SIZE];

	//if (argc != 3)
	//	err_quit("usage: client <hostname or IPaddr> <port>");
	//sockfd = Tcp_connect(argv[1], argv[2]);
	sockfd = Tcp_connect("127.0.0.1", "19293");

	enum CONTROL_STATUS  control_stat = MASHCMD;
	/*
	 login first.
	*/
	memcpy(request, "Mashcmd:help", 12);
	Write(sockfd, request, 12);

	FD_ZERO(&rset);
	while(1){
		FD_SET(sockfd, &rset);
		FD_SET(STDIN_FILENO, &rset);
		select (sockfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(sockfd, &rset)){
			memset(reply, '\0', BUF_SIZE);
			if ( (nbytes = read(sockfd, reply, BUF_SIZE)) <= 0){
				printf("server returned %d bytes error %s\n", nbytes, strerror(errno));
				break;
			}

			if (writen(STDOUT_FILENO, reply, nbytes) != nbytes)
				printf("writen stdout error.\n");
			if(!strncmp(reply, "Into interface mode:\n", 21))
				control_stat = INTERFACE;
			if(!strncmp(reply, "Into mashcmd mode:\n", 19))
				control_stat = MASHCMD;
				
		}
		if(FD_ISSET(STDIN_FILENO, &rset)){
			/* Add Magic code to message! */
			memcpy(request, "Mashcmd:", 8);
			if ( (nbytes = read(STDIN_FILENO, request + 8, BUF_SIZE)) < 0)
				break;
			//printf("read %d bytes from stdin : %s\n", nbytes, reply);

			Write(sockfd, request, nbytes + 8);
			memset(reply, '\0', BUF_SIZE);
			nbytes = 0;
		}
	}
	Close(sockfd);
	exit(1);
}
