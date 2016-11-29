#include	<stdbool.h>

#define MAXN   65535
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000

struct mashdata
{
        int selected;
        int connfd;
	int role;
        struct sockaddr_in client;
	int reqult;
        int nrequest, nreply;
        char request[MAXN];
        char reply[MAXN];
};

int setnonblocking( int fd );

void addevent(int epollfd, int fd, bool one_shot);
void modevent(int epollfd, int fd, int ev);
void delevent(int epollfd, int fd);

int mash_init(struct mashdata *data, int sockfd, struct sockaddr_in client);
int mash_process(struct mashdata *data, int sockfd, int epollfd);
int mash_read(struct mashdata *data, int sockfd);
int mash_write(struct mashdata *data, int sockfd);
int mash_close(struct mashdata *data, int sockfd);
