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

//int total = 0; not good solution need to figure out how to store total
//make a struct that stores the roots for files and total words. Store it in an array(WFD Repo)

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


parent_node* tokenize(char* filePath) { //return the root maybe
   
    FILE *fp = fopen(filePath, "r");

    char temp;
    parent_node* parent = makeParent();
    node_bst *root = NULL;
    strbuf_t word;
    sb_init(&word, 32);

    while((temp = fgetc(fp)) != EOF ) {
        if(isalpha(temp) != 0) {
            sb_append(&word, tolower(temp));
        }
        else if(isspace(temp) && word.used != 1){
            char *temp2 = malloc(sizeof(char) * word.used);
            strcpy(temp2, word.data);
            sb_destroy(&word);
            sb_init(&word, 32);
            root = toAdd(temp2, root);
            parent->totalWords++;
        }
    }

    if(temp == EOF && word.used != 1){
        char *temp2 = malloc(sizeof(char) * word.used);
        strcpy(temp2, word.data);
        sb_destroy(&word);
        sb_init(&word, 32);
        root = toAdd(temp2, root);
        parent->totalWords++;
    }

    parent->child_root = root;

    toFreq(root,parent->totalWords); // assigns the frequences
    toPrint(root);
    printf("%d\n",parent->totalWords);
    fclose(fp);
    return parent;
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

float getKLD(node_bst* tree1, node_bst* meanTree){
    node_bst* temp;
    float num = 0.0;
    
    if(tree1 != NULL){
        temp = findWord(meanTree,tree1->word);
        num = tree1->frequency * log2(tree1->frequency/temp->frequency);
        return num + getKLD(tree1->left,meanTree) + getKLD(tree1->right,meanTree);
    }
    else{
        return 0.0;
    }
}

float getJSD(node_bst* tree1, node_bst* tree2, node_bst* meanTree){
    return sqrt((.5 * getKLD(tree1,meanTree)) + (.5 * getKLD(tree2,meanTree)));
}


int main(int argc, char* argv[]){
    parent_node* one = makeParent();
    parent_node* two = makeParent();
    two = tokenize("file2.txt");
    one = tokenize("/ilab/users/yp315/CS214_project/file1.txt");
    parent_node* mean;
    mean = getMeanFreq(one,two);
    printf("%f\n",getJSD(one->child_root,two->child_root,mean->child_root));
}