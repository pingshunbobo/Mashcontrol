#include "threadpool.h"

int push_queue(MASHDATA *one_data)
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
	return 1;
}

int unique_push_queue(MASHDATA *one_data)
{
	int i = 0;
	QUEUE_NODE *index_node = NULL;
	for(i = 0; i< all_thread_number; i++){
		// the thread is processing this data;
		if(thread_work_data[i] == one_data && one_data){
			return 0;
		}
	}

	index_node = mash_queue_data.front;
	while(index_node){
		//There is data in the queue already;
		if( one_data == index_node -> data ){
			return 0;
		}
		index_node = index_node->next;
	}
	return push_queue(one_data);
}

void *pop_queue()
{
	void *data = NULL;
	QUEUE_NODE *first_node;
	first_node = mash_queue_data.front;

	if( first_node ){
		data = first_node -> data;
		mash_queue_data.front = first_node -> next;
	}
	if(mash_queue_data.front)
		mash_queue_data.front -> prve = NULL;
	else
		mash_queue_data.rear = NULL;
	free(first_node);
	return data;
}

void *thread_run(void *id)
{
	int thread_id = *(int *)id;
	QUEUE_NODE *first_data = NULL;
	MASHDATA *the_data = NULL;
	//printf("my thread id: %d\n", thread_id);
	while ( 1 ){
		wait_get(&have_mash_sem);
		lock(&mash_queue_mutex);
		the_data = pop_queue();
		if ( the_data == NULL ){
			unlock(&mash_queue_mutex);
			continue;
		}
		thread_work_data[thread_id] = the_data;
		unlock(&mash_queue_mutex);
		if (the_data){
			lock(&the_data->mash_mutex);
			mash_thread_proc(the_data);
			unlock(&the_data->mash_mutex);
		}
		lock(&thread_work_mutex);
		thread_work_data[thread_id] = NULL;
		unlock(&thread_work_mutex);
     }
}

int make_threadpool( int thread_number ) 
{
	int i = 0;
	int *thread_work_id;
	mash_queue_data.max_requests = MAX_REQUEST_NUM;
	mash_queue_data.now_requests = 0;
	mash_queue_data.front == NULL;
	mash_queue_data.rear == NULL;

	all_thread_number = thread_number;
	thread_work_id = malloc(thread_number *sizeof(int));
	thread_work_data = malloc(thread_number * sizeof(void *));
	all_thread_t = malloc(sizeof(pthread_t) * thread_number);

	init_locker(&mash_queue_mutex);
	init_locker(&thread_work_mutex);
	init_sem (&have_mash_sem);

	for ( i = 0; i < thread_number; i++ ){
		thread_work_id[i] = i;
		Pthread_create( all_thread_t + i, NULL, thread_run, &thread_work_id[i] );
		Pthread_detach( all_thread_t[i] ); 
    	}
	log_serv( "server create the thread pool.\n");
	return i;
}

int threadpool_append( MASHDATA* the_data )
{
	static thread_id = 0;
	lock(&mash_queue_mutex);
	if ( mash_queue_data.now_requests > mash_queue_data.max_requests ){
		unlock(&mash_queue_mutex);
		return false;
	}
	if(unique_push_queue(the_data))
		post(&have_mash_sem);	//post signal say that add a the_data ect!
	unlock(&mash_queue_mutex);
	return true;
}
