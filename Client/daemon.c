#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

/* daemon_init */ 

void sig_child(int signo)
{
	pid_t pid;
	int stat;
	pid = wait(&stat);
}

void daemon_init(int *pid )
{
	if(( *pid = Fork())!= 0)
		exit(0);
	setsid();
	signal(SIGCHLD, sig_child);
	signal(SIGINT, SIG_IGN);

	umask(0);
	close(0);
	close(1);
	close(2);
	/* end daemon_init */
}
