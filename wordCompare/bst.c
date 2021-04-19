#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "stringbuffer.h"
#include "bst.h"

node_bst * makeNode(char* word){
    node_bst *newNode = malloc(sizeof(node_bst));
    char * temp = malloc(strlen(word) + 1);
    strcpy(temp, word);
    newNode->word = temp;
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
        //free(word);
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

void toFree(node_bst *root){
    if(root == NULL){
        return;
    }
    toFree(root->left);
    toFree(root->right);
    free(root->word);
    free(root);
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
            //char *temp2 = malloc(word.used);
            //strcpy(temp2, word.data);
            root = toAdd(word.data, root);
            sb_destroy(&word);
            sb_init(&word, 32);
            parent->totalWords++;
            wroteWord = 1;
        }
    }

    if(temp == EOF && word.used > 1){
        //char *temp2 = malloc(word.used);
        //strcpy(temp2, word.data);
        root = toAdd(word.data, root);
        sb_destroy(&word);
        sb_init(&word, 32);
        parent->totalWords++;
        wroteWord = 1;
    }
    sb_destroy(&word);
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
    if(root == NULL || strcmp(word, root->word) == 0) { //return NULL
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
    double temp = sqrt((.5 * getKLD(tree1,meanTree)) + (.5 * getKLD(tree2,meanTree)));
    toFree(meanTree);//CHECK
    return temp;
}