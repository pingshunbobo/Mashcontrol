#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

#define  MAXN   65535
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000


void delevent(int epollfd, int fd);
void modevent(int epollfd, int fd, int event);
void addevent(int epollfd, int fd, int event);

int main(int argc, char **argv)
{
	int	i;
	int	nread;
	int 	number;
	int 	epollfd;
	int	listenfd;
	socklen_t addrlen;

	char request[MAXN];
	char result[MAXN];
	struct epoll_event events[MAX_EVENT_NUMBER];

	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv [ <host> ] <port#> ");

	epollfd = Epoll_create( 5 );
	addevent(epollfd, listenfd, EPOLLIN);

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
				addevent(epollfd, connfd, EPOLLIN);
				setnonblocking(connfd);
			}else if( events[i].events & EPOLLIN ){
				if ( (nread = Readline(sockfd, result, MAXN)) == 0){
					close(sockfd);		/* connection closed by other end */
					delevent(epollfd,sockfd);
				}
				printf("2, Return %d bytes: %s\n", nread, result);
				fflush(stdout);
				modevent(epollfd, sockfd, EPOLLOUT);
			}else if( events[i].events & EPOLLOUT ){
				modevent(epollfd, sockfd, EPOLLIN);
			}else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ){
				close(sockfd);
				delevent(epollfd,sockfd);
			}else{
				/*anything else happend*/
			}
		}//end epoll event scan.
	}
	close( epollfd );
	close( listenfd );
	return 0;
}

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    Fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addevent( int epollfd, int fd, int ev )
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLET | EPOLLRDHUP;
    Epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
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
