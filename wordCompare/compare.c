#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    int occurences;
    int frequency;
    char* word;
    struct node *left;
    struct node *right;  
} node;

node * makeNode(char* word){
    node *newNode = malloc(sizeof(node));
    newNode->word = word;
    newNode->left = NULL;
    newNode->right = NULL;

    return newNode;
}

node* toAdd(char* word, node* root){
    if(root ==  NULL){
        return makeNode(word);
    }
    int comp = strcmp(word,root->word);
    if(comp == 0){
        root->occurences ++;
    }
    if(comp < 0){
        root->left = toAdd(word, root->left);
    }
    else{
        root->right = toAdd(word, root->right);
    }
    return root;
}

void toPrint(node * root){
    if(root == NULL){
        return;
    }
    toPrint(root->left);
    printf("%s\t", root->word);
    toPrint(root->right);
}

int main(int argc, char* argv[]){
    return 0;
}