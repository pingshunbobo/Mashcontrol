#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

enum MESSAGE_TYPE {MASH_CMD=1, MASH_CNTL=2, MASH_INFO=3, MASH_DATA=4, MASH_FILE=5, MASH_HEART=6, MASH_UNKNOW=7};
enum MESSAGE_STATUS {CHECK_OPEN=0, CHECK_OK=1, CHECK_HEADER=2, CHECK_BODY=3, CHECK_ERROR=4};

typedef struct mash_message
{
	enum MESSAGE_STATUS status;
	enum MESSAGE_TYPE type;
	short len;
	char *content;
}MASH_MESSAGE;

enum MESSAGE_STATUS get_message(MASH_MESSAGE *message, char *buf, int *checked_idx, int *read_idx);
MASH_MESSAGE *make_message(enum MESSAGE_TYPE type, char *buf, int len);

#endif
