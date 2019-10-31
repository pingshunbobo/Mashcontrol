#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/select.h>
#include	<termios.h>
#include 	"mash.h"
#include	"message.h"

extern enum CLIENT_STATUS client_stat;
extern int	client_fd;
extern int	fdm;
extern pid_t	work_pid;
extern fd_set	rset, wset;
extern int message_seq;

void mash_info()
{
	MASH_MESSAGE *message;
	char buf[1024];
	/* add Client hello info */
	memset(buf, '\0', 1024);
	memcpy(buf, "Mashinfo: ", 10);

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	if(getsockname(client_fd, (struct sockaddr*)&addr, &addrlen) == -1){
		fprintf(stderr, "Get client information failed,=%d.\n", client_fd);
	}
	if((inet_ntop(addr.sin_family, &addr.sin_addr, buf+10, addrlen)) == NULL){
		fprintf(stderr, "Get client information failed, fd=%d .\n", client_fd);        
	}
	message = make_message(MASH_INFO, buf + 10, 15);
	mash_send_message(message);
	free(message);
}

int mash_proc_cntl(MASH_MESSAGE *message)
{
	char *request = message->content;
	int nbytes = message->len;
	if( !strncmp(request, "work!", 5) ){
		if( client_stat == STANDBY && work_pid == 0){
			create_work(&work_pid, &fdm);
			FD_SET(fdm, &rset);
		}
		client_stat = WORK;
		mash_send_cntl("work!", 5);
	}else if( !strncmp(request, "standby!", 8) ){
		client_stat = STANDBY;
		mash_send_cntl("standby!", 8);
	}else{
		return -1;
	}
	return 0;
}

int mash_proc_data(MASH_MESSAGE *message)
{
	char *request = message->content;
	int nbytes = message->len;
	if ( client_stat == WORK ){
		/* write content which from server to the pty bash */
		if (writen(fdm, request, nbytes) != nbytes)
			printf("proc_data: writen error to master pty!\n");
		FD_SET(fdm, &rset);
	}else {
		mash_send_cntl("standby!", 8);
		/* do nothing !*/
	}
	return 0;
}

int mash_proc_heart(MASH_MESSAGE *message)
{
	/* update last check time */
	return 0;
}

void mash_proc(char *request, int *checked_idx, int *read_idx)
{
	static MASH_MESSAGE message;

	while(CHECK_OK == get_message(&message, request, checked_idx, read_idx)){
		log_message("inmessage", &message);
		if(MASH_CNTL == message.type ){
			mash_proc_cntl(&message);
		}else if(MASH_DATA == message.type){
			mash_proc_data(&message);
		}else if(MASH_HEART == message.type){
			mash_proc_heart(&message);
		}else{
			/* Server message error! */
		}
		memset(&message, '\0', sizeof(MASH_MESSAGE));
	}
        if(*checked_idx >= *read_idx){
                *checked_idx = 0;
                *read_idx = 0;
                memset(request, '\0', BUF_SIZE);
        }else if( CHECK_HEADER == message.status || CHECK_BODY == message.status ){
                memcpy(request, request + *checked_idx, *read_idx - *checked_idx);
                *read_idx -= *checked_idx;
                *checked_idx = 0;
        }else if( CHECK_ERROR == message.status ){
                *read_idx = 0;
                *checked_idx = 0;
                memset(&message, '\0', sizeof(MASH_MESSAGE));
                memset(request, '\0', BUF_SIZE);
        }
}

int mash_send_message(MASH_MESSAGE *message)
{
	message_seq++;
	log_message("outmessage", message);
	char *buf = malloc(message->len + 4);
	memcpy(buf, "M", 1);
	memcpy(buf+1, &message->type, 1);
	memcpy(buf+2, &message->len, 2);
	memcpy(buf+4, message->content, message->len);

rewrite:
	if( writen(client_fd, buf, message->len + 4) == -1 ){
		if(errno == EAGAIN)
			goto rewrite;
	}
	free(buf);
	return 0;
}

int mash_send_cntl(char *buf, int len)
{
	MASH_MESSAGE *message = make_message(MASH_CNTL, buf, len);
	int nbytes = mash_send_message(message);
	free(message);
	return nbytes;
}

int mash_send_data(char *buf, int len)
{
	MASH_MESSAGE *message = make_message(MASH_DATA, buf, len);
	int nbytes = mash_send_message(message);
	free(message);
	return nbytes;
}

int mash_send_heart(char *buf, int len)
{
	MASH_MESSAGE *message = make_message(MASH_HEART, buf, len);
	int nbytes = mash_send_message(message);
	free(message);
	return nbytes;
}
