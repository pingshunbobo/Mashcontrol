#include <stdlib.h>

pid_t heartbeat_fork(int sockfd)
{
	pid_t pid = 0;
	pid = fork();
	if( pid < 0){
		/* fork error! */
	}else if(pid == 0){
		while(1){
			if( write(sockfd, "Mashheart:ping!", 15) < 0 ){
				close(sockfd);
				break;
			}
			sleep(30);
		}
		exit(1);
	}
	return pid;
}
