#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

#define REPLY_SIZE   4096
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000

enum MASH_DATA_TYPE {MASH_CMD, MASH_DATA, MASH_UNKNOW};
enum MASH_STATUS {CMD, CLI, INTERFACE};

struct mashdata
{
        int selected;
        int connfd;
	int role;
	enum MASH_STATUS status;
	struct sockaddr_in client;
	int result;
        int nrequest, nreply;
        char request[REPLY_SIZE];
        char reply[REPLY_SIZE];
	void * data;
};

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
        //event.events |= EPOLLONESHOT;
    }
    Epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
    return;
}

void modevent( int epollfd, int fd, int ev )
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    Epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
    return;
}

void delevent(int epollfd, int fd)
{
    Epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
}

enum MASH_DATA_TYPE mash_type(char *reply)
{
	if(!strncmp(reply, "Mashcmd:", 8))
                return MASH_CMD;
	return MASH_DATA;
}

void mash_display(struct mashdata *data, int sockfd)
{
	int i;
	int nbytes = 0;
	for(i = 0; i < 10; ++i){
		if(data[i].role == 1){
			if( data[i].selected == sockfd)
				nbytes += snprintf(data[sockfd].reply + nbytes, 3, "++");
			else if( data[i].selected == 0)
				nbytes += snprintf(data[sockfd].reply + nbytes, 3, "--");
			else
				nbytes += snprintf(data[sockfd].reply + nbytes, 3, "+-");
			nbytes += snprintf(data[sockfd].reply + nbytes, 1024,\
				"%d role: %d add: %s\n", i, data[i].role, inet_ntoa(data[i].client.sin_addr)
			);
		}//end if role 
	}
	Write(sockfd, data[sockfd].reply, nbytes);
}

void mash_show(struct mashdata *data)
{
	int i;
	for(i = 0; i < 10; ++i){
		if( data[i].selected){
			printf("%s",data[i].reply);
		}
	}
	return;
}

int mash_select(struct mashdata *data, int id, int sockfd)
{
	int nbytes = 0;
	int admin_id = sockfd;
	if( data[id].selected > 0 && data[id].selected != admin_id){
		nbytes += snprintf(data[admin_id].reply + nbytes, 1024, "Has selected by another admin!\n");
	}else if(data[id].role == 1 && data[admin_id].role == 9){
		data[id].selected = sockfd;
		nbytes += snprintf(data[admin_id].reply + nbytes, 1024, "Select slave: %d ok!\n", id);
	}
	Write(sockfd, data[admin_id].reply, nbytes);
	return 0;
}

int mash_unselect(struct mashdata *data, int n, int admin_id)
{
	if(data[n].role > 0 && data[n].selected == admin_id)
		data[n].selected = 0;
}

int selected_num(struct mashdata *data, int admin_id)
{
	int j;
	int num = 0;
	for(j = 0; j < 10; ++j){
		if( data[j].selected == admin_id){
			++num;
		}
	}
	return num;
}

void mash_help(struct mashdata *data, int sockfd)
{
	int nbytes;
	nbytes = snprintf(data[sockfd].reply, 1024, "\n \
	help info: \n \
	mashcmd  	basic command mode.\n \
	mashcli  	command mode, sent cmd to all the selected slave.\n \
	interface  	inter interactive mode with the selected slave.\n \
	select  	choose one slave.\n \
	unselect  	unselect one slave.\n \
	display  	show client list.\n \
	show  		show all the client run result.\n \
	help  		show this page.\n\n"
	); 

	Write(sockfd, data[sockfd].reply, nbytes);
}

int mash_login(struct mashdata *data)
{
	char login_ip[] = "127.0.0.1";
	if(data->role == 9)	/* Already login */
		return 1;
	if(!strncmp(inet_ntoa(data->client.sin_addr), login_ip, 9)){
		/* Is admin */
		data->role = 9;
		data->status = CMD;
		data->selected = 0;
		return 1;
	}
	return 0;
}

int mash_cmd(struct mashdata *data, int sockfd, int epollfd)
{
	int j;
	int nbytes;
	int cmd_size = data[sockfd].nreply - 8;
	char * cmd = data[sockfd].reply + 8;

	/* check mashcmd command */
	if(!strncmp(cmd, "mashcmd", 7)){
		data[sockfd].status = CMD;
		return 1;
	}
	if(!strncmp(cmd, "mashcli", 7)){
		if( selected_num( data, sockfd) < 1){
			nbytes = snprintf(data[sockfd].reply, 1024,\
				"Error: select at least one interface.\r\n");
			Write(sockfd, data[sockfd].reply, nbytes);
		}else
			data[sockfd].status = CLI;
		return 1;
	}
	if(!strncmp(cmd, "interface", 9)){
		if( selected_num( data , sockfd) != 1){
			nbytes = snprintf(data[sockfd].reply, 1024,\
				"Error: Please choose only one interface.\r\n");
			Write(sockfd, data[sockfd].reply, nbytes);
		}else{
			data[sockfd].status = INTERFACE;
			write(sockfd, "interface#", 10);
		}
		return 1;
	}
	if(!strncmp(cmd, "display", 7)){
		mash_display(data, sockfd);
		return 1;
	}
	if(!strncmp(cmd, "show", 4)){
		mash_show(data);
		write(sockfd, "show", 4);
		return 1;
	}
	if(!strncmp(cmd, "select", 6)){
		mash_select(data, atoi(cmd + 6), sockfd);

		//write(sockfd, "select", 6);
		return 1;
	}
	if(!strncmp(cmd, "unselect", 8)){
		mash_unselect(data, atoi(cmd + 8), sockfd);
		//write(sockfd, "unselect", 8);
		return 1;
	}
	if(!strncmp(cmd, "help", 4)){
		mash_help(data, sockfd);
		return 1;
	}

	/* On CLI status sent command to client. */
	if(CLI == data[sockfd].status | \
		INTERFACE == data[sockfd].status){
		/* unknow cmd, send cmd to cliend*/
		for(j = 0; j < 100; ++j){
			if( data[j].selected == sockfd ){
				data[j].nrequest = cmd_size;
				memcpy(data[j].request, cmd, cmd_size);
				modevent(epollfd, j, EPOLLOUT);
			}
		}
		return cmd_size;
	}

	if(!strncmp(cmd, "\n", 1)){
		return 1;
	}else{
		/* unknow cmd info sent to controls */
		nbytes = snprintf(data[sockfd].reply, 1024,\
			"Error: unknow mash cmd!.\r\n");
		Write(sockfd, data[sockfd].reply, nbytes);
	}
	return 0;
}

int mash_console(struct mashdata *data, int sockfd, int epollfd)
{
	/* login ok! */
	if(mash_login(data + sockfd)){
		mash_cmd(data, sockfd, epollfd);
		if(CLI == data[sockfd].status)
			write(sockfd, "mashcli#", 8);
		else if(CMD == data[sockfd].status)
			write(sockfd, "mashcmd%", 8);
	}else
		mash_close(data, sockfd);
	return 0;
}

int mash_init(struct mashdata *data, int sockfd, struct sockaddr_in client_addr)
{
	setnonblocking(sockfd);
	data[sockfd].connfd = sockfd;
	data[sockfd].role = 1;
	data[sockfd].client = client_addr;
	data[sockfd].nrequest = 0;
	data[sockfd].nreply = 0;
	memset(data[sockfd].request, '\0', REPLY_SIZE);
	memset(data[sockfd].reply, '\0', REPLY_SIZE);
}

int mash_process(struct mashdata *data, int sockfd, int epollfd)
{
	int i;
	int nbytes;
	/* Check Magic number from data.reply */
	switch( mash_type(data[sockfd].reply) ){
		case (MASH_CMD):
			mash_console(data, sockfd, epollfd);
			break;
		case (MASH_DATA):
			nbytes = data[sockfd].nreply;
			/*
			 * return reply to admin console
			*/
			for(i = 0; i < 10; i++){
				if( 9 == data[i].role && data[i].status == INTERFACE ){
					Write(data[i].connfd, data[sockfd].reply, nbytes);
				}
			}
			//Write(STDOUT_FILENO, data[sockfd].reply, nbytes);
        		//fflush(stdout);

			break;
		default:
			printf("unknow client");
			mash_close(sockfd);
	}
	return 0;
}

int mash_read(struct mashdata *data, int sockfd)
{
	int nread;
        memset(data[sockfd].reply, '\0', REPLY_SIZE);
	if ( (nread = read(sockfd, data[sockfd].reply, REPLY_SIZE)) == 0){
		printf("connectionclosed by other end");
		return 0;
	}
	data[sockfd].nreply = nread;
	return nread;
}

int mash_write(struct mashdata *data, int sockfd)
{
	Write(sockfd, data[sockfd].request, data[sockfd].nrequest);
	return 0;
}

int mash_close(struct mashdata *data, int sockfd)
{
	int i = 0;
	int admin_id = 0;
	if(data[sockfd].role == 1){
		admin_id = data[sockfd].selected;
		data[admin_id].status = CMD;
		Write(admin_id, "mashcmd%", 8);
	}else if( data[sockfd].role == 9 ){
		admin_id = sockfd;
		for(i = 0; i < 10; ++i){
			if(data[i].role == 1 && data[i].selected == admin_id){
				data[i].selected = 0;
                	}
        	}
	}

	data[sockfd].selected = 0;
	close(sockfd);
	return 0;
}

