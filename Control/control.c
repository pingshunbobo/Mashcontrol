#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include "unp.h"
#include <signal.h>

#define	BUF_SIZE	16384		/* max #bytes to request from server */
enum CONTROL_STATUS {MASHCMD, INTERFACE};

int sig_quit_flag = 0;
struct termios saved_stermios;

static void set_noecho(int fd)
{
	struct termios	stermios;

	if (tcgetattr(fd, &stermios) < 0)
		printf("tcgetattr error");
	memcpy(&saved_stermios, &stermios, sizeof(struct termios));
	//stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	/* also turn off NL to CR/NL mapping on output */
	//stermios.c_oflag &= ~(ONLCR);

	if (tcsetattr(fd, TCSANOW, &stermios) < 0)
		printf("tcsetattr error");
}

static void set_nobrk(int fd)
{
	struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");
	stermios.c_iflag |= (IGNPAR | ICRNL);
	stermios.c_iflag &= ~( IGNCR | IXON );
	stermios.c_lflag &= ~( ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK );

        if (tcsetattr(fd, TCSANOW, &stermios) < 0)
                printf("tcsetattr error");
}

static void save_termios(int fd)
{
	struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");
	memcpy(&saved_stermios, &stermios, sizeof(struct termios));
}

static void restore_termios(int fd)
{
	if (tcsetattr(fd, TCSANOW, &saved_stermios) < 0)
		printf("tcsetattr error %s\n", strerror(errno));
}

void signal_handler()
{
	sig_quit_flag = 1;
	fflush(stdout);
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
	save_termios(STDIN_FILENO);
	/*
	 login first.
	*/
	memcpy(request, "Mashcmd:help", 12);
	Write(sockfd, request, 12);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, signal_handler);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, signal_handler);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, signal_handler);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, signal_handler);

	FD_ZERO(&rset);
	while(1){
		FD_SET(sockfd, &rset);
		FD_SET(STDIN_FILENO, &rset);
		if (select (sockfd + 1, &rset, NULL, NULL, NULL) < 0 ){
			printf("select error, agen!");
			break;
		}
		if(FD_ISSET(sockfd, &rset)){
			memset(reply, '\0', BUF_SIZE);
			if ( (nbytes = read(sockfd, reply, BUF_SIZE)) <= 0){
				printf("server returned %d bytes error %s\n", nbytes, strerror(errno));
				break;
			}
			if(!strncmp(reply, "Into interface mode.\n", 21)){
				control_stat = INTERFACE;
				set_nobrk(STDIN_FILENO);
			}
			if(!strncmp(reply, "Into mashcmd mode.\n", 19)){
				control_stat = MASHCMD;
				restore_termios(STDIN_FILENO);
			}
			if (writen(STDOUT_FILENO, reply, nbytes) != nbytes)
				printf("writen stdout error.\n");
				
		}
		if(FD_ISSET(STDIN_FILENO, &rset)){
			/* Add Magic code to message! */
			if( MASHCMD == control_stat ){
				memcpy(request, "Mashcmd:", 8);
				if ( (nbytes = read(STDIN_FILENO, request + 8, BUF_SIZE)) <= 0)
					break;
				Write(sockfd, request, nbytes + 8);
			}else if( INTERFACE == control_stat ){
				memcpy(request, "Mashdata:", 9);
				if ( (nbytes = read(STDIN_FILENO, request + 9, BUF_SIZE)) <= 0)
					break;
				Write(sockfd, request, nbytes + 9);

			}
			//printf("read %d bytes from stdin : %s\n", nbytes, reply);
			//memset(reply, '\0', BUF_SIZE);
			bzero(reply, BUF_SIZE);
			nbytes = 0;
		}
	}
	Close(sockfd);
	restore_termios(STDIN_FILENO);
	exit(0);
}
