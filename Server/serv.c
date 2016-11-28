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
	int nrequest,nreply;
	char request[MAXN];
	char reply[MAXN];
};


void delevent(int epollfd, int fd);
void modevent(int epollfd, int fd, int event);
void addevent(int epollfd, int fd, bool one_shot);

int main(int argc, char **argv)
{
	int	i,j;
	int	nread;
	int 	number;
	int 	epollfd;
	int	listenfd;
	socklen_t addrlen;

	char request[MAXN];
	struct epoll_event events[MAX_EVENT_NUMBER];

	struct mashdata * coredata = malloc(1024 * sizeof(struct mashdata));
/*	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv [ <host> ] <port#> ");
*/
	listenfd = Tcp_listen(NULL, "8080", &addrlen);
	epollfd = Epoll_create( 5 );
	addevent(epollfd, listenfd, false);
	addevent(epollfd, STDIN_FILENO, false);

	//loop process io events.
	for ( ; ; ){
		if((number = Epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 )) <= -1)
			continue;
		for ( i = 0; i < number; i++ ){
			int sockfd = events[i].data.fd;
			if( sockfd == listenfd ){
			/*Here is a connect request*/
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof( client_address );
				int connfd = Accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
				if ( connfd < 0 )
					continue;

				/*connect success ,now go to initial the mash data struct*/
				addevent(epollfd, connfd, false);
				setnonblocking(connfd);
				coredata[connfd].selected = 1;
				coredata[connfd].connfd = connfd;
				coredata[connfd].nrequest = 0;
				coredata[connfd].nreply = 0;
				memset(coredata[connfd].request, '\0', MAXN);
				memset(coredata[connfd].reply, '\0', MAXN);
			}else if( sockfd == STDIN_FILENO ){
				nread = read(sockfd, request, MAXN);
				for(j=0; j < 1024; ++j){
					if(coredata[j].selected){
						coredata[j].nrequest = nread;
						memcpy(coredata[j].request, request, nread);
						modevent(epollfd, coredata[j].connfd, EPOLLOUT);
					}
				}
			}else if( events[i].events & EPOLLIN ){
				//if ( (nread = Readline(sockfd, coredata[sockfd].reply, MAXN)) == 0){
				if ( (nread = read(sockfd, coredata[sockfd].reply, MAXN)) == 0){
					printf("connectionclosed by other end");
					delevent(epollfd,sockfd);
					coredata[sockfd].selected = 0;
					close(sockfd);		/* connection closed by other end */
					continue;
				}
				//debug_printf("2, Return %d bytes.\n", nread);
				Write(STDOUT_FILENO, coredata[sockfd].reply, nread);
				fflush(stdout);
				memset(coredata[sockfd].reply, '\0', MAXN);
				modevent(epollfd, sockfd, EPOLLIN);
			}else if( events[i].events & EPOLLOUT ){
				Write(sockfd, coredata[sockfd].request, coredata[sockfd].nrequest);
				modevent(epollfd, sockfd, EPOLLIN);
			}else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ){
				delevent(epollfd, sockfd);
				close(sockfd);
			}else{
				/*anything else happend*/
			}
		}//end epoll event scan.
	}
	close( epollfd );
	close( listenfd );
	exit(1);
}

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
    if( one_shot )
    {
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
