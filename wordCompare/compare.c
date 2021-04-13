#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stringbuffer.h"

typedef struct node
{
    int totalWords; //root of the bst holds tha total words in the file
    int occurences;
    double frequency;
    char* word;
    struct node *left;
    struct node *right;  
} node;


//int total = 0; not good solution need to figure out how to store total
//make a struct that stores the roots for files and total words. Store it in an array(WFD Repo)

node * makeNode(char* word){
    node *newNode = calloc(5,sizeof(node));
    newNode->word = word;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->occurences = 1;

    return newNode;
}

node* toAdd(char* word, node* root){
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

void toFreq(node * root, int total){
    if(root == NULL){
        return;
    }
    toFreq(root->left,total);
    //TODO make total
    root->frequency = (double)root->occurences/total;
    toFreq(root->right,total);
}

void toPrint(node * root){
    if(root == NULL){
        return;
    }
    toPrint(root->left);
    printf("%s->%f\t", root->word,root->frequency);
    toPrint(root->right);
}


int tokenize(char* filePath) {
   
    FILE *fp = fopen(filePath, "r");

    char temp;
    node *root = NULL;
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
            root->totalWords++;
        }
    }

    if(temp == EOF && word.used != 1){
        char *temp2 = malloc(sizeof(char) * word.used);
        strcpy(temp2, word.data);
        sb_destroy(&word);
        sb_init(&word, 32);
        root = toAdd(temp2, root);
        root->totalWords++;
    }

    toFreq(root,root->totalWords); // assigns the frequences
    toPrint(root);
    printf("%d\n",root->totalWords);
    fclose(fp);
    return 0;
}

node* findWord(node * root, char * word) {
    if(root == NULL || strcmp(word, root->word) == 0) {
        return root;
    }
    else if(strcmp(word, root->word) < 0) { //if word we are searching for is less than current, search left
        findWord(root->left, word);
    }
    else {
        findword(root->right, word); //if word we are searching is greater, search right
    }

}

int calcJSD(node *root1, node *root2) { 
    node* forMeanFreq;
    //go through root1 first
    if(root1 == NULL){
        return;
    }
    calcJSD(root1->left,root2->right);



}
int main(int argc, char* argv[]){
    tokenize("file2.txt");
    tokenize("file1.txt");
}