#include "pthread.h"

void
thread_make(int i)
{
	void	*thread_main(void *);

	Pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *) i);
	return;		/* main thread returns */
}

void *
thread_main(void *arg)
{
	int		connfd;
	void		web_child(int);
	socklen_t	clilen;
	struct sockaddr	cliaddr;

	printf("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, &cliaddr, &clilen);
		tptr[(int) arg].thread_count++;

		web_child(connfd);		/* process the request */
		Close(connfd);
	}
}
