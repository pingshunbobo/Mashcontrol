#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>
#include 	<string.h>
#include 	"mash.h"
#include	"message.h"

extern int epollfd;
extern MASHDATA *coredata;
extern int message_seq;

int setnonblocking( int fd )
{
	int old_option = fcntl( fd, F_GETFL );
	int new_option = old_option | O_NONBLOCK;
	Fcntl( fd, F_SETFL, new_option );
	return old_option;
}

void addevent( int epollfd, int fd, bool one_shot )
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if( one_shot ){
		event.events |= EPOLLONESHOT;
	}
	Epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
	setnonblocking( fd );
	return;
}

void modevent( int epollfd, int fd, int ev )
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLRDHUP;
	Epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
	return;
}

void delevent(int epollfd, int fd)
{
	Epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
}

void mash_display(struct mashdata *the_data)
{
	int i = 0;
	int nbytes = 0;
	char buf[BUF_SIZE];
	for(i = 0; i < 100; ++i){
		if(coredata[i].role == 1){
			if( the_data == coredata[i].selected )
				nbytes += snprintf(buf + nbytes, 3, "++");
			else if( NULL == coredata[i].selected )
				nbytes += snprintf(buf + nbytes, 3, "--");
			else
				nbytes += snprintf(buf + nbytes, 3, "+-");
			nbytes += snprintf(buf + nbytes, 1024,\
				"id: %d, role: client, addess: %s @%s\n", i, coredata[i].client_pri, coredata[i].client_pub);
		}//end if role 
	}
	mash_send_cmd(the_data, buf, nbytes);
}

void mash_show(struct mashdata *the_data)
{
	int i = 0;
	for(i = 0; i < 10; ++i){
		if( the_data[i].selected && the_data[i].role == 1){
			log_serv("%s", the_data->readbuf);
		}
	}
	return;
}

int mash_select(struct mashdata *the_data, int slave_id)
{
	int nbytes = 0;
	char buf[BUF_SIZE];
	MASHDATA *slave_data = coredata + slave_id;
	if( slave_data->selected != NULL && slave_data->selected != the_data){
		nbytes += snprintf(buf, BUF_SIZE, "Has selected by another admin!\n");
	}else if(slave_data->role == 1 && the_data->role == 9){
		slave_data->selected = the_data;
		the_data->selected = slave_data;
		nbytes += snprintf(buf, BUF_SIZE, "Select slave: %d ok!\n", slave_id);
	}
	mash_send_cmd(the_data, buf, nbytes);
	return nbytes;
}

int mash_unselect(struct mashdata *the_data, int slave_id)
{
	MASHDATA *slave_data = coredata + slave_id;
	if(the_data->role == 9 && the_data->selected == slave_data){
		slave_data->selected = NULL;
		the_data->selected = NULL;
		return 1;
	}
	return 0;
}

int selected_num(struct mashdata *the_data)
{
	MASHDATA *one_data = the_data->selected;
	int count = 0;
	while(one_data){
		++count;
		one_data = one_data->next;
	}
	return count;
}

void mash_help(struct mashdata *the_data)
{
	int nbytes = 0;
	char buf[BUF_SIZE];
	nbytes = snprintf(buf, BUF_SIZE, "\n \
	help info: \n \
	mashcmd  	Basic command mode.\n \
	mashcli  	Into control mode, sent cmdline to all the selected slaves.\n \
	interface  	Into interactive mode with the selected slave.\n \
	select  	Choose one slave.\n \
	unselect  	Unselect one slave.\n \
	display  	Display slave list.\n \
	show  		Show result of  the client run.\n \
	help  		Show this page.\n"
	);
	mash_send_cmd(the_data, buf, nbytes); 
}

int mash_auth(struct mashdata *the_data)
{
	char login_ip[] = "127.0.0.1";
	if(the_data->role == 1)	/* Already login */
		return 1;
	if(the_data->role == 9)	/* Already login */
		return 9;
	/* Is admin */
	if(!strncmp( the_data->client_pub, login_ip, 9)){
		the_data->role = 9;
		the_data->status = CMD;
		return 9;
	}else{
		the_data->role = 1;
                the_data->status = STANDBY;
                return 1;
	}
	return 0;
}

int mash_init(MASHDATA *the_data, int sockfd, struct sockaddr_in client_addr)
{
	int auth_ret = 0;

	if( sockfd < 0 )
		return -1;

	memset(the_data, '\0', sizeof(struct mashdata));
	setnonblocking(sockfd);
	the_data->connfd = sockfd;
	the_data->selected = NULL;
	the_data->next = NULL;
	the_data->prev = NULL;
	the_data->nwritebuf = 0;
	the_data->nreadbuf = 0;
	the_data->checked_idx = 0;
	the_data->in_message_seq = 0;
	the_data->out_message_seq = 0;
	memset(&the_data->in_message, '\0', sizeof(MASH_MESSAGE));
	the_data->out_message = NULL;

	memset(the_data->writebuf, '\0', BUF_SIZE);
	memset(the_data->readbuf, '\0', BUF_SIZE);
	memcpy(the_data->client_pub, inet_ntoa(client_addr.sin_addr), 16);

	/*auth in init func*/
	auth_ret = mash_auth(the_data);
	if( 1 == auth_ret ){
		the_data->role = 1;
                the_data->status = STANDBY;
		return 1;
	}else if( 9 == auth_ret ){
		the_data->role = 9;
                the_data->status = CMD;
		return 9;
	}else{
		mash_close(the_data);
                return -1;
	}
	return 0;
}

int mash_proc_data(struct mashdata *the_data)
{
	int id = 0;
	int selected_count = 0;
	MASHDATA *one_data = NULL;
	MASHDATA *admin_data = NULL;
	int nbytes = 0;
	int data_size = the_data -> in_message.len;
	char *data_buf = the_data -> in_message.content;

	/* If data from slaves */
	if( 1 == the_data->role ){
		if( NULL == the_data->selected )
			return 1;
		else
			admin_data = the_data->selected;
		/* return readbuf to admin console. */
		if( 9 == admin_data->role && admin_data->status == INTERFACE ){
			mash_send_data(admin_data, data_buf, data_size);
		}
		return 1;
	}

	/* If data from controls */
	if( 9 == the_data->role ){
		/* On CLI status sent command to client. */
		if(CLI == the_data->status | \
				INTERFACE == the_data->status){
			one_data = the_data->selected;
			while(one_data){
				if( one_data->selected == the_data \
					& WORK == one_data->status ){
					selected_count ++;
					mash_send_data(one_data, data_buf, data_size);
				}
				one_data = one_data -> next;
			}
			/* Slave was disconnected, rest control to mashcmd  */
			if( 0 == selected_count ){
				the_data->status = CMD;
				mash_send_cntl(the_data, "mashcmd!", 8);
			}
		}else if( CMD == the_data->status ){
			/* Control is already in CMD status. */
			mash_send_cntl(the_data, "mashcmd!", 8);
		}
	}
	return 3;
}

int mash_proc_cntl(struct mashdata *the_data)
{
	int id = 0;
	int nbytes = 0;
	MASHDATA *admin_data = NULL;
	char *cntl = the_data->in_message.content;
	/* If cntl from slaves */
	if( 1 == the_data->role ){
		if(!strncmp(cntl, "work!", 5)){
			the_data->status = WORK;
			admin_data = the_data->selected;
			if( admin_data ){
				admin_data->status = INTERFACE;
				mash_send_cntl(admin_data, "interface!", 10);
			}
		}
		if(!strncmp(cntl, "standby!", 8)){
			the_data->status = STANDBY;
			admin_data = the_data->selected;
			if( admin_data ){
				admin_data->status = CMD;
				mash_send_cntl(admin_data, "mashcmd!", 8);
				mash_send_cmd(admin_data, "[mashcmd]", 9);
			}
		}
		return 1;
	}
	return 0;
}

int mash_control_cntl(struct mashdata *the_data)
{	
	int id = 0;
	int nbytes = 0;
	int admin_id = 0;
	struct mashdata *one_data = NULL;
	char *cntl = the_data->in_message.content;
	int sockfd = the_data->connfd;

	/* For role is not 9. */
	if(9 != the_data->role){
		return 0;
	}
	if( !strncmp(cntl, "interface", 9) ){
		one_data = the_data -> selected;
		while(one_data){
			mash_send_cntl(one_data, "work!", 5);
			one_data = one_data -> next;
		}
		return 9;
	}else if( !strncmp(cntl, "mashcmd", 7) ){
		the_data->status = CMD;
		mash_send_cntl(the_data, "mashcmd!", 8);
	}

	return 0;
}

int mash_proc_cmd(struct mashdata *the_data)
{
	int id = 0;
	int admin_id = 0;
	int nbytes = 0;
	char buf[BUF_SIZE];
	int sockfd = the_data -> connfd;
	char *cmd = the_data->in_message.content;

	/* check mashcntl command */
	if(!strncmp(cmd, "mashcmd", 7)){
		mash_control_cntl(the_data);
	}else if(!strncmp(cmd, "mashcli", 7)){
		if( selected_num(the_data) < 1){
			nbytes = snprintf(buf, 1024, \
				"Error: Please choose only one interface.\r\n");
			mash_send_cmd(the_data, buf, nbytes);
		}else
			the_data->status = CLI;
	}else if(!strncmp(cmd, "interface", 9)){
		if( selected_num(the_data) != 1){
			nbytes = snprintf(buf, 1024, \
				"Error: Please choose only one interface.\r\n");
			mash_send_cmd(the_data, buf, nbytes);
		}else{
			mash_control_cntl(the_data);
		}
	}else if(!strncmp(cmd, "display", 7)){
		mash_display(the_data);
	}else if(!strncmp(cmd, "show", 4)){
		mash_show(the_data);
	}else if(!strncmp(cmd, "select", 6)){
		mash_select(the_data, atoi(cmd + 6));
	}else if(!strncmp(cmd, "unselect", 8)){
		mash_unselect(the_data, atoi(cmd + 8));
	}else if(!strncmp(cmd, "help", 4)){
		mash_help(the_data);
	}else if(!strncmp(cmd, "\4", 1)){
		mash_close(the_data);
	}else if(!strncmp(cmd, "\n", 1)){
		//mash_send_cmd(the_data, "\n", 1);
	}else{
		/* unknow cntl info sent to controls */
		nbytes = snprintf(buf, 1024,\
			"Error: unknow mash cmd!.\r\n");
		mash_send_cmd(the_data, buf, nbytes);
	}
	mash_send_cmd(the_data, "[mashcmd]", 9);
	return 9;
}

int mash_proc_info(struct mashdata *the_data)
{
	int len = the_data->in_message.len;
	char *info = the_data->in_message.content;
        memcpy(the_data->client_pri, info, len);
	return len;
}

int mash_proc_heart(struct mashdata *the_data)
{
	int nbytes = the_data->nwritebuf;
	char buf[BUF_SIZE];
	char *heart = the_data->in_message.content;
	if(!strncmp(heart, "ping!", 5)){
		mash_send_heart(the_data, "pong!", 5);
	}else if(!strncmp(heart, "pong!", 15)){
		/* update last session time */
	}
	return nbytes;
}

int mash_proc(struct mashdata *the_data)
{
	while(CHECK_OK == mash_get_message(the_data)){
		the_data->in_message_seq++;
		if(!strncmp(the_data->in_message.content, "y ACPI", 6))
		{
			int flag = 5;
			log_messages(the_data->connfd, the_data->in_message_seq, \
				"i_message: ", &the_data->in_message);
		}

		log_messages(the_data->connfd, the_data->in_message_seq, \
			"i_message: ", &the_data->in_message);

		/* Check message type */
		switch( the_data->in_message.type ){
			case (MASH_CMD):
				mash_proc_cmd(the_data);
				break;
			case (MASH_CNTL):
				mash_proc_cntl(the_data);
				break;
			case (MASH_INFO):
				mash_proc_info(the_data);
				break;
			case (MASH_DATA):
				mash_proc_data(the_data);
				break;
			case (MASH_HEART):
				mash_proc_heart(the_data);
				break;
			default:
				log_serv("unknow client");
				mash_close(the_data);
		}
		if(CHECK_OK == the_data->in_message.status)
			memset(&the_data->in_message, '\0', sizeof(MASH_MESSAGE));
	}
	MASH_MESSAGE *message = &the_data->in_message;
	if( the_data->checked_idx > the_data->nreadbuf ){
		log_serv("checked error!\n");
		//exit(1);
	}
	if( the_data->checked_idx == the_data->nreadbuf ){
		the_data->nreadbuf = 0;
		the_data->checked_idx = 0;
        	memset(the_data->readbuf, '\0', BUF_SIZE);
	}else if( CHECK_HEADER == message->status || CHECK_BODY == message->status){
		/* copy readbuf to memory head */
		memcpy(the_data->readbuf, the_data->readbuf + the_data->checked_idx, \
				the_data->nreadbuf - the_data->checked_idx);
		the_data->nreadbuf -= the_data->checked_idx;
		the_data->checked_idx = 0;
        	memset(the_data->readbuf + the_data->nreadbuf, '\0', \
				BUF_SIZE - the_data->nreadbuf);
	}else if( CHECK_ERROR == message->status ){
		log_serv("message error: ");
		log_messages(the_data->connfd, the_data->in_message_seq, \
				"i_message: ", &the_data->in_message);
		memset(message, '\0', sizeof(MASH_MESSAGE));
		the_data->nreadbuf = 0;
		the_data->checked_idx = 0;
        	memset(the_data->readbuf, '\0', BUF_SIZE);
	}
	return 0;
}

MESSAGE_STATUS mash_get_message(MASHDATA *mashdata)
{
	return get_message(&mashdata->in_message, mashdata->readbuf, &mashdata->checked_idx, mashdata->nreadbuf); 
}


int mash_send_message(MASHDATA *the_data, MESSAGE_TYPE type, char *buf, int len)
{
	int connfd = the_data->connfd;
	int write_idx = the_data->nwritebuf;

	MASH_MESSAGE *message = make_message(type, buf, len);
	
	if( NULL == the_data->out_message )
		the_data->out_message = message;
	else{
		MASH_MESSAGE *tail_message = the_data->out_message;
		while(NULL != tail_message->next){
			tail_message = tail_message->next;
		}
		tail_message->next = message;
		the_data->out_message_seq ++;
	}
	modevent(epollfd, connfd, EPOLLOUT);
	return the_data->out_message_seq;
}

int mash_send_cmd(MASHDATA *mashdata, char *buf, int len)
{
	return mash_send_message(mashdata, MASH_CMD, buf, len);
}

int mash_send_cntl(MASHDATA *mashdata, char *buf, int len)
{
	return mash_send_message(mashdata, MASH_CNTL, buf, len);
}

int mash_send_data(MASHDATA *mashdata, char *buf, int len)
{
	return mash_send_message(mashdata, MASH_DATA, buf, len);
}

int mash_send_heart(MASHDATA *mashdata, char *buf, int len)
{
	return mash_send_message(mashdata, MASH_HEART, buf, len);
}

int mash_read(struct mashdata *the_data)
{
	int nread = 0;
	int sockfd = the_data -> connfd;
	int read_idx = the_data -> nreadbuf;
	MASH_MESSAGE *message = &the_data -> in_message;

reread:
	if ( (nread = read(sockfd, the_data->readbuf + read_idx, BUF_SIZE - read_idx)) < 0 ){
		if(errno == EINTR)
			goto reread;
		else if(errno == EAGAIN)
			return 0;
		else
			return -1;
	}else if(nread == 0){
		log_serv("Connection closed by other end");
		return -1;
	}
	log_read(the_data->readbuf + read_idx, nread);

	the_data->nreadbuf += nread;
	return nread;
}

int mash_write(struct mashdata *the_data)
{
	int nbytes = 0;
	int connfd = the_data->connfd;
	MASH_MESSAGE * need_free_message = NULL;

	/* Write the left message on the writebuf */
	if( the_data->nwritebuf > 0 ){
		writen(connfd, the_data->writebuf, the_data->nwritebuf);
		memset(the_data->writebuf, '\0', BUF_SIZE);
		the_data->nwritebuf = 0;
	}

	/* Write message data */
	MASH_MESSAGE *tail_message = the_data->out_message;
	while(NULL != tail_message){
		log_messages(the_data->connfd, the_data->out_message_seq, \
			"o_message", tail_message);

		char *buf = malloc(tail_message->len + 4);
		int buf_size = tail_message->len + 4;
		memcpy(buf, "M", 1);
		memcpy(buf+1, &tail_message->type, 1);
		memcpy(buf+2, &tail_message->len, 2);
		memcpy(buf+4, tail_message->content, tail_message->len);

		nbytes = writen(connfd, buf, buf_size);
		if( 0 > nbytes )
			return -1;
		else if(nbytes <  buf_size ){
			int nleft = buf_size - nbytes;
			if( the_data->nwritebuf + nleft < BUF_SIZE ){
				memcpy(the_data->writebuf + the_data->nwritebuf, \
					buf + nbytes, nleft);
				the_data->nwritebuf += nleft;
				modevent(epollfd, connfd, EPOLLOUT);
				break;
			}else{
				//socket write busy
				log_serv("server busy buf error!");
				return -1;
			}
			break;
		}
		tail_message = tail_message->next;
		free(the_data->out_message);
		the_data->out_message = tail_message;
	}

	return nbytes;
}

int mash_close(struct mashdata *the_data)
{
	int sockfd = the_data -> connfd;
	MASHDATA * one_data = NULL;
	MASHDATA * admin_data = NULL;
	int admin_id = 0;
	if(the_data->role == 1){
		if( the_data->selected ){
			admin_data = the_data->selected;
			admin_id = admin_data->connfd;
			mash_send_cmd(admin_data, "\nclient closed.\n", 16);
				if(INTERFACE == admin_data->status){
					admin_data->status = CMD;
					mash_send_cntl(admin_data, "mashcmd!", 8);
				}
			mash_send_cmd(admin_data, "[mashcmd]", 9);
		}
	}else if( the_data->role == 9 ){
		one_data = the_data -> selected;
		while( one_data ){
			if(one_data->role == 1 && one_data->selected == the_data){
				one_data->selected = NULL;
				one_data->status = STANDBY;
                	}
			one_data = one_data -> next;
		}
	}
	the_data->role = 0;
	the_data->connfd = -1;
	the_data->selected = 0;
	delevent(epollfd, sockfd);
	close(sockfd);
	return 0;
}

