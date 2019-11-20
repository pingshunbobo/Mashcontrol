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

int in_message_seq = 0;
int out_message_seq = 0;

enum CLIENT_STATUS  client_stat = STANDBY;
int	client_fd = 0;
int	fdm = 0;
pid_t	work_pid = 0;
pid_t	heart_pid = 0;
fd_set	rset, wset;
int	reply_size = 0;
char	reply[BUF_SIZE];

int main(int argc, char **argv)
{
	int	nbytes;
	int	logined;
	int	read_idx = 0;
	int	checked_idx = 0;
	char	fdm_buf[BUF_SIZE];
	char	request[BUF_SIZE];
	MASH_MESSAGE message;

	daemon_init(&work_pid);

connect:
	client_fd = Tcp_connect(SERVER_ADDR, SERVER_PORT);
	mash_upload_info(reply, &reply_size);
	//heart_pid = heartbeat_fork();
	
	FD_ZERO(&rset);
	FD_ZERO(&wset);

	/*  parent process loop wait io events */
	while(1){
		FD_SET(client_fd, &rset);
		if( fdm > 0 )
			FD_SET(fdm, &rset);
		select (client_fd + 10, &rset, &wset, NULL, NULL);
		if(FD_ISSET(client_fd, &wset)){
			if( reply_size > 0){
				nbytes = writen(client_fd, reply, reply_size);
				memset(reply, '\0', BUF_SIZE);
				reply_size = 0;
			}
			FD_CLR(client_fd, &wset);
		}
		if(FD_ISSET(client_fd, &rset)){
			nbytes = mash_read(client_fd, request, BUF_SIZE);
			if ( nbytes < 0 ){
				goto reconnect;	/* read EOF sockfd closed! */
			}else if( nbytes == 0 ){
				continue;
			}
			read_idx += nbytes;
			mash_proc(request, &checked_idx, &read_idx);
		}
		if(FD_ISSET(fdm, &rset)){
			memset(fdm_buf, '\0', BUF_SIZE);
			nbytes = mash_read(fdm, fdm_buf, BUF_SIZE);
			if( nbytes < 0 ){
				out_work(&work_pid, &fdm);
			}else if( nbytes == 0){
				continue;
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
	if(heart_pid){
		kill(heart_pid, SIGKILL);
		waitpid(heart_pid, 0, 0);
	}
	if( fdm ){
		if (writen(fdm, "\3", 1) != 1);
			log_client("writen error to master pty");
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
