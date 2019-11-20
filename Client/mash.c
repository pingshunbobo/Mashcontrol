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
extern int in_message_seq;
extern int out_message_seq;

extern int	reply_size;
extern char	reply[BUF_SIZE];

void mash_upload_info()
{
	MASH_MESSAGE *message;
	char buf[1024];
	/* add Client hello info */
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	if(getsockname(client_fd, (struct sockaddr*)&addr, &addrlen) == -1){
		log_client("Get client information failed.\n");
	}
	if((inet_ntop(addr.sin_family, &addr.sin_addr, buf, addrlen)) == NULL){
		log_client("Get client information failed.\n");
	}
	mash_send_info(buf, 15);
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
			log_client("writen error to master pty!\n");
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

int mash_read(int fd, char *buf, int read_size)
{
	int nbytes = 0;
reread:
	nbytes = read(fd, buf, read_size);
	if ( nbytes == 0 ){
		return -1;
	}else if ( nbytes < 0 ){
		if (errno == EINTR)
			goto reread;
		if (errno == EAGAIN)
			nbytes = 0;
	}
	return nbytes;
}

void mash_proc(char *request, int *checked_idx, int *read_idx)
{
	static MASH_MESSAGE message;

	while(CHECK_OK == get_message(&message, request, checked_idx, read_idx)){
		in_message_seq ++;
		log_message(client_fd, in_message_seq, "inmessage", &message);
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

int mash_send_message(MESSAGE_TYPE type, char *content, int len)
{
	out_message_seq++;
	MASH_MESSAGE *message = make_message(type, content, len); 
	log_message(client_fd, out_message_seq, "outmessage", message);

	int buf_size = message->len + 4;
	char *buf = malloc(buf_size);

	memcpy(buf, "M", 1);
	memcpy(buf+1, &message->type, 1);
	memcpy(buf+2, &message->len, 2);
	memcpy(buf+4, message->content, message->len);

	//log_write(buf, len);
	if (writen(client_fd, buf, buf_size) < 0)
		return -1;
	free(buf);
	free_message(message);
	return 0;
}

int mash_send_info(char *buf, int len)
{
	return mash_send_message(MASH_INFO, buf, 15);
}
int mash_send_cntl(char *buf, int len)
{
	return mash_send_message(MASH_CNTL, buf, len);
}

int mash_send_data(char *buf, int len)
{
	return mash_send_message(MASH_DATA, buf, len);
}

int mash_send_heart(char *buf, int len)
{
	return mash_send_message(MASH_HEART, buf, len);
}
