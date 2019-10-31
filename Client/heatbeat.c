#include <stdlib.h>
#include "mash.h"

void heart_loop(int sockfd)
{
	while(1){
		mash_send_heart("ping!", 5);
		sleep(30);
	}

}

pid_t heartbeat_fork(int sockfd)
{
	pid_t pid = 0;
	pid = fork();
	if( pid < 0){
		/* fork error! */
	}else if(pid == 0){
		heart_loop(sockfd);
	}
	return pid;
}
