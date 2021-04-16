#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <math.h>
#include <regex.h>
#include "stringbuffer.h"

/*#ifndef QSIZE
#define QSIZE 8 // bounded queue
#endif*/

typedef struct node_bst
{
    int occurences;
    double frequency;
    char* word;
    struct node_bst *left;
    struct node_bst *right;  
} node_bst;

typedef struct parent_node
{
    int totalWords;
    node_bst* child_root;
}parent_node;

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

//int total = 0; not good solution need to figure out how to store total
//make a struct that stores the roots for files and total words. Store it in an array(WFD Repo)

node_wfd* addWfdNode(node_wfd* root,parent_node* item, char* path){
    //pthread_mutex_lock(&root->lock);
    node_wfd* curr = root;
    node_wfd* prev = NULL;

    node_wfd* newNode = malloc(sizeof(node_wfd));
    newNode->fileRoot = item;
    newNode->next = NULL;
    newNode->filePath = path;

    if(root == NULL){
        newNode->totalFiles = 1;
        return newNode;
    }
    while(curr != NULL){
        prev = curr;
        curr = curr->next;
    }
    if(prev == NULL){
        root->next = newNode;
    }
    else{
        prev ->next = newNode;
    }
    root->totalFiles++;
    //pthread_mutex_unlock(&root->lock);
    return root;
}

/*int wfdInit(node_wfd* root){
    pthread_mutex_init(&root->lock, NULL);
    root->fileRoot = NULL;
    root->next = NULL;
}*/

node_bst * makeNode(char* word){
    node_bst *newNode = malloc(sizeof(node_bst));
    newNode->word = word;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->occurences = 1;

    return newNode;
}

parent_node* makeParent(){
    parent_node* temp = malloc(sizeof(parent_node));
    temp->child_root = NULL;
    temp->totalWords = 0;
    return temp;
}

node_bst* toAdd(char* word, node_bst* root){
    if(root ==  NULL){
        //total++;//CHECK
        return makeNode(word);
    }
    int comp = strcmp(word,root->word);
    if(comp == 0){
        root->occurences ++;
    }
    else if(comp < 0){
        root->left = toAdd(word, root->left);
    }
    else{
        root->right = toAdd(word, root->right);
    }
    return root;
}

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
    temp->path = str;
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

		while (Q->count != 0 && Q->front == NULL) {
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

void toFreq(node_bst *root, int total){
    if(root == NULL){
        return;
    }
    toFreq(root->left,total);
    //TODO make total
    root->frequency = (double)root->occurences/total;
    toFreq(root->right,total);
}

void toPrint(node_bst * root){
    if(root == NULL){
        return;
    }
    toPrint(root->left);
    printf("%s->%f\t", root->word,root->frequency);
    toPrint(root->right);
}

int isdir(char* name) {
	struct stat data;
	
	int err = stat(name, &data);
	
	if (err == -1) {
		perror(name);  // print error message
		return -1;
	}
    // S_ISDIR macro is true if the st_mode says the file is a directory
	// S_ISREG macro is true if the st_mode says the file is a regular file
	if (S_ISDIR(data.st_mode)) {
		return 1;
	} 
    else if (S_ISREG(data.st_mode)) {
        return 0;
    } 
	
	return -1;
}


parent_node* tokenize(char* filePath) { //CHECK include -
   
    FILE *fp = fopen(filePath, "r");
    if(fp == NULL){
        printf("File %s could not be open", filePath);
        return NULL;
    }

    int wroteWord;
    char temp;
    parent_node* parent = makeParent();
    node_bst *root = NULL;
    strbuf_t word;
    sb_init(&word, 32);

    while((temp = fgetc(fp)) != EOF ) {
        if(isalpha(temp) != 0 || temp == '-') {
            sb_append(&word, tolower(temp));
        }
        else if(isspace(temp) && word.used != 1){
            char *temp2 = calloc(word.used,sizeof(char));
            strcpy(temp2, word.data);
            sb_destroy(&word);
            sb_init(&word, 32);
            root = toAdd(temp2, root);
            parent->totalWords++;
            wroteWord = 1;
        }
    }

    if(temp == EOF && word.used != 1){
        char *temp2 = calloc(word.used,sizeof(char));
        strcpy(temp2, word.data);
        sb_destroy(&word);
        sb_init(&word, 32);
        root = toAdd(temp2, root);
        parent->totalWords++;
        wroteWord = 1;
    }
    if(wroteWord){
        parent->child_root = root;
        toFreq(root,parent->totalWords); // assigns the frequences
        //toPrint(root);
        //printf("%d\n",parent->totalWords);
        fclose(fp);
        return parent;
    }
    else{
        return NULL;
    }
}

node_bst* findWord(node_bst * root, char * word) {
    if(root == NULL || strcmp(word, root->word) == 0) {
        return root;
    }
    else if(strcmp(word, root->word) < 0) { //if word we are searching for is less than current, search left
        findWord(root->left, word);
    }
    else {
        findWord(root->right, word); //if word we are searching is greater, search right
    }
}

void getMeanFreqHelper(node_bst* tree1, node_bst* tree2, parent_node* meanTree){
    node_bst* temp;
    node_bst* temp2;

    if(tree1 != NULL){
        getMeanFreqHelper(tree1->left,tree2,meanTree);

        temp = findWord(meanTree->child_root,tree1->word);

        if(temp == NULL){

            meanTree->child_root = toAdd(tree1->word,meanTree->child_root);
            temp = findWord(meanTree->child_root,tree1->word);

            temp2 = findWord(tree2, tree1->word);

            if(temp2 != NULL){
                temp->frequency = .5*(temp2->frequency + tree1->frequency);
            }
            else{
                temp->frequency = .5*(tree1->frequency);
            }
        }
        getMeanFreqHelper(tree1->right,tree2,meanTree);
    }
}

parent_node* getMeanFreq(parent_node* tree1, parent_node* tree2){
    parent_node* temp = makeParent();
    getMeanFreqHelper(tree1->child_root,tree2->child_root,temp);
    getMeanFreqHelper(tree2->child_root,tree1->child_root,temp);
    return temp;
}

double getKLD(node_bst* tree1, node_bst* meanTree){
    node_bst* temp;
    double num = 0.0;
    
    if(tree1 != NULL){
        temp = findWord(meanTree,tree1->word);
        num = tree1->frequency * log2(tree1->frequency/temp->frequency);
        return num + getKLD(tree1->left,meanTree) + getKLD(tree1->right,meanTree);
    }
    else{
        return 0.0;
    }
}

double getJSD(node_bst* tree1, node_bst* tree2, node_bst* meanTree){
    return sqrt((.5 * getKLD(tree1,meanTree)) + (.5 * getKLD(tree2,meanTree)));
}

void* dirThread(void *A) //CHECK SUFFIX
{
    int error = 0;
    targs_d *args = A;
    //gets tids,queue*dir and file,suffix
    while(args->dir->activeThreads != 0){
        node_Q* returned = dequeue(args->dir); //returned from queue

        if(returned == NULL){
            continue; // get out of the loop because the queue is empty
        }
        DIR *dirp = opendir(returned->path);  // open the current directory
        struct dirent *folder;

        regex_t regex;
        int reti;

        while ((folder = readdir(dirp))) { //testDir/.

            strbuf_t path;
            sb_init(&path, 32);
            sb_concat(&path,returned->path);
            sb_append(&path,'/');
            sb_concat(&path,folder->d_name);

            if(folder->d_name[0] == '.') {
                sb_destroy(&path);
                continue;
            }

            int check2 = isdir(path.data);

            if(check2 == 0){ //file
                //if file starts with .
                reti = regcomp(&regex, args->suffix ,REG_EXTENDED);

                if (reti) {
                    write(2,"regex failed",13);
                    exit(1); 
                }

                reti = regexec(&regex, folder->d_name,0,NULL,0);
                if(!reti){
                    //string buffer has the path copy it to a string destroy it and then enqueue the char*
                    char * str = calloc(path.used,sizeof(char));
                    strcpy(str, path.data);
                    //printf("%s\n", str);
                    enqueue(args->file,str); //enqueue the file
                }
                else if (reti == REG_NOMATCH) {
                    sb_destroy(&path);
                    continue;
                }
            }
            else if(check2 == 1){ //dir
                //copy enqueue of file
                char *str1 = calloc(path.used,sizeof(char));
                strcpy(str1, path.data);
                //printf("%s\n", str1);
                enqueue(args->dir,str1);
                //enqueue to the dir queue
            }
            else{
                write(2,"The path is not dir or file",29);
                error = 1;
            }
            sb_destroy(&path);
        }
        closedir(dirp);
        free(returned);
    }
    if(error){
        return (void*)1;
    }
    else{
        return 0;
    }
}

void *fileThread(void *A)
{
    int error = 1;
    targs_f *args = A;
    while(args->file->activeThreads != 0){
       node_Q* returned =  dequeue(args->file);

       if(returned == NULL){
            continue; // get out of the loop because the queue is empty
        }
        parent_node* temp = tokenize(returned->path);

        char* path = malloc(strlen(returned->path));//to free the queue node but pass the path
        strcpy(path,returned->path);

        if(temp == NULL){
            error = 1;
        }
        else{
            args->root_wfd = addWfdNode(args->root_wfd,temp,path);
        }
        
        free(returned);
    }
    if(error){
        return (void*)1;
    }
    else{
        return 0;
    }
}

void createPermutations(jsd_t** array, node_wfd* head){
    node_wfd* p1 = head;
    node_wfd* p2 = head->next;
    int i = 0;

    while(p1 != NULL){
        while(p2 != NULL){
            array[i] = malloc(sizeof(jsd_t));
            array[i]->file1 = p1;
            array[i]->file2 = p2;
            array[i]->combinedWords = p1->fileRoot->totalWords + p2->fileRoot->totalWords;
            p2 = p2->next;
            i++;
        }
        p1 = p1->next;
        if(p1 != NULL)
            p2 = p1->next;
    }
}

void* analThread(void* A){
    
    return 0;
}
int main(int argc, char* argv[]){
    int error = 0;
    int dThreads = 1;
    int fThreads = 1;
    int aThreads = 1;
    int change = 1; // checks the status of suffix if true it needs to change(assigned something)
    char* suffix = NULL;
    queue_t file;
    queue_t dir;
    targs_d args_d;
    targs_f args_f;
    pthread_t *tids_d;
    pthread_t *tids_f;
    node_wfd* root = NULL;
    
    //wfdInit(root);
    init(&file);
    init(&dir);
    
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            int len = strlen(argv[i]);
            if(len > 2){
                char option = tolower(argv[i][1]);
                
                switch (option)
                {
                    case 'd':
                        if(isdigit(argv[i][2]))
                            dThreads = (int)argv[i][2] -48;
                        else
                            write(2,"not a number", 13);
                        break;
                    
                    case 'f':
                        if(isdigit(argv[i][2]))
                            fThreads = (int)argv[i][2] -48;
                        else
                            write(2,"not a number", 13);
                        break;
                        
                    case 'a':
                        if(isdigit(argv[i][2]))
                            aThreads = (int)argv[i][2] - 48;
                        else
                            write(2,"not a number", 13);
                        break;

                    case 's':
                        change = 0;
                        suffix = calloc(len - 1,sizeof(char));
                        for(int j = 2; j < len; j++){
                            suffix[j - 2] = argv[i][j];
                        }
                        //suffix[len - 2] = '\0';
                        break;

                    default:
                            write(2,"not a valid option", 20);
                        break;
                }
            }
            else{
                write(2,"not a valid option", 20);
            }
        }
        else{
            if(change){
                suffix = malloc(5);
                suffix = ".txt";
                change = 0;
            }

            int typeCheck = isdir(argv[i]);
            if(typeCheck == 1){
                enqueue(&dir,argv[i]);
            }
            else if(typeCheck == 0){
                enqueue(&file,argv[i]);
            }
            else{
                //error bad path
            }
        }
    }

    printf("Dir Threads:%d\n", dThreads);
    file.activeThreads = fThreads;
    dir.activeThreads = dThreads;

    args_d.file = &file;
    args_d.dir = &dir;
    args_d.suffix = suffix;
    dir.activeThreads = dThreads;

    args_f.file = &file;
    args_f.root_wfd = root;
    file.activeThreads = fThreads;

    tids_d = malloc(dThreads * sizeof(pthread_t));
    tids_f = malloc(fThreads * sizeof(pthread_t));

    for(int i = 0; i < dThreads;i++){
        pthread_create(&tids_d[i],NULL,dirThread,&args_d);
    }
    for(int i = 0; i < dThreads; i++){
        pthread_join(tids_d[i],NULL);
    }
    
    for(int i = 0; i < fThreads;i++){
        pthread_create(&tids_f[i],NULL,fileThread,&args_f);
    }
    for(int i = 0; i < fThreads; i++){
        pthread_join(tids_f[i],NULL);
    }

    root = args_f.root_wfd;

    /*node_wfd* curr = root;
    while(curr != NULL){
        printf("%s: ", curr->filePath);
        toPrint(curr->fileRoot->child_root);
        printf("\n");
        curr = curr->next;
    }*/
    
    int compares = .5 * (root->totalFiles)*(root->totalFiles - 1);
    jsd_t* jsd_repo[compares];
    createPermutations(jsd_repo,root);
    printf("%s\t %s", jsd_repo[2]->file1->filePath, jsd_repo[2]->file2->filePath);
}