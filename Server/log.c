#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mash.h"

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#define LOG_LEVEL  LOG_LEVEL_INFO

void log_serv(char *str)
{
	int nbytes = 0;
	int file_fd = 0;
	char buf[BUF_SIZE];
	if(LOG_LEVEL >= LOG_LEVEL_ERROR){
		file_fd = open("./logs/serv.log", \
			O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR );
		if(file_fd >= 0){
			nbytes = sprintf(buf, str);
			write(file_fd, buf, nbytes);
			close(file_fd);
		}
	}
}

void log_read(char *buf, int nbytes)
{
	int file_fd = 0;
	if(LOG_LEVEL >= LOG_LEVEL_INFO){
		file_fd = open("./logs/read.log", \
			O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR );
		if(file_fd >= 0){
			write(file_fd, buf, nbytes);
			close(file_fd);
		}
	}
}

void log_write(char *buf, int nbytes)
{
	int file_fd = 0;
	if(LOG_LEVEL >= LOG_LEVEL_INFO){
		file_fd = open("./logs/write.log", \
			O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR );
		if(file_fd >= 0){
			write(file_fd, buf, nbytes);
			close(file_fd);
		}
	}
}

void log_messages(int connfd, int seq, char *str, MASH_MESSAGE *message)
{
	int nbytes = 0;
	int file_fd = 0;
	char buf[4100];
	if(LOG_LEVEL >= LOG_LEVEL_INFO){
		nbytes = sprintf(buf, "connfd: %d %s seq: %d stat: %d type: %d len: %d content:\n", connfd, str, seq, message->status, \
                        (int)message->type, (int)message->len );
        	file_fd = open("./logs/message.log", O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR );
		if(file_fd >= 0){
			write(file_fd, buf, nbytes);
			if(LOG_LEVEL >= LOG_LEVEL_DEBUG){
        			write(file_fd, message->content, (int)message->len);
			}
			write(file_fd, "\n", 1);
			close(file_fd);
		}
	}
}
