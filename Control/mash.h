#ifndef MASH_H
#define MASH_H
#include	<sys/epoll.h>

#include "message.h"

#define	BUF_SIZE	4100		/* max #bytes to request from server */

enum CONTROL_STATUS {MASHCMD, INTERFACE};

void save_termios(int fd);
void restore_termios(int fd);

int mash_proc_cmd(MASH_MESSAGE *message);
int mash_proc_cntl(MASH_MESSAGE *message);
int mash_proc_data(MASH_MESSAGE *message);
int mash_proc_heart(MASH_MESSAGE *message);
int mash_proc(char *reply, int *checked_idx, int *nbytes);

int mash_send_cmd(char *reply, int len);
int mash_send_cntl(char *reply, int len);
int mash_send_data(char *reply, int len);
int mash_send_heart(char *reply, int len);

#endif
