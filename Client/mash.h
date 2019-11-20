#ifndef MASH_H
#define MASH_H

#include "message.h"
#define BUF_SIZE    4100

enum CLIENT_STATUS {WORK, STANDBY};

void mash_proc(char *request, int *checked_idx, int *read_idx);

int mash_send_message(MESSAGE_TYPE type, char *content, int len);
int mash_send_cntl(char *buf, int nbytes);
int mash_send_data(char *buf, int nbytes);
int mash_send_heart(char *buf, int nbytes);

#endif
