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
#include "bst.h"
#include "queue.h"
#include "thread.h"

/*#ifndef QSIZE
#define QSIZE 8 // bounded queue
#endif*/

void createPermutations(jsd_t** array, node_wfd* head){
    node_wfd* p1 = head;
    node_wfd* p2 = head->next;
    int i = 0;

    while(p1 != NULL){
        while(p2 != NULL){
            array[i] = malloc(sizeof(jsd_t));
            if(array[i] == NULL){
                write(2, "malloc failed\n",15);
                exit(1);
            }
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

int cmpArr(const void* one, const void* two){
    double a = (*(jsd_t**)one)->combinedWords;
    double b = (*(jsd_t**)two)->combinedWords;

    return b - a;
}
int main(int argc, char* argv[]){
    //int error = 0; CHECK sets error at the end
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
                char* digit = calloc(strlen(argv[i]), sizeof(char));

                switch (option)
                {
                    case 'd':
                        for(int j = 2; j < len ; j++){
                            if(isdigit(argv[i][j])){
                                digit[j - 2] = argv[i][j];
                            }
                            else{
                                write(2,"not a number", 13);
                                exit(1);
                            }
                        }
                        dThreads = atoi(digit);
                        break;
                    
                    case 'f':
                        for(int j = 2; j < len; j++){
                            if(isdigit(argv[i][j])){
                                digit[j - 2] = argv[i][j];
                            }
                            else{
                                write(2,"not a number", 13);
                                exit(1);
                            }
                        }
                        fThreads = atoi(digit); 
                        break;
                        
                    case 'a':
                        for(int j = 2; j < len; j++){
                            if(isdigit(argv[i][j])){
                                digit[j - 2] = argv[i][j];
                            }
                            else{
                                write(2,"not a number", 13);
                                exit(1);
                            }
                        }
                        aThreads = atoi(digit);
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
                free(digit);
            }
            else{
                write(2,"not a valid option", 20);
            }
        }
        else{
            if(change){
                suffix = malloc(5);
                if(suffix == NULL){
                    write(2, "malloc failed\n",15);
                    exit(1);
                }
                strcpy(suffix,".txt");
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

    if(tids_d == NULL || tids_f == NULL){
        write(2, "malloc failed\n",15);
        exit(1);
    }

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

    index_t i;
    pthread_t *tids_a;
    targs_a args_a;
    tids_a = malloc(aThreads * sizeof(pthread_t));

    if(tids_a == NULL){
        write(2, "malloc failed\n",15);
        exit(1);
    }

    int compares = .5 * (root->totalFiles)*(root->totalFiles - 1);

    jsd_t* jsd_repo[compares];
    createPermutations(jsd_repo,root);
    indexInit(&i,compares,aThreads);
    args_a.i = &i;
    args_a.jsd = jsd_repo;
    
    for(int i = 0; i < aThreads;i++){
        pthread_create(&tids_a[i],NULL,analThread,&args_a);
    }
    for(int i = 0; i < aThreads; i++){
        pthread_join(tids_a[i],NULL);
    }

    qsort(jsd_repo, compares, sizeof(jsd_t*), cmpArr);

    for(int i = 0; i < compares;i++){
        printf("%f %s %s\n",jsd_repo[i]->jsd,jsd_repo[i]->file1->filePath,jsd_repo[i]->file2->filePath);
    }

    destroy(&dir);
    destroy(&file);
    free(suffix);
    free(tids_d);
    free(tids_f);
    free(tids_a);
    node_wfd *temp1;
    while(root!=NULL) //frees wfdLL and the bst in child root and parent_node
    {
        toFree(root->fileRoot->child_root);
        free(root->filePath);
        free(root->fileRoot);
        temp1 = root;
        root = root->next;
        free(temp1);
    }
    for(int i = 0; i < compares;i++){
        free(jsd_repo[i]);
    }

        temp1 = NULL;

    if(compares < 1){
        write(2, "not enough files\n",18);
        exit(1);
    }
    
    return 0;
}