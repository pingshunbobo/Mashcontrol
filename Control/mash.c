#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>
#include	<termios.h>
#include	<string.h>
#include 	"mash.h"
#include	"message.h"

extern CONTROL_STATUS control_stat;
extern struct termios saved_stermios;
extern int connfd;
extern fd_set  rset;
extern int in_message_seq;
extern int out_message_seq;

void set_nobrk(int fd)
{
        struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
                log_control("tcgetattr error");
        stermios.c_iflag |= (IGNPAR | ICRNL);
        stermios.c_iflag &= ~( IGNCR | IXON );
        stermios.c_lflag &= ~( ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK );

        if (tcsetattr(fd, TCSANOW, &stermios) < 0)
                log_control("tcsetattr error");
}

void save_termios(int fd)
{
        struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
		log_control("tcgetattr error");
        memcpy(&saved_stermios, &stermios, sizeof(struct termios));
}

void restore_termios(int fd)
{
        if (tcsetattr(fd, TCSANOW, &saved_stermios) < 0)
		log_control("tcsetattr error %s\n", strerror(errno));
}

int mash_proc_cmd(MASH_MESSAGE *message)
{
	char *reply = message->content;
	int nbytes = message->len;
	if (writen(STDOUT_FILENO, reply, nbytes) != nbytes)
		log_control("writen stdout error.\n");
	return 0;
}

int mash_proc_cntl(MASH_MESSAGE *message)
{
	int nbytes = message->len;
	char *cntl = message->content;
	if(!strncmp(cntl, "interface!", 10)){
		control_stat = INTERFACE;
		set_nobrk(STDIN_FILENO);
	}else if(!strncmp(cntl, "mashcmd!", 8)){
		control_stat = MASHCMD;
		restore_termios(STDIN_FILENO);
	}else{
		/*unknow mashctl info */
	}

	return 0;
}

int mash_proc_data(MASH_MESSAGE *message)
{
	char *data = message->content;
	int nbytes = message->len;
	if (writen(STDOUT_FILENO, data, nbytes) != nbytes)
		log_control("writen stdout error.\n");
	return 0;
}

int mash_proc_heart(MASH_MESSAGE *message)
{
	char *heart = message->content;
	int nbytes = message->len;
	/* Do nothing. */
	return 0;
}

int mash_proc(char *reply, int *checked_idx, int *read_idx)
{
	static MASH_MESSAGE message;
	while(CHECK_OK == get_message(&message, reply, checked_idx, read_idx)){
		in_message_seq ++;
		log_message(connfd, in_message_seq, "inmessage", &message);
		if(MASH_CMD == message.type ){
			mash_proc_cmd(&message);
		}else if(MASH_CNTL == message.type ){
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
		memset(reply, '\0', BUF_SIZE);
	}else if( CHECK_HEADER == message.status || CHECK_BODY == message.status ){
		memcpy(reply, reply + *checked_idx, *read_idx - *checked_idx);
		*read_idx -= *checked_idx;
		*checked_idx = 0;
	}else if( CHECK_ERROR == message.status ){
		*read_idx = 0;
		*checked_idx = 0;
		memset(&message, '\0', sizeof(MASH_MESSAGE));
		memset(reply, '\0', BUF_SIZE);
	}

	return 0;
}

int mash_send_message(MESSAGE_TYPE type, char *content, int len)
{
	out_message_seq++;
	MASH_MESSAGE *message = make_message(type, content, len); 
	log_message(connfd, out_message_seq, "outmessage", message);

	int buf_size = message->len + 4;
	char *buf = malloc(buf_size);

	memcpy(buf, "M", 1);
	memcpy(buf+1, &message->type, 1);
	memcpy(buf+2, &message->len, 2);
	memcpy(buf+4, message->content, message->len);

	//log_write(buf, len);
	if (writen(connfd, buf, buf_size) < 0)
		return -1;
	free(buf);
	free_message(message);
	return 0;
}

int mash_send_cmd(char *buf, int len)
{
	return mash_send_message(MASH_CMD, buf, 15);
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
