#include	"unp.h"
#include	"mash.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

int 	epollfd = 0;
int main(int argc, char **argv)
{
	int	i = 0;
	int	nread = 0;
	int 	number = 0;
	int	listenfd = 0;
	socklen_t addrlen;

	struct epoll_event events[MAX_EVENT_NUMBER];

	struct mashdata * coredata = \
		malloc(MAX_CLIENT_NUM * sizeof(struct mashdata));
	memset(coredata, '\0', MAX_CLIENT_NUM * sizeof(struct mashdata));
	listenfd = Tcp_listen(NULL, "19293", &addrlen);
	epollfd = Epoll_create( 5 );
	addevent(epollfd, listenfd, false);
	addevent(epollfd, STDIN_FILENO, false);
	/* loop process io events. */
	for ( ; ; ){
		if( (number = Epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 )) <= -1 )
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

				/* If Connect success initial the mash data struct*/
				mash_init(coredata, connfd, client_address);
				addevent(epollfd, connfd, false);
			}else if( events[i].events & EPOLLIN ){
				if(mash_read(coredata, sockfd) > 0){
					mash_process(coredata, sockfd, epollfd);
					//modevent(epollfd, sockfd, EPOLLIN|EPOLLOUT);
					continue;
				}else
					mash_close(coredata, sockfd);
			}else if( events[i].events & EPOLLOUT ){
				if( mash_write(coredata, sockfd) < 0 )
					mash_close(coredata, sockfd);
				else
					modevent(epollfd, sockfd, EPOLLIN);
				continue;
			}else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ){
				mash_close(coredata, sockfd);
			}else{
				/*anything else happend*/
			}
		}//end epoll event scan.
	}
	close(epollfd);
	close(listenfd);
	exit(1);
}
