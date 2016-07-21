#include <pthread.h>
#include <sys/socket.h>
typedef struct{
        pthread_t             thread_tid;     /* thread ID */
        long                  thread_count;   /* #connections handled */
} Thread;

Thread  *tptr;          /* array of Thread structures; calloc'ed */

int         listenfd, nthreads;
socklen_t   addrlen;
