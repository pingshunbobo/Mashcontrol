#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/select.h>
#include	<termios.h>
#include 	"mash.h"

extern enum CLIENT_STATUS client_stat;
extern int	client_fd;
extern int	fdm;
extern pid_t	work_pid;
extern fd_set	rset;

enum MASH_DATA_TYPE mash_type(char *request, int nbytes)
{
	if(!strncmp(request, "Mashctl:", 8))
                return MASH_CTL;
	if(!strncmp(request, "Mashinfo:", 9))
                return MASH_INFO;
	if(!strncmp(request, "Mashdata:", 9))
                return MASH_DATA;
	if(!strncmp(request, "Mashheart:", 10))
                return MASH_HEART;
	return MASH_UNKNOW;
}

int mash_proc_ctl(char *request, int nbytes)
{
	char buf[1024];
	if( !strncmp(request, "Mashctl:work!", 13) ){
		if( client_stat == STANDBY && work_pid == 0){
			create_work(&work_pid, &fdm);
			FD_SET(fdm, &rset);
		}
		client_stat = WORK;
		memcpy(buf, "Mashctl:work!", 13);
		if (writen(client_fd, buf, 13) != 13){
			printf("Writen error!");
			return -1;
		}
	}else if( !strncmp(request, "Mashctl:standby!", 16) ){
		client_stat = STANDBY;
		memcpy(buf, "Mashctl:standby!", 16);
		if (writen(client_fd, buf, 16) != 16){
			printf("Writen error!");
			return -1;
		}
	}else{
		return -1;
	}
	return 0;
}

int mash_proc_data(char *request, int nbytes)
{
	char buf[1024];
	if ( client_stat == WORK ){
		/* write content which from server to the pty bash */
		memcpy(buf, request + 9, nbytes - 9);
		if (writen(fdm, buf, nbytes - 9) != nbytes)
			printf("writen error to master pty");
		FD_SET(fdm, &rset);
	}else {
		mash_send_ctl("standby!", 8);
		/* do nothing !*/
	}
	return 0;
}

int mash_heart(char *request, int nbytes)
{
	/* update last check time */
	return 0;
}

int mash_send_ctl(char *reply, int nbytes)
{
	char buf[BUF_SIZE + 8];
	memcpy(buf, "Mashctl:", 8);
	memcpy(buf + 8, reply, nbytes);
	return writen(client_fd, buf, nbytes + 8);
}

int mash_send_data(char *reply, int nbytes)
{
	char buf[BUF_SIZE + 9];
	memcpy(buf, "Mashdata:", 9);
	memcpy(buf + 9, reply, nbytes);
	return writen(client_fd, buf, nbytes + 9);
}
