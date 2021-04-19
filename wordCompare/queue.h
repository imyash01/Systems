#ifndef QUEUE
#define QUEUE

#include <pthread.h>
typedef struct node_Q
{
    char* path;
    struct node_Q* next;
}node_Q;

typedef struct queue_t{
	node_Q* front;
    node_Q* rear;
	unsigned count;
	int activeThreads;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
} queue_t;

int init(queue_t*);
int destroy(queue_t*);
node_Q* newQNode(char*);
int enqueue(queue_t *, char*);
node_Q* dequeue(queue_t*);
#endif