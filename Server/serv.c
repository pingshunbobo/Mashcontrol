#include	"unp.h"
#include	"mash.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>

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
	listenfd = Tcp_listen("192.168.78.154", "8080", &addrlen);
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
				mash_init(coredata, connfd);
				addevent(epollfd, connfd, false);
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
				if(!mash_read(coredata, sockfd)){
					/* Closed sockfd will be removed autoticly from epollfd */
					mash_close(coredata, sockfd);
					continue;
				}
				modevent(epollfd, sockfd, EPOLLIN);
			}else if( events[i].events & EPOLLOUT ){
				mash_write(coredata, sockfd);
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
