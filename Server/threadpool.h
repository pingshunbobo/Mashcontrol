#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include "locker.h"
#include "mash.h"

#define MAX_REQUEST_NUM 10000;

typedef struct queue_node
{
        struct queue_node *prve;
        struct queue_node *next;
        MASHDATA  *data;
}QUEUE_NODE;

typedef struct mash_queue
{
	int max_requests;
	int now_requests;
	QUEUE_NODE *front;
	QUEUE_NODE *rear;
}MASH_QUEUE;

MASH_QUEUE mash_queue_data;
MASHDATA **thread_work_data;

static int all_thread_number = 0;
static pthread_t *all_thread_t = NULL;
static pthread_mutex_t	mash_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t	thread_work_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t		have_mash_sem;

int make_threadpool(int thread_number);
int threadpool_append(MASHDATA *data);

#endif
