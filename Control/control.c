#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include "unp.h"
#include <signal.h>
#include "mash.h"

#define	BUF_SIZE	16384		/* max #bytes to request from server */

int	sockfd = 0;
int	sig_quit_flag = 0;
struct	termios	saved_stermios;
enum	CONTROL_STATUS	control_stat = MASHCTL;

void signal_handler()
{
	sig_quit_flag = 1;
	fflush(stdout);
}

int main(int argc, char **argv)
{
	int	nbytes;
	fd_set	rset;	

	char	request[BUF_SIZE];
	char	reply[BUF_SIZE];

	//if (argc != 3)
	//	err_quit("usage: client <hostname or IPaddr> <port>");
	//sockfd = Tcp_connect(argv[1], argv[2]);
	sockfd = Tcp_connect("127.0.0.1", "19293");

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
				if( errno == EAGAIN)
					continue;
				else
					//printf("server returned %d bytes error %s\n", nbytes, strerror(errno));
					printf("\nServer Closed\n");
				break;
			}
			if( MASH_CTL == mash_type(reply, nbytes)  ){
				mash_ctl(reply, nbytes);
			}else if( MASH_NOTE == mash_type(reply, nbytes)  ){
				mash_note(reply, nbytes);
			}else if( MASH_DATA == mash_type(reply, nbytes)  ){
				mash_data(reply, nbytes);
			}
		}
		if(FD_ISSET(STDIN_FILENO, &rset)){
			/* Add Magic code to message! */
			if( MASHCTL == control_stat ){
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
			memset(request, '\0', BUF_SIZE);
			nbytes = 0;
		}
	}
	Close(sockfd);
	restore_termios(STDIN_FILENO);
	exit(0);
}
