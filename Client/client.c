#include "unp.h"
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include "mash.h"

#define SERVER_ADDR "mashcontrol.pingshunbobo.com"
#define SERVER_PORT "19293"

enum CLIENT_STATUS  client_stat = STANDBY;
int	client_fd = 0;
int	fdm = 0;
pid_t	work_pid = 0;
pid_t	heart_pid = 0;
fd_set	rset;

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
	char 	slave_name[20];
	char	buf[BUF_SIZE];

	char	request[BUF_SIZE];
	char	reply[BUF_SIZE];

	daemon_init(&work_pid);

connect:
	client_fd = Tcp_connect(SERVER_ADDR, SERVER_PORT);
	heart_pid = setnonblocking(client_fd);

	/* add Client hello info */
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	if(getsockname(client_fd, (struct sockaddr*)&addr, &addrlen) == -1){
		fprintf(stderr,"Get client information failed,=%d.\n", client_fd);
		return 0;
	}
	if((inet_ntop(addr.sin_family, &addr.sin_addr, buf, addrlen)) == NULL){
		fprintf(stderr,"Get client information failed, fd=%d .\n", client_fd);        
		return 0;
	}
	memset(reply, '\0', BUF_SIZE);
	snprintf(reply, 11, "Mashinfo: " );
	memcpy(reply + 10, buf, 16);
	nbytes = write(client_fd, reply, 26);

	heart_pid = heartbeat_fork();
	
	FD_ZERO(&rset);
	/*  parent process  */
	while(1){
		FD_SET(client_fd, &rset);
		select (client_fd + 10, &rset, NULL, NULL, NULL);
		if(FD_ISSET(client_fd, &rset)){
			memset(request, '\0', BUF_SIZE);
			nbytes = read(client_fd, request, BUF_SIZE);
			if ( nbytes <= 0 ){
				goto reconnect;	/* read error! */
			}
			if(MASH_CTL == mash_type(request, nbytes)){
				mash_proc_ctl(request, nbytes);
			}else if(MASH_DATA == mash_type(request, nbytes)){
				mash_proc_data(request, nbytes);
			}else if(MASH_HEART == mash_type(request, nbytes)){
				mash_proc_ctl(request, nbytes);
			}else
				return -1;
		}
		if(FD_ISSET(fdm, &rset)){
			memset(reply, '\0', BUF_SIZE);
			nbytes = read(fdm, reply, BUFFSIZE); 
			if( nbytes <= 0 ){
				if (errno == EAGAIN)
					continue;
				else{
					work_pid = 0;
					client_stat = STANDBY;
					FD_CLR(fdm, &rset);
					close(fdm);
					mash_send_ctl("standby!", 8);
					continue;
				}
			}else {
				/* Copy reply data to server. */
				nbytes = mash_send_data(reply, nbytes);
				if( nbytes < 0 )
					goto restart;
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
	sleep(5);
	goto connect;

restart:
	kill(work_pid, SIGKILL);
	waitpid(work_pid, 0, 0);
	work_pid = 0;
	client_stat = STANDBY;
	Close(client_fd);
        Close(fdm);
	sleep(5);
	goto connect;
}
