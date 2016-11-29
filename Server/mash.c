#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

#define MAXN   65535
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000

enum MASH_TYPE {MASH_CMD, MASH_DATA, MASH_UNKNOW};
enum MASH_STATUS {CMD, INTERFACE, INTERFACEV};

struct mashdata
{
        int selected;
        int connfd;
	int role;
	enum MASH_STATUS status;
	struct sockaddr_in client;
	int result;
        int nrequest, nreply;
        char request[MAXN];
        char reply[MAXN];
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

enum MASH_TYPE mash_type(char *reply)
{
	if(!strncmp(reply, "Mashcmd:", 8))
                return MASH_CMD;
	return MASH_DATA;
}

int mash_selected(struct mashdata *data)
{
	int j;
	int num = 0;
	for(j = 0; j < 10; ++j){
		if( data[j].selected ){
			++num;
		}
	}
	return num;
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
	int cmd_size = data[sockfd].nreply - 8;
	char * cmd = data[sockfd].reply + 8;
	write(STDOUT_FILENO, data[sockfd].reply + 8, cmd_size);

	if( CMD == data[sockfd].status ){
		if(!strncmp(data[sockfd].request, "mashcli", 7)){
			// 提示只能选中一个。
			if( mash_selected(data) > 1){
				data[sockfd].nrequest = 7;
				memcpy(data[j].request, "Error", 6);
			}else{
				data[sockfd].status = INTERFACE;
				data[sockfd].nrequest = 7;
				memcpy(data[j].request, "mashcli", 7);
			}
			modevent(epollfd, sockfd, EPOLLOUT);
			return 1;	
		}
		if(!strncmp(data[sockfd].request, "show", 4)){
			//select
		}
		if(!strncmp(data[sockfd].request, "select", 6)){
			//select
		}
		if(!strncmp(data[sockfd].request, "unselect", 6)){
			//select
		}
	}else{
		/* send cmd to cliend*/
		for(j = 0; j < 10; ++j){
			if( data[j].selected ){
				data[j].nrequest = cmd_size;
				memcpy(data[j].request, cmd, cmd_size);
				modevent(epollfd, j, EPOLLOUT);
			}
		}
	}

	return cmd_size;

}

int mash_console(struct mashdata *data, int sockfd, int epollfd)
{
	/* login ok! */
	if(mash_login(data + sockfd)){
		mash_cmd(data, sockfd, epollfd);
	}else
		mash_close(data, sockfd);
	return 0;
}

void mash_display(struct mashdata *data, int selected)
{
	int i;
	for(i = 0; i < 10; ++i){
		if( data[i].selected ){
			printf("%d %s\n", i, inet_ntoa(data[i].client.sin_addr));
		}
	}

}

int mash_init(struct mashdata *data, int sockfd, struct sockaddr_in client_addr)
{
	setnonblocking(sockfd);
	data[sockfd].selected = 1;
	data[sockfd].connfd = sockfd;
	data[sockfd].role = 0;
	data[sockfd].client = client_addr;
	data[sockfd].nrequest = 0;
	data[sockfd].nreply = 0;
	memset(data[sockfd].request, '\0', MAXN);
	memset(data[sockfd].reply, '\0', MAXN);
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
			//return reply to admin console
			for(i = 0; i < 10; ++i){
				if( 9 == data[i].role ){
					Write(data[i].connfd, data[sockfd].reply, nbytes);
				}
			}
			Write(STDOUT_FILENO, data[sockfd].reply, nbytes);
        		fflush(stdout);

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
        memset(data[sockfd].reply, '\0', MAXN);
	//if ( (nread = Readline(sockfd, data[sockfd].reply, MAXN)) == 0){
	if ( (nread = read(sockfd, data[sockfd].reply, MAXN)) == 0){
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
	data[sockfd].selected = 0;
	close(sockfd);
}

