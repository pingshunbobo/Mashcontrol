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
	int reqult;
        int nrequest,nreply;
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


int mash_init(struct mashdata *data, int sockfd)
{
	setnonblocking(sockfd);
	data[sockfd].selected = 1;
	data[sockfd].connfd = sockfd;
	data[sockfd].nrequest = 0;
	data[sockfd].nreply = 0;
	memset(data[sockfd].request, '\0', MAXN);
	memset(data[sockfd].reply, '\0', MAXN);
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
	Write(STDOUT_FILENO, data[sockfd].reply, nread);
	fflush(stdout);
	memset(data[sockfd].reply, '\0', MAXN);
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

