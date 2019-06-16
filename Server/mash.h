#define REPLY_SIZE   4096
#define MAX_CLIENT_NUM 65535
#define MAX_EVENT_NUMBER 10000

enum MASH_DATA_TYPE {MASH_CMD, MASH_INFO, MASH_DATA, MASH_UNKNOW};
enum MASH_STATUS {CMD, CLI, INTERFACE};

struct mashdata
{
        int selected;
        int connfd;
	int role;
	enum MASH_STATUS status;
	struct sockaddr_in client;
	char client_pub[16];
	char client_pri[16];
        int nrequest, nreply;
        char request[REPLY_SIZE];
        char reply[REPLY_SIZE];
	void * data;
};

int setnonblocking( int fd );
int mash_init(struct mashdata *data, int sockfd, struct sockaddr_in client_addr);
int mash_process(struct mashdata *data, int sockfd, int epollfd);
int mash_read(struct mashdata *data, int sockfd);
int mash_write(struct mashdata *data, int sockfd);
int mash_close(struct mashdata *data, int sockfd);

