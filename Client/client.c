#include "unp.h"
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include "mash.h"
#include "message.h"

#define SERVER_ADDR "mashcontrol.pingshunbobo.com"
#define SERVER_PORT "19293"

int message_seq = 0;

enum CLIENT_STATUS  client_stat = STANDBY;
int	client_fd = 0;
int	fdm = 0;
pid_t	work_pid = 0;
pid_t	heart_pid = 0;
fd_set	rset, wset;

void server(char *host, char *port);

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    Fcntl(fd, F_SETFL, new_option );
    return old_option;
}

int main(int argc, char **argv)
{
	int	nbytes;
	int	logined;
	int	read_idx = 0;
	int	checked_idx = 0;
	int	reply_size = 0;
	char	fdm_buf[BUF_SIZE];
	char	request[BUF_SIZE];
	char	reply[BUF_SIZE];
	MASH_MESSAGE message;

	daemon_init(&work_pid);

connect:
	client_fd = Tcp_connect(SERVER_ADDR, SERVER_PORT);
	setnonblocking(client_fd);
	mash_info(reply, &reply_size);
	heart_pid = heartbeat_fork();
	
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	/*  parent process  */
	while(1){
		FD_SET(client_fd, &rset);
		if( fdm > 0 )
			FD_SET(fdm, &rset);
		select (client_fd + 10, &rset, &wset, NULL, NULL);
		if(FD_ISSET(client_fd, &rset)){
			nbytes = read(client_fd, request, BUF_SIZE);
			if ( nbytes < 0 ){
				if (errno == EAGAIN)
					continue;
			}else if( nbytes == 0 ){
				goto reconnect;	/* read EOF sockfd closed! */
			}
			read_idx += nbytes;
			mash_proc(request, &checked_idx, &read_idx);
		}
		if(FD_ISSET(client_fd, &wset)){
			if( reply_size > 0){
				writen(client_fd, reply, reply_size);
				memset(reply, '\0', BUF_SIZE);
				reply_size = 0;
			}
			FD_CLR(client_fd, &wset);
		}
		if(FD_ISSET(fdm, &rset)){
			memset(reply, '\0', BUF_SIZE);
			nbytes = read(fdm, fdm_buf, BUF_SIZE);
			if( nbytes <= 0 ){
				if (errno == EAGAIN)
					continue;
				else
					out_work(reply, &reply_size, &work_pid, &fdm);
			}else {
				/* Copy reply data to server. */
				nbytes = mash_send_data(fdm_buf, nbytes);
				if( nbytes < 0 )
					goto restart;
				memset(fdm_buf, '\0', BUF_SIZE);
			}
		}
	} /* end loop */

reconnect:
	Close(client_fd);
	kill(heart_pid, SIGKILL);
	waitpid(heart_pid, 0, 0);
	if( fdm ){
		if (writen(fdm, "\3", 1) != nbytes)
			printf("writen error to master pty");
	}
	sleep(3);
	goto connect;

restart:
	kill(work_pid, SIGKILL);
	waitpid(work_pid, 0, 0);
	work_pid = 0;
	client_stat = STANDBY;
	Close(client_fd);
        Close(fdm);
	sleep(3);
	goto connect;
}
