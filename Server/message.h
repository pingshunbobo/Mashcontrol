#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

typedef enum message_type {MASH_CMD=1, MASH_CNTL=2, MASH_INFO=3, MASH_DATA=4, MASH_FILE=5, MASH_HEART=6, MASH_UNKNOW=7} MESSAGE_TYPE;
typedef enum message_status {CHECK_OPEN=0, CHECK_OK=1, CHECK_HEADER=2, CHECK_BODY=3, CHECK_ERROR=4} MESSAGE_STATUS;

typedef struct mash_message
{
	MESSAGE_STATUS status;
	MESSAGE_TYPE type;
	short len;
	char *content;
	struct mash_message *next;
}MASH_MESSAGE;

MESSAGE_STATUS get_message(MASH_MESSAGE *message, char *buf, int *checked_idx, int read_idx);
MASH_MESSAGE *make_message(MESSAGE_TYPE type, char *buf, int len);

#endif
