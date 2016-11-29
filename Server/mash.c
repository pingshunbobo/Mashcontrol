#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

#define MAXN   65535
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000

struct mashdata
{
        int selected;
        int connfd;
	int role;
	struct sockaddr_in client;
	int result;
        int nrequest, nreply;
        char request[MAXN];
        char reply[MAXN];
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

int is_mashcon(char * reply)
{
	if(!strncmp(reply, "Mashcmd:", 8))
		return 1;
	return 0;
}

int mash_login(struct mashdata *data)
{
	char login_ip[] = "127.0.0.1";
	if(data->role == 9)	/* Already login */
		return 1;
	if(!strncmp(inet_ntoa(data->client.sin_addr), login_ip, 9)){
		/* Is admin */
		data->role = 9;
		return 1;
	}
	return 0;
}

int mash_cmd(struct mashdata *data, int sockfd)
{
	int j;
	int cmd_size = data[sockfd].nreply - 8;
	char * cmd = data[sockfd].reply + 8;
	write(STDOUT_FILENO, data[sockfd].reply, cmd_size);

	/* send cmd to cliend*/
	for(j = 0; j < 1024; ++j){
		if(data[j].selected){
			data[j].nrequest = cmd_size;
			memcpy(data[j].request, cmd, cmd_size);
		}
	}
	return cmd_size;

}

int mash_console(struct mashdata *data, int sockfd)
{
	/* login ok! */
	if(mash_login(data + sockfd)){
		mash_cmd(data, sockfd);
	}else
		mash_close(data, sockfd);
	return 0;
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

int mash_process(struct mashdata *data, int sockfd)
{
	int nread = data[sockfd].nreply;
	/* Check Magic number from data.reply */
	if(is_mashcon(data[sockfd].reply)){
		mash_console(data, sockfd);
	}else{
		Write(STDOUT_FILENO, data[sockfd].reply, nread);
        	fflush(stdout);
        	memset(data[sockfd].reply, '\0', MAXN);
	}
	return 0;
}

int mash_read(struct mashdata *data, int sockfd)
{
	int nread;
	//if ( (nread = Readline(sockfd, data[sockfd].reply, MAXN)) == 0){
	if ( (nread = read(sockfd, data[sockfd].reply, MAXN)) == 0){
		printf("connectionclosed by other end");
		return 0;
	}
	//debug_printf("2, Return %d bytes.\n", nread);
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

