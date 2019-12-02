#include "threadpool.h"

void push_queue(MASHDATA *one_data);
void pop_queue();
void *thread_run();

void push_queue(MASHDATA *one_data)
{
	QUEUE_NODE *new_node;
	new_node = malloc(sizeof(QUEUE_NODE));
	new_node -> data = one_data;
	new_node -> next = NULL;
	
	if(mash_queue_data.front == NULL){
		mash_queue_data.front = new_node;
		new_node -> prve = NULL;
	}else{
		mash_queue_data.rear -> next = new_node;
		new_node -> prve = mash_queue_data.rear;
	}
	mash_queue_data.rear = new_node;
}

void pop_queue()
{
	QUEUE_NODE *first_node;
	first_node = mash_queue_data.front;
	mash_queue_data.front = first_node -> next;
	if(mash_queue_data.front)
		mash_queue_data.front -> prve = NULL;
	else
		mash_queue_data.rear = NULL;
	free(first_node);
}

void *thread_run()
{
	QUEUE_NODE *first_node;
	MASHDATA *the_data = NULL;
	while ( 1 ){
		wait_get(&have_mash_sem);
		lock(&mash_queue_mutex);
		first_node = mash_queue_data.front;
		if ( first_node == NULL ){
			unlock(&mash_queue_mutex);
			continue;
		}
		the_data = first_node->data;
		pop_queue();
		unlock(&mash_queue_mutex);
		if (the_data){
			lock(&the_data->mash_mutex);
			mash_thread_proc(the_data);
			unlock(&the_data->mash_mutex);
		}
     }
}

int make_threadpool( int thread_number ) 
{
	int i = 0;
	mash_queue_data.max_requests = MAX_REQUEST_NUM;
	mash_queue_data.now_requests = 0;
	mash_queue_data.front == NULL;
	mash_queue_data.rear == NULL;

	all_thread_number = thread_number;
	all_thread_t = malloc(sizeof(pthread_t) * thread_number);

	init_locker(&mash_queue_mutex);
	init_sem (&have_mash_sem);

	for ( i = 0; i < thread_number; ++i ){
		Pthread_create( all_thread_t + i, NULL, thread_run, NULL );
		Pthread_detach( all_thread_t[i] ); 
    	}
	log_serv( "create the thread pool.\n");
	return i;
}

int threadpool_append( MASHDATA* the_data )
{
	lock(&mash_queue_mutex);
	if ( mash_queue_data.now_requests > mash_queue_data.max_requests ){
		unlock(&mash_queue_mutex);
		return false;
	}
	push_queue(the_data);
	unlock(&mash_queue_mutex);
	post(&have_mash_sem);	//post signal say that add a the_data ect!
	return true;
}
