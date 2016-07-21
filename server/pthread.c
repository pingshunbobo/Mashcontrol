#include <pthread.h>
#include <sys/socket.h>
typedef struct{
	pthread_t             thread_tid;     /* thread ID */
	long                  thread_count;   /* #connections handled */
} Thread;

Thread  *tptr;          /* array of Thread structures; calloc'ed */

int			listenfd, nthreads;
socklen_t               addrlen;

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
	int				connfd;
	void			web_child(int);
	socklen_t		clilen;
	struct sockaddr	*cliaddr;

	cliaddr = Malloc(addrlen);

	printf("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		clilen = addrlen;
		connfd = Accept(listenfd, cliaddr, &clilen);
		tptr[(int) arg].thread_count++;

		web_child(connfd);		/* process the request */
		Close(connfd);
	}
}
