#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "message.h"

extern int message_seq;

enum MESSAGE_STATUS get_message(MASH_MESSAGE *message, char *buf, int *checked_idx, int read_idx)
{
	if( NULL == message )
		return CHECK_ERROR;

	switch(message->status){
		case(CHECK_OPEN):
			goto DO_CHECK_HEADER;
		case(CHECK_OK):
			return CHECK_OK;
		case(CHECK_HEADER):
			goto DO_CHECK_HEADER;
		case(CHECK_BODY):
			goto DO_CHECK_BODY;
		default:
			message->status = CHECK_ERROR;
			return CHECK_ERROR;
	}

DO_CHECK_HEADER:
	message->status = CHECK_HEADER;
	message->len = 0;
	message->content = NULL;
	if( read_idx - *checked_idx < 4 )
		return CHECK_HEADER;

	/* Check Mash flag M */
	if(strncmp(buf + *checked_idx, "M", 1)){
		*checked_idx += 1;
		message->status = CHECK_ERROR;
		return CHECK_ERROR;
	}
	*checked_idx += 1;

	/* Get message type.  */
	switch( *(char*)(buf+*checked_idx) ){
		case(MASH_CMD):
			message->type = MASH_CMD;
			break;
		case(MASH_CNTL):
			message->type = MASH_CNTL;
			break;
		case(MASH_INFO):
			message->type = MASH_INFO;
			break;
		case(MASH_DATA):
			message->type = MASH_DATA;
			break;
		case(MASH_FILE):
			message->type = MASH_FILE;
			break;
		case(MASH_HEART):
			message->type = MASH_HEART;
			break;
		default:
			message->type = MASH_UNKNOW;
			return CHECK_HEADER;
	}
	*checked_idx += 1;

	/* Get message len.  */
	message->len = *(short*)(buf + *checked_idx);
	if(message->len > 4096){
		message->status = CHECK_ERROR;
		return CHECK_ERROR;
	}
	*checked_idx += 2;

	/* Get message body. */
DO_CHECK_BODY:
	message->status = CHECK_BODY;
	if(read_idx - *checked_idx < 1)
		return CHECK_BODY;
	message->content = buf + *checked_idx;
	if( read_idx - *checked_idx >= message->len ){
		message->status = CHECK_OK;
		*checked_idx += message->len;
	}
	return message->status;
}

MASH_MESSAGE *make_message(enum MESSAGE_TYPE type, char *buf, int len)
{
	MASH_MESSAGE *message = malloc(sizeof(MASH_MESSAGE));
	memset(message, '\0', sizeof(MASH_MESSAGE));
	message->status = CHECK_OK;
	message->type = type;
	message->len = len;
	message->content = buf;
	return message;
}

void log_messages(char *str, MASH_MESSAGE *message)
{
	FILE * file = fopen("./log/messages.log", "a+");
	fprintf(file, "%s ", str);
	if(50 > (int)message->len)
		fprintf(file, "seq: %d stat: %d type: %d len: %d content: %s\n", message_seq, message->status, \
			(int)message->type, (int)message->len,  message->content);
	else
		fprintf(file, "seq: %d stat: %d type: %d len: %d content: ...\n", message_seq, message->status, \
			(int)message->type, (int)message->len);
	
	fclose(file);
}
