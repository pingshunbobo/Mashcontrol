#ifndef MASH_H
#define MASH_H

#include "unp.h"
#include "message.h"

#define BUF_SIZE   4100
#define MAX_CLIENT_NUM 1024
#define MAX_EVENT_NUMBER 10000

enum MASH_MODE {CMD, CLI, INTERFACE, WORK, STANDBY};

typedef struct mashdata
{
        int connfd;
	int role;
	struct mashdata *selected;
	enum MASH_MODE status;
	struct sockaddr_in client;
	char client_pub[16];
	char client_pri[16];
        int nreadbuf, nwritebuf;
	int checked_idx;
	int in_message_seq, out_message_seq;
	MASH_MESSAGE in_message;
	MASH_MESSAGE *out_message;
        char readbuf[BUF_SIZE];
        char writebuf[BUF_SIZE];
	struct mashdata *prev;
	struct mashdata *next;
}MASHDATA;

int setnonblocking(int fd);
int mash_init(MASHDATA *the_data, int sockfd, struct sockaddr_in client_addr);
int mash_proc(MASHDATA *the_data);
MESSAGE_STATUS mash_get_message(MASHDATA *mashdata);

int mash_send_cmd(MASHDATA *mashdata, char *buf, int len);
int mash_send_cntl(MASHDATA *mashdata, char *buf, int len);
int mash_send_data(MASHDATA *mashdata, char *buf, int len);
int mash_send_heart(MASHDATA *mashdata, char *buf, int len);

int mash_read(MASHDATA *the_data);
int mash_write(MASHDATA *the_data);
int mash_close(MASHDATA *the_data);

#endif
