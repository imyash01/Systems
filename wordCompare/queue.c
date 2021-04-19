#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"
#include "bst.h"

int init(queue_t *Q)
{
	Q->count = 0;
	Q->activeThreads=0;
    Q->front = NULL;
    Q->rear = NULL;
	pthread_mutex_init(&Q->lock, NULL);
	pthread_cond_init(&Q->read_ready, NULL);
	
	return 0;
}

int destroy(queue_t *Q)
{
	pthread_mutex_destroy(&Q->lock);
	pthread_cond_destroy(&Q->read_ready);
	//pthread_cond_destroy(&Q->write_ready);

	return 0;
}

node_Q* newQNode(char* str){
    node_Q* temp = malloc(sizeof(node_Q));
	char * temp1 = malloc(strlen(str) + 1);
	if(temp == NULL || temp1 == NULL){
		write(2, "malloc failed\n",15);
        exit(1);
	}
    strcpy(temp1, str);
    temp->path = temp1;
    temp->next = NULL;
    return temp;
}

// add item to end of queue
// if the queue is full, block until space becomes available
int enqueue(queue_t *Q, char* item) //change this to add to the LL
{
	pthread_mutex_lock(&Q->lock);
	
	node_Q* temp = newQNode(item);

	if(Q->rear == NULL){
        Q->front = temp;
        Q->rear = temp;
    }
    else{
        Q->rear->next = temp;
        Q->rear = temp;
    }
	++Q->count;
	
	pthread_cond_signal(&Q->read_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}

node_Q* dequeue(queue_t *Q) //CHECK add active threads return char*.
{
	pthread_mutex_lock(&Q->lock);

	if(Q->count == 0){
		Q->activeThreads--;
        //printf("%d threads \n", Q->activeThreads);
		if(Q->activeThreads == 0){
			pthread_mutex_unlock(&Q->lock);
			pthread_cond_broadcast(&Q->read_ready);
			return NULL;
		}

		while (Q->count == 0 && Q->activeThreads != 0) {
			pthread_cond_wait(&Q->read_ready, &Q->lock);
		}

		if (Q->count == 0) {
			pthread_mutex_unlock(&Q->lock);
			return NULL;
		}
		++Q->activeThreads;
	}
	
    node_Q* temp = Q->front;

    Q->front = Q->front->next;

    if(Q->front == NULL)
        Q->rear = NULL;

    //free(temp); free after using it in thread
    --Q->count;
	//pthread_cond_signal(&Q->write_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return temp;
}