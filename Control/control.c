#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include "unp.h"
#include <signal.h>
#include "mash.h"

int	connfd = 0;
int	sig_quit_flag = 0;
struct	termios	saved_stermios;
enum	CONTROL_STATUS	control_stat = MASHCMD;

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
	int	read_idx = 0;
	int	checked_idx = 0;

	//if (argc != 3)
	//	err_quit("usage: client <hostname or IPaddr> <port>");
	//connfd = Tcp_connect(argv[1], argv[2]);
	connfd = Tcp_connect("127.0.0.1", "19293");

	save_termios(STDIN_FILENO);
	/*
	 login first.
	*/
	mash_send_cmd("help", 4);
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
		FD_SET(connfd, &rset);
		FD_SET(STDIN_FILENO, &rset);
		if (select (connfd + 1, &rset, NULL, NULL, NULL) < 0 ){
			printf("select error, agen!\n");
			break;
		}
		if(FD_ISSET(connfd, &rset)){
			if ((nbytes = read(connfd, reply + read_idx, BUF_SIZE - read_idx)) <= 0){
				if( errno == EAGAIN)
					continue;
				else
					printf("\nServer Closed\n");
				break;
			}
			int file_fd = open("./log/read.log", O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR);
			write(file_fd, reply, nbytes);
			close(file_fd);
			read_idx += nbytes;
			mash_proc(reply, &checked_idx, &read_idx);
		}
		if(FD_ISSET(STDIN_FILENO, &rset)){
			if ((nbytes = read(STDIN_FILENO, request, BUF_SIZE)) <= 0)
				break;
			if(!strncmp(request, "\33OP", 3)){
				/* Hot key F1." */
				mash_send_cmd("mashcmd", 7);
				continue;
			}
			if( MASHCMD == control_stat ){
				mash_send_cmd(request, nbytes);
			}else if( INTERFACE == control_stat ){
				mash_send_data(request, nbytes);

			}
			memset(request, '\0', BUF_SIZE);
			nbytes = 0;
		}
	}
	Close(connfd);
	restore_termios(STDIN_FILENO);
	exit(0);
}
