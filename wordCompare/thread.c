#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <dirent.h>
#include <pthread.h>


#include "stringbuffer.h"
#include "bst.h"
#include "queue.h"
#include "thread.h"

int indexInit(index_t* i,int totalCompares, int aThreads){
    i->aThreads = aThreads;
    i->compares = totalCompares;
    i->currIndex = 0;
    pthread_mutex_init(&i->lock, NULL);
    return 0;
}

int getIndex(index_t* i){
    pthread_mutex_lock(&i->lock);
    int temp = i->currIndex;
    if(i->currIndex < i->compares){
        i->currIndex++;
    }
    else{
        temp = -1;
    }
    pthread_mutex_unlock(&i->lock);
    return temp;
}

node_wfd* addWfdNode(node_wfd* root,parent_node* item, char* path){
    //pthread_mutex_lock(&root->lock);
    node_wfd* curr = root;
    node_wfd* prev = NULL;

    node_wfd* newNode = malloc(sizeof(node_wfd));
    if(newNode == NULL){
        write(2, "malloc failed\n",15);
        exit(1);
    }
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

int isdir(char* name) { //checks dir for threads
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

int isSuffix(char* suffix, char* string){
    for(int i = strlen(suffix) - 1; i >= 0; i--){
        if(suffix[i] != string[strlen(string) - (strlen(suffix) - i)]){
            return 1;
        }
    }
    return 0;
}

void* dirThread(void *A) //CHECK SUFFIX
{
    int error = 0;
    targs_d *args = (targs_d*)A;
    
    //gets tids,queue*dir and file,suffix
    while(args->dir->activeThreads != 0){
        node_Q* returned = dequeue(args->dir); //returned from queue

        if(returned == NULL){
            continue; // get out of the loop because the queue is empty
        }
        DIR *dirp = opendir(returned->path);  // open the current directory
        struct dirent *folder;

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
                int suff = isSuffix(args->suffix,folder->d_name);
                if(!suff){
                    enqueue(args->file,path.data); //enqueue the file
                }
                else{
                    sb_destroy(&path);
                    continue;
                }
            }
            else if(check2 == 1){ //dir
                enqueue(args->dir,path.data);
                //enqueue to the dir queue
            }
            else{
                write(2,"The path is not dir or file",29);
                error = 1;
            }
            sb_destroy(&path);
        }
        closedir(dirp);
        free(returned->path);
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
    targs_f *args = (targs_f*)A;
    while(args->file->activeThreads != 0){
       node_Q* returned =  dequeue(args->file);

       if(returned == NULL){
            continue; // get out of the loop because the queue is empty
        }
        parent_node* temp = tokenize(returned->path);

        char* path = malloc(strlen(returned->path) + 1);//to free the queue node but pass the path
        if(path == NULL){
            write(2, "malloc failed\n",15);
            exit(1);
        }
        strcpy(path,returned->path);

        if(temp == NULL){
            error = 1;
        }
        else{
            args->root_wfd = addWfdNode(args->root_wfd,temp,path);
        }
        free(returned->path);
        free(returned);
    }
    if(error){
        return (void*)1;
    }
    else{
        return 0;
    }
}

void* analThread(void* A){
    targs_a *args = (targs_a*)A;
    int i;
    parent_node* temp;
    while((i = getIndex(args->i)) >= 0){
        temp = getMeanFreq(args->jsd[i]->file1->fileRoot, args->jsd[i]->file2->fileRoot);
        args->jsd[i]->jsd = getJSD(args->jsd[i]->file1->fileRoot->child_root, args->jsd[i]->file2->fileRoot->child_root , temp->child_root);
        free(temp);//CHECK
    }
    return 0;
}