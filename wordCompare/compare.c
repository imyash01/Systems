#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stringbuffer.c"

typedef struct node
{
    int occurences;
    double frequency;
    char* word;
    struct node *left;
    struct node *right;  
} node;

int total = 0; //not good solution need to figure out how to store total
//make a struct that stores the roots for files and total words. Store it in an array(WFD Repo)

node * makeNode(char* word){
    node *newNode = malloc(sizeof(node));
    newNode->word = word;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->occurences = 1;

    return newNode;
}

node* toAdd(char* word, node* root){
    if(root ==  NULL){
        total++;//CHECK
        return makeNode(word);
    }
    int comp = strcmp(word,root->word);
    if(comp == 0){
        root->occurences ++;
        total++;//CHECK
    }
    if(comp < 0){
        root->left = toAdd(word, root->left);
    }
    else{
        root->right = toAdd(word, root->right);
    }
    return root;
}

void toFreq(node * root){
    if(root == NULL){
        return;
    }
    toFreq(root->left);
    //TODO make total
    root->frequency = (double)root->occurences/total;
    toFreq(root->right);
}

//CHECK change words to lowercase
int tokenize(char* filePath) {
   
    FILE *fp = fopen(filePath, "r");

    char temp;
    node *root = NULL;
    strbuf_t word;
    sb_init(&word, 32);

    while((temp = fgetc(fp)) != EOF ) {
        if(isalpha(temp) != 0) {
            sb_append(&word, temp);
        }
        else if(isspace(temp) && word.used != 1){
            char *temp2 = malloc(sizeof(char) * word.used);
            strcpy(temp2, word.data);
            sb_destroy(&word);
            sb_init(&word, 32);
            root = toAdd(temp2, root);
        }
    }

    if(temp == EOF && word.used != 1){
        char *temp2 = malloc(sizeof(char) * word.used);
        strcpy(temp2, word.data);
        sb_destroy(&word);
        sb_init(&word, 32);
        root = toAdd(temp2, root);
    }

    toFreq(root); // assigns the frequences
    fclose(fp);
    return 0;
}

int main(int argc, char* argv[]){
    tokenize("file1.txt");
}