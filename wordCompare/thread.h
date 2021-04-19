#ifndef THREAD
#define THREAD

#include "stringbuffer.h"
#include "bst.h"
#include "queue.h"

typedef struct node_wfd{ //LL wfdrepo
    parent_node* fileRoot;
    struct node_wfd* next;
    char* filePath;
    int totalFiles;
    pthread_mutex_t lock;
}node_wfd;

typedef struct targs_f{
    queue_t* file;
    node_wfd* root_wfd;
    char* path;
    int id;
}targs_f;

typedef struct targs_d{
    queue_t* dir;
    queue_t* file;
    char* suffix;
    int id;
}targs_d;

typedef struct jsd_t{
    node_wfd* file1;
    node_wfd* file2;
    int combinedWords;
    double jsd;
}jsd_t;

typedef struct index_t{
    int currIndex;
    int compares;
    int aThreads;
    pthread_mutex_t lock;
}index_t;

typedef struct targs_a{
    jsd_t** jsd;
    index_t* i;
}targs_a;

int indexInit(index_t*,int, int);
int getIndex(index_t*);
node_wfd* addWfdNode(node_wfd*,parent_node*, char*);
int isdir(char*);
void* dirThread(void *);
void *fileThread(void *);
void* analThread(void*);

#endif